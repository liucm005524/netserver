#ifndef _NETWORK_HELP_
#define _NETWORK_HELP_

#include <WinSock2.h>
#include <Mswsock.h>  
#include <windows.h>
#include <process.h>
#include <string>
#include <memory>
#include <list>
#include <map>

#pragma comment(lib, "ws2_32.lib")


#include "..\LogDll\LogDll.h"


//���ض�̬��
#ifdef _DEBUG
#pragma comment( lib, "..\\lib\\LogDll_D.lib")

#else
#pragma comment( lib, "..\\lib\\LogDll.lib")
#endif



#define BUF_LEN_8192	8192	

/*****************************
	˵������ɶ˿��¼�����
	�����������
	����ֵ����
*****************************/
enum IocpType
{
	type_Default = 0,				//Ĭ������
	type_Accept,					//��������
	type_Connect,					//����Զ�̷�����
	type_Write,						//������
	type_Read						//д����
};

/*****************************
	˵�����ؼ������������
	�����������
	����ֵ����
*****************************/
class CAutoLock
{
public:
	CAutoLock(LPCRITICAL_SECTION lpCs):_lpcs(lpCs)
	{
		EnterCriticalSection(_lpcs);
	}
	~CAutoLock()
	{
		LeaveCriticalSection(_lpcs);
	}

private:
	CAutoLock(const CAutoLock&);
	CAutoLock operator=(const CAutoLock&);

private:
	LPCRITICAL_SECTION _lpcs;
};



//�ͷž��
#define ReleaseHandle(handle) do{if(handle != INVALID_HANDLE_VALUE){CloseHandle(handle); handle = INVALID_HANDLE_VALUE;}}while(0)

//�ͷ�ָ��
#define ReleasePtr(ptr) do{if(ptr != NULL){delete ptr; ptr = NULL;}}while(0)

//�ͷ�socket
#define ReleaseSock(sock) do{if(sock != INVALID_SOCKET){closesocket(sock); sock = INVALID_SOCKET;}}while(0)

//�ͷ�����
#define ReleaseArrPtr(arrPtr) do{if(arrPtr != NULL){delete[] arrPtr; arrPtr = NULL;}}while(0)

#endif