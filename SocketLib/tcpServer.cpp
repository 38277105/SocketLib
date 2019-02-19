/*
 * tcpServer.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */
#include"tcpServer.h"
typedef   std::map < US, CLisnThrdParam* >   LISNMAP;
typedef std::list<CRecvThrdParam*> RCVLIST;


RecvNode::RecvNode(int nSock,const char *szIP, US usPort)
{
	m_nCltSock = nSock;
	m_usPort = usPort;
	memset(m_szIP, 0, sizeof(m_szIP));
	if (szIP)
	{
		strncpy(m_szIP, szIP, sizeof(m_szIP));
	}
	
}
CLisnThrdParam::CLisnThrdParam(int nSock):CThrdParam(TCPSrvLisn),m_nSock(nSock),
m_pFunConCB(NULL),
m_pFunDisCB(NULL),
m_pFunRecvCB(NULL)
{

}
CLisnThrdParam::~CLisnThrdParam()
{
	CloseSock();
}
int CLisnThrdParam::SetCB(TCPSrvConCB pFunConCB, TCPSrvDisCB pFunDisCB, TCPSrvRecvCB pFunRecvCB)
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	m_pFunConCB = pFunConCB;
	m_pFunDisCB = pFunDisCB;
	m_pFunRecvCB = pFunRecvCB;
	return 0;
}
inline void CLisnThrdParam::CloseSock()
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	if (m_nSock > 0)
	{
		CLOSESOCKET(m_nSock);
	}
	m_nSock = SOCKET_ERROR;
}
inline int CLisnThrdParam::GetSock()
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	return m_nSock;
}
inline TCPSrvConCB CLisnThrdParam::GetConCB()
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	return m_pFunConCB;
}
inline TCPSrvDisCB CLisnThrdParam::GetDisCB()
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	return m_pFunDisCB;
}
inline TCPSrvRecvCB CLisnThrdParam::GetRecvCB()
{
	boost::lock_guard<CLisnThrdParam> guard(*this);
	return m_pFunRecvCB;
}
CRecvThrdParam::CRecvThrdParam(int nSrvSock,TCPSrvDisCB pDisCB,TCPSrvRecvCB pRecvCB):m_nSrvSock(nSrvSock),
CThrdParam(TCPSrvRecv),
m_pFunDisCB(pDisCB),
m_pFunRecvCB(pRecvCB)
{

}
CRecvThrdParam::~CRecvThrdParam()
{
	ClearNode();
}
void CRecvThrdParam::AddRecv(RecvNode *cltNode)
{
	if (NULL != cltNode)
	{
		boost::lock_guard<CThrdParam> guard(*this);
		m_quRcv.push_back(cltNode);
	}
	
}
void CRecvThrdParam::AddRecv(int nCltSock,const char *szIP,US usPort)
{
	if (NULL == szIP)
	{
		return;
	}
	RecvNode *pNode = new RecvNode(nCltSock,szIP,usPort);
	AddRecv(pNode);
}
int CRecvThrdParam::SetCB(TCPSrvDisCB pFunDisCB, TCPSrvRecvCB pFunRecvCB)
{
	boost::lock_guard<CThrdParam> guard(*this);
	m_pFunDisCB = pFunDisCB;
	m_pFunRecvCB = pFunRecvCB;
	return 0;
}
inline TCPSrvRecvCB CRecvThrdParam::GetRecvCB()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_pFunRecvCB;
}
inline TCPSrvDisCB CRecvThrdParam::GetDisCB()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_pFunDisCB;
}
RecvNode* CRecvThrdParam::GetNode()
{
	boost::lock_guard<CThrdParam> guard(*this);
	if (m_quRcv.size() == 0)
	{
		return NULL;
	}
	RecvNode *pNode = m_quRcv.front();
	m_quRcv.pop_front();
	return pNode;
}
void CRecvThrdParam::ClearNode()
{
	int nSize = GetNodeSize();
	while (nSize > 0)
	{
		RecvNode *pNode = GetNode();
		if (pNode)
		{
			CLOSESOCKET(pNode->m_nCltSock);
			delete pNode;
		}
		nSize = GetNodeSize();
	}
}
inline int CRecvThrdParam::GetNodeSize()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_quRcv.size();
}


