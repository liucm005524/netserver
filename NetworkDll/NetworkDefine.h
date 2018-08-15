#ifndef _NETWORK_DEFINE_
#define _NETWORK_DEFINE_

#define BUF_LEN_30	30

enum NetworkEventCode
{
	default_event = 0,			//Ĭ���¼�
	listen_event,				//socket�����¼�
	accept_event,				//socket���������¼�
	connect_event,				//socket����Զ���¼�
	connect_init_event,			//socket����Զ�̳�ʼ���¼�
	close_event,				//sockt�ر��¼�
	send_event,					//socket���������¼�
	recv_event,					//socket���������¼�
	udp_server_event,			//udp�����¼�
};


enum NetworkErrorCode
{
	no_error = 0,			//
	create_error,			//��������
};

/*****************************
	˵���������ص���Ϣ��
	�����������
	����ֵ����
*****************************/
typedef struct tag_NetworkInfoPacket
{
	NetworkErrorCode errorCode;						//������
	NetworkEventCode eventCode;						//�¼���
	int networkChannel;								//����ͨ��
	char remoteIPInfo[BUF_LEN_30];					//Զ�̵�ַ��Ϣ
	char localIPInfo[BUF_LEN_30];					//���ص�ַ��Ϣ
	int dataBufLen;									//���ݳ���
	char* databuf;									//����buf
	void* additionPtr;								//����ָ��
}NetworkInfoPacket, *LPNetworkInfoPacket;

/*****************************
	˵���������ص���������
	��������������ص���Ϣ��
	����ֵ����
*****************************/
typedef void(__stdcall *NetworkServerCallback)(NetworkInfoPacket* networkPacketPtr);

#endif