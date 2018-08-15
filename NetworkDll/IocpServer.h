#pragma once
#include "NetworkHelp.h"
#include "NetworkDefine.h"

class TaskEvent;
class IocpWork;

class IocpServer
{
public:
	/*****************************
	说明:静态获取对象实例
	输入参数：无
	返回值：无
	*****************************/
	static IocpServer* GetInstance();

	/*****************************
	说明:创建服务器
	*****************************/
	int StartServer();

	/*****************************
	说明:停止服务器
	*****************************/
	int StopServer();

	/*****************************
	说明:创建TCP服务器
	*****************************/
	int StartTcpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback);

	/*****************************
	说明：创建UDP服务器
	*****************************/
	int StartUdpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback);

	/*****************************
	说明：建立IP组播
	*****************************/
	int StartBroadcastTask();

	/*****************************
	说明：建立tcp客户端
	*****************************/
	int StartTcpClientTask(std::string localIP, int localPort, std::string remoteIP, int remotePort, 
		void* additionPtr, NetworkServerCallback callback);

	/*****************************
	说明：往目的点发送数据
	*****************************/
	int TcpSend(int channelID, const char* bufPtr, const int bufLen);

	/*****************************
	说明：往目的点发送数据
	*****************************/
	int UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort);

	/*****************************
	说明：注销通道
	*****************************/
	int CancelChannelID(int channelID);

	/*****************************
	说明：完成端口句柄
	*****************************/
	HANDLE IocpHandle();

	/*****************************
	说明：网络层回调函数
	*****************************/
	void PacketNetworkCallback(NetworkErrorCode errCode, NetworkEventCode eventCode, int networkChannel, SOCKADDR_IN remoteIPInfo,
		SOCKADDR_IN localIPInfo, char* bufPtr, int bufLen, void* additionPtr, NetworkServerCallback callbackFunc);

private:
	IocpServer(void);
	~IocpServer(void);

	IocpServer(const IocpServer&);
	IocpServer operator=(const IocpServer&);

	static UINT WINAPI ThreadProc(LPVOID lpVoid);

	/*****************************
	说明：线程函数实体
	*****************************/
	void RunThreadEntry();

	/*****************************
	说明：停止工作线程
	*****************************/
	void StopWorkThread();

	/*****************************
	说明：处理信号事件
	*****************************/
	void ProcessSignalEvent();

private:
	static IocpServer* iocpServerPtr;				//完成端口服务对象
						
	HANDLE icopHandle;								//完成端口句柄
	HANDLE threadHandle;							//完成端口服务对象线程句柄
	HANDLE exitHandle;								//退出事件句柄
	HANDLE signalHandle;							//信号句柄

	CRITICAL_SECTION csTaskEvent;					//任务列表关键代码段锁
	std::list<std::shared_ptr<TaskEvent>> taskEventList;	//任务列表


	IocpWork* iocpWorkPtr;							//完成端口工作对象指针
};

