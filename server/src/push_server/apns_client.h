//
//  apns_client.h
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__apns_client__
#define __my_push_server__apns_client__

#include <stdio.h>
#include "socket/ssl_client_async.h"
#include "socket/epoll_io_loop.h"
#include <string>
#include "apns_client_handler.h"
#include "apns_feedback_handler.h"
#include "timer/Timer.hpp"
#include <memory>
using namespace std;

typedef struct tag_apn_server {
    const char *host;
    int port;
}ST_APNS, *LPST_APNS;
static ST_APNS s_apn_servers[4] =
{
    {"gateway.sandbox.push.apple.com", 2195},
    {"feedback.sandbox.push.apple.com", 2196},
    {"gateway.push.apple.com", 2195},
    {"feedback.push.apple.com", 2196}
};


class CAPNSClient : public std::enable_shared_from_this<CAPNSClient>
{
public:
    CAPNSClient(CEpollIOLoop& io);
    virtual ~CAPNSClient();
    
    static void TimerProc(int nIndex, void* param);
    
    void SetCertPath(const char* cert_path) { m_strCertPath = cert_path; }
    string GetCertPath() { return m_strCertPath; }
    void SetKeyPath(const char* key_path) { m_strKeyPath = key_path; }
    string GetKeyPath() { return m_strKeyPath; }
    void SetKeyPassword(const char* key_password) { m_strKeyPassword = key_password; }
    string GetKeyPassword() { return m_strKeyPassword; }
    void SetSandBox(BOOL bSandBox) { m_bSandBox = bSandBox; }
    BOOL GetSandBox() { return m_bSandBox; }
    string GetGatewayIP() { return m_strGatewayIP; }
    uint32_t GetGatewayPort() { return m_nGatewayPort; }
    string GetFeedbackIP() { return m_strFeedbackIP; }
    uint32_t GetFeedbackPort() { return m_nFeedbackPort; }
    
    BOOL Start();
    BOOL Stop();
    BOOL StopGateWayClient();
    BOOL StopFeedBackClient();
    
    BOOL ConnectGateway();
    BOOL ConnectFeedback();
    BOOL ReConnectGateway();
    BOOL ReConnectFeedback();
    
    void CheckConnectGateway();
    void CheckConnectFeedback();
    
    void StartReConnectGateway();
    void StopReConnectGateway();
    void StartCheckConnectGateway();
    void StopCheckConnectGateway();
    
    void StartReConnectFeedback();
    void StopReConnectFeedback();
    void StartCheckConnectFeedback();
    void StopCheckConnectFeedback();
    
    BOOL SendPushMsgToGateway(const char* szBuf, uint32_t nBufLen);
private:
    BOOL _GetAPNSServerAddress();
private:
    string m_strCertPath;
    string m_strKeyPath;
    string m_strKeyPassword;
    BOOL   m_bSandBox;
    string m_strGatewayIP;
    uint32_t m_nGatewayPort;
    string m_strFeedbackIP;
    uint32_t m_nFeedbackPort;
    
    CSSLClientAsync* m_pGatewayClient;
    CSSLClientAsync* m_pFeedbackClient;
    CEpollIOLoop& m_io;
    CAPNSClientHandler m_gatewayhandler;
    CAPNSFeedBackHandler m_feedbackhandler;
    
    CTimer m_timer;
};

typedef std::shared_ptr<CAPNSClient> apns_client_ptr;

#endif /* defined(__my_push_server__apns_client__) */
