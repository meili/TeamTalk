//
//  session_manager.h
//  my_push_server
//
//  Created by luoning on 14-11-11.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__session_manager__
#define __my_push_server__session_manager__

#include <stdio.h>
#include "socket/base_io_stream.h"
#include "thread/base_thread.hpp"
#ifdef _WIN32
#include <hash_map>
#include <list>
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#include "push_session.h"
#include "apns_client.h"
#include "push_server.h"
#include "timer/Timer.hpp"

class CSessionManager
{
public:
    CSessionManager();
    virtual ~CSessionManager();
    
    static CSessionManager* GetInstance();
    static void TimerProc(int32_t nIndex, void* param);
    
    void StartCheckPushSession();
    void StopCheckPushSession();
    void CheckPushSessionTimeOut();
    void CheckPushSessionDelete();
    
    void AddPushSessionBySockID(uint32_t nsockid, push_session_ptr pSession);
    void RemovePushSessionBySockID(uint32_t nsockid);
    push_session_ptr GetPushSessionBySockID(uint32_t nsockid);
    void ClearPushSession();
    void StopAllPushSession();
    
    void SetAPNSClient(apns_client_ptr pClient) { m_pAPNSClient = pClient; }
    apns_client_ptr GetAPNSClient() { return m_pAPNSClient; }
    void RemoveAPNSClient();
    
    void SetPushServer(push_server_ptr pServer) { m_pPushServer = pServer; }
    push_server_ptr GetPushServer() { return m_pPushServer; }
    void RemovePushServer();
    
private:
    void _ClearPushSessionForMap();
    
private:
    hash_map<uint32_t /* sockid */, push_session_ptr> m_MapPushSessionBySockID;
    CBaseMutex m_MapIOPushSessionBySockIDMutex; 
    
    apns_client_ptr m_pAPNSClient;
    push_server_ptr m_pPushServer;
    
    CTimer m_checktimer;
};

#endif /* defined(__my_push_server__session_manager__) */
