//
//  ssl_client_async.h
//  my_push_server
//
//  Created by luoning on 14-11-14.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#ifndef __my_push_server__ssl_client_async__
#define __my_push_server__ssl_client_async__

#include <stdio.h>
#include "tcp_client_async.h"
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <string>

class CSSLClientAsync : public CTCPClientAsync
{
public:
    CSSLClientAsync(CIOLoop* pIO);
    virtual ~CSSLClientAsync();
    
    SSL_CTX* GetSSLCTX() const { return m_ctx; }
    SSL* GetSSL() const { return m_ssl; }
    
    void SetSSLConnectStatus(BOOL bStatus) { m_bSSLConnectStatus = bStatus; }
    BOOL GetSSLConnectStatus() { return m_bSSLConnectStatus; }
    string GetCertFile() { return m_strCertFile; }
    string GetKeyFile() { return m_strKeyFile; }
    string GetKeyPassword() { return m_strKeyPassword; }
    
    BOOL InitSSL(const char* cert_file = NULL, const char* key_file = NULL, const char* key_password = NULL);
    void UnInitSSL();
    virtual void OnConnect(BOOL bConnected);
    virtual void OnRecv();
    int32_t SSLConnect();
    virtual int32_t ReConnectAsync();

    virtual int32_t SendMsgAsync(const char* szBuf, int32_t nBufSize);
    virtual int32_t SendBufferAsync();

    virtual void Close();
    virtual void ShutDown();

    /*uint32_t nsockid*/
    sigslot::signal1<uint32_t> DoSSLConnect;
    
protected:
    virtual void _Close();
    
protected:
    BOOL m_bSSLConnectStatus;
    
    string m_strCertFile;
    string m_strKeyFile;
    string m_strKeyPassword;
    
    SSL_CTX* m_ctx;
    SSL* m_ssl;
};

#endif /* defined(__my_push_server__ssl_client_async__) */
