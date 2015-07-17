//
//  base_thread.hpp
//  my-code
//
//  Created by shiwei on 14-8-4.
//  Copyright (c) 2014Äê shiwei. All rights reserved.
//

#ifndef _BASE_THREAD_HPP
#define _BASE_THREAD_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "../type/base_type.h"

typedef void* (*thread_func_cb)(void* param);


/**
 *  thread class
 */
class CBaseThread
{
public:
	CBaseThread()
    {
        m_thread_id = 0;
    }
	virtual ~CBaseThread()
    {
#ifdef _WIN32
        if (m_thread_id != 0) {
            CloseHandle(m_thread_id);
        }
#endif
    }
    
	virtual BOOL Start(thread_func_cb func_cb, void* param)
    {
        BOOL bRet = FALSE;
#ifdef _WIN32
        DWORD dwThread;
        m_thread_id = CreateThread(NULL, 0, func_cb, param, 0, &dwThread);
        if (m_thread_id != 0) {
            bRet = TRUE;
        }
#else
        if (0 == pthread_create(&m_thread_id, NULL, func_cb, param))
        {
            bRet = TRUE;
        }
        else
        {
            m_thread_id = 0;
        }
#endif
        return bRet;
    }
    
    void Wait()
    {
#ifdef _WIN32
        int32_t nRet = 0;
        nRet = WaitForSingleObject(m_thread_id, INFINITE);
        if (m_thread_id != 0) {
            CloseHandle(m_thread_id);
        }
#else
        pthread_join(m_thread_id, NULL);
#endif
        m_thread_id = 0;
    }
    
    operator bool() const
    {
        bool bRet = false;
        if (m_thread_id != 0) {
            bRet = true;
        }
        return bRet;
    }
protected:
#ifdef _WIN32
	HANDLE		m_thread_id;
#else
	pthread_t	m_thread_id;
#endif
};


/**
 *  Mutex class
 */
class CBaseMutex
{
public:
	CBaseMutex()
    {
#ifdef _WIN32
        InitializeCriticalSection(&m_critical_section);
#else
        pthread_mutexattr_init(&m_mutexattr);
        pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &m_mutexattr);
#endif
    }
    
	~CBaseMutex()
    {
#ifdef _WIN32
        DeleteCriticalSection(&m_critical_section);
#else
        pthread_mutexattr_destroy(&m_mutexattr);
        pthread_mutex_destroy(&m_mutex);
#endif
    }
    
	void Lock(void)
    {
#ifdef _WIN32
        EnterCriticalSection(&m_critical_section);
#else
        pthread_mutex_lock(&m_mutex);
#endif
    }
    
	void Unlock(void)
    {
#ifdef _WIN32
        LeaveCriticalSection(&m_critical_section);
#else
        pthread_mutex_unlock(&m_mutex);
#endif
    }
    
private:
    
#ifdef _WIN32
	CRITICAL_SECTION	m_critical_section;
#else
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;
#endif
    
};


/**
 *  Guard class, bind with Mutex
 */
class CBaseGuard
{
public:
	CBaseGuard(CBaseMutex* lock)
	{
		m_lock = lock;
		if (m_lock)
			m_lock->Lock();
	}
    
	~CBaseGuard()
	{
		if (m_lock)
			m_lock->Unlock();
	}
private:
	CBaseMutex*	m_lock;
};
#endif
