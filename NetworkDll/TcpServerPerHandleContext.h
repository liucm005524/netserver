#pragma once
#include "perhandlecontext.h"


class TcpServerPerHandleContext :
	public PerHandleContext
{
public:
	TcpServerPerHandleContext(void);
	virtual ~TcpServerPerHandleContext(void);

	/*****************************
	˵�����������������
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	˵����������ɶ˿ڳɹ����
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr);

	/*****************************
	˵����������ɶ˿�ʧ�ܼǹ�
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr);

	/*****************************
	˵������ɾ������
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr);

private:
	/*****************************
	˵������ʼͶ�ݼ���
	*****************************/
	void InitPostListen();

	/*****************************
	˵����Ͷ��socket����
	*****************************/
	bool PostAccept(PerIoContext* perIoContextPtr);

private:
	static const int DEFAULT_POST_NUMBER = 10;
	static const int DEFAULT_LISTEN_NUMBER = 100;

	LPFN_ACCEPTEX lpfnAcceptEx;	                     // AcceptEx������ַ
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs; // GetAcceptExSockaddrs������ַ

	long currentPostPacketNumber;					//��ǰͶ�ݵĴ����ӵİ���Ŀ
	CRITICAL_SECTION csLock;						//��ɾ����
};

