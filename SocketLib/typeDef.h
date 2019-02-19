/*
 * typeDef.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_
extern BOOL g_bRun;
extern int g_nRcvLen;
extern void CLOSESOCKET(int nSock);
extern int ACCEPTSOCKET(int nSock, void *addr, int *pLen);
extern int RECVSOCKET(int nSock, char *szRecv, int nLen, int nFlag);
extern int SENDSOCKET(int nSock, const char *szSend, int nLen);
typedef unsigned short US;

typedef void (* TCPSrvConCB)(int nSrvSock,int nCltSock,const char *szIP,US usPort);
typedef void (* TCPSrvRecvCB)(int nSrvSock,int nCltSock,char *szRecv,int nLen,const char *szIP,US usPort);
typedef void (* TCPSrvDisCB)(int nSrvSock,int nCltSock,char *szIP,US usPort);
typedef void (* TCPCltRecvCB) (int nSock, char *szRecv, int nLen);
typedef void (* TCPCltDisCB) (int nSock);
typedef void(*UDPRecvCB)(int nSock, char *szRecv, int nLen, const char *szIp, US usPort);
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define RCVLEN 2048
#define UDPLEN 1024
#endif /* TYPEDEF_H_ */
