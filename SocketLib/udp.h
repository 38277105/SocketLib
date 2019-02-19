/*
 * udp.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef UDP_H_
#define UDP_H_
#include "thrdParam.h"
class CUDPRecvThrdParam : public CThrdParam
{
public:
	CUDPRecvThrdParam(int nSock, UDPRecvCB pRecvCB);
	~CUDPRecvThrdParam();
	void AddNode(int nSock);
	int GetNode();
	void DelNode(int nSock);
	void ClearNode();
	UDPRecvCB GetCB();
	BOOL Exist(int nSock);
private:
	UDPRecvCB m_pRecvCB;
	std::set< int > m_setNode;
};

class CUDPSock
{
private:
	CUDPSock();
public:
	~CUDPSock();
	static CUDPSock * GetInstance();
	static void Process(void *pParam);
	int Setup(US usPort = 0, char *szIP = 0);
	int SendData(int nSock,const char *szSend,int nLen, const char *szIp, US usPort);
	int RecvData(int nSock, char *szRecv,int nLen, const char *szIp, US usPort);
	int SetCB(int nSock, UDPRecvCB pRecvCB);
	int Stop(int nSock);
	void Clear();
	CUDPRecvThrdParam * GetRecvThrdParam(int nSock);
	CUDPRecvThrdParam * GetRecvThrdParam(UDPRecvCB pRecvCB);
	void DelRecvThrd(CThrdParam *p);
private:
	std::list< CUDPRecvThrdParam *> m_lstParam;
	boost::object_pool< CUDPRecvThrdParam > m_objProcPool;
	boost::object_pool< boost::thread > m_objThrdPool;
	HANDLE m_utex;
};

#endif /* UDP_H_ */
