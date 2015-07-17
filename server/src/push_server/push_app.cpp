//
//  push_app.cpp
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "push_app.h"
#include "push_define.h"
#include "ConfigFileReader.h"
#include "session_manager.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "slog_api.h"

CSLog g_pushlog = CSLog(LOG_MODULE_PUSH);

CPushApp::CPushApp()
{
    m_bInit = FALSE;
}

CPushApp::~CPushApp()
{}

CPushApp* CPushApp::GetInstance()
{
    static CPushApp app;
    return &app;
}

BOOL CPushApp::Init()
{
    if (!m_bInit) {
        /* SSL 库初始化 */
        SSL_library_init();
        /* 载入所有 SSL 算法 */
        //OpenSSL_add_all_algorithms();
        /* 载入所有 SSL 错误消息 */
        SSL_load_error_strings();
        
        m_bInit = TRUE;
        PUSH_SERVER_DEBUG("push app init successed.");
    }
    else
    {
        PUSH_SERVER_WARN("warning: push app has inited.");
    }
    
    return TRUE;
}

BOOL CPushApp::UnInit()
{
    Stop();
    if (m_bInit)
    {
        m_bInit = FALSE;
        PUSH_SERVER_DEBUG("push app uninit successed.");
    }
    else
    {
        PUSH_SERVER_WARN("warning: push app has uninited.");
    }
    return TRUE;
}

BOOL CPushApp::Start()
{
    if (m_bInit)
    {
        string file_name = "pushserver.conf";
        CConfigFileReader config_file(file_name.c_str());
        char* listen_ip = config_file.GetConfigName("ListenIP");
        char* str_listen_port = config_file.GetConfigName("ListenPort");
        char* cert_path = config_file.GetConfigName("CertPath");
        char* key_path = config_file.GetConfigName("KeyPath");
        char* key_password = config_file.GetConfigName("KeyPassword");
        char* sand_box = config_file.GetConfigName("SandBox");
        if (!listen_ip || !str_listen_port || !cert_path || !key_path || !sand_box || !key_password)
        {
            PUSH_SERVER_ERROR("push app config file: %s not exist or miss required parameter obtained.", file_name.c_str());
            return FALSE;
        }
        uint32_t nsand_box = atoi(sand_box);
        if (nsand_box != 1 && nsand_box != 0)
        {
            PUSH_SERVER_ERROR("push app config parameter: sand_box has invaid value: %u.", nsand_box)
            return FALSE;
        }
        apns_client_ptr pAPNSClient(new CAPNSClient(m_io));
        pAPNSClient->SetCertPath(cert_path);
        pAPNSClient->SetKeyPath(key_path);
        pAPNSClient->SetKeyPassword(key_password);
        pAPNSClient->SetSandBox((BOOL)atoi(sand_box));
        CSessionManager::GetInstance()->SetAPNSClient(pAPNSClient);
        
        push_server_ptr pPushServer(new CPushServer(m_io));
        pPushServer->SetListenIP(listen_ip);
        pPushServer->SetPort(atoi(str_listen_port));
        CSessionManager::GetInstance()->SetPushServer(pPushServer);
        
        m_io.Start();
        CSessionManager::GetInstance()->StartCheckPushSession();
        if (pAPNSClient)
        {
            if (pAPNSClient->Start() == FALSE)
            {
                return FALSE;
            }
        }
        if (pPushServer)
        {
            if (pPushServer->Start() == FALSE)
            {
                return FALSE;
            }
        }
        PUSH_SERVER_DEBUG("push app start successed.");
    }
    else
    {
        PUSH_SERVER_WARN("push app not init before.");
    }
    
    return TRUE;
}

BOOL CPushApp::Stop()
{
    if (m_bInit)
    {
        m_io.Stop();
        CSessionManager::GetInstance()->StopCheckPushSession();
        apns_client_ptr pAPNSClient = CSessionManager::GetInstance()->GetAPNSClient();
        if (pAPNSClient)
        {
            pAPNSClient->Stop();
        }
        push_server_ptr pPushServer = CSessionManager::GetInstance()->GetPushServer();
        if (pPushServer)
        {
            pPushServer->Stop();
        }
        CSessionManager::GetInstance()->StopAllPushSession();
        
        CSessionManager::GetInstance()->RemoveAPNSClient();
        CSessionManager::GetInstance()->RemovePushServer();
        CSessionManager::GetInstance()->ClearPushSession();
        PUSH_SERVER_DEBUG("push app stop successed.");
    }
    else
    {
        PUSH_SERVER_WARN("push app not init before.");
    }
    return TRUE;
}