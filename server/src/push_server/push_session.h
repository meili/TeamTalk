//
//  push_session.h
//  my_push_server
//
//  Created by luoning on 14-11-6.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__push_session__
#define __my_push_server__push_session__

#include <stdio.h>
#include "socket/epoll_io_loop.h"
#include "socket/tcp_session_async.h"
#include "push_session_handler.h"
#include <memory>
class CPushSession : public std::enable_shared_from_this<CPushSession>
{
public:
    CPushSession(CEpollIOLoop& io, S_SOCKET sock);
    virtual ~CPushSession();
    
    uint32_t GetSocketID() { return m_pSession->GetSocketID(); }
    const char* GetRemoteIP() { return m_pSession->GetRemoteIP(); }
    int32_t GetRemotePort() { return m_pSession->GetRemotePort(); }
    
    BOOL Start();
    BOOL Stop();
    BOOL SendMsg(const char* szMsg, uint32_t nMsgSize);
    
    void SetHeartBeat(uint64_t nHeartBeat) { m_nLastHeartBeat = nHeartBeat; }
    uint64_t GetLastHeartBeat() { return m_nLastHeartBeat; }
private:
    uint64_t m_nLastHeartBeat;
    CTCPSessionAsync* m_pSession;
    CEpollIOLoop& m_io;
    CPushSessionHandler m_handler;
};

typedef std::shared_ptr<CPushSession> push_session_ptr;

#endif /* defined(__my_push_server__push_session__) */
