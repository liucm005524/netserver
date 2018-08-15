#pragma once
#include "NetworkHelp.h"

class PerHandleContext;
class IocpServer;

class IocpWork
{
public:
	IocpWork(void);
	~IocpWork(void);

	/*****************************
	˵��:������ɶ˿�
	*****************************/
	int StartIocpWork(IocpServer* tmpIocpServerPtr);

	/*****************************
	˵��:ֹͣ��ɶ˿�
	*****************************/
	int StopIocpWork();

	/*****************************
	˵��:ע�ᵥ�������
	*****************************/
	int RegisterPerHandleContext(PerHandleContext* perHandleContextPtr);

	/*****************************
	˵��:ע�����������
	*****************************/
	int CancelPerHandleContext(int channelID);
	int RemovePerHandleContext(int channelID);

	/*****************************
	˵������Ŀ�ĵ㷢������
	*****************************/
	int TcpSend(int channelID, const char* bufPtr, const int bufLen);

	/*****************************
	˵������Ŀ�ĵ㷢������
	*****************************/
	int UdpSend(int channelID, const char* bufPtr, const int bufLen, std::string remoteIP, int remotePort);

	/*****************************
	˵�����������ͨ������
	*****************************/
	void OnTimerCheckExpire();

private:
	IocpWork(const IocpWork&);
	IocpWork operator=(const IocpWork&);

	static UINT WINAPI ThreadProc(LPVOID lpVoid);
	void RunThreadEntry();

	void ClearAllPerHandleContext();


private:
	int icopWorkNumber;				//�����̶߳���
	HANDLE *arrHandle;				//�����߳̾��

	CRITICAL_SECTION csPerHandleContext;			//ͨ���ؼ��������
	std::map<int, PerHandleContext*> mapPerHandleContext;		//ͨ���ֵ�

	IocpServer* iocpServerPtr;		//��ɶ˿ڷ������ָ��
};

