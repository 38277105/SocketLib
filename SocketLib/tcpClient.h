/*
 * tcpClient.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_
#include "thrdParam.h"
class CClientThrdParam : public CThrdParam
{
public:
	CClientThrdParam(TCPCltRecvCB pFunRcv=NULL,TCPCltDisCB pFunDis=NULL);
	~CClientThrdParam();
	void SetCB(TCPCltRecvCB pFunRecv, TCPCltDisCB pFunDis);
	TCPCltRecvCB GetRecvCB();
	TCPCltDisCB GetDisCB();
	int GetNodeSize();
	int GetNode();
	void AddNode(int nSock);
	void DelNode(int nSock);
	void ClearNode();
	BOOL Exist(int nSock);
private:
	TCPCltRecvCB m_pFunRecv;
	TCPCltDisCB m_pFunDis;
	std::set<int> m_setNode;
};


class CTCPClient
{
private:
	CTCPClient();
public:
	~CTCPClient();
	static CTCPClient *GetInstance();
	static void Process(void *pParam);
	int Setup(US usPort = 0, char *szIP = 0);
	int ConnectSrv(int nSock, const char *szSrvIp, US usPort);
	int SetCB(int nSock, TCPCltRecvCB pFunRcv, TCPCltDisCB pFunDis);
	int Stop(int nSock);
	CClientThrdParam *GetThrdParam(int nSock);
	CClientThrdParam *GetThrdParam(TCPCltRecvCB pFunRcv, TCPCltDisCB pFunDis);
	void Clear();
	void DelRecvThrd(CThrdParam *p);
private:
	std::list<CClientThrdParam *> m_lstParam;
	boost::object_pool<CClientThrdParam> m_objProcPool;
	boost::object_pool<boost::thread> m_objThrdPool;
	HANDLE m_utex;

};
#endif /* TCPCLIENT_H_ */
