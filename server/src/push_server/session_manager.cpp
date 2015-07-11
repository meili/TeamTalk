//
//  session_manager.cpp
//  my_push_server
//
//  Created by luoning on 14-11-11.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "session_manager.h"
#include "push_define.h"
#define TIMER_INDEX_CHECK_PUSHSESSION           1

#define TIME_CHECK_PUSHSESSION                  5000

#define TIMEOUT_PUSHSESSION                     30 * 1000
CSessionManager::CSessionManager()
{
}

CSessionManager::~CSessionManager()
{
    StopCheckPushSession();
    RemoveAPNSClient();
    RemovePushServer();
    ClearPushSession();
}

CSessionManager* CSessionManager::GetInstance()
{
    static CSessionManager session_manager;
    return &session_manager;
}

void CSessionManager::TimerProc(int32_t nIndex, void *param)
{
    CSessionManager* pInstance = (CSessionManager*)param;
    switch (nIndex) {
        case TIMER_INDEX_CHECK_PUSHSESSION:
            pInstance->CheckPushSessionTimeOut();
            break;
        default:
            break;
    }
}

void CSessionManager::AddPushSessionBySockID(uint32_t nsockid, push_session_ptr pSession)
{
    m_MapIOPushSessionBySockIDMutex.Lock();
    m_MapPushSessionBySockID[nsockid] = pSession;
    m_MapIOPushSessionBySockIDMutex.Unlock();
}

void CSessionManager::RemovePushSessionBySockID(uint32_t nsockid)
{
    //此处的机制导致锁最好是重入锁，假如没有先调用stop，而是直接先调用该接口，会进入一个递归。对象删除的时候，
    //会调用stop，又进入了该接口
    //目前，Mutex在linux/mac/windows下都是重入锁
    m_MapIOPushSessionBySockIDMutex.Lock();
    m_MapPushSessionBySockID.erase(nsockid);
    m_MapIOPushSessionBySockIDMutex.Unlock();
}

push_session_ptr CSessionManager::GetPushSessionBySockID(uint32_t nsockid)
{
    m_MapIOPushSessionBySockIDMutex.Lock();
    push_session_ptr pSession;
    hash_map<uint32_t, push_session_ptr>::iterator it = m_MapPushSessionBySockID.find(nsockid);
    if (it != m_MapPushSessionBySockID.end())
    {
        pSession = it->second;
    }
    m_MapIOPushSessionBySockIDMutex.Unlock();
    return pSession;
}

void CSessionManager::ClearPushSession()
{
    _ClearPushSessionForMap();
}

void CSessionManager::StopAllPushSession()
{
    push_session_ptr pSession;
    hash_map<uint32_t, push_session_ptr>::iterator it, it_old;
    for (it = m_MapPushSessionBySockID.begin(); it != m_MapPushSessionBySockID.end();)
    {
        it_old = it;
        it++;
        pSession = it_old->second;
        pSession->Stop();
    }
}

void CSessionManager::_ClearPushSessionForMap()
{
    m_MapPushSessionBySockID.clear();
}

void CSessionManager::RemoveAPNSClient()
{
    if (m_pAPNSClient)
    {
        m_pAPNSClient->Stop();
        m_pAPNSClient = nullptr;
    }
}

void CSessionManager::RemovePushServer()
{
    if (m_pPushServer)
    {
        m_pPushServer->Stop();
        m_pPushServer = nullptr;
    }
}

void CSessionManager::StartCheckPushSession()
{
    m_checktimer.StartTimer(TIMER_INDEX_CHECK_PUSHSESSION, CSessionManager::TimerProc,
                            TIME_CHECK_PUSHSESSION, this);
}

void CSessionManager::StopCheckPushSession()
{
    m_checktimer.StopTimer();
}

void CSessionManager::CheckPushSessionTimeOut()
{
    m_MapIOPushSessionBySockIDMutex.Lock();
    hash_map<uint32_t /* sockid */, push_session_ptr> tmp = m_MapPushSessionBySockID;
    m_MapIOPushSessionBySockIDMutex.Unlock();

    hash_map<uint32_t /* sockid */, push_session_ptr>::iterator it = tmp.begin();
    uint64_t cur_time = S_GetTickCount();
    for (; it != tmp.end(); it++)
    {
        push_session_ptr pSession = it->second;
        uint64_t last_time = pSession->GetLastHeartBeat();
        if (cur_time - last_time >= TIMEOUT_PUSHSESSION)
        {
            PUSH_SERVER_WARN("push session time out, remote ip: %s, port: %d.", pSession->GetRemoteIP(),
                             pSession->GetRemotePort());
            pSession->Stop();
        }
    }
    tmp.clear();
}
