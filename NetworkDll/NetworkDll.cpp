#include "NetworkDll.h"
#include "IocpServer.h"

static IocpServer* networkServerPtr = nullptr;

int NetworkDll __stdcall InitNetworkDll()
{
	if (networkServerPtr == nullptr)
	{
		networkServerPtr = IocpServer::GetInstance();
		if (networkServerPtr->StartServer() != 0)
		{
			return -1;
		}
	}
	return 0;
}


int NetworkDll __stdcall UnInitNetworkDll()	
{
	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->StopServer();
	}
	return -1;
}

int NetworkDll __stdcall StartTcpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback)
{
	if (localIP == nullptr || strlen(localIP) <= 0)
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->StartTcpServerTask(localIP, localPort, additionPtr, callback);
	}
	return -1;
}

int NetworkDll __stdcall StartTcpClient(char* localIP, int localPort, char* remoteIP, int remotePort, 
										void* additionPtr, NetworkServerCallback callback)
{
	if (localIP == nullptr || strlen(localIP) <= 0 ||remoteIP == nullptr || strlen(remoteIP) <= 0)
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->StartTcpClientTask(localIP, localPort, remoteIP, remotePort, additionPtr, callback);
	}
	return -1;
}

int NetworkDll __stdcall StartUdpServer(char* localIP, int localPort, void* additionPtr, NetworkServerCallback callback)
{
	if (localIP == nullptr || strlen(localIP) <= 0 )
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->StartUdpServerTask(localIP, localPort, additionPtr, callback);
	}
	return -1;
}

int NetworkDll __stdcall TcpSend(int channelID, const char* bufPtr, const int bufLen)
{
	if (bufPtr == nullptr || bufLen <= 0 || channelID <= 0)
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->TcpSend(channelID, bufPtr, bufLen);
	}
	return -1;
}

int NetworkDll __stdcall UdpSend(int channelID, const char* bufPtr, const int bufLen, char* remoteIP, int remotePort)
{
	if (bufPtr == nullptr || bufLen <= 0 || channelID <= 0 || remoteIP == nullptr || strlen(remoteIP) <= 0)
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->UdpSend(channelID, bufPtr, bufLen, remoteIP, remotePort);
	}
	return -1;
}

int NetworkDll __stdcall CancelServer(int channelID)
{
	if (channelID <= 0)
	{
		return -1;
	}

	if (networkServerPtr != nullptr)
	{
		return networkServerPtr->CancelChannelID(channelID);
	}
	return -1;
}