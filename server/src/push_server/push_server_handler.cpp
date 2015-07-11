//
//  push_server_handler.cpp
//  my_push_server
//
//  Created by luoning on 14-11-11.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#include "push_server_handler.h"
#include "push_define.h"
#include "push_session.h"
#include "push_app.h"
#include "session_manager.h"

void CPushServerHandler::OnClose(uint32_t nsockid)
{
    PUSH_SERVER_DEBUG("push server closed, sockid: %u.", nsockid);
}

void CPushServerHandler::OnAccept(uint32_t nsockid, S_SOCKET sock, const char *szIP,int32_t nPort)
{
    push_server_ptr pServer = CSessionManager::GetInstance()->GetPushServer();
    push_session_ptr pSession(new CPushSession(pServer->GetIOLoop(), sock));
    CSessionManager::GetInstance()->AddPushSessionBySockID(pSession->GetSocketID(), pSession);
    PUSH_SERVER_INFO("push server accept session, remote ip: %s, port: %u, sockid: %u, real socket: %u.", szIP, nPort, pSession->GetSocketID(), sock);
    pSession->Start();
}