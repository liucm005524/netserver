#include "PerIoContext.h"


PerIoContext::PerIoContext(void):
	socket(INVALID_SOCKET),
	iocpType(IocpType::type_Default),
	bufLen(0),
	perIoContextIndex(0)
{
	ZeroMemory(buf, BUF_LEN_8192);
	ZeroMemory((char*)&ol, sizeof(ol));
	ZeroMemory((char*)&remoteAddr, sizeof(SOCKADDR_IN));
	wsaBuf.buf= nullptr;
	wsaBuf.len = 0;
}


PerIoContext::~PerIoContext(void)
{
}

void PerIoContext::SetPostParams(SOCKET sock, int bufLen, unsigned long index, IocpType iocpType)
{
	this->ResetDefault();
	this->socket = sock;
	this->bufLen = bufLen;
	this->perIoContextIndex = index;
	this->iocpType = iocpType;
	wsaBuf.buf = this->buf;
	wsaBuf.len = this->bufLen;
}

void PerIoContext::SetPostData(char* bufPtr, int bufLen)
{
	wsaBuf.len = this->bufLen = bufLen;
	memcpy(buf, bufPtr, bufLen);
}

unsigned long PerIoContext::GetIndex()
{
	return perIoContextIndex;
}

void PerIoContext::SetRecvDataLen(int recvDataLen)
{
	wsaBuf.len = this->bufLen = recvDataLen;
}

int PerIoContext::GetBufDataLen()
{
	return this->bufLen;
}

char* PerIoContext::GetBufPtr()
{
	return buf;
}

LPWSABUF PerIoContext::GetWsaBufPtr()
{
	return &wsaBuf;
}

SOCKET PerIoContext::GetPerIoContextocket()
{
	return socket;
}

IocpType PerIoContext::PerIoContextIocpType()
{
	return this->iocpType;
}

void PerIoContext::ResetDefault()
{
	ZeroMemory(buf, BUF_LEN_8192);
	ZeroMemory((char*)&ol, sizeof(ol));
	ZeroMemory((char*)&remoteAddr, sizeof(SOCKADDR_IN));
	wsaBuf.buf= nullptr;
	wsaBuf.len = 0;
	socket = INVALID_SOCKET;
	iocpType = IocpType::type_Default;
	bufLen = 0;
	perIoContextIndex = 0;
}


