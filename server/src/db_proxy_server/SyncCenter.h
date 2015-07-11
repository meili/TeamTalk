/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：CacheManager.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月02日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <list>
#include <map>

#include "ostype.h"
#include "Lock.h"
#include "Condition.h"
#include "ImPduBase.h"
#include "public_define.h"
#include "IM.BaseDefine.pb.h"

class CSyncCenter
{
public:
    static CSyncCenter* getInstance();
    uint32_t getLastUpdateGroup() { return m_nLastUpdateGroup; } 
    string getDeptName(uint32_t nDeptId);
    void startSync();
    void stopSync();
    void init();
private:
    void updateLastUpdateGroup(uint32_t nUpdated);
    
    CSyncCenter();
    ~CSyncCenter();
    static void* doSyncGroupChat(void* arg);
    
private:
    static CSyncCenter* m_pInstance;
    uint32_t    m_nLastUpdateGroup;
    
    CCondition* m_pCondGroupChat;
    CLock*      m_pLockGroupChat;
    static bool        m_bSyncGroupChatRuning;
    bool m_bSyncGroupChatWaitting;
#ifdef _WIN32
    DWORD		m_nGroupChatThreadId;
#else
    pthread_t	m_nGroupChatThreadId;
#endif

};


#endif /*defined(__CACHEMANAGER_H__) */