CTCPServer::CTCPServer()
{
	m_utex = CreateMutex(NULL, FALSE, NULL);
}
CTCPServer::~CTCPServer()
{
	Clear();
}
CTCPServer * CTCPServer::GetInstance()
{
	static CTCPServer tcpSrv;
	return &tcpSrv;
}
void CTCPServer::AcceptProc(void *pParam)
{
	CLisnThrdParam* pLisn = (CLisnThrdParam*)pParam;
	if (NULL == pLisn)
	{
		return;
	}
	CTCPServer* pInstance = GetInstance();
	if (NULL == pInstance)
	{
		return;
	}
	
	//accept the connection from client
	while (TRUE == g_bRun)
	{
		sockaddr_in adr;
		memset(&adr, 0, sizeof(sockaddr_in));
		int nLen = sizeof(sockaddr_in);
		int nClient = ACCEPTSOCKET(pLisn->m_nSock, &adr, &nLen);
		if (nClient <= 0)
		{
			continue;
		}
		TCPSrvConCB pCon = pLisn->GetConCB();
		if (pCon)
		{
			pCon(pLisn->m_nSock, nClient, inet_ntoa(adr.sin_addr), htons(adr.sin_port));
		}
		
		CRecvThrdParam *pRecv = pInstance->GetRecvThrdParam(pLisn->m_nSock);
		if (NULL == pRecv)
		{
			pRecv = pInstance->m_objRecvPool.construct(pLisn->m_nSock, pLisn->m_pFunDisCB, pLisn->m_pFunRecvCB);
			if (NULL == pRecv)
			{
				continue;
			}
			boost::thread* pthrd = pInstance->m_objThrdPool.construct(RecvDataProc, pRecv);
			if (NULL == pthrd)
			{
				pInstance->m_objRecvPool.free(pRecv);
				continue;
			}
			pRecv->SetThrd(pthrd);
		}
		pRecv->AddRecv(nClient, inet_ntoa(adr.sin_addr),htons(adr.sin_port));
	}	
}
void CTCPServer::RecvDataProc(void *pParam)
{
	CRecvThrdParam* pRecv = (CRecvThrdParam*)pParam;
	if (NULL == pRecv)
	{
		return;
	}
	CTCPServer* pInstance = GetInstance();
	if (NULL == pInstance)
	{
		return;
	}
	timeval tv = { 0, 0 };
	while (TRUE == g_bRun)
	{
		//if the thread is inactivated for certain time, end this thread
		if (TRUE == pRecv->IsTimeOff())
		{
			return;
		}
		RecvNode* node = pRecv->GetNode();
		if (NULL == node)
		{
			Sleep(100);
			continue;
		}
		fd_set fdr;
		FD_ZERO(&fdr);
		FD_SET(node->m_nCltSock, &fdr);
		//get the read status of the socket
		int nStatus = select(node->m_nCltSock, &fdr, NULL, NULL, &tv);
		//hava data to recv
		if (nStatus > 0)
		{
			char szRecv[2048] = { 0 };
			int nLen = RECVSOCKET(node->m_nCltSock, szRecv, sizeof(szRecv), 0);
			if (nLen > 0)
			{
				pRecv->AddRecv(node);
				TCPSrvRecvCB pFun = pRecv->GetRecvCB();
				if (pFun)
				{
					pFun(pRecv->m_nSrvSock, node->m_nCltSock, szRecv, nLen, node->m_szIP, node->m_usPort);
				}
				
			}
			//the connection is closed
		    else if (0 == nLen)
			{
				delete node;
			} 
			//error happens
			else 
			{
				//reset the connection
				if (WSAECONNRESET == WSAGetLastError())
				{
					TCPSrvDisCB pDis = pRecv->GetDisCB();
					if (pDis)
					{
						pDis(pRecv->m_nSrvSock, node->m_nCltSock, node->m_szIP, node->m_usPort);

					}
					
				}
				else
				{
					CLOSESOCKET(node->m_nCltSock);
				}	
				delete node;
			}			
		}
		//no data to be recv
		else if (0 == nStatus)
		{
			pRecv->AddRecv(node);
		}
		//error happens()
		else
		{
			CLOSESOCKET(node->m_nCltSock);
			delete node;
		}	
	}	
}
CLisnThrdParam * CTCPServer::GetThrdListen(US usPort)
{
	LISNMAP::iterator it = m_mpLisn.find(usPort);
	if (it != m_mpLisn.end())
	{
		return it->second;
	}
	return NULL;
}
CLisnThrdParam * CTCPServer::GetThrdListen(int nSrvSock)
{
	LISNMAP::iterator it = m_mpLisn.begin();
	for (it; it != m_mpLisn.end();it++)
	{
		if (it->second && (it->second->m_nSock == nSrvSock))
		{
			return it->second;
		}
	}
	
	return NULL;
}
LISNMAP::iterator CTCPServer::GetThrdListen()
{
	LISNMAP::iterator it = m_mpLisn.begin();
	for (it; it != m_mpLisn.end(); it++)
	{
		if (it->second && (it->second->m_nSock <= 0))
		{
			return it;
		}
	}
	return m_mpLisn.end();
}
CRecvThrdParam * CTCPServer::GetRecvThrdParam(int nSrvSock)
{
	WaitForSingleObject(m_utex, 1000);
	RCVLIST::iterator it = m_lstRecvParam.begin();
	for (it; it != m_lstRecvParam.end();it++)
	{
		if ((*it)->m_nSrvSock == nSrvSock && (*it)->m_quRcv.size() < g_nRcvLen)
		{
			ReleaseMutex(m_utex);
			return *it;
		}
		
	}
	ReleaseMutex(m_utex);
	return NULL;
}
void CTCPServer::GetFunCB(int nSrvSock,TCPSrvDisCB *pDisCB,TCPSrvRecvCB *pRecvCB)
{
	
}
int CTCPServer::Setup(US usPort,char *szIP/*=0*/)
{
	CLisnThrdParam* pLisn = GetThrdListen(usPort);
	if ((NULL != pLisn) && (pLisn->m_nSock > 0))
	{
		return pLisn->m_nSock;
	}
	
	int nSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(nSock < 0)
	{
		return nSock;
	}
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(usPort);
	if(szIP)
	{
		sa.sin_addr.s_addr = inet_addr(szIP);
	}
	else
	{
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	if(bind(nSock,(sockaddr*)&sa,sizeof(sockaddr_in)) < 0)
	{

		return BIND_ERR;
	}
    if(listen(nSock,5) < 0)
    {
    	CLOSESOCKET(nSock);
    	return LISTEN_ERR;
    }
	
	LISNMAP::iterator it = GetThrdListen();
	if ((m_mpLisn.end() != it) && it->second)
	{
		(it->second)->CloseSock();
		m_mpLisn[usPort] = it->second;
		m_mpLisn.erase(it);
		return nSock;
	}
	
    CLisnThrdParam *pParam = m_objLisnPool.construct(nSock);
    if(NULL == pParam)
    {
    	CLOSESOCKET(nSock);
    	return CREATE_ERR;
    }
    boost::thread *pThrd = m_objThrdPool.construct(AcceptProc,pParam);
    if(NULL == pThrd)
    {
    	CLOSESOCKET(nSock);
		m_objLisnPool.free(pParam);
    	return CREATE_ERR;
    }
    pParam->SetThrd(pThrd);
    m_mpLisn[usPort] = pParam;
	return nSock;
}
int CTCPServer::SetCB(int nSrvSock,TCPSrvConCB pConCB,TCPSrvDisCB pDisCB,TCPSrvRecvCB pRecvCB)
{
	CLisnThrdParam * pLisn = GetThrdListen(nSrvSock);
	if(NULL == pLisn)
	{
		return -1;
	}
	else
	{
		pLisn->SetCB(pConCB,pDisCB,pRecvCB);
	}
	WaitForSingleObject(m_utex, 1000);
	RCVLIST::iterator it = m_lstRecvParam.begin();
	
	for(;it!=m_lstRecvParam.end();it++)
	{
		if ((*it)->m_nSrvSock == nSrvSock)
		{
			(*it)->SetCB(pDisCB, pRecvCB);
		}
	}
	ReleaseMutex(m_utex);
	return 0;
}
int CTCPServer::Stop(int nSrvSock)
{
	CLisnThrdParam *pLisn = GetThrdListen(nSrvSock);
	if (pLisn)
	{
		pLisn->CloseSock();
	}
	return 0;
}
int CTCPServer::Release(int nSrvSock)
{
	Stop(nSrvSock);
	WaitForSingleObject(m_utex, 1000);
	RCVLIST::iterator it = m_lstRecvParam.begin();
	for (it; it != m_lstRecvParam.end();it++)
	{
		(*it)->ClearNode();
	}
	ReleaseMutex(m_utex);
	return 0;
}
void CTCPServer::Clear()
{
	LISNMAP::iterator itLisn = m_mpLisn.begin();
	for (itLisn; itLisn != m_mpLisn.end(); itLisn++)
	{
		if (itLisn->second)
		{
			boost::thread *pThrd = itLisn->second->GetThrd();
			m_objThrdPool.free(pThrd);
			m_objLisnPool.free(itLisn->second);
		}
	}
	WaitForSingleObject(m_utex, 1000);
	RCVLIST::iterator itRcv = m_lstRecvParam.begin();
	for (itRcv; itRcv != m_lstRecvParam.end(); itRcv++)
	{
		if (*itRcv)
		{
			boost::thread *pThrd = (*itRcv)->GetThrd();
			m_objThrdPool.free(pThrd);
			m_objRecvPool.free(*itRcv);
		}

	}
	ReleaseMutex(m_utex);
}
void CTCPServer::DelRecvThrd(CThrdParam *p)
{
	if (NULL == p)
	{
		return;
	}
	WaitForSingleObject(m_utex, 1000);
	m_lstRecvParam.remove((CRecvThrdParam*)p);

	boost::thread *pThrd = p->GetThrd();
	if (pThrd)
	{
		pThrd->join();
		m_objThrdPool.free(pThrd);
	}
	m_objRecvPool.free((CRecvThrdParam*)p);
	ReleaseMutex(m_utex);
}