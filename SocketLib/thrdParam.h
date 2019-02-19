#ifndef THRD_PARAM_H_
#define THRD_PARAM_H_
#include "common.h"
enum EThrdType
{
	TCPSrvLisn = 0,
	TCPSrvRecv = 1,
	TCPCltRecv = 2,
	UDPRecv = 3
};
class CThrdParam :public boost::basic_lockable_adapter<boost::mutex>
{
public:
	CThrdParam(int nType) :m_nType(nType),m_pThrd(NULL){}
	int GetType(){ return m_nType; }
	void SetThrd(boost::thread *pThrd){ m_pThrd = pThrd; }
	void SetTimeOff(BOOL bTimeOff);
	BOOL IsTimeOff();
	boost::thread *GetThrd(){ return m_pThrd; }
private:
	int m_nType;
	boost::thread *m_pThrd;
	BOOL m_bIsTimeOff;
};
#endif