#include "IocpWork.h"
#include "PerHandleContext.h"
#include "NetworkHelp.h"
#include "IocpServer.h"
#include "PerIoContext.h"


IocpWork::IocpWork(void):
	icopWorkNumber(0),
	arrHandle(nullptr),
	iocpServerPtr(nullptr)
{
	InitializeCriticalSection(&csPerHandleContext);
}


IocpWork::~IocpWork(void)
{
	DeleteCriticalSection(&csPerHandleContext);
}


int IocpWork::StartIocpWork(IocpServer* tmpIocpServerPtr)
{
	iocpServerPtr = tmpIocpServerPtr;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	icopWorkNumber = si.dwNumberOfProcessors * 2;
	arrHandle = new HANDLE[icopWorkNumber];
	if (arrHandle != nullptr)
	{
		for (int i = 0; i < icopWorkNumber; ++i)
		{
			DWORD threadID = 0;
			HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, (UINT*)&threadID);
			if (threadHandle != INVALID_HANDLE_VALUE)
			{
				arrHandle[i] = threadHandle;
			}
		}
	}
	return 0;
}

int IocpWork::StopIocpWork()
{
	this->ClearAllPerHandleContext();

	if (arrHandle != nullptr)
	{
		for (int i = 0; i < icopWorkNumber; ++i)
		{
			::PostQueuedCompletionStatus(iocpServerPtr->IocpHandle(), 0, 0, NULL);
		}
		WaitForMultipleObjects(icopWorkNumber, arrHandle, TRUE, INFINITE);

		for(int i = 0; i < icopWorkNumber; ++i)
		{
			ReleaseHandle(arrHandle[i]);
		}
		delete []arrHandle;
	}
	return 0;
}

int IocpWork::RegisterPerHandleContext(PerHandleContext* perHandleContextPtr)
{
	if (perHandleContextPtr == nullptr)
	{
		return -1;
	}
	CAutoLock lock(&csPerHandleContext);
	mapPerHandleContext.insert(std::make_pair(perHandleContextPtr->ChannelID(), perHandleContextPtr));
	return 0;
}

int IocpWork::CancelPerHandleContext(int channelID)
{
	PerHandleContext* perHandleContextPtr = nullptr;
	CAutoLock lock(&csPerHandleContext);
	std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.find(channelID);
	if (iter != mapPerHandleContext.end())
	{
		perHandleContextPtr = iter->second;
		if(perHandleContextPtr->GetHandleContextType() != PerHandleContextType::typeEmpty)
		{
			perHandleContextPtr->SetHandleContextType(PerHandleContextType::typeEmpty);
		}
		//mapPerHandleContext.erase(iter);
	}

	if (perHandleContextPtr != nullptr)
	{
		perHandleContextPtr->ExitIcopWork();
	}
	return 0;
}

int IocpWork::RemovePerHandleContext(int channelID)
{
	CAutoLock lock(&csPerHandleContext);
	std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.find(channelID);
	if (iter != mapPerHandleContext.end())
	{
		mapPerHandleContext.erase(iter);
	}
	return 0;
}

int IocpWork::TcpSend(int channelID, const char* bufPtr, const int bufLen)
{
	PerHandleContext* tmpPtr = nullptr;
	{
		CAutoLock lock(&csPerHandleContext);
		std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.find(channelID);
		if (iter != mapPerHandleContext.end())
		{
			tmpPtr = iter->second;
		}
	}

	int retValue = -1;
	if (tmpPtr != nullptr)
	{
		retValue = tmpPtr->SendNetworkInfo(const_cast<char*>(bufPtr), bufLen, "", 0);
	}
	return retValue;
}

int IocpWork::UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort)
{
	PerHandleContext* tmpPtr = nullptr;
	{
		CAutoLock lock(&csPerHandleContext);
		std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.find(channelID);
		if (iter != mapPerHandleContext.end())
		{
			tmpPtr = iter->second;
		}
	}

	int retValue = -1;
	if (tmpPtr != nullptr)
	{
		retValue = tmpPtr->SendNetworkInfo(const_cast<char*>(bufPtr), bufLen, remoteIP, remotePort);
	}
	return retValue;
}

void IocpWork::OnTimerCheckExpire()
{
	CAutoLock lock(&csPerHandleContext);
	for (std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.begin(); iter != mapPerHandleContext.end();)
	{
		if (iter->second->NetworkPerHandleContextType() == PerHandleContextType::typeEmpty)
		{
			if (iter->second->QueryCheckExpireTimes() >= 0)
			{
				PerHandleContext* tmpPtr = iter->second;
				iter = mapPerHandleContext.erase(iter);
				LOGINFO(NetworkLogger, __FILE__, "Release Network Channel:%d; ", tmpPtr->ChannelID());
				ReleasePtr(tmpPtr);
				continue;
			}
			iter->second->AddCheckExpireTimes();
		}
		++iter;
	}
}

UINT WINAPI IocpWork::ThreadProc(LPVOID lpVoid)
{
	IocpWork* objPtr = (IocpWork*)lpVoid;
	if (objPtr != nullptr)
	{
		objPtr->RunThreadEntry();
	}
	return 0;
}

void IocpWork::RunThreadEntry()
{
	ULONG_PTR dwKey = 0;
	DWORD dwTrans = 0;
	LPOVERLAPPED lpOl = nullptr;
	PerIoContext* perIoContextPtr = nullptr;
	while (true)
	{
		dwKey = 0;
		dwTrans = 0;
		lpOl = nullptr;
		perIoContextPtr = nullptr;
		BOOL bResult = GetQueuedCompletionStatus(iocpServerPtr->IocpHandle(), &dwTrans, &dwKey, (LPOVERLAPPED*)&lpOl, WSA_INFINITE);
		if (dwKey == 0 && lpOl == 0)
		{
			break;
		}

		if (!bResult)
		{
			PerHandleContext* perHandleContextPtr = (PerHandleContext*)dwKey;
			perIoContextPtr = CONTAINING_RECORD(lpOl, PerIoContext, ol);
			perHandleContextPtr->ProcessIocpFailedResult(perIoContextPtr);
		}else
		{
			perIoContextPtr = CONTAINING_RECORD(lpOl, PerIoContext, ol);
			PerHandleContext* perHandleContextPtr = (PerHandleContext*)dwKey;
			if (dwTrans == 0 && (perIoContextPtr->PerIoContextIocpType() == IocpType::type_Read || perIoContextPtr->PerIoContextIocpType() == IocpType::type_Write))
			{
				//客户端主动关闭
				perHandleContextPtr->CloseSocket();
				perHandleContextPtr->ReleasePerIoContext(perIoContextPtr);
			}else
			{
				perIoContextPtr->SetRecvDataLen(dwTrans);
				perHandleContextPtr->ProcessIocpSuccessResult(perIoContextPtr);
			}
		}
	}
}

void IocpWork::ClearAllPerHandleContext()
{
	CAutoLock lock(&csPerHandleContext);
	if (mapPerHandleContext.size() > 0)
	{
		for (std::map<int, PerHandleContext*>::iterator iter = mapPerHandleContext.begin(); iter != mapPerHandleContext.end();)
		{
			if (iter->second != nullptr)
			{
				iter->second->ExitIcopWork();
				iter = mapPerHandleContext.erase(iter);
				continue;
			}
			++iter;
		}
	}
}