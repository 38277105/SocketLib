/*
 * tcpServer.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#include"thrdParam.h"

struct RecvNode
{
	int m_nCltSock;
	char m_szIP[IPLEN];
	US m_usPort;
	RecvNode(int nSock, const char *szIP, US usPort);
};
class CLisnThrdParam : public CThrdParam
{
public:
	CLisnThrdParam(int nSock);
	~CLisnThrdParam();
	int SetCB(TCPSrvConCB pFunConCB, TCPSrvDisCB pFunDisCB, TCPSrvRecvCB pFunRecvCB);
	TCPSrvConCB GetConCB();
	TCPSrvDisCB GetDisCB();
	TCPSrvRecvCB GetRecvCB();
	void CloseSock();
	int GetSock();
public:
	int m_nSock;
	
	TCPSrvConCB m_pFunConCB;
	TCPSrvDisCB m_pFunDisCB;
	TCPSrvRecvCB m_pFunRecvCB;
	//US m_usPort;

};
class CRecvThrdParam : public CThrdParam
{
public:
	CRecvThrdParam(int nSrvSock,TCPSrvDisCB pDisCB,TCPSrvRecvCB pRecvCB);
	~CRecvThrdParam();
	void AddRecv(RecvNode *cltNode);
	void AddRecv(int nCltSock,const char *szIP,US usPort);
	int SetCB(TCPSrvDisCB pFunDisCB, TCPSrvRecvCB pFunRecvCB);
	TCPSrvRecvCB GetRecvCB();
	TCPSrvDisCB GetDisCB();
	RecvNode* GetNode();
	void ClearNode();
	int GetNodeSize();
	//void DelRecv(int nSrvSock,int nCltSock);
	int m_nSrvSock;
	TCPSrvDisCB m_pFunDisCB;
	TCPSrvRecvCB m_pFunRecvCB;
	std::deque<RecvNode *> m_quRcv;
};
class CTCPServer
{
private:
	CTCPServer();
public:
	~CTCPServer();
	static CTCPServer * GetInstance();
	static void AcceptProc(void *pParam);
	static void RecvDataProc(void *pParam);
	CLisnThrdParam * GetThrdListen(US usPort);
	CLisnThrdParam * GetThrdListen(int nSrvSock);
	std::map<US, CLisnThrdParam*>::iterator GetThrdListen();
	CRecvThrdParam * GetRecvThrdParam(int nSrvSock);
	void GetFunCB(int nSrvSock,TCPSrvDisCB *pDisCB,TCPSrvRecvCB *pRecvCB);

	int Setup(US usPort,char *szIP=0);
	int SetCB(int nSrvSock,TCPSrvConCB pConCB,TCPSrvDisCB pDisCB,TCPSrvRecvCB pRecvCB);
	//stop accept
	int Stop(int nSrvSock);
	//stop accept and release all related clients
	int Release(int nSrvSock);
	//clear all the threadparams
	void Clear();
	void DelRecvThrd(CThrdParam *p);
private:
	
	std::list<CRecvThrdParam*> m_lstRecvParam;
	std::map<US,CLisnThrdParam*> m_mpLisn;

	boost::object_pool<CRecvThrdParam> m_objRecvPool;
	boost::object_pool<CLisnThrdParam> m_objLisnPool;
	boost::object_pool<boost::thread>  m_objThrdPool;
	HANDLE m_utex;

};





#endif /* TCPSERVER_H_ */
