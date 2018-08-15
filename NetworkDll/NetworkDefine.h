#ifndef _NETWORK_DEFINE_
#define _NETWORK_DEFINE_

#define BUF_LEN_30	30

enum NetworkEventCode
{
	default_event = 0,			//默认事件
	listen_event,				//socket监听事件
	accept_event,				//socket接受连接事件
	connect_event,				//socket连接远程事件
	connect_init_event,			//socket连接远程初始化事件
	close_event,				//sockt关闭事件
	send_event,					//socket发送数据事件
	recv_event,					//socket接受数据事件
	udp_server_event,			//udp服务事件
};


enum NetworkErrorCode
{
	no_error = 0,			//
	create_error,			//创建错误
};

/*****************************
	说明：网络层回调信息包
	输入参数：无
	返回值：无
*****************************/
typedef struct tag_NetworkInfoPacket
{
	NetworkErrorCode errorCode;						//错误码
	NetworkEventCode eventCode;						//事件码
	int networkChannel;								//网络通道
	char remoteIPInfo[BUF_LEN_30];					//远程地址信息
	char localIPInfo[BUF_LEN_30];					//本地地址信息
	int dataBufLen;									//数据长度
	char* databuf;									//数据buf
	void* additionPtr;								//附加指针
}NetworkInfoPacket, *LPNetworkInfoPacket;

/*****************************
	说明：网络层回调函数定义
	输入参数：网络层回调信息包
	返回值：无
*****************************/
typedef void(__stdcall *NetworkServerCallback)(NetworkInfoPacket* networkPacketPtr);

#endif