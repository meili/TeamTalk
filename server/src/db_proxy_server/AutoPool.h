/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：AutoPool.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月18日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __AUTOPOOl_H__
#define __AUTOPOOl_H__

class CDBConn;
class CacheConn;

class CAutoDB
{
public:
    CAutoDB(const char* pDBName, CDBConn** pDBConn);
    ~CAutoDB();
private:
    CDBConn* m_pDBConn;
};

class CAutoCache
{
    CAutoCache(const char* pCacheName, CacheConn** pCacheConn);
    ~CAutoCache();
private:
    CacheConn* m_pCacheConn;
};
#endif /*defined(__AUTOPOOl_H__) */
