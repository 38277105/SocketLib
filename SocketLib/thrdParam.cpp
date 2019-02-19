#include "thrdParam.h"
void CThrdParam::SetTimeOff(BOOL bTimeOff)
{
	boost::lock_guard<CThrdParam> guard(*this);
	m_bIsTimeOff = bTimeOff;
}
BOOL CThrdParam::IsTimeOff()
{
	boost::lock_guard<CThrdParam> guard(*this);
	return m_bIsTimeOff;
}