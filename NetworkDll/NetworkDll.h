#ifndef _NETWORK_DLL_
#define _NETWORK_DLL_

#include "NetworkDefine.h"

#ifndef NetworkDll
#define NetworkDll  _declspec(dllexport)
#else
#define NetworkDll _declspec(dllimport)
#endif // NetworkServerDllExport

/*****************************
	˵���������������
*****************************/
extern "C" int NetworkDll __stdcall InitNetworkDll();


/*****************************
	˵����ж���������
*****************************/
extern "C" int NetworkDll __stdcall UnInitNetworkDll();


/*****************************
	˵������������TCP������
	*****************************/
extern "C" int NetworkDll __stdcall StartTcpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback);


/*****************************
	˵��������Զ��TCP������
*****************************/
extern "C" int NetworkDll __stdcall StartTcpClient(char* localIP, int localPort, char* remoteIP, int remotePort, 
												   void* additionPtr, NetworkServerCallback callback);


/*****************************
˵������������UDP������
*****************************/
extern "C" int NetworkDll __stdcall StartUdpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback);


/*****************************
˵������������IP�鲥����
*****************************/
extern "C" int NetworkDll __stdcall StartBroadcastServer();


/*****************************
˵����TCPͨ������
*****************************/
extern "C" int NetworkDll __stdcall TcpSend(int channelID, const char* bufPtr, const int bufLen);

/*****************************
˵����UDPͨ������
*****************************/
extern "C" int NetworkDll __stdcall UdpSend(int channelID, const char* bufPtr, const int bufLen, char* remoteIP, int remotePort);

/*****************************
˵����IP�鲥����
*****************************/
extern "C" int NetworkDll __stdcall BroadcastSend();


/*****************************
˵����ע��ͨ��
*****************************/
extern "C" int NetworkDll __stdcall CancelServer(int channelID);


#endif

