
#ifndef __TIMER_HPP
#define __TIMER_HPP

#include <map>
#include <utility>
#include <assert.h>
#include <stdio.h>
#include "../thread/base_thread.hpp"
#include "../type/base_type.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

static void S_Sleep(uint32_t milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

static uint64_t S_GetTickCount()
{
#ifdef _WIN32
    LARGE_INTEGER liCounter;
    LARGE_INTEGER liCurrent;
    
    if (!QueryPerformanceFrequency(&liCounter))
        return GetTickCount();
    
    QueryPerformanceCounter(&liCurrent);
    return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
    struct timeval tval;
    uint64_t ret_tick;
    
    gettimeofday(&tval, NULL);
    
    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
#endif
}


using namespace std;

typedef void (*pTimerProc)(int, void* );

class CTimer
{
public:
	CTimer()
	{
		m_iQuit = 0;
		m_pTimerProc = NULL;
	}
	virtual ~CTimer()
	{
		StopTimer();
	}

public:

	//启动定时器
	BOOL StartTimer(int nIndex, pTimerProc pfnTimerProc, unsigned int nMilliSeconds, void* param)
	{
		BOOL bRet = FALSE;
        if (NULL == pfnTimerProc)
        {
            return bRet;
        }
		m_Mutex.Lock();
		if (!m_timerthread)
		{
			m_iQuit = 0;
			m_MapMutex.Lock();
			m_MapInterval[nIndex] = make_pair(nMilliSeconds, 0);
			m_MapParam[nIndex] = param;
			m_MapMutex.Unlock();
			m_pTimerProc = pfnTimerProc;

			//set to false before thread running
			bRet = m_timerthread.Start(ThreadProc, this);
		}
		else
		{
			m_MapMutex.Lock();
			m_MapInterval[nIndex] = make_pair(nMilliSeconds, 0);
			m_MapParam[nIndex] = param;
			m_MapMutex.Unlock();
		}
		m_Mutex.Unlock();
		return bRet;
	}

	//停止定时器
	int StopTimer()
	{
		m_iQuit = 1;
        m_timerthread.Wait();
		m_MapInterval.clear();
		m_MapParam.clear();
		m_pTimerProc = NULL;
		return 0;
	}

	int KillTimer(int nIndex)
	{
		m_MapMutex.Lock();
		m_MapInterval.erase(nIndex);
		m_MapParam.erase(nIndex);
		m_MapMutex.Unlock();
		return 0;
	}

	static void* ThreadProc(void* pParam)
	{
		//detach the thread
		CTimer* pInstance = reinterpret_cast<CTimer*>(pParam);
		int iTime = 0;
		while (pInstance->m_iQuit == 0)
		{
			S_Sleep(500);

			//复制一个MAP表进行操作，即使在m_pTimerProc()函数中再次创建一个定时器，也不会出现死锁
			pInstance->m_MapMutex.Lock();
			map<int, pair<unsigned int, unsigned int > > tmp = pInstance->m_MapInterval;
			map<int, void*> paramtmp = pInstance->m_MapParam;
			pInstance->m_MapMutex.Unlock();

			map<int, pair<unsigned int, unsigned int > >::iterator iter = tmp.begin();
			while (iter != tmp.end())
			{
				iter->second.second += 500;
				if (iter->second.second >= iter->second.first)
				{
					pInstance->m_pTimerProc(iter->first, paramtmp[iter->first]);
					//触发定时器，则在原MAP表中将当前时间重置
					pInstance->m_MapMutex.Lock();
					map<int, pair<unsigned int, unsigned int > >::iterator it = pInstance->m_MapInterval.find(iter->first);
					if (it != pInstance->m_MapInterval.end())
					{
						//当前时间重置为0
						it->second.second = 0;
					}
					pInstance->m_MapMutex.Unlock();
				}
				else
				{
					//没有触发定时器，则在原MAP表中将时间叠加
					pInstance->m_MapMutex.Lock();
					map<int, pair<unsigned int, unsigned int > >::iterator it = pInstance->m_MapInterval.find(iter->first);
					if (it != pInstance->m_MapInterval.end())
					{
						//当前时间继续增加
						it->second.second += 500;
					}
					pInstance->m_MapMutex.Unlock();
				}
				iter++;
			}
		}
		return NULL;
	}
public:
	//Quit flag 1:quit; 0--not
	int m_iQuit;

	map<int/* nIndex */, pair<unsigned int/* time of miliseconds 预设定时器的时间 */ , unsigned int/* time of miliseconds，当前时间 */> > m_MapInterval;
	map<int/* nIndex */, void* > m_MapParam;

	//时间线程
    CBaseThread m_timerthread;
    
    CBaseMutex m_Mutex;
    CBaseMutex m_MapMutex;
    
	//定时回调函数
	pTimerProc m_pTimerProc;
};

#endif
