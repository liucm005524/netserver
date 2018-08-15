#include "TcpServerPerHandleContext.h"
#include "IocpServer.h"
#include "IocpWork.h"
#include "PerIoContext.h"
#include "NetworkHelp.h"
#include "TcpClientPerHandleContext.h"
#include "TaskEvent.h"



TcpServerPerHandleContext::TcpServerPerHandleContext(void):
	lpfnAcceptEx(nullptr),
	lpfnGetAcceptExSockaddrs(nullptr),
	currentPostPacketNumber(0)
{
	InitializeCriticalSection(&csLock);
}


TcpServerPerHandleContext::~TcpServerPerHandleContext(void)
{
	DeleteCriticalSection(&csLock);
}


int TcpServerPerHandleContext::CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
														 std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock)
{
	if (PerHandleContext::CreatePerHandleContextObj(iocpServerPtr, iocpWorkPtr, taskEventPtr, tmpSock) != 0)
	{
		return -1;
	}
	
	this->contextType = PerHandleContextType::typeTcpServer;
	sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (::bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR 
		|| ::listen(sock, DEFAULT_LISTEN_NUMBER) == SOCKET_ERROR )
	{
		return -1;
	}

	// 加载扩展函数AcceptEx
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL);

	// 加载扩展函数GetAcceptExSockaddrs
	GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs), &lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs), &dwBytes, NULL, NULL);

	if (NULL == CreateIoCompletionPort((HANDLE)sock, iocpServerPtr->IocpHandle(), (DWORD_PTR)this, 0))
	{
		return -1;
	}

	iocpWorkPtr->RegisterPerHandleContext(this);
	this->InitPostListen();
	LOGINFO(NetworkLogger, __FILE__, "Create Network TcpServerChannel:%d; ", channelID);
	return 0;
}

int TcpServerPerHandleContext::ProcessIocpSuccessResult(PerIoContext* perIoContextPtr)
{
	InterlockedDecrement((long*)&currentPostPacketNumber);
	SOCKADDR_IN *pLocalAddr = nullptr;
	SOCKADDR_IN *pRemoteAddr = nullptr;
	int nLocalLen = 0, nRmoteLen = 0;
// 	(lpfnGetAcceptExSockaddrs)(perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen() - ((sizeof(sockaddr_in) + 16) * 2), sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
// 		(SOCKADDR **)&pLocalAddr, &nLocalLen, (SOCKADDR **)&pRemoteAddr, &nRmoteLen);

	(lpfnGetAcceptExSockaddrs)(perIoContextPtr->GetBufPtr(), 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (SOCKADDR **)&pLocalAddr, &nLocalLen, (SOCKADDR **)&pRemoteAddr, &nRmoteLen);
	std::shared_ptr<TaskEvent> taskEventPtr = std::shared_ptr<TaskEvent>(new TaskEvent(TaskEventCode::tcp_accept_task, inet_ntoa(pRemoteAddr->sin_addr), 
		ntohs(pRemoteAddr->sin_port), inet_ntoa(pLocalAddr->sin_addr), ntohs(pLocalAddr->sin_port), additionPtr, callback));

	bool bSign = false;
	if (taskEventPtr.get() != nullptr)
	{
		PerHandleContext* tmpPerHandleContextPtr = new TcpClientPerHandleContext();
		if (tmpPerHandleContextPtr != nullptr)
		{
			if (tmpPerHandleContextPtr->CreatePerHandleContextObj(iocpServerPtr, iocpWorkPtr, taskEventPtr, perIoContextPtr->GetPerIoContextocket()) != 0)
			{
				ReleasePtr(tmpPerHandleContextPtr);
			}else
			{
				bSign = true;
			}
		}
	}	
	if (!bSign)
	{
		SOCKET sock = perIoContextPtr->GetPerIoContextocket();
		ReleaseSock(sock);
	}
	
	if (!this->PostAccept(perIoContextPtr))
	{
		ReleasePtr(perIoContextPtr);
	}
	return true;
}

int TcpServerPerHandleContext::ProcessIocpFailedResult(PerIoContext* perIoContextPtr)
{
	DWORD dwError = GetLastError();
	iocpServerPtr->PacketNetworkCallback((NetworkErrorCode)dwError,  NetworkEventCode::default_event, channelID, remoteAddr, localAddr, 
		perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen(), additionPtr, callback);
	if (dwError == 1234)
	{
		InterlockedDecrement(&currentPostPacketNumber);
		if (!PostAccept(perIoContextPtr))
		{
			ReleasePtr(perIoContextPtr);
		}
	}else
	{
		ReleasePerIoContext(perIoContextPtr);
	}
	return 0;
}

void TcpServerPerHandleContext::ReleasePerIoContext(PerIoContext* perIoContextPtr)
{
	{
		CAutoLock lock(&csLock);
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

void TcpServerPerHandleContext::InitPostListen()
{
	for (int i = 0; i < DEFAULT_POST_NUMBER; ++i)
	{
		PerIoContext* tmpPerIoContextPtr = new PerIoContext();
		if (tmpPerIoContextPtr != nullptr)
		{
			if (!PostAccept(tmpPerIoContextPtr))
			{
				ReleasePtr(tmpPerIoContextPtr);
			}
		}
	}
}

bool TcpServerPerHandleContext::PostAccept(PerIoContext* perIoContextPtr)
{
	SOCKET tmpSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	perIoContextPtr->SetPostParams(tmpSocket, ((sizeof(sockaddr_in) + 16) * 2), 0, IocpType::type_Accept);

	DWORD dwBytes = 0;
	int len = perIoContextPtr->GetBufDataLen() - ((sizeof(sockaddr_in) + 16)*2);
	BOOL bResult = (lpfnAcceptEx)(sock, perIoContextPtr->GetPerIoContextocket(), perIoContextPtr->GetBufPtr(), perIoContextPtr->GetBufDataLen() - ((sizeof(sockaddr_in) + 16) * 2), 
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, &perIoContextPtr->ol);

	if(!bResult)	
	{
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			ReleaseSock(tmpSocket);
			return false;
		}
	}
	InterlockedIncrement((long*)&currentPostPacketNumber);
	return true;
}