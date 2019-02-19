#include "thrdMonitor.h"
#include "typeDef.h"
#include "udp.h"
#include "tcpClient.h"
#include "tcpServer.h"
int CThrdMonitor::m_nMaxTime = 1800;
CThrdMonitor::CThrdMonitor()
{

}
CThrdMonitor::~CThrdMonitor()
{

}
CThrdMonitor &CThrdMonitor::GetInstance()
{
	static CThrdMonitor instance;
	return instance;
}
void CThrdMonitor::Process(void *pParam)
{
	while (TRUE == g_bRun)
	{
		time_t t;
		CThrdParam *pThrd = GetInstance().GetNode(t);
		if (NULL == pThrd)
		{
			Sleep(1000);
			continue;
		}
		time_t tn;
		tn = time(&tn);
		if (tn - t > m_nMaxTime)
		{
			pThrd->SetTimeOff(TRUE);
			CUDPSock::GetInstance()->DelRecvThrd(pThrd);
			switch (pThrd->GetType())
			{
			case TCPCltRecv:
				break;
			case TCPSrvRecv:
				break;
			case UDPRecv:
				break;
			default:
				break;
			}
		}
	}
}
void CThrdMonitor::AddNode(CThrdParam *p)
{
	if (NULL == p)
	{
		return;
	}
	std::map<CThrdParam*, time_t>::iterator it = m_mpMoniter.find(p);
	if (m_mpMoniter.end() == it)
	{
		boost::lock_guard<CThrdMonitor> guard(*this);
		time_t t;
		m_mpMoniter[p] = time(&t);
	}
}
void CThrdMonitor::DelNode(CThrdParam *p)
{
	std::map<CThrdParam*, time_t>::iterator it = m_mpMoniter.find(p);
	if (m_mpMoniter.end() != it)
	{
		boost::lock_guard<CThrdMonitor> guard(*this);
		std::map<CThrdParam*, time_t>::iterator it2 = m_mpMoniter.find(p);
		m_mpMoniter.erase(it2);
	}
}
CThrdParam* CThrdMonitor::GetNode(time_t &nTime)
{
	boost::lock_guard<CThrdMonitor> guard(*this);
	static std::map<CThrdParam*, time_t>::iterator it = m_mpMoniter.begin();
	if (m_mpMoniter.end() == it)
	{
		it = m_mpMoniter.begin();
	} 
	if (m_mpMoniter.end() == it)
	{
		return NULL;
	}
	CThrdParam *p = it->first;
	nTime = it->second;
	it++;
	return p;
}