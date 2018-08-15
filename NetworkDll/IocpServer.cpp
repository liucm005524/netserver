#include "IocpServer.h"
#include "IocpWork.h"
#include "TaskEvent.h"
#include "PerHandleContext.h"
#include "TcpServerPerHandleContext.h"
#include "TcpClientPerHandleContext.h"
#include "UdpServerPerHandleContext.h"

IocpServer* IocpServer::iocpServerPtr = nullptr;

IocpServer::IocpServer(void):
	icopHandle(INVALID_HANDLE_VALUE),
	threadHandle(INVALID_HANDLE_VALUE),
	exitHandle(INVALID_HANDLE_VALUE),
	signalHandle(INVALID_HANDLE_VALUE),
	iocpWorkPtr(nullptr)
{
	InitializeCriticalSection(&csTaskEvent);
}


IocpServer::~IocpServer(void)
{
	DeleteCriticalSection(&csTaskEvent);
}

IocpServer* IocpServer::GetInstance()
{
	if (iocpServerPtr == nullptr)
	{
		iocpServerPtr = new IocpServer();
	}
	return iocpServerPtr;
}

int IocpServer::StartServer()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		DWORD dwErr = GetLastError();
		return -1;
	}

	if (threadHandle == INVALID_HANDLE_VALUE)
	{
		DWORD threadID = 0;
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, IocpServer::ThreadProc, this, 0, (UINT*)&threadID);
		if (threadHandle == INVALID_HANDLE_VALUE)
		{
			return -1;
		}
	}
	return 0;
}

int IocpServer::StopServer()
{
	this->StopWorkThread();

	if (exitHandle != INVALID_HANDLE_VALUE && threadHandle != INVALID_HANDLE_VALUE)
	{
		SetEvent(exitHandle);
		WaitForSingleObject(threadHandle, INFINITE);
		ReleaseHandle(exitHandle);
		ReleaseHandle(threadHandle);
	}
	this->taskEventList.clear();
	WSACleanup();

	delete this;
	return 0;
}

int IocpServer::StartTcpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback)
{
	if (signalHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	std::shared_ptr<TaskEvent> taskEventPtr = 
		std::shared_ptr<TaskEvent>(new TaskEvent(TaskEventCode::tcp_server_task, "", 0, localIP, localPort, additionPtr, callback));
	if (taskEventPtr.get() != nullptr)
	{
		CAutoLock lock(&csTaskEvent);
		taskEventList.push_back(taskEventPtr);
		ReleaseSemaphore(signalHandle, 1, 0);
	}
	return 0;
}

int IocpServer::StartUdpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback)
{
	if (signalHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	std::shared_ptr<TaskEvent> taskEventPtr = 
		std::shared_ptr<TaskEvent>(new TaskEvent(TaskEventCode::udp_server_task, "", 0, localIP, localPort, additionPtr, callback));
	if (taskEventPtr.get() != nullptr)
	{
		CAutoLock lock(&csTaskEvent);
		taskEventList.push_back(taskEventPtr);
		ReleaseSemaphore(signalHandle, 1, 0);
	}
	return 0;
}

int IocpServer::StartTcpClientTask(std::string localIP, int localPort, std::string remoteIP, int remotePort, 
					   void* additionPtr, NetworkServerCallback callback)
{
	if (signalHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	std::shared_ptr<TaskEvent> taskEventPtr = 
		std::shared_ptr<TaskEvent>(new TaskEvent(TaskEventCode::tcp_connect_task, remoteIP, remotePort, localIP, localPort, additionPtr, callback));
	if (taskEventPtr.get() != nullptr)
	{
		CAutoLock lock(&csTaskEvent);
		taskEventList.push_back(taskEventPtr);
		ReleaseSemaphore(signalHandle, 1, 0);
	}
	return 0;
}

int IocpServer::TcpSend(int channelID, const char* bufPtr, const int bufLen)
{
	return iocpWorkPtr->TcpSend(channelID, bufPtr, bufLen);
}

int IocpServer::UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort)
{
	return iocpWorkPtr->UdpSend(channelID, bufPtr, bufLen, remoteIP, remotePort);
}

int IocpServer::CancelChannelID(int channelID)
{
	return iocpWorkPtr->CancelPerHandleContext(channelID);
}

HANDLE IocpServer::IocpHandle()
{
	return icopHandle;
}

UINT WINAPI IocpServer::ThreadProc(LPVOID lpVoid)
{
	IocpServer* objPtr = (IocpServer*)lpVoid;
	if (objPtr != nullptr)
	{
		objPtr->RunThreadEntry();
	}
	return 0;
}

void IocpServer::RunThreadEntry()
{
	icopHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	exitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	signalHandle = CreateSemaphore(NULL, 0, 0x0fffffff, NULL);

	if (iocpWorkPtr == nullptr)
	{
		iocpWorkPtr = new IocpWork();
		if (iocpWorkPtr == nullptr || iocpWorkPtr->StartIocpWork(this) != 0)
		{
			return;
		}
	}

	HANDLE arrHandle[] = {exitHandle, signalHandle};
	bool bExist = false;
	DWORD tmpTick = GetTickCount();
	while (!bExist)
	{
		DWORD dwRet = WaitForMultipleObjects(2, arrHandle, FALSE, 1000 * 30);
		switch (dwRet)
		{
		case WAIT_OBJECT_0:{bExist = true;break;}
		case WAIT_OBJECT_0 + 1:{this->ProcessSignalEvent();break;}
		default:break;
		}

		if ((GetTickCount() - tmpTick) > 1000 * 60)
		{
			iocpWorkPtr->OnTimerCheckExpire();
			tmpTick = GetTickCount();
		}
	}
}

void IocpServer::StopWorkThread()
{
	if (iocpWorkPtr != nullptr)
	{
		iocpWorkPtr->StopIocpWork();
		ReleasePtr(iocpWorkPtr);
	}
}

void IocpServer::PacketNetworkCallback(NetworkErrorCode errCode, NetworkEventCode eventCode, int networkChannel, SOCKADDR_IN remoteIPInfo,
									   SOCKADDR_IN localIPInfo, char* bufPtr, int bufLen, void* additionPtr, NetworkServerCallback callbackFunc)
{
	if (callbackFunc != nullptr)
	{
		NetworkInfoPacket networkInfoPacket;
		ZeroMemory((char*)&networkInfoPacket, sizeof(NetworkInfoPacket));
		networkInfoPacket.errorCode = errCode;
		networkInfoPacket.eventCode = eventCode;
		networkInfoPacket.networkChannel = networkChannel;

		char tmpBuf[BUF_LEN_30] = {0};
		sprintf_s(tmpBuf, BUF_LEN_30, "%s:%d", inet_ntoa(remoteIPInfo.sin_addr), ntohs(remoteIPInfo.sin_port));
		memcpy((char*)&(networkInfoPacket.remoteIPInfo), tmpBuf, strlen(tmpBuf));
		ZeroMemory(tmpBuf, 30);
		sprintf_s(tmpBuf, BUF_LEN_30, "%s:%d", inet_ntoa(localIPInfo.sin_addr), ntohs(localIPInfo.sin_port));
		memcpy((char*)&(networkInfoPacket.localIPInfo), tmpBuf, strlen(tmpBuf));

		networkInfoPacket.databuf = bufPtr;
		networkInfoPacket.dataBufLen = bufLen;
		networkInfoPacket.additionPtr = additionPtr;

		callbackFunc(&networkInfoPacket);
	}
}

void IocpServer::ProcessSignalEvent()
{
	std::shared_ptr<TaskEvent> taskEventPtr;
	{
		CAutoLock lock(&csTaskEvent);
		if (taskEventList.size() > 0)
		{
			taskEventPtr = taskEventList.front();
			taskEventList.pop_front();
		}
	}

	
	if (taskEventPtr.get() != nullptr)
	{
		PerHandleContext* perHandleContextPtr = nullptr;
		NetworkEventCode eventCode = NetworkEventCode::default_event;
		switch (taskEventPtr->eventCode)
		{
		case TaskEventCode::tcp_server_task :{perHandleContextPtr = new TcpServerPerHandleContext(); eventCode = NetworkEventCode::listen_event; break;}
		case TaskEventCode::tcp_connect_task :{perHandleContextPtr = new TcpClientPerHandleContext(); eventCode = NetworkEventCode::connect_init_event; break;}
		case TaskEventCode::udp_server_task :{perHandleContextPtr = new UdpServerPerHandleContext(); eventCode = NetworkEventCode::udp_server_event; break;}
		case TaskEventCode::udp_broadcast_task : 
		default:break;
		}

		if (perHandleContextPtr != nullptr)
		{
			if (perHandleContextPtr->CreatePerHandleContextObj(this, iocpWorkPtr, taskEventPtr) != 0)
			{
				//创建失败
				this->PacketNetworkCallback(NetworkErrorCode::create_error, eventCode, perHandleContextPtr->ChannelID(), perHandleContextPtr->RemoteAddress(),
					perHandleContextPtr->LocalAddress(), nullptr, 0, perHandleContextPtr->AdditionPtr(), perHandleContextPtr->CallbackFunc());
			}else
			{
				//创建成功
				this->PacketNetworkCallback(NetworkErrorCode::no_error, eventCode, perHandleContextPtr->ChannelID(), perHandleContextPtr->RemoteAddress(),
					perHandleContextPtr->LocalAddress(), nullptr, 0, perHandleContextPtr->AdditionPtr(), perHandleContextPtr->CallbackFunc());
				
			}
		}
	}
}



