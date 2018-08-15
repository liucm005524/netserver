#pragma once
#include "perhandlecontext.h"
class UdpServerPerHandleContext :
	public PerHandleContext
{
public:
	UdpServerPerHandleContext(void);
	virtual ~UdpServerPerHandleContext(void);

	/*****************************
	说明：创建单句柄对象
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	说明：发送网络信息
	*****************************/
	virtual int SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort);

	/*****************************
	说明：处理完成端口成功结果
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr);

	/*****************************
	说明：处理完成端口失败结果
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr);

	/*****************************
	说明：自删除函数
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr);

private:
	/*****************************
	说明：初始投递接收
	*****************************/
	void InitPostRecv();

	/*****************************
	说明：投递接收
	*****************************/
	bool PostRecv(PerIoContext* perIoContextPtr);

	/*****************************
	说明：读取数据
	*****************************/
	int ProcessIocpRead(PerIoContext* perIoContextPtr);

	/*****************************
	说明：写入数据
	*****************************/
	int ProcessIocpWrite(PerIoContext* perIoContextPtr);

private:
	static const int DEFAULT_POST_NUMBER = 20;
	static const int UDP_MAX_LEN = 1400;

	long currentPostPacketNumber;					//当前投递的待连接的包数目
};

