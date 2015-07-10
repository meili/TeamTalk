/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Condition.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月14日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>
#include "Lock.h"

class CCondition
{
public:
    CCondition(CLock* pLock);
    ~CCondition();
    void wait();
    /*
     * nWaitTime ms
     * if recv a signal then return true;
     * else return false;
     */
    bool waitTime(uint64_t nWaitTime);
    void notify();
    void notifyAll();
private:
    CLock* m_pLock;
    pthread_cond_t m_cond;
};

#endif /*defined(__CONDITION_H__) */
