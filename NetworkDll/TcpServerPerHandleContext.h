#pragma once
#include "perhandlecontext.h"


class TcpServerPerHandleContext :
	public PerHandleContext
{
public:
	TcpServerPerHandleContext(void);
	virtual ~TcpServerPerHandleContext(void);

	/*****************************
	说明：创建单句柄对象
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	说明：处理完成端口成功结果
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr);

	/*****************************
	说明：处理完成端口失败记过
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr);

	/*****************************
	说明：自删除函数
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr);

private:
	/*****************************
	说明：初始投递监听
	*****************************/
	void InitPostListen();

	/*****************************
	说明：投递socket监听
	*****************************/
	bool PostAccept(PerIoContext* perIoContextPtr);

private:
	static const int DEFAULT_POST_NUMBER = 10;
	static const int DEFAULT_LISTEN_NUMBER = 100;

	LPFN_ACCEPTEX lpfnAcceptEx;	                     // AcceptEx函数地址
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs; // GetAcceptExSockaddrs函数地址

	long currentPostPacketNumber;					//当前投递的待连接的包数目
	CRITICAL_SECTION csLock;						//自删除锁
};

