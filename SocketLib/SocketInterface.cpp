#include "common.h"
void CLOSESOCKET(int nSock)
{
#ifdef WIN32
	closesocket(nSock);
#else
	close(nSock);
#endif
}
int ACCEPTSOCKET(int nSock, void *addr, int *pLen)
{
	if ((NULL == addr) || (NULL == pLen))
	{
		return -1;
	}

#ifdef WIN32
	sockaddr *adrin = (sockaddr *)addr;
	return accept(nSock, adrin, pLen);
#else
#endif

}
int RECVSOCKET(int nSock, char *szRecv, int nLen, int nFlag)
{
#ifdef WIN32
	return recv(nSock, szRecv, nLen, nFlag);
#else
	return 0;
#endif
}
int SENDSOCKET(int nSock, const char *szSend,int nLen)
{
#ifdef WIN32
	return send(nSock, szSend, nLen, 0);
#else
#endif
}