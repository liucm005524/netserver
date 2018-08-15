#ifndef _NETWORK_DLL_
#define _NETWORK_DLL_

#include "NetworkDefine.h"

#ifndef NetworkDll
#define NetworkDll  _declspec(dllexport)
#else
#define NetworkDll _declspec(dllimport)
#endif // NetworkServerDllExport

/*****************************
	说明：加载网络服务
*****************************/
extern "C" int NetworkDll __stdcall InitNetworkDll();


/*****************************
	说明：卸载网络服务
*****************************/
extern "C" int NetworkDll __stdcall UnInitNetworkDll();


/*****************************
	说明：开启本地TCP服务器
	*****************************/
extern "C" int NetworkDll __stdcall StartTcpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback);


/*****************************
	说明：连接远程TCP服务器
*****************************/
extern "C" int NetworkDll __stdcall StartTcpClient(char* localIP, int localPort, char* remoteIP, int remotePort, 
												   void* additionPtr, NetworkServerCallback callback);


/*****************************
说明：开启本地UDP服务器
*****************************/
extern "C" int NetworkDll __stdcall StartUdpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback);


/*****************************
说明：开启本地IP组播服务
*****************************/
extern "C" int NetworkDll __stdcall StartBroadcastServer();


/*****************************
说明：TCP通道发送
*****************************/
extern "C" int NetworkDll __stdcall TcpSend(int channelID, const char* bufPtr, const int bufLen);

/*****************************
说明：UDP通道发送
*****************************/
extern "C" int NetworkDll __stdcall UdpSend(int channelID, const char* bufPtr, const int bufLen, char* remoteIP, int remotePort);

/*****************************
说明：IP组播发送
*****************************/
extern "C" int NetworkDll __stdcall BroadcastSend();


/*****************************
说明：注销通道
*****************************/
extern "C" int NetworkDll __stdcall CancelServer(int channelID);


#endif

