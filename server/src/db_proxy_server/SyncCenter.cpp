/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：CacheManager.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月02日
*   描    述：
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
 *  单例
 *
 *  @return 返回CSyncCenter的单例指针
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
 *  构造函数
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
 *  析构函数
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

void CSyncCenter::getDept(uint32_t nDeptId, DBDeptInfo_t** pDept)
{
    auto it = m_pDeptInfo->find(nDeptId);
    if (it != m_pDeptInfo->end()) {
        *pDept = it->second;
    }
}

string CSyncCenter::getDeptName(uint32_t nDeptId)
{
    CAutoRWLock autoLock(g_pRWDeptLock);
    string strDeptName;
    DBDeptInfo_t* pDept = NULL;;
    getDept(nDeptId, &pDept);
    if (pDept != NULL) {
        strDeptName =  pDept->strName;
    }
    return strDeptName;
}
/**
 *  开启内网数据同步以及群组聊天记录同步
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
 *  停止同步，为了"优雅"的同步，使用了条件变量
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
 * 初始化函数，从cache里面加载上次同步的时间信息等
 */
void CSyncCenter::init()
{
    // Load total update time
    CacheManager* pCacheManager = CacheManager::getInstance();
    // increase message count
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn)
    {
        string strTotalUpdate = pCacheConn->get("total_user_updated");

        string strLastUpdateGroup = pCacheConn->get("last_update_group");
        pCacheManager->RelCacheConn(pCacheConn);
	if(strTotalUpdate != "")
        {
            m_nLastUpdate = string2int(strTotalUpdate);
        }
        else
        {
            updateTotalUpdate(time(NULL));
        }
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
 *  更新上次同步内网信息时间
 *
 *  @param nUpdated 时间
 */

void CSyncCenter::updateTotalUpdate(uint32_t nUpdated)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn) {
        last_update_lock_.lock();
        m_nLastUpdate = nUpdated;
        last_update_lock_.unlock();
        string strUpdated = int2string(nUpdated);
        pCacheConn->set("total_user_update", strUpdated);
        pCacheManager->RelCacheConn(pCacheConn);
    }
    else
    {
        log("no cache connection to get total_user_updated");
    }
}

/**
 *  更新上次同步群组信息时间
 *
 *  @param nUpdated 时间
 */
void CSyncCenter::updateLastUpdateGroup(uint32_t nUpdated)
{
    CacheManager* pCacheManager = CacheManager::getInstance();
    CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
    if (pCacheConn) {
        last_update_lock_.lock();
        m_nLastUpdateGroup = nUpdated;
        string strUpdated = int2string(nUpdated);
        last_update_lock_.unlock();
  
        pCacheConn->set("last_update_group", strUpdated);
        pCacheManager->RelCacheConn(pCacheConn);
    }
    else
    {
        log("no cache connection to get total_user_updated");
    }
}

/**
 *  同步群组聊天信息
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
