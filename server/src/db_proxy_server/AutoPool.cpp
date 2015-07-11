/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：AutoPool.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月18日
*   描    述：
*
================================================================*/
#include "AutoPool.h"
#include "DBPool.h"
#include "CachePool.h"

CAutoDB::CAutoDB(const char* pDBName, CDBConn** pDBConn)
{
    m_pDBConn = CDBManager::getInstance()->GetDBConn(pDBName);
    *pDBConn = m_pDBConn;
}

CAutoDB::~CAutoDB()
{
    if (m_pDBConn != NULL) {
        CDBManager::getInstance()->RelDBConn(m_pDBConn);
        m_pDBConn = NULL;
    }
}

CAutoCache::CAutoCache(const char* pCacheName, CacheConn** pCacheConn)
{
    m_pCacheConn = CacheManager::getInstance()->GetCacheConn(pCacheName);
    *pCacheConn = m_pCacheConn;
}

CAutoCache::~CAutoCache()
{
    if (m_pCacheConn != NULL) {
        CacheManager::getInstance()->RelCacheConn(m_pCacheConn);
        m_pCacheConn = NULL;
    }
}