/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：Lock.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __LOCK_H__
#define __LOCK_H__

#include "ostype.h"

class CLock
{
public:
    CLock();
    virtual ~CLock();
    void lock();
    void unlock();
    pthread_mutex_t& getMutex() { return m_lock; }
#ifndef _WIN32
    virtual bool try_lock();
#endif
private:
#ifdef _WIN32
	CRITICAL_SECTION m_critical_section;
#else
    pthread_mutex_t m_lock;
#endif
};

#ifndef _WIN32
class CRWLock
{
public:
    CRWLock();
    virtual ~CRWLock();
    void rlock();
    void wlock();
    void unlock();
    bool try_rlock();
    bool try_wlock();
private:
    pthread_rwlock_t m_lock;
};

class CAutoRWLock
{
public:
    CAutoRWLock(CRWLock* pLock, bool bRLock = true);
    virtual ~CAutoRWLock();
private:
    CRWLock* m_pLock;
};

#endif

class CAutoLock
{
public:
    CAutoLock(CLock* pLock);
    virtual ~CAutoLock();
private:
    CLock* m_pLock;
};

#endif
