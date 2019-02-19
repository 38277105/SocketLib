/*
 * udp.cpp
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#include "udp.h"
CUDPRecvThrdParam::CUDPRecvThrdParam(int nSock, UDPRecvCB pRecvCB) :CThrdParam(UDPRecv), m_pRecvCB(pRecvCB)
{
	m_setNode.insert(nSock);
}
CUDPRecvThrdParam::~CUDPRecvThrdParam()
{
	ClearNode();
}
void CUDPRecvThrdParam::AddNode(int nSock)
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	m_setNode.insert(nSock);
}
int CUDPRecvThrdParam::GetNode()
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	static std::set < int >::iterator it = m_setNode.begin();
	if (m_setNode.end() == it)
	{
		it = m_setNode.end();
		return SOCKET_ERROR;
	}
	int nSock = *it;
	it++;
	return nSock;
}
void CUDPRecvThrdParam::DelNode(int nSock)
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	CLOSESOCKET(nSock);
	m_setNode.erase(nSock);
}
void CUDPRecvThrdParam::ClearNode()
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	std::set< int >::iterator it = m_setNode.begin();
	for (; it != m_setNode.end(); it++)
	{
		CLOSESOCKET(*it);
	}
	m_setNode.clear();
}
UDPRecvCB CUDPRecvThrdParam::GetCB()
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	return m_pRecvCB;
}
BOOL CUDPRecvThrdParam::Exist(int nSock)
{
	boost::lock_guard<CUDPRecvThrdParam> guard(*this);
	std::set<int>::iterator it = m_setNode.find(nSock);
	if (it != m_setNode.end())
	{
		return TRUE;
	}
	
	return FALSE;
}
CUDPSock::CUDPSock()
{
	m_utex = CreateMutex(NULL, FALSE, NULL);
}
CUDPSock::~CUDPSock()
{
	Clear();
}
CUDPSock * CUDPSock::GetInstance()
{
	static CUDPSock pInstance;
	return &pInstance;
}
void CUDPSock::Process(void *pParam)
{
	CUDPRecvThrdParam *pThrdParam = (CUDPRecvThrdParam *)pParam;
	if (NULL == pParam)
	{
		return;
	}
	UDPRecvCB pRecvCB = pThrdParam->GetCB();
	if (NULL == pRecvCB)
	{
		return;
	}
	
	fd_set fdr;
	timeval tv = { 0, 0 };
	sockaddr_in adr;
	memset(&adr, 0, sizeof(sockaddr_in));
	int nAdr = sizeof(sockaddr_in);
	while (g_bRun)
	{
		//if the thread is inactivated for certain time, end this thread
		if (TRUE == pThrdParam->IsTimeOff())
		{
			return;
		}
		int nSock = pThrdParam->GetNode();
		if (nSock <= 0)
		{
			Sleep(1000);
			continue;
		}
		
		
		FD_ZERO(&fdr);
		FD_SET(nSock, &fdr);
		int nStatus = select(nSock, &fdr, NULL, NULL, &tv);
		if (nStatus > 0)
		{
			char szRecv[UDPLEN] = { 0 };

			int nLen = recvfrom(nSock, szRecv, UDPLEN, 0,(sockaddr *)&adr, &nAdr);
			if (nLen > 0)
			{
				pRecvCB(nSock, szRecv, nLen, inet_ntoa(adr.sin_addr), htons(adr.sin_port));

			} 
			else
			{
				pThrdParam->DelNode(nSock);
			}
			
		} 
		else if (0 == nStatus)
		{
			continue;
		}
		else
		{
			pThrdParam->DelNode(nSock);
		}
		
	}
	
	
}
int CUDPSock::Setup(US usPort /* = 0 */,char *szIP/*=NULL*/)
{
	int nSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (SOCKET_ERROR == nSock)
	{
		return CREATE_ERR;
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
	
	if (SOCKET_ERROR == bind(nSock,(sockaddr *)&adr,sizeof(sockaddr_in)))
	{
		return BIND_ERR;
	}
	return nSock;
}
int CUDPSock::SendData(int nSock, const char *szSend, int nLen,const char *szIp, US usPort)
{
	if (NULL == szSend || NULL == szIp)
	{
		return -1;
	}
	
	sockaddr_in adr;
	memset(&adr, 0, sizeof(sockaddr_in));
	adr.sin_family = AF_INET;
	adr.sin_addr.S_un.S_addr = inet_addr(szIp);
	adr.sin_port = htons(usPort);
	return sendto(nSock, szSend, nLen, 0, (sockaddr *)&adr,sizeof(sockaddr_in));
}
int CUDPSock::RecvData(int nSock, char *szRecv,int nLen, const char *szIp, US usPort)
{
	if (NULL == szRecv || NULL == szIp)
	{
		return -1;
	}
	sockaddr_in adr;
	memset(&adr, 0, sizeof(sockaddr_in));
	adr.sin_family = AF_INET;
	int nAdr = 0;
	return recvfrom(nSock, szRecv, nLen, 0, (sockaddr *)&adr, &nAdr);
}
int CUDPSock::SetCB(int nSock, UDPRecvCB pRecvCB)
{
	CUDPRecvThrdParam *pThrd = GetRecvThrdParam(nSock);
	if (NULL != pThrd )
	{
		if (NULL == pRecvCB)
		{
			pThrd->DelNode(nSock);
			return TRUE;
		}
		if (pThrd->GetCB() == pRecvCB)
		{
			return TRUE;
		}
		else
		{
			pThrd->DelNode(nSock);
		}
	}
	pThrd = GetRecvThrdParam(pRecvCB);
	if (pThrd)
	{
		pThrd->AddNode(nSock);
	} 
	else
	{
		pThrd = m_objProcPool.construct(nSock, pRecvCB);
		if (NULL == pThrd)
		{
			return FALSE;
		}
		boost::thread *p = m_objThrdPool.construct(Process, pThrd);
		if (NULL == p)
		{
			m_objProcPool.free(pThrd);
			return FALSE;
		}
		pThrd->SetThrd(p);
		WaitForSingleObject(m_utex, 1000);
		m_lstParam.push_back(pThrd);
		ReleaseMutex(m_utex);
		
	}	
	return TRUE;
}

int CUDPSock::Stop(int nSock)
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CUDPRecvThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end(); it++)
	{
		if (*it && (TRUE == (*it)->Exist(nSock)))
		{
			(*it)->DelNode(nSock);
			ReleaseMutex(m_utex);
			return TRUE;
		}
	}
	ReleaseMutex(m_utex);
	return FALSE;
}
void CUDPSock::Clear()
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CUDPRecvThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end(); it++)
	{
		if (NULL == *it)
		{
			continue;
		}
		
		m_objThrdPool.free((*it)->GetThrd());
		m_objProcPool.free(*it);
	}
	m_lstParam.clear();
	ReleaseMutex(m_utex);
}
CUDPRecvThrdParam *CUDPSock::GetRecvThrdParam(UDPRecvCB pRecvCB)
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CUDPRecvThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end();it++)
	{
		if (*it && ((*it)->GetCB() == pRecvCB))
		{
			ReleaseMutex(m_utex);
			return *it;
		}
		
	}
	ReleaseMutex(m_utex);
	return NULL;
}
CUDPRecvThrdParam *CUDPSock::GetRecvThrdParam(int nSock)
{
	WaitForSingleObject(m_utex, 1000);
	std::list<CUDPRecvThrdParam *>::iterator it = m_lstParam.begin();
	for (; it != m_lstParam.end();it++)
	{
		if (*it &&(TRUE == (*it)->Exist(nSock)))
		{
			ReleaseMutex(m_utex);
			return *it;
		}
	}
	ReleaseMutex(m_utex);
	return NULL;
}
void CUDPSock::DelRecvThrd(CThrdParam *p)
{
	if (NULL == p)
	{
		return;
	}
	WaitForSingleObject(m_utex, 1000);
	m_lstParam.remove((CUDPRecvThrdParam*)p);

	boost::thread *pThrd = p->GetThrd();
	if (pThrd)
	{
		pThrd->join();
		m_objThrdPool.free(pThrd);
	}
	m_objProcPool.free((CUDPRecvThrdParam*)p);
	ReleaseMutex(m_utex);
}