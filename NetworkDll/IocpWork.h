#pragma once
#include "NetworkHelp.h"

class PerHandleContext;
class IocpServer;

class IocpWork
{
public:
	IocpWork(void);
	~IocpWork(void);

	/*****************************
	说明:开启完成端口
	*****************************/
	int StartIocpWork(IocpServer* tmpIocpServerPtr);

	/*****************************
	说明:停止完成端口
	*****************************/
	int StopIocpWork();

	/*****************************
	说明:注册单句柄对象
	*****************************/
	int RegisterPerHandleContext(PerHandleContext* perHandleContextPtr);

	/*****************************
	说明:注销单句柄对象
	*****************************/
	int CancelPerHandleContext(int channelID);
	int RemovePerHandleContext(int channelID);

	/*****************************
	说明：往目的点发送数据
	*****************************/
	int TcpSend(int channelID, const char* bufPtr, const int bufLen);

	/*****************************
	说明：往目的点发送数据
	*****************************/
	int UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort);

	/*****************************
	说明：检测网络通道过期
	*****************************/
	void OnTimerCheckExpire();

private:
	IocpWork(const IocpWork&);
	IocpWork operator=(const IocpWork&);

	static UINT WINAPI ThreadProc(LPVOID lpVoid);
	void RunThreadEntry();

	void ClearAllPerHandleContext();


private:
	int icopWorkNumber;				//工作线程对象
	HANDLE *arrHandle;				//工作线程句柄

	CRITICAL_SECTION csPerHandleContext;			//通道关键代码段锁
	std::map<int, PerHandleContext*> mapPerHandleContext;		//通道字典

	IocpServer* iocpServerPtr;		//完成端口服务对象指针
};

