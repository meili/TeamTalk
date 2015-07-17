//
//  apns_feedback_handler.cpp
//  my_push_server
//
//  Created by luoning on 14-11-17.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "apns_feedback_handler.h"
#include "push_define.h"
#include "session_manager.h"

void CAPNSFeedBackHandler::OnClose(uint32_t nsockid)
{
    PUSH_SERVER_WARN("apns feedback client closed, sockid: %u", nsockid);
    m_Msg.Clear();
    apns_client_ptr pClient = CSessionManager::GetInstance()->GetAPNSClient();
    if (pClient)
    {
        //暂时不进行重连操作
        //pClient->StartReConnectFeedback();
    }
}

void CAPNSFeedBackHandler::OnException(uint32_t nsockid, int32_t nErrorCode)
{
    PUSH_SERVER_WARN("apns feedback client has exception, sockid: %u, error code: %u.", nsockid, nErrorCode);
    apns_client_ptr pClient = CSessionManager::GetInstance()->GetAPNSClient();
    if (pClient)
    {
        pClient->StopFeedBackClient();
    }
}

void CAPNSFeedBackHandler::OnConnect(uint32_t nsockid)
{
}

void CAPNSFeedBackHandler::OnSSLConnect(uint32_t nsockid)
{
    PUSH_SERVER_INFO("apns feedback ssl connect successed.");    
}

void CAPNSFeedBackHandler::OnRecvData(const char* szBuf, int32_t nBufSize)
{
    //TODO
    m_Msg.Append(szBuf, nBufSize);
    while (m_Msg.CheckMsgAvailable())
    {
        CAPNSFeedBackResMsg msg;
        if (msg.ParseFromArray(m_Msg.Data(), m_Msg.GetDataLength()))
        {
            uint32_t time = msg.GetTime();
            string token = msg.GetToken();
            PUSH_SERVER_INFO("apns feedback client recv resp, token: %s.", token.c_str());
        }
        else
        {
            PUSH_SERVER_ERROR("CAPNSFeedBackResMsg, msg parse failed.");
            apns_client_ptr pClient = CSessionManager::GetInstance()->GetAPNSClient();
            if (pClient)
            {
                pClient->StopFeedBackClient();
            }
        }
        m_Msg.Remove(m_Msg.GetDataLength());
    }
}