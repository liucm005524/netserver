#pragma once
#include "NetworkHelp.h"
#include "NetworkDefine.h"

class PerIoContext;
class IocpServer;
class IocpWork;
class TaskEvent;

enum PerHandleContextType
{
	typeDefault = 0,
	typeTcpServer,
	typeUdpServer,
	typeTcpClient,
	typeUdpClient,
	typeEmpty,
};

class PerHandleContext
{
public:
	PerHandleContext(void);
	virtual ~PerHandleContext(void);

	/*****************************
	说明：创建单句柄对象
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	说明：发送网络信息
	*****************************/
	virtual int SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort){return -1;}

	/*****************************
	说明：处理完成端口成功结果
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr){return -1;}

	/*****************************
	说明：处理完成端口失败结果
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr){return -1;}

	/*****************************
	说明：单句柄ID
	*****************************/
	int ChannelID();

	/*****************************
	说明：句柄类型
	*****************************/
	PerHandleContextType NetworkPerHandleContextType(void);

	/*****************************
	说明：检测次数
	*****************************/
	int QueryCheckExpireTimes(void);
	void AddCheckExpireTimes();

	/*****************************
	说明：远程地址
	*****************************/
	SOCKADDR_IN RemoteAddress();

	/*****************************
	说明：本地地址
	*****************************/
	SOCKADDR_IN LocalAddress();

	/*****************************
	说明：回调附加指针
	*****************************/
	void* AdditionPtr();

	/*****************************
	说明：回调函数指针
	*****************************/
	NetworkServerCallback CallbackFunc();

	/*****************************
	说明：退出完成端口
	*****************************/
	virtual int ExitIcopWork();
	virtual void CloseSocket();

	/*****************************
	说明：自删除函数
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr){}

	/*****************************
	说明：通道索引
	*****************************/
	static int ChannelIndex();

	/*****************************
	说明：句柄状态
	*****************************/
	void SetHandleContextType(PerHandleContextType contextType){this->contextType = contextType;}
	PerHandleContextType GetHandleContextType(){return this->contextType;}
protected:
	PerHandleContext(const PerHandleContext&);
	PerHandleContext operator=(const PerHandleContext&);

protected:
	void* additionPtr;					//附加指针
	NetworkServerCallback callback;		//回调函数

	int channelID;						//通道ID
	SOCKET sock;						//
	SOCKADDR_IN remoteAddr;				//远程地址
	SOCKADDR_IN localAddr;				//本地地址

	IocpServer* iocpServerPtr;			//完成端口线程对象指针
	IocpWork* iocpWorkPtr;				//完成端口对象指针

	PerHandleContextType contextType;	//通道类型
	int checkIndex;						//检测索引

	static int channelIDIndex;
};

