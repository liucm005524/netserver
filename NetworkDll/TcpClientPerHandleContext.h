#pragma once
#include "perhandlecontext.h"
class TcpClientPerHandleContext :
	public PerHandleContext
{
public:
	TcpClientPerHandleContext(void);
	virtual ~TcpClientPerHandleContext(void);

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
	void InitPostRecvPacket();

	/*****************************
	˵����Ͷ�ݽ���
	*****************************/
	bool PostRecv(PerIoContext* perIoContextPtr);

	/*****************************
	˵����Ͷ�ݽ��հ�����
	*****************************/
	long PacketIndex();

	/*****************************
	˵��������Զ�̷�����
	*****************************/
	int ProcessIocpConnect(PerIoContext* perIoContextPtr);

	/*****************************
	˵������ȡ����
	*****************************/
	int ProcessIocpRead(PerIoContext* perIoContextPtr);

	/*****************************
	˵����д������
	*****************************/
	int ProcessIocpWrite(PerIoContext* perIoContextPtr);

	/*****************************
	˵��������ָ�����������ݰ�
	*****************************/
	PerIoContext* CurrentParsePacketIndex(long index);

private:
	static const int DEFAULT_POST_NUMBER = 10;

	LPFN_CONNECTEX lpfnConnectEx;					//ConnectEx������ַ

	long currentPostPacketNumber;					//��ǰͶ�ݵĴ����ӵİ���Ŀ

	CRITICAL_SECTION csPacketIndex;
	long packetIndex;								//Ͷ�ݰ�����
	long currentParsePacketIndex;					//
	std::list<PerIoContext*> listRecvPerIoContext;	//�ȴ������б�
};

