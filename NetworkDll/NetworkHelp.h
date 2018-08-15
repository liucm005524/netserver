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


//加载动态库
#ifdef _DEBUG
#pragma comment( lib, "..\\lib\\LogDll_D.lib")

#else
#pragma comment( lib, "..\\lib\\LogDll.lib")
#endif



#define BUF_LEN_8192	8192	

/*****************************
	说明：完成端口事件类型
	输入参数：无
	返回值：无
*****************************/
enum IocpType
{
	type_Default = 0,				//默认类型
	type_Accept,					//接收连接
	type_Connect,					//连接远程服务器
	type_Write,						//读数据
	type_Read						//写数据
};

/*****************************
	说明：关键代码段锁对象
	输入参数：无
	返回值：无
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



//释放句柄
#define ReleaseHandle(handle) do{if(handle != INVALID_HANDLE_VALUE){CloseHandle(handle); handle = INVALID_HANDLE_VALUE;}}while(0)

//释放指针
#define ReleasePtr(ptr) do{if(ptr != NULL){delete ptr; ptr = NULL;}}while(0)

//释放socket
#define ReleaseSock(sock) do{if(sock != INVALID_SOCKET){closesocket(sock); sock = INVALID_SOCKET;}}while(0)

//释放数组
#define ReleaseArrPtr(arrPtr) do{if(arrPtr != NULL){delete[] arrPtr; arrPtr = NULL;}}while(0)

#endif