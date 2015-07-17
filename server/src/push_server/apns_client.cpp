//
//  apns_client.cpp
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "apns_client.h"
#include "push_define.h"

#define TIMER_INDEX_RECONNECT_APNS_GATEWAY          1
#define TIMER_INDEX_RECONNECT_APNS_FEEDBACK         2
#define TIMER_INDEX_CHECK_CONNECT_APNS_GATEWAY      3
#define TIMER_INDEX_CHECK_CONNECT_APNS_FEEDBACK     4

#define TIME_RECONNECT_APNS_GATEWAY                 5 * 1000
#define TIME_RECONNECT_APNS_FEEDBACK                3 * 1000

#define TIME_CHECK_CONNECT_APNS_GATEWAY             30 * 1000
#define TIME_CHECK_CONNECT_APNS_FEEDBACK            30 * 1000

CAPNSClient::CAPNSClient(CEpollIOLoop& io) : m_io(io)
{
    m_pGatewayClient = new CSSLClientAsync(&m_io);
    m_pFeedbackClient = new CSSLClientAsync(&m_io);
    m_bSandBox = TRUE;
}

CAPNSClient::~CAPNSClient()
{
    Stop();
    if (m_pGatewayClient != NULL)
    {
        delete m_pGatewayClient;
        m_pGatewayClient = NULL;
    }
    
    if (m_pFeedbackClient != NULL)
    {
        delete m_pFeedbackClient;
        m_pFeedbackClient = NULL;
    }
}

void CAPNSClient::TimerProc(int nIndex, void *param)
{
    CAPNSClient* pClient = (CAPNSClient*)param;
    switch (nIndex){
        case TIMER_INDEX_RECONNECT_APNS_GATEWAY:
            pClient->ReConnectGateway();
            break;
        case TIMER_INDEX_RECONNECT_APNS_FEEDBACK:
            pClient->ReConnectFeedback();
            break;
        case TIMER_INDEX_CHECK_CONNECT_APNS_GATEWAY:
            pClient->CheckConnectGateway();
            break;
        case TIMER_INDEX_CHECK_CONNECT_APNS_FEEDBACK:
            pClient->CheckConnectFeedback();
            break;
        default:
            break;
    }
}

BOOL CAPNSClient::Start()
{
    BOOL bRet = FALSE;
    bRet = ConnectGateway();
    if (bRet == FALSE)
    {
        return bRet;
    }
    bRet = ConnectFeedback();
    return bRet;
}

BOOL CAPNSClient::Stop()
{
    StopFeedBackClient();
    StopGateWayClient();
    StopReConnectFeedback();
    StopReConnectGateway();
    StopCheckConnectGateway();
    StopCheckConnectFeedback();
    return TRUE;
}

BOOL CAPNSClient::StopFeedBackClient()
{
    if (m_pFeedbackClient)
    {
        m_pFeedbackClient->ShutDown();
    }
    return TRUE;
}

BOOL CAPNSClient::StopGateWayClient()
{
    if (m_pGatewayClient)
    {
        m_pGatewayClient->ShutDown();
    }
    return TRUE;
}

BOOL CAPNSClient::_GetAPNSServerAddress()
{
    BOOL bRet = FALSE;
    char szIP[64] = {0};
    uint32_t gateway_index = 0;
    uint32_t feedback_index = 0;
    if (GetSandBox())
    {
        gateway_index = 0;
        feedback_index = 1;
    }
    else
    {
        gateway_index = 2;
        feedback_index = 3;
    }
    
    //gateway host
    if (S_GetHostByName(s_apn_servers[gateway_index].host, szIP) == 0)
    {
        m_strGatewayIP = szIP;
        m_nGatewayPort = s_apn_servers[gateway_index].port;
    }
    else
    {
        PUSH_SERVER_WARN("parse gateway host failed, %s.", s_apn_servers[gateway_index].host);
        return bRet;
    }
    
    //feedback host
    if (S_GetHostByName(s_apn_servers[feedback_index].host, szIP) == 0)
    {
        m_strFeedbackIP = szIP;
        m_nFeedbackPort = s_apn_servers[feedback_index].port;
    }
    else
    {
        PUSH_SERVER_WARN("parse feedback host failed, %s.", s_apn_servers[feedback_index].host);
        return bRet;
    }
    bRet = TRUE;
    return bRet;
}

BOOL CAPNSClient::ConnectGateway()
{
    BOOL bRet = FALSE;
    m_pGatewayClient->DoConnect.connect(&m_gatewayhandler, &CAPNSClientHandler::OnConnect);
    m_pGatewayClient->DoClose.connect(&m_gatewayhandler, &CAPNSClientHandler::OnClose);
    m_pGatewayClient->DoException.connect(&m_gatewayhandler, &CAPNSClientHandler::OnException);
    m_pGatewayClient->DoRecv.connect((CBaseHandler*)&m_gatewayhandler, &CAPNSClientHandler::OnRecv);
    m_pGatewayClient->DoSSLConnect.connect(&m_gatewayhandler, &CAPNSClientHandler::OnSSLConnect);
    
    if (m_pGatewayClient->InitSSL(GetCertPath().c_str(), GetKeyPath().c_str(), GetKeyPassword().c_str()) == FALSE)
    {
        PUSH_SERVER_ERROR("gateway client init ssl failed.");
        return bRet;
    }
    
    if (_GetAPNSServerAddress())
    {
        m_pGatewayClient->ConnectAsync(GetGatewayIP().c_str(), GetGatewayPort());
        bRet = TRUE;
    }
    return bRet;
}

