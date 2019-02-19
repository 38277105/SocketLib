/*
 * socketLib.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef SOCKETLIB_H_
#define SOCKETLIB_H_


typedef unsigned short US;
typedef void (* RecvCB)(int nSock, char *szRecv, int nLen, const char *szIp, US usPort);

typedef void (* CltRecvCB) (int nSock, char *szRecv, int nLen);
typedef void (* CltDisCB) (int nSock);
typedef void (* SrvConCB)(int nSrvSock,int nCltSock,const char *szIP,US usPort);
typedef void (* SrvRecvCB)(int nSrvSock,int nCltSock,char *szRecv,int nLen,const char *szIP,US usPort);
typedef void (* SrvDisCB)(int nSrvSock,int nCltSock,char *szIP,US usPort);
#ifdef __cplusplus
extern "C" {
#endif
int UDPSetup(US usPort=0,char *szIp=0);
int UDPSend(int nSock,const char *szSend,int nLen,const char *szIP,US usPort);
int UDPReceive(int nSock,char *szRecv,int nLen,const char *szIP,US usPort);
int UDPStop(int nSock);
int UDPSetReceiveCB(int nSock,RecvCB pFun);
//
int TCPSend(int nSock,const char *szSend,int nLen);
int TCPRecv(int nSock,char *szRecv,int nLen);
int TCPStop(int nSock);
//
int TCPCltSetup(US usPort = 0, char *szIp = 0);
int TCPCltConnectSrv(int nSock,const char *szIP,US usPort);
int TCPCltSetCB(int nSock,CltRecvCB pRecvFun,CltDisCB pDisConCB);
//
int TCPSrvSetup(US usPort,char *szIP=0);
int TCPSrvStop(int nSrvSock);
int TCPSrvRelease(int nSrvSock);
int TCPSrvSetCB(int nSrvSock,SrvConCB pConCB,SrvDisCB pDisCB,SrvRecvCB pRecvCB);

int ClearUp();




#ifdef __cplusplus
}
#endif

#endif /* SOCKETLIB_H_ */
