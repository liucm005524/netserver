#pragma once
#include "perhandlecontext.h"
class UdpServerPerHandleContext :
	public PerHandleContext
{
public:
	UdpServerPerHandleContext(void);
	virtual ~UdpServerPerHandleContext(void);

	/*****************************
	˵�����������������
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	˵��������������Ϣ
	*****************************/
	virtual int SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort);

	/*****************************
	˵����������ɶ˿ڳɹ����
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr);

	/*****************************
	˵����������ɶ˿�ʧ�ܽ��
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr);

	/*****************************
	˵������ɾ������
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr);

private:
	/*****************************
	˵������ʼͶ�ݽ���
	*****************************/
	void InitPostRecv();

	/*****************************
	˵����Ͷ�ݽ���
	*****************************/
	bool PostRecv(PerIoContext* perIoContextPtr);

	/*****************************
	˵������ȡ����
	*****************************/
	int ProcessIocpRead(PerIoContext* perIoContextPtr);

	/*****************************
	˵����д������
	*****************************/
	int ProcessIocpWrite(PerIoContext* perIoContextPtr);

private:
	static const int DEFAULT_POST_NUMBER = 20;
	static const int UDP_MAX_LEN = 1400;

	long currentPostPacketNumber;					//��ǰͶ�ݵĴ����ӵİ���Ŀ
};

