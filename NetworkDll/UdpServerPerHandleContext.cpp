#include "UdpServerPerHandleContext.h"
#include "IocpServer.h"
#include "IocpWork.h"
#include "PerIoContext.h"
#include "NetworkHelp.h"


UdpServerPerHandleContext::UdpServerPerHandleContext(void)
{
	currentPostPacketNumber = 0;
}


UdpServerPerHandleContext::~UdpServerPerHandleContext(void)
{
}


int UdpServerPerHandleContext::CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
									  std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock)
{
	if (PerHandleContext::CreatePerHandleContextObj(iocpServerPtr, iocpWorkPtr, taskEventPtr, tmpSock) != 0)
	{
		return -1;
	}

	this->contextType = PerHandleContextType::typeUdpServer;
	sock = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (::bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR 
		|| NULL == CreateIoCompletionPort((HANDLE)sock, iocpServerPtr->IocpHandle(), (DWORD_PTR)this, 0))
	{
		return -1;
	}

	iocpWorkPtr->RegisterPerHandleContext(this);
	this->InitPostRecv();
	LOGINFO(NetworkLogger, __FILE__, "Create Network UdpServerChannel:%d; ", channelID);
	return 0;
}

int UdpServerPerHandleContext::SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort)
{
	if (bufPtr == nullptr || len <= 0 || len > UDP_MAX_LEN || remoteIP.length() <= 0 || remotePort == 0)
	{
		return -1;
	}
	
	PerIoContext* tmpPtr = new PerIoContext();
	tmpPtr->SetPostParams(sock, BUF_LEN_8192, 0, IocpType::type_Write);
	tmpPtr->SetPostData(bufPtr, len);
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	tmpPtr->remoteAddr.sin_family = AF_INET;
	tmpPtr->remoteAddr.sin_addr.S_un.S_addr = inet_addr(const_cast<char*>(remoteIP.c_str()));
	tmpPtr->remoteAddr.sin_port = htons(remotePort);
	if(::WSASendTo(tmpPtr->GetPerIoContextocket(), tmpPtr->GetWsaBufPtr(), 1, &dwBytes, dwFlags, (sockaddr*)&tmpPtr->remoteAddr, sizeof(SOCKADDR_IN), &tmpPtr->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
		{
			return -1;
		}
	}
	InterlockedIncrement((long*)&currentPostPacketNumber);
	return 0;
}

int UdpServerPerHandleContext::ProcessIocpSuccessResult(PerIoContext* perIoContextPtr)
{
	int retValue = 0;
	switch (perIoContextPtr->PerIoContextIocpType())
	{
	case IocpType::type_Read :{retValue = this->ProcessIocpRead(perIoContextPtr);break;}
	case IocpType::type_Write :{retValue = this->ProcessIocpWrite(perIoContextPtr); break;}
	default: {retValue = -1; break;};
	}
	return retValue;
}

int UdpServerPerHandleContext::ProcessIocpFailedResult(PerIoContext* perIoContextPtr)
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

void UdpServerPerHandleContext::ReleasePerIoContext(PerIoContext* perIoContextPtr)
{
	ReleasePtr(perIoContextPtr);
	InterlockedDecrement(&currentPostPacketNumber);
	if (currentPostPacketNumber <= 0)
	{
		//iocpWorkPtr->RemovePerHandleContext(channelID);
		iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error, NetworkEventCode::close_event, channelID, remoteAddr, localAddr, nullptr, 0, additionPtr, callback);
		this->contextType = PerHandleContextType::typeEmpty;
		//delete this;
	}
}

void UdpServerPerHandleContext::InitPostRecv()
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

bool UdpServerPerHandleContext::PostRecv(PerIoContext* perIoContextPtr)
{
	perIoContextPtr->SetPostParams(perIoContextPtr->GetPerIoContextocket(), BUF_LEN_8192, 0, IocpType::type_Read);
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	int addrLen = sizeof(SOCKADDR_IN);
	if(::WSARecvFrom(sock, perIoContextPtr->GetWsaBufPtr(), 1, &dwBytes, &dwFlags, (sockaddr*)&perIoContextPtr->remoteAddr, &addrLen, &perIoContextPtr->ol, NULL) != NO_ERROR)
	{
		if(::WSAGetLastError() != WSA_IO_PENDING)
		{
			return false;
		}
	}
	InterlockedIncrement((long*)&currentPostPacketNumber);
	return true;
}

int UdpServerPerHandleContext::ProcessIocpRead(PerIoContext* perIoContextPtr)
{
	InterlockedDecrement(&currentPostPacketNumber);
	iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::recv_event, channelID, perIoContextPtr->remoteAddr,
		localAddr, perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);

	if (!this->PostRecv(perIoContextPtr))
	{
		ReleasePtr(perIoContextPtr);
	}
	return 0;
}

int UdpServerPerHandleContext::ProcessIocpWrite(PerIoContext* perIoContextPtr)
{
	InterlockedDecrement(&currentPostPacketNumber);
	iocpServerPtr->PacketNetworkCallback(NetworkErrorCode::no_error,  NetworkEventCode::send_event, channelID, perIoContextPtr->remoteAddr,
		localAddr, perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);

	ReleasePtr(perIoContextPtr);
	return 0;
}