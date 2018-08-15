#pragma once
#include "NetworkHelp.h"
#include "NetworkDefine.h"

class TaskEvent;
class IocpWork;

class IocpServer
{
public:
	/*****************************
	˵��:��̬��ȡ����ʵ��
	�����������
	����ֵ����
	*****************************/
	static IocpServer* GetInstance();

	/*****************************
	˵��:����������
	*****************************/
	int StartServer();

	/*****************************
	˵��:ֹͣ������
	*****************************/
	int StopServer();

	/*****************************
	˵��:����TCP������
	*****************************/
	int StartTcpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback);

	/*****************************
	˵��������UDP������
	*****************************/
	int StartUdpServerTask(std::string localIP, int localPort, void* additionPtr, NetworkServerCallback callback);

	/*****************************
	˵��������IP�鲥
	*****************************/
	int StartBroadcastTask();

	/*****************************
	˵��������tcp�ͻ���
	*****************************/
	int StartTcpClientTask(std::string localIP, int localPort, std::string remoteIP, int remotePort, 
		void* additionPtr, NetworkServerCallback callback);

	/*****************************
	˵������Ŀ�ĵ㷢������
	*****************************/
	int TcpSend(int channelID, const char* bufPtr, const int bufLen);

	/*****************************
	˵������Ŀ�ĵ㷢������
	*****************************/
	int UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort);

	/*****************************
	˵����ע��ͨ��
	*****************************/
	int CancelChannelID(int channelID);

	/*****************************
	˵������ɶ˿ھ��
	*****************************/
	HANDLE IocpHandle();

	/*****************************
	˵���������ص�����
	*****************************/
	void PacketNetworkCallback(NetworkErrorCode errCode, NetworkEventCode eventCode, int networkChannel, SOCKADDR_IN remoteIPInfo,
		SOCKADDR_IN localIPInfo, char* bufPtr, int bufLen, void* additionPtr, NetworkServerCallback callbackFunc);

private:
	IocpServer(void);
	~IocpServer(void);

	IocpServer(const IocpServer&);
	IocpServer operator=(const IocpServer&);

	static UINT WINAPI ThreadProc(LPVOID lpVoid);

	/*****************************
	˵�����̺߳���ʵ��
	*****************************/
	void RunThreadEntry();

	/*****************************
	˵����ֹͣ�����߳�
	*****************************/
	void StopWorkThread();

	/*****************************
	˵���������ź��¼�
	*****************************/
	void ProcessSignalEvent();

private:
	static IocpServer* iocpServerPtr;				//��ɶ˿ڷ������
						
	HANDLE icopHandle;								//��ɶ˿ھ��
	HANDLE threadHandle;							//��ɶ˿ڷ�������߳̾��
	HANDLE exitHandle;								//�˳��¼����
	HANDLE signalHandle;							//�źž��

	CRITICAL_SECTION csTaskEvent;					//�����б�ؼ��������
	std::list<std::shared_ptr<TaskEvent>> taskEventList;	//�����б�


	IocpWork* iocpWorkPtr;							//��ɶ˿ڹ�������ָ��
};

