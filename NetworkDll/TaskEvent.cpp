#include "TaskEvent.h"


TaskEvent::TaskEvent(void)
{
}


TaskEvent::~TaskEvent(void)
{
}

TaskEvent::TaskEvent(TaskEventCode tmpEventCode, std::string tmpRemoteIP, int tmpRemotePort, std::string tmpLocalIP, 
		  int tmpLocalPort, void* tmpAdditionPtr, NetworkServerCallback tmpCallback):
	eventCode(tmpEventCode),
	remoteIP(tmpRemoteIP),
	remotePort(tmpRemotePort),
	localIP(tmpLocalIP),
	localPort(tmpLocalPort),
	additionPtr(tmpAdditionPtr),
	callback(tmpCallback)
{
}