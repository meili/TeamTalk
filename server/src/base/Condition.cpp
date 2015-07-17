/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Condition.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月14日
*   描    述：
*
================================================================*/
#include <assert.h>
#include "Condition.h"

CCondition::CCondition(CLock* pLock):m_pLock(pLock)
{
    if(!pLock)
    {
        assert(false);
    }
    pthread_cond_init(&m_cond, NULL);
}
CCondition::~CCondition()
{
    pthread_cond_destroy(&m_cond);
}
void CCondition::wait()
{
    pthread_cond_wait(&m_cond, &m_pLock->getMutex());
}
/*
 * nWaitTime ms
 * if recv a signal then return true;
 * else return false;
 */
bool CCondition::waitTime(uint64_t nWaitTime)
{
    uint64_t nTime = nWaitTime * 1000000;
    struct timespec sTime;
    uint64_t nSec = nTime / (1000000000);
    uint64_t nNsec = nTime % (1000000000);
    sTime.tv_sec = time(NULL) + (uint32_t)nSec;
    sTime.tv_nsec = (uint32_t)nNsec;
    if(ETIMEDOUT == pthread_cond_timedwait(&m_cond, &m_pLock->getMutex(), &sTime))
    {
        return false;
    }
    return true;
}
void CCondition::notify()
{
    pthread_cond_signal(&m_cond);
}
void CCondition::notifyAll()
{
    pthread_cond_broadcast(&m_cond);
}