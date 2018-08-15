#pragma once
#include "NetworkHelp.h"
#include "NetworkDefine.h"

/*****************************
	˵��:�����¼�����
*****************************/
enum TaskEventCode
{
	default_task = 0,
	tcp_server_task,
	udp_server_task,
	tcp_connect_task,
	udp_broadcast_task,
	tcp_accept_task,
};

/*****************************
	˵��:�����¼�����
*****************************/
class TaskEvent
{
public:
	TaskEvent(void);
	~TaskEvent(void);

	TaskEvent(TaskEventCode eventCode, std::string remoteIP, int remotePort, std::string localIP, 
		int localPort, void* addition, NetworkServerCallback callback);

private:
	TaskEvent(const TaskEvent&);
	TaskEvent operator=(const TaskEvent&);

private:
	friend class IocpServer;
	friend class PerHandleContext;
	friend class TcpClientPerHandleContext;
	TaskEventCode eventCode;
	std::string remoteIP;
	int remotePort;
	std::string localIP;
	int localPort;
	void* additionPtr;
	NetworkServerCallback callback;
};

