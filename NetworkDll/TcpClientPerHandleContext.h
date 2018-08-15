#pragma once
#include "perhandlecontext.h"
class TcpClientPerHandleContext :
	public PerHandleContext
{
public:
	TcpClientPerHandleContext(void);
	virtual ~TcpClientPerHandleContext(void);

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
	void InitPostRecvPacket();

	/*****************************
	说明：投递接收
	*****************************/
	bool PostRecv(PerIoContext* perIoContextPtr);

	/*****************************
	说明：投递接收包索引
	*****************************/
	long PacketIndex();

	/*****************************
	说明：连接远程服务器
	*****************************/
	int ProcessIocpConnect(PerIoContext* perIoContextPtr);

	/*****************************
	说明：读取数据
	*****************************/
	int ProcessIocpRead(PerIoContext* perIoContextPtr);

	/*****************************
	说明：写入数据
	*****************************/
	int ProcessIocpWrite(PerIoContext* perIoContextPtr);

	/*****************************
	说明：查找指定索引的数据包
	*****************************/
	PerIoContext* CurrentParsePacketIndex(long index);

private:
	static const int DEFAULT_POST_NUMBER = 10;

	LPFN_CONNECTEX lpfnConnectEx;					//ConnectEx函数地址

	long currentPostPacketNumber;					//当前投递的待连接的包数目

	CRITICAL_SECTION csPacketIndex;
	long packetIndex;								//投递包索引
	long currentParsePacketIndex;					//
	std::list<PerIoContext*> listRecvPerIoContext;	//等待解释列表
};