BOOL CAPNSClient::ConnectFeedback()
{
    BOOL bRet = FALSE;
    m_pFeedbackClient->DoConnect.connect(&m_feedbackhandler, &CAPNSFeedBackHandler::OnConnect);
    m_pFeedbackClient->DoClose.connect(&m_feedbackhandler, &CAPNSFeedBackHandler::OnClose);
    m_pFeedbackClient->DoException.connect(&m_feedbackhandler, &CAPNSFeedBackHandler::OnException);
    m_pFeedbackClient->DoRecv.connect((CBaseHandler*)&m_feedbackhandler, &CAPNSFeedBackHandler::OnRecv);
    m_pFeedbackClient->DoSSLConnect.connect(&m_feedbackhandler, &CAPNSFeedBackHandler::OnSSLConnect);
    
    if (m_pFeedbackClient->InitSSL(GetCertPath().c_str(), GetKeyPath().c_str(), GetKeyPassword().c_str()) == FALSE)
    {
        PUSH_SERVER_ERROR("feedback client init ssl failed.");
        return bRet;
    }
    
    if (_GetAPNSServerAddress())
    {
        m_pFeedbackClient->ConnectAsync(GetFeedbackIP().c_str(), GetFeedbackPort());
        bRet = TRUE;
    }
    return bRet;
}

BOOL CAPNSClient::ReConnectGateway()
{
    BOOL bRet = FALSE;
    StopGateWayClient();
    if (m_pGatewayClient)
    {
        _GetAPNSServerAddress();
        PUSH_SERVER_INFO("gateway client begin to reconnect...");
        m_pGatewayClient->SetRemoteIP(GetGatewayIP().c_str());
        m_pGatewayClient->SetRemotePort(GetGatewayPort());
        m_pFeedbackClient->SetRemoteIP(GetFeedbackIP().c_str());
        m_pFeedbackClient->SetRemotePort(GetFeedbackPort());
        m_pGatewayClient->ReConnectAsync();
        StopReConnectGateway();
        StartCheckConnectGateway();
        bRet = TRUE;
    }
    return bRet;
}

BOOL CAPNSClient::ReConnectFeedback()
{
    BOOL bRet = FALSE;
    StopFeedBackClient();
    if (m_pFeedbackClient)
    {
        PUSH_SERVER_INFO("feedback client begin to reconnect...");
        m_pFeedbackClient->ReConnectAsync();
        StopReConnectFeedback();
        StartCheckConnectFeedback();
    }
    return bRet;
}

void CAPNSClient::CheckConnectGateway()
{
    StopCheckConnectGateway();
    //在规定得时间内没有连上，则可能是openssl连接又出现问题，强制关闭
    if (m_pGatewayClient->GetSSLConnectStatus() == FALSE) {
        PUSH_SERVER_ERROR("gateway client ssl connect timeout.");
        m_pGatewayClient->Close();
    }
}

void CAPNSClient::CheckConnectFeedback()
{
    StopCheckConnectFeedback();
    //在规定得时间内没有连上，则可能是openssl连接又出现问题，强制关闭
    if (m_pFeedbackClient->GetSSLConnectStatus() == FALSE) {
        PUSH_SERVER_ERROR("feedback client ssl connect timeout.");
        m_pFeedbackClient->Close();
    }
}

void CAPNSClient::StartReConnectGateway()
{
    m_timer.StartTimer(TIMER_INDEX_RECONNECT_APNS_GATEWAY, CAPNSClient::TimerProc,TIME_RECONNECT_APNS_GATEWAY, this);
}

void CAPNSClient::StopReConnectGateway()
{
    m_timer.KillTimer(TIMER_INDEX_RECONNECT_APNS_GATEWAY);
}

void CAPNSClient::StartCheckConnectGateway()
{
    m_timer.StartTimer(TIMER_INDEX_CHECK_CONNECT_APNS_GATEWAY, CAPNSClient::TimerProc,TIME_CHECK_CONNECT_APNS_GATEWAY, this);
}

void CAPNSClient::StopCheckConnectGateway()
{
    m_timer.KillTimer(TIMER_INDEX_CHECK_CONNECT_APNS_GATEWAY);
}

void CAPNSClient::StartReConnectFeedback()
{
    m_timer.StartTimer(TIMER_INDEX_RECONNECT_APNS_FEEDBACK, CAPNSClient::TimerProc,TIME_RECONNECT_APNS_FEEDBACK, this);
}

void CAPNSClient::StopReConnectFeedback()
{
    m_timer.KillTimer(TIMER_INDEX_RECONNECT_APNS_FEEDBACK);
}

void CAPNSClient::StartCheckConnectFeedback()
{
    m_timer.StartTimer(TIMER_INDEX_CHECK_CONNECT_APNS_FEEDBACK, CAPNSClient::TimerProc,TIME_CHECK_CONNECT_APNS_FEEDBACK, this);
}

void CAPNSClient::StopCheckConnectFeedback()
{
    m_timer.KillTimer(TIMER_INDEX_CHECK_CONNECT_APNS_FEEDBACK);
}

BOOL CAPNSClient::SendPushMsgToGateway(const char* szBuf, uint32_t nBufLen)
{
    if (m_pGatewayClient && m_pGatewayClient->GetSSLConnectStatus())
    {
        m_pGatewayClient->SendMsgAsync(szBuf, nBufLen);
    }
    else
    {
        PUSH_SERVER_ERROR("gateway client ssl connection error, discard the msg.");
    }
    return TRUE;
}