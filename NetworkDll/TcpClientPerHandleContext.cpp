#include "TcpClientPerHandleContext.h"
#include "TaskEvent.h"
#include "PerIoContext.h"
#include "IocpServer.h"
#include "IocpWork.h"

TcpClientPerHandleContext::TcpClientPerHandleContext(void):
	lpfnConnectEx(nullptr),
	currentPostPacketNumber(0),
	packetIndex(0),
	currentParsePacketIndex(1)
{
	InitializeCriticalSection(&csPacketIndex);
}


TcpClientPerHandleContext::~TcpClientPerHandleContext(void)
{
	DeleteCriticalSection(&csPacketIndex);
}

int TcpClientPerHandleContext::CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
														 std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock)
{
	if (PerHandleContext::CreatePerHandleContextObj(iocpServerPtr, iocpWorkPtr, taskEventPtr, tmpSock) != 0)
	{
		return -1;
	}

	this->contextType = PerHandleContextType::typeTcpClient;
	if (taskEventPtr->eventCode == TaskEventCode::tcp_accept_task)
	{
		LINGER lingerStruct;
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
		setsockopt(sock, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct));

		// 远程客户端连接本地服务器
		if (NULL == CreateIoCompletionPort((HANDLE)sock, iocpServerPtr->IocpHandle(), (DWORD_PTR)this, 0))
		{
			return -1;
		}
		this->InitPostRecvPacket();
		iocpWorkPtr->RegisterPerHandleContext(this);
		iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error, NetworkEventCode::accept_event, channelID, remoteAddr, localAddr, nullptr, 0, additionPtr, callback);
		LOGINFO(NetworkLogger, __FILE__, "Create Network TcpClientChannel:%d; ", channelID);
	}else if (taskEventPtr->eventCode == TaskEventCode::tcp_connect_task)
	{
		//本地客户端连接远程服务器
		sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		LINGER lingerStruct;
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
		setsockopt(sock, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct));
		PerIoContext* perIoContextPtr = new PerIoContext();
		if (::bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR 
			|| NULL == CreateIoCompletionPort((HANDLE)sock, iocpServerPtr->IocpHandle(), (DWORD_PTR)this, 0)
			|| nullptr == perIoContextPtr)
		{
			ReleaseSock(sock);
			ReleasePtr(perIoContextPtr);
			return -1;
		}

		GUID GuidConnectEx = WSAID_CONNECTEX;
		DWORD dwBytes = 0;
		WSAIoctl(sock,SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
		
		dwBytes = 0;
		perIoContextPtr->SetPostParams(sock, 0, 0, IocpType::type_Connect);
		BOOL bResult = (lpfnConnectEx)(sock, (sockaddr*)&remoteAddr, sizeof(remoteAddr), NULL, 0, &dwBytes, &perIoContextPtr->ol);
		if(!bResult)	
		{
			if (::WSAGetLastError() != WSA_IO_PENDING)
			{
				ReleaseSock(sock);
				ReleasePtr(perIoContextPtr);
				return -1;
			}
		}
		InterlockedIncrement((long*)&currentPostPacketNumber);
		LOGINFO(NetworkLogger, __FILE__, "Create Network TcpClientChannel:%d; ", channelID);
	}
	return 0;
}


int TcpClientPerHandleContext::SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort)
{
	if (bufPtr == nullptr || len <= 0)
	{
		return -1;
	}

	int retValue = 0;
	int mode = len % BUF_LEN_8192;
	int count = (len - mode)/BUF_LEN_8192;
	int index = 0;
	while (index < count)
	{
		PerIoContext* tmpPtr = new PerIoContext();
		tmpPtr->SetPostParams(sock, BUF_LEN_8192, 0, IocpType::type_Write);
		tmpPtr->SetPostData((char*)&(bufPtr[index * BUF_LEN_8192]), BUF_LEN_8192);
		DWORD dwBytes = 0;
		DWORD dwFlags = 0;
		if(WSASend(tmpPtr->GetPerIoContextocket(), tmpPtr->GetWsaBufPtr(), 1, &dwBytes, dwFlags, &tmpPtr->ol, NULL) != NO_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING) 
			{
				retValue = -1;
				break;
			}	
		}
		InterlockedIncrement((long*)&currentPostPacketNumber);
		++index;
	}

	if (mode > 0 && retValue == 0)
	{
		PerIoContext* tmpPtr = new PerIoContext();
		tmpPtr->SetPostParams(sock, BUF_LEN_8192, 0, IocpType::type_Write);
		tmpPtr->SetPostData((char*)&(bufPtr[index * BUF_LEN_8192]), mode);
		DWORD dwBytes = 0;
		DWORD dwFlags = 0;
		if(WSASend(tmpPtr->GetPerIoContextocket(), tmpPtr->GetWsaBufPtr(), 1, &dwBytes, dwFlags, &tmpPtr->ol, NULL) != NO_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING) 
			{
				retValue = -1;
			}	
		}
		InterlockedIncrement((long*)&currentPostPacketNumber);
	}
	return retValue;
}


int TcpClientPerHandleContext::ProcessIocpSuccessResult(PerIoContext* perIoContextPtr)
{
	int retValue = 0;
	switch (perIoContextPtr->PerIoContextIocpType())
	{
	case IocpType::type_Connect :{retValue = this->ProcessIocpConnect(perIoContextPtr); break;}
	case IocpType::type_Read :{retValue = this->ProcessIocpRead(perIoContextPtr);break;}
	case IocpType::type_Write :{retValue = this->ProcessIocpWrite(perIoContextPtr); break;}
	default: {retValue = -1; break;};
	}
	return retValue;
}

int TcpClientPerHandleContext::ProcessIocpFailedResult(PerIoContext* perIoContextPtr)
{
	DWORD dwError = GetLastError();
	iocpServerPtr->PacketNetworkCallback((NetworkErrorCode)dwError,  NetworkEventCode::default_event, channelID, remoteAddr, localAddr, 
		perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);
	if (dwError == 1234)
	{
		InterlockedDecrement(&currentPostPacketNumber);
		if (!PostRecv(perIoContextPtr))
		{
			ReleasePtr(perIoContextPtr);
		}
	}else
	{
		ReleasePerIoContext(perIoContextPtr);
	}
	return 0;
}

void TcpClientPerHandleContext::ReleasePerIoContext(PerIoContext* perIoContextPtr)
{
	{
		CAutoLock lock(&csPacketIndex);
		ReleasePtr(perIoContextPtr);
		InterlockedDecrement(&currentPostPacketNumber);
	}

	if (currentPostPacketNumber <= 0)
	{
		//iocpWorkPtr->RemovePerHandleContext(channelID);
		iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error, NetworkEventCode::close_event, channelID, remoteAddr, localAddr, nullptr, 0, additionPtr, callback);
		this->contextType = PerHandleContextType::typeEmpty;
		//delete this;
	}
}


void TcpClientPerHandleContext::InitPostRecvPacket()
{
	for (int i = 0; i < DEFAULT_POST_NUMBER; ++i)
	{
		PerIoContext* tmpPerIoContextPtr = new PerIoContext();
		if (tmpPerIoContextPtr != nullptr)
		{
			if (!PostRecv(tmpPerIoContextPtr))
			{
				ReleasePtr(tmpPerIoContextPtr);
			}
		}
	}
}

bool TcpClientPerHandleContext::PostRecv(PerIoContext* perIoContextPtr)
{
	perIoContextPtr->SetPostParams(perIoContextPtr->GetPerIoContextocket(), BUF_LEN_8192, PacketIndex(), IocpType::type_Read);
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	if(::WSARecv(sock, perIoContextPtr->GetWsaBufPtr(), 1, &dwBytes, &dwFlags, &perIoContextPtr->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING) 
		{
			return false;
		}
	}
	InterlockedIncrement((long*)&currentPostPacketNumber);
	return true;
}

long TcpClientPerHandleContext::PacketIndex()
{
	CAutoLock lock(&csPacketIndex);
	if (packetIndex > 0x0fffffff)
	{
		packetIndex = 0;
	}
	return ++packetIndex;
}

int TcpClientPerHandleContext::ProcessIocpConnect(PerIoContext* perIoContextPtr)
{
	InterlockedDecrement(&currentPostPacketNumber);
	this->InitPostRecvPacket();
	iocpWorkPtr->RegisterPerHandleContext(this);

	iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::connect_event, channelID, remoteAddr, localAddr, 
		perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);

	ReleasePtr(perIoContextPtr);
	return 0;
}

int TcpClientPerHandleContext::ProcessIocpRead(PerIoContext* perIoContextPtr)
{
	InterlockedDecrement(&currentPostPacketNumber);
	{
		CAutoLock lock(&csPacketIndex);
		if (currentParsePacketIndex > 0x0fffffff)
		{
			currentParsePacketIndex = 1;
		}

		if (perIoContextPtr->GetIndex() == currentParsePacketIndex)
		{
			iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::recv_event, channelID, remoteAddr, localAddr, 
				perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);

			PerIoContext* tmpPtr = nullptr;
			long index = InterlockedIncrement(&currentParsePacketIndex);
			if (currentParsePacketIndex > 0x0fffffff)
			{
				currentParsePacketIndex = 1;
			}

			while (tmpPtr = this->CurrentParsePacketIndex(index))
			{
				iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::recv_event, channelID, remoteAddr, localAddr, 
					perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);
				if (!this->PostRecv(tmpPtr))
				{
					ReleasePtr(perIoContextPtr);
				}

				index = InterlockedIncrement(&currentParsePacketIndex);
				if (currentParsePacketIndex > 0x0fffffff)
				{
					currentParsePacketIndex = 1;
				}
			}
		}	
	}

	if (!this->PostRecv(perIoContextPtr))
	{
		ReleasePtr(perIoContextPtr);
	}
	return 0;
}

int TcpClientPerHandleContext::ProcessIocpWrite(PerIoContext* perIoContextPtr)
{
	iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::send_event, channelID, remoteAddr, localAddr, 
		perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);

	ReleasePtr(perIoContextPtr);
	InterlockedDecrement(&currentPostPacketNumber);
	return true;
}

PerIoContext* TcpClientPerHandleContext::CurrentParsePacketIndex(long index)
{
	PerIoContext* retPtr = nullptr;
	for (std::list<PerIoContext*>::iterator iter = listRecvPerIoContext.begin(); iter != listRecvPerIoContext.end(); ++iter)
	{
		if ((*iter)->GetIndex() == index)
		{
			retPtr = (*iter);
			listRecvPerIoContext.erase(iter);
			break;
		}
	}
	return retPtr;
}