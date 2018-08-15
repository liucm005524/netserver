#pragma once
#include "NetworkHelp.h"

class PerIoContext
{
public:
	WSAOVERLAPPED ol;
	SOCKADDR_IN remoteAddr;

public:
	PerIoContext(void);
	~PerIoContext(void);

	/*****************************
	说明：完成端口事件类型
	*****************************/
	void SetPostParams(SOCKET sock, int bufLen, unsigned long index, IocpType iocpType);

	/*****************************
	说明：设置投递数据
	*****************************/
	void SetPostData(char* bufPtr, int bufLen);

	/*****************************
	说明：数据缓冲区长度
	*****************************/
	int GetBufDataLen();

	/*****************************
	说明：数据缓冲区指针
	*****************************/
	char* GetBufPtr();

	/*****************************
	说明：数据缓冲区指针
	*****************************/
	LPWSABUF GetWsaBufPtr();

	/*****************************
	说明：对象索引
	*****************************/
	unsigned long GetIndex();

	/*****************************
	说明：完成端口事件类型
	*****************************/
	void SetRecvDataLen(int recvDataLen);

	/*****************************
	说明：
	*****************************/
	SOCKET GetPerIoContextocket();

	/*****************************
	说明：投递事件类型
	*****************************/
	IocpType PerIoContextIocpType();

private:
	PerIoContext(const PerIoContext&);
	PerIoContext operator=(const PerIoContext&);

	void ResetDefault();

private:
	WSABUF wsaBuf;						//重叠buffer
	SOCKET socket;						//socket
	IocpType iocpType;					//iocp类型
	int bufLen;							//缓冲区长度
	char buf[BUF_LEN_8192];				//缓冲区
	unsigned long perIoContextIndex;	//投递包索引
};

