/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�AutoPool.cpp
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��18��
*   ��    ����
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