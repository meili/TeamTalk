/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：Thread.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
#include "Thread.h"
================================================================*/
#include "Thread.h"

CThread::CThread()
{
	m_thread_id = 0;
}

CThread::~CThread()
{
}

#ifdef _WIN32
DWORD WINAPI CThread::StartRoutine(LPVOID arg)
#else
void* CThread::StartRoutine(void* arg)
#endif
{
	CThread* pThread = (CThread*)arg;
	
	pThread->OnThreadRun();
    
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void CThread::StartThread()
{
#ifdef _WIN32
	(void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
	(void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
#endif
}

CEventThread::CEventThread()
{
	m_bRunning = false;
}

CEventThread::~CEventThread()
{
	StopThread();
}

void CEventThread::StartThread()
{
	m_bRunning = true;
	CThread::StartThread();
}

void CEventThread::StopThread()
{
	m_bRunning = false;
}

void CEventThread::OnThreadRun()
{
	while (m_bRunning)
	{
		OnThreadTick();
	}
}

CThreadNotify::CThreadNotify()
{
	pthread_mutexattr_init(&m_mutexattr);
	pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &m_mutexattr);
    
	pthread_cond_init(&m_cond, NULL);
}

CThreadNotify::~CThreadNotify()
{
	pthread_mutexattr_destroy(&m_mutexattr);
	pthread_mutex_destroy(&m_mutex);
    
	pthread_cond_destroy(&m_cond);
}
