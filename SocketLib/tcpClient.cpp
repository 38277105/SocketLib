/*
 * tcpClient.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#include "tcpClient.h"
CClientThrdParam::CClientThrdParam(TCPCltRecvCB pFunRcv, TCPCltDisCB pFunDis) :CThrdParam(TCPCltRecv)
, m_pFunRecv(pFunRcv)
, m_pFunDis(pFunDis)
{

}
CClientThrdParam::~CClientThrdParam()
{
	ClearNode();
}
void CClientThrdParam::SetCB(TCPCltRecvCB pFunRecv, TCPCltDisCB pFunDis)
{
	boost::lock_guard<CThrdParam> guard(*this);
	m_pFunRecv = pFunRecv;
	m_pFunDis = pFunDis;
}
TCPCltRecvCB CClientThrdParam::GetRecvCB()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_pFunRecv;
}
TCPCltDisCB CClientThrdParam::GetDisCB()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_pFunDis;
}
int CClientThrdParam::GetNodeSize()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_setNode.size();
}
int CClientThrdParam::GetNode()
{
	boost::lock_guard<CThrdParam> guard(*this);
	static std::set<int>::iterator it = m_setNode.begin();
	int nNode = SOCKET_ERROR;
	if (it != m_setNode.end())
	{
		nNode = *it;
	}
	it++;
	if (m_setNode.end() == it)
	{
		it = m_setNode.begin();
	}
	return nNode;
}
void CClientThrdParam::AddNode(int nSock)
{
	boost::lock_guard<CThrdParam> guard(*this);
	m_setNode.insert(nSock);
}
void CClientThrdParam::DelNode(int nSock)
{
	boost::lock_guard<CThrdParam> guard(*this);
	m_setNode.erase(nSock);
}
void CClientThrdParam::ClearNode()
{
	boost::lock_guard<CThrdParam> guard(*this);
	std::set<int>::iterator it = m_setNode.begin();
	for (it; it != m_setNode.end();it++)
	{
		CLOSESOCKET(*it);
	}
	m_setNode.clear();
	
}
BOOL CClientThrdParam::Exist(int nSock)
{
	boost::lock_guard<CThrdParam> guard(*this);
	std::set< int >::iterator it = m_setNode.find(nSock);
	if (m_setNode.end() != it)
	{
		return TRUE;
	}
	return FALSE;
}
CTCPClient::CTCPClient()
{
	m_utex = CreateMutex(NULL, FALSE, NULL);
}
CTCPClient::~CTCPClient()
{
	Clear();
	
}
CTCPClient *CTCPClient::GetInstance()
{
	static CTCPClient instance;
	return &instance;
}
void CTCPClient::Process(void *pParam)
{
	CClientThrdParam *pCltParam = (CClientThrdParam*)pParam;
	if (NULL == pCltParam)
	{
		return;
	}
	CTCPClient *pInstance = GetInstance();
	if (NULL == pInstance)
	{
		return;
	}
	fd_set fdr;
	timeval tv = { 0, 0 };
	int nSock = SOCKET_ERROR;
	int nStatus = 0;
	while (TRUE == g_bRun)
	{
		//if the thread is inactivated for certain time, end this thread
		if (TRUE == pCltParam->IsTimeOff())
		{
			return;
		}
		
		nSock = pCltParam->GetNode();
		if (nSock <= 0)
		{
			Sleep(1000);
			continue;
		}
		TCPCltRecvCB pRcv = pCltParam->GetRecvCB();
		if (pRcv)
		{
			FD_ZERO(&fdr);
			FD_SET(nSock, &fdr);
			nStatus = select(nSock, &fdr, NULL, NULL, &tv);
			if (nStatus > 0)
			{
				char szRecv[RCVLEN] = { 0 };
				int nLen = RECVSOCKET(nSock, szRecv, RCVLEN, 0);
				if (nLen > 0)
				{
					pRcv(nSock, szRecv, nLen);
				}
				else if (0 == nLen)
				{
					pCltParam->DelNode(nSock);
					continue;
				}
				else//error happens
				{
					//resetthe connection
					if (WSAECONNRESET == WSAGetLastError())
					{
						TCPCltDisCB pDis = pCltParam->GetDisCB();
						if (pDis)
						{
							pDis(nSock);
						}
						
					}
					else
					{
						CLOSESOCKET(nSock);
					}
					pCltParam->DelNode(nSock);
				}
			}
			else if (0 == nStatus)
			{
				continue;
			}
			else
			{
				pCltParam->DelNode(nSock);
			}	
		}			
	}	
}
int CTCPClient::Setup(US usPort /* = 0 */, char *szIP/* = 0*/)
{
	int nSock = socket(AF_INET, SOCK_STREAM, 0);
	if (nSock < 0)
	{
		return nSock;
	}
	sockaddr_in adr;
	memset(&adr, 0, sizeof(sockaddr_in));
	adr.sin_family = AF_INET;
	if (0 != usPort && NULL != szIP)
	{
		adr.sin_port = htons(usPort);
		adr.sin_addr.S_un.S_addr = inet_addr(szIP);
	}
	else if (0 != usPort)
	{
		adr.sin_port = htons(usPort);
		adr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);
	}
	else if (NULL != szIP)
	{
		adr.sin_addr.S_un.S_addr = inet_addr(szIP);
	}
	else
	{
		return nSock;
	}
	if (SOCKET_ERROR == bind(nSock, (sockaddr *)&adr, sizeof(sockaddr_in)))
	{
		return BIND_ERR;
	}
	return nSock;
}
int CTCPClient::ConnectSrv(int nSock,const char *szSrvIp, US usPort)
{
	if (!szSrvIp)
	{
		return NULLSTR;
	}
	sockaddr_in adr;
	memset(&adr, 0, sizeof(sockaddr_in));
	adr.sin_family = AF_INET;
	adr.sin_addr.S_un.S_addr = inet_addr(szSrvIp);
	adr.sin_port = htons(usPort);
	return connect(nSock,(sockaddr *)&adr,sizeof(sockaddr_in));
}
int CTCPClient::SetCB(int nSock, TCPCltRecvCB pFunRcv, TCPCltDisCB pFunDis)
{
	CClientThrdParam *pThrdParam = GetThrdParam(nSock);
	if (NULL != pThrdParam)
	{
		pThrdParam->SetCB(pFunRcv, pFunDis);
		return TRUE;
	}
	pThrdParam = GetThrdParam(pFunRcv, pFunDis);
	if (NULL != pThrdParam)
	{
		pThrdParam->AddNode(nSock);
		return TRUE;
	}
	pThrdParam = m_objProcPool.construct(pFunRcv, pFunDis);
	if (NULL == pThrdParam)
	{
		return FALSE;
	}
	
	boost::thread *pThrd = m_objThrdPool.construct(Process, pThrdParam);
	if (NULL == pThrd)
	{
		return FALSE;
	}
	pThrdParam->SetThrd(pThrd);
	return TRUE;
}
int CTCPClient::Stop(int nSock)
{
	CLOSESOCKET(nSock);
	return 0;
}
CClientThrdParam *CTCPClient::GetThrdParam(TCPCltRecvCB pFunRcv, TCPCltDisCB pFunDis)
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CClientThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end();it++)
	{
		if (*it)
		{
			CClientThrdParam *pParam = *it;
			if (pParam->GetRecvCB() == pFunRcv && pParam->GetDisCB() == pFunDis)
			{
				ReleaseMutex(m_utex);
				return *it;
			}
		}		
	}	
	ReleaseMutex(m_utex);
	return NULL;
}
CClientThrdParam *CTCPClient::GetThrdParam(int nSock)
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CClientThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end();it++)
	{
		CClientThrdParam *pParam = *it;
		if (!pParam)
		{
			continue;
		}
		if (TRUE == pParam->Exist(nSock))
		{
			ReleaseMutex(m_utex);
			return pParam;
		}
	}
	ReleaseMutex(m_utex);
	return FALSE;
}
void CTCPClient::Clear()
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CClientThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end();it++)
	{
		CClientThrdParam *pParam = *it;
		if (pParam)
		{
			m_objThrdPool.free(pParam->GetThrd());
			m_objProcPool.free(pParam);
		}	
	}
	m_lstParam.clear();
	ReleaseMutex(m_utex);
}
void CTCPClient::DelRecvThrd(CThrdParam *p)
{
	if (NULL == p)
	{
		return;
	}
	WaitForSingleObject(m_utex, 1000);
	m_lstParam.remove((CClientThrdParam*)p);
	
	boost::thread *pThrd = p->GetThrd();
	if (pThrd)
	{
		pThrd->join();
		m_objThrdPool.free(pThrd);
	}
	m_objProcPool.free((CClientThrdParam*)p);
	ReleaseMutex(m_utex);
}