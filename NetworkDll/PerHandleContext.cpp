#include "PerHandleContext.h"
#include "TaskEvent.h"
#include "IocpServer.h"
#include "IocpWork.h"

int PerHandleContext::channelIDIndex = 0;

PerHandleContext::PerHandleContext(void):
	additionPtr(nullptr),
	callback(nullptr),
	channelID(0),
	sock(INVALID_SOCKET),
	iocpServerPtr(nullptr),
	contextType(PerHandleContextType::typeDefault),
	checkIndex(-1)
{
	ZeroMemory((char*)&remoteAddr, sizeof(remoteAddr));
	ZeroMemory((char*)&localAddr, sizeof(localAddr));
}


PerHandleContext::~PerHandleContext(void)
{
	ReleaseSock(sock);
}

int PerHandleContext::CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
												std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock)
{
	additionPtr = taskEventPtr->additionPtr;
	callback = taskEventPtr->callback;
	channelID = PerHandleContext::ChannelIndex();
	this->iocpServerPtr = iocpServerPtr;
	this->iocpWorkPtr = iocpWorkPtr;

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(taskEventPtr->remotePort);
	if (taskEventPtr->remoteIP.length() <= 0)
	{
		remoteAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	}else
	{
		remoteAddr.sin_addr.S_un.S_addr = inet_addr(const_cast<char*>(taskEventPtr->remoteIP.c_str()));
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(taskEventPtr->localPort);
	if (taskEventPtr->localIP.length() <= 0)
	{
		localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	}else
	{
		localAddr.sin_addr.S_un.S_addr = inet_addr(const_cast<char*>(taskEventPtr->localIP.c_str()));
	}

	sock = tmpSock;
	return 0;
}

int PerHandleContext::ChannelID()
{
	return channelID;
}

PerHandleContextType PerHandleContext::NetworkPerHandleContextType()
{
	return contextType;
}

int PerHandleContext::QueryCheckExpireTimes(void)
{
	return checkIndex;
}

void PerHandleContext::AddCheckExpireTimes()
{
	++checkIndex;
}

SOCKADDR_IN PerHandleContext::RemoteAddress()
{
	return remoteAddr;
}

SOCKADDR_IN PerHandleContext::LocalAddress()
{
	return localAddr;
}

void* PerHandleContext::AdditionPtr()
{
	return additionPtr;
}

NetworkServerCallback PerHandleContext::CallbackFunc()
{
	return callback;
}

int PerHandleContext::ExitIcopWork()
{
//	::PostQueuedCompletionStatus(iocpServerPtr->IocpHandle(), 0, (ULONG_PTR)this, 0);
// 	if (sock != INVALID_SOCKET)
// 	{
// 		::CancelIo((HANDLE)sock);
// 	}
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
		//shutdown(sock, 2);
		sock = INVALID_SOCKET;
	}
	return 0;
}

void PerHandleContext::CloseSocket()
{
	ReleaseSock(sock);
}

int PerHandleContext::ChannelIndex()
{
	if (channelIDIndex >= 0x0fffffff)
	{
		channelIDIndex = 0;
	}
	return ++channelIDIndex;
}
