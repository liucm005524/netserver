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
	˵������ɶ˿��¼�����
	*****************************/
	void SetPostParams(SOCKET sock, int bufLen, unsigned long index, IocpType iocpType);

	/*****************************
	˵��������Ͷ������
	*****************************/
	void SetPostData(char* bufPtr, int bufLen);

	/*****************************
	˵�������ݻ���������
	*****************************/
	int GetBufDataLen();

	/*****************************
	˵�������ݻ�����ָ��
	*****************************/
	char* GetBufPtr();

	/*****************************
	˵�������ݻ�����ָ��
	*****************************/
	LPWSABUF GetWsaBufPtr();

	/*****************************
	˵������������
	*****************************/
	unsigned long GetIndex();

	/*****************************
	˵������ɶ˿��¼�����
	*****************************/
	void SetRecvDataLen(int recvDataLen);

	/*****************************
	˵����
	*****************************/
	SOCKET GetPerIoContextocket();

	/*****************************
	˵����Ͷ���¼�����
	*****************************/
	IocpType PerIoContextIocpType();

private:
	PerIoContext(const PerIoContext&);
	PerIoContext operator=(const PerIoContext&);

	void ResetDefault();

private:
	WSABUF wsaBuf;						//�ص�buffer
	SOCKET socket;						//socket
	IocpType iocpType;					//iocp����
	int bufLen;							//����������
	char buf[BUF_LEN_8192];				//������
	unsigned long perIoContextIndex;	//Ͷ�ݰ�����
};

