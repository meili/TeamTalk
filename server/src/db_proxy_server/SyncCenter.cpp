/*================================================================
*     Copyright (c) 2014�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�CacheManager.cpp
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2014��12��02��
*   ��    ����
*
================================================================*/
#include <stdlib.h>
#include <sys/signal.h>
#include "SyncCenter.h"
#include "Lock.h"
#include "HttpClient.h"
#include "json/json.h"
#include "DBPool.h"
#include "CachePool.h"
#include "business/Common.h"
#include "business/UserModel.h"
#include "business/GroupModel.h"
#include "business/SessionModel.h"

static CLock* g_pLock = new CLock();
static CRWLock *g_pRWDeptLock = new CRWLock();

CSyncCenter* CSyncCenter::m_pInstance = NULL;
bool CSyncCenter::m_bSyncGroupChatRuning = false;
/**
 *  ����
 *
 *  @return ����CSyncCenter�ĵ���ָ��
 */
CSyncCenter* CSyncCenter::getInstance()
{
    CAutoLock autoLock(g_pLock);
    if(m_pInstance == NULL)
    {
        m_pInstance = new CSyncCenter();
    }
    return m_pInstance;
}

/**
 *  ���캯��
 */
CSyncCenter::CSyncCenter()
:m_nGroupChatThreadId(0),
m_nLastUpdateGroup(time(NULL)),
m_bSyncGroupChatWaitting(true),
m_pLockGroupChat(new CLock())
//m_pLock(new CLock())
{
    m_pCondGroupChat = new CCondition(m_pLockGroupChat);
}

/**
 *  ��������
 */
CSyncCenter::~CSyncCenter()
{
    if(m_pLockGroupChat != NULL)
    {
        delete m_pLockGroupChat;
    }
    if(m_pCondGroupChat != NULL)
    {
        delete m_pCondGroupChat;
    }
}

/**
 *  ������������ͬ���Լ�Ⱥ�������¼ͬ��
 */
void CSyncCenter::startSync()
{
#ifdef _WIN32
    (void)CreateThread(NULL, 0, doSyncGroupChat, NULL, 0, &m_nGroupChatThreadId);
#else
    (void)pthread_create(&m_nGroupChatThreadId, NULL, doSyncGroupChat, NULL);
#endif
}

/**
 *  ֹͣͬ����Ϊ��"����"��ͬ����ʹ������������
 */
void CSyncCenter::stopSync()
{
    m_bSyncGroupChatWaitting = false;
    m_pCondGroupChat->notify();
    while (m_bSyncGroupChatRuning ) {
        usleep(500);
    }
}

/*
 * ��ʼ����������cache��������ϴ�ͬ����ʱ����Ϣ��
 */
void CSyncCenter::init()
{
    // Load total update time
    CacheManager* pCacheManager = CacheManager::getInstance();
    // increase message count
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn)
    {
        string strLastUpdateGroup = pCacheConn->get("last_update_group");
        pCacheManager->RelCacheConn(pCacheConn);
        if(strLastUpdateGroup.empty())
        {
            m_nLastUpdateGroup = string2int(strLastUpdateGroup);
        }
        else
        {
            updateLastUpdateGroup(time(NULL));
        }
    }
    else
    {
        log("no cache connection to get total_user_updated");
    }
}

/**
 *  �����ϴ�ͬ��Ⱥ����Ϣʱ��
 *
 *  @param nUpdated ʱ��
 */
void CSyncCenter::updateLastUpdateGroup(uint32_t nUpdated)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn) {
        m_nLastUpdateGroup = nUpdated;
        string strUpdated = int2string(nUpdated);
        pCacheConn->set("last_update_group", strUpdated);
        pCacheManager->RelCacheConn(pCacheConn);
    }
    else
    {
        log("no cache connection to get total_user_updated");
    }
}

/**
 *  ͬ��Ⱥ��������Ϣ
 *
 *  @param arg NULL
 *
 *  @return NULL
 */
void* CSyncCenter::doSyncGroupChat(void* arg)
{
    m_bSyncGroupChatRuning = true;
    CDBManager* pDBManager = CDBManager::getInstance();
    map<uint32_t, uint32_t> mapChangedGroup;
    do{
        mapChangedGroup.clear();
        CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
        if(pDBConn)
        {
            string strSql = "select id, lastChated from IMGroup where status=0 and lastChated >=" + int2string(m_pInstance->getLastUpdateGroup());
            CResultSet* pResult = pDBConn->ExecuteQuery(strSql.c_str());
            if(pResult)
            {
                while (pResult->Next()) {
                    uint32_t nGroupId = pResult->GetInt("id");
                    uint32_t nLastChat = pResult->GetInt("lastChated");
                    if(nLastChat != 0)
                    {
                        mapChangedGroup[nGroupId] = nLastChat;
                    }
                }
                delete pResult;
            }
            pDBManager->RelDBConn(pDBConn);
        }
        else
        {
            log("no db connection for teamtalk_slave");
        }
        m_pInstance->updateLastUpdateGroup(time(NULL));
        for (auto it=mapChangedGroup.begin(); it!=mapChangedGroup.end(); ++it)
        {
            uint32_t nGroupId =it->first;
            list<uint32_t> lsUsers;
            uint32_t nUpdate = it->second;
            CGroupModel::getInstance()->getGroupUser(nGroupId, lsUsers);
            for (auto it1=lsUsers.begin(); it1!=lsUsers.end(); ++it1)
            {
                uint32_t nUserId = *it1;
                uint32_t nSessionId = INVALID_VALUE;
                nSessionId = CSessionModel::getInstance()->getSessionId(nUserId, nGroupId, IM::BaseDefine::SESSION_TYPE_GROUP, true);
                if(nSessionId != INVALID_VALUE)
                {
                    CSessionModel::getInstance()->updateSession(nSessionId, nUpdate);
                }
                else
                {
                    CSessionModel::getInstance()->addSession(nUserId, nGroupId, IM::BaseDefine::SESSION_TYPE_GROUP);
                }
            }
        }
//    } while (!m_pInstance->m_pCondSync->waitTime(5*1000));
    } while (m_pInstance->m_bSyncGroupChatWaitting && !(m_pInstance->m_pCondGroupChat->waitTime(5*1000)));
//    } while(m_pInstance->m_bSyncGroupChatWaitting);
    m_bSyncGroupChatRuning = false;
    return NULL;
}
