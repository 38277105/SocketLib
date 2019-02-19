/*
 * socketLib.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#include "socketLib.h"
#include "tcpClient.h"
#include "tcpServer.h"
#include "udp.h"
int g_bRun = 1;
int g_nRcvLen = 50;
int UDPSetup(US usPort,char *szIp)
{
	return CUDPSock::GetInstance()->Setup(usPort,szIp);
}
int UDPSend(int nSock,const char *szSend,int nLen,const char *szIP,US usPort)
{
	return CUDPSock::GetInstance()->SendData(nSock, szSend, nLen, szIP, usPort);
}
int UDPReceive(int nSock,char *szRecv,int nLen,const char *szIP,US usPort)
{
	return CUDPSock::GetInstance()->RecvData(nSock, szRecv, nLen, szIP, usPort);
}
int UDPStop(int nSock)
{
	return CUDPSock::GetInstance()->Stop(nSock);
}
int UDPSetReceiveCB(int nSock,RecvCB pFun)
{
	return CUDPSock::GetInstance()->SetCB(nSock,pFun);
}
//
int TCPSend(int nSock,const char *szSend,int nLen)
{
	return SENDSOCKET(nSock,szSend,nLen);
}
int TCPRecv(int nSock,char *szRecv,int nLen)
{
	return RECVSOCKET(nSock,szRecv,nLen,0);
}
//
int TCPCltSetup(US usPort/* = 0*/, char *szIp/* = 0*/)
{
	return CTCPClient::GetInstance()->Setup(usPort,szIp);
}
int TCPCltConnectSrv(int nSock,const char *szIP,US usPort)
{
	return CTCPClient::GetInstance()->ConnectSrv(nSock,szIP,usPort);
}
int TCPCltStop(int nSock)
{
	return CTCPClient::GetInstance()->Stop(nSock);
}
int TCPCltSetCB(int nSock,CltRecvCB pRecvFun,CltDisCB pDisConCB)
{
	return CTCPClient::GetInstance()->SetCB(nSock,pRecvFun,pDisConCB);
}
//
int TCPSrvSetup(US usPort,char *szIP)
{
	return CTCPServer::GetInstance()->Setup(usPort,szIP);
}
int TCPSrvStop(int nSrvSock)
{
	return CTCPServer::GetInstance()->Stop(nSrvSock);
}
int TCPSrvRelease(int nSrvSock)
{
	return CTCPServer::GetInstance()->Release(nSrvSock);
}
int TCPSrvSetCB(int nSrvSock, SrvConCB pConCB, SrvDisCB pDisCB, SrvRecvCB pRecvCB)
{
	return CTCPServer::GetInstance()->SetCB(nSrvSock,pConCB,pDisCB,pRecvCB);
}

int ClearUp()
{
	g_bRun = 0;
	return 1;
}

