//
//  push_session.cpp
//  my_push_server
//
//  Created by luoning on 14-11-6.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#include "push_session.h"
#include "timer/Timer.hpp"
CPushSession::CPushSession(CEpollIOLoop& io, S_SOCKET sock) : m_io(io)
{
    m_pSession = new CTCPSessionAsync(&m_io, sock);
    m_nLastHeartBeat = 0;
}

CPushSession::~CPushSession()
{
    if (m_pSession)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

BOOL CPushSession::Start()
{
    m_pSession->DoClose.connect(&m_handler, &CPushSessionHandler::OnClose);
    m_pSession->DoException.connect(&m_handler, &CPushSessionHandler::OnException);
    m_pSession->DoRecv.connect((CBaseHandler*)&m_handler, &CPushSessionHandler::OnRecv);
    SetHeartBeat(S_GetTickCount());
    return TRUE;
}

BOOL CPushSession::Stop()
{
    m_pSession->Close();
    return TRUE;
}

BOOL CPushSession::SendMsg(const char *szMsg, uint32_t nMsgSize)
{
    if (m_pSession)
    {
        m_pSession->SendMsgAsync(szMsg, nMsgSize);
    }
    return TRUE;
}