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
	˵�����������������
	*****************************/
	virtual int CreatePerHandleContextObj(IocpServer* iocpServerPtr, IocpWork* iocpWorkPtr,
		std::shared_ptr<TaskEvent>& taskEventPtr, SOCKET tmpSock = INVALID_SOCKET);

	/*****************************
	˵��������������Ϣ
	*****************************/
	virtual int SendNetworkInfo(char* bufPtr, int len, std::string remoteIP, int remotePort){return -1;}

	/*****************************
	˵����������ɶ˿ڳɹ����
	*****************************/
	virtual int ProcessIocpSuccessResult(PerIoContext* perIoContextPtr){return -1;}

	/*****************************
	˵����������ɶ˿�ʧ�ܽ��
	*****************************/
	virtual int ProcessIocpFailedResult(PerIoContext* perIoContextPtr){return -1;}

	/*****************************
	˵���������ID
	*****************************/
	int ChannelID();

	/*****************************
	˵�����������
	*****************************/
	PerHandleContextType NetworkPerHandleContextType(void);

	/*****************************
	˵����������
	*****************************/
	int QueryCheckExpireTimes(void);
	void AddCheckExpireTimes();

	/*****************************
	˵����Զ�̵�ַ
	*****************************/
	SOCKADDR_IN RemoteAddress();

	/*****************************
	˵�������ص�ַ
	*****************************/
	SOCKADDR_IN LocalAddress();

	/*****************************
	˵�����ص�����ָ��
	*****************************/
	void* AdditionPtr();

	/*****************************
	˵�����ص�����ָ��
	*****************************/
	NetworkServerCallback CallbackFunc();

	/*****************************
	˵�����˳���ɶ˿�
	*****************************/
	virtual int ExitIcopWork();
	virtual void CloseSocket();

	/*****************************
	˵������ɾ������
	*****************************/
	virtual void ReleasePerIoContext(PerIoContext* perIoContextPtr){}

	/*****************************
	˵����ͨ������
	*****************************/
	static int ChannelIndex();

	/*****************************
	˵�������״̬
	*****************************/
	void SetHandleContextType(PerHandleContextType contextType){this->contextType = contextType;}
	PerHandleContextType GetHandleContextType(){return this->contextType;}
protected:
	PerHandleContext(const PerHandleContext&);
	PerHandleContext operator=(const PerHandleContext&);

protected:
	void* additionPtr;					//����ָ��
	NetworkServerCallback callback;		//�ص�����

	int channelID;						//ͨ��ID
	SOCKET sock;						//
	SOCKADDR_IN remoteAddr;				//Զ�̵�ַ
	SOCKADDR_IN localAddr;				//���ص�ַ

	IocpServer* iocpServerPtr;			//��ɶ˿��̶߳���ָ��
	IocpWork* iocpWorkPtr;				//��ɶ˿ڶ���ָ��

	PerHandleContextType contextType;	//ͨ������
	int checkIndex;						//�������

	static int channelIDIndex;
};

