#ifndef THRD_MONITOR_H_
#define THRD_MONITOR_H_
#include "thrdParam.h"
#include <time.h>
class CThrdMonitor :public boost::basic_lockable_adapter<boost::mutex>
{
private:
	CThrdMonitor();
public:
	~CThrdMonitor();
	static CThrdMonitor &GetInstance();
	static void Process(void *pParam);
	void AddNode(CThrdParam *p);
	void DelNode(CThrdParam *p);
	CThrdParam *GetNode(time_t &nTime);
private:
	static int m_nMaxTime;//max seconds to e to judge the thread is inactivate
	std::map<CThrdParam*, time_t> m_mpMoniter;
};
#endif