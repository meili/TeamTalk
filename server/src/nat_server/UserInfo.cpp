//
//  UserInfo.cpp
//  im-server-TT
//
//  Created by luoning on 14-10-23.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "UserInfo.h"
#include "ImPduBase.h"
#include "public_define.h"
#include "IM.BaseDefine.pb.h"
using namespace IM::BaseDefine;
CUserInfo::CUserInfo()
{}

CUserInfo::~CUserInfo()
{}

void CUserInfo::AddClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.find(client_type);
    if (it != m_ClientTypeList.end())
    {
        it->second += 1;
    }
    else
    {
        m_ClientTypeList[client_type] = 1;
    }
}

void CUserInfo::RemoveClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.find(client_type);
    if (it != m_ClientTypeList.end())
    {
        uint32_t count = it->second;
        count -= 1;
        if (count > 0)
        {
            it->second = count;
        }
        else
        {
            m_ClientTypeList.erase(client_type);
        }
    }
}

bool CUserInfo::FindRouteConn(CRouteConn *pConn)
{
    set<CRouteConn*>::iterator it = m_RouteConnSet.find(pConn);
    if (it != m_RouteConnSet.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t CUserInfo::GetCountByClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.find(client_type);
    if (it != m_ClientTypeList.end()) {
        return it->second;
    }
    else
    {
        return 0;
    }
}

bool CUserInfo::IsMsgConnNULL()
{
    if (m_ClientTypeList.size() == 0) {
        return true;
    }
    else
    {
        return false;
    }
}

void CUserInfo::ClearClientType()
{
    m_ClientTypeList.clear();
}

bool CUserInfo::IsPCClientLogin()
{
    bool bRet = false;
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.begin();
    for (; it != m_ClientTypeList.end(); it++)
    {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_PC(client_type))
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

bool CUserInfo::IsMobileClientLogin()
{
    bool bRet = false;
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.begin();
    for (; it != m_ClientTypeList.end(); it++)
    {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_MOBILE(client_type))
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

uint32_t CUserInfo::GetStatus()
{
    uint32_t status = USER_STATUS_OFFLINE;
    map<uint32_t, uint32_t>::iterator it = m_ClientTypeList.begin();
    for (; it != m_ClientTypeList.end(); it++)
    {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_PC(client_type))
        {
            status = USER_STATUS_ONLINE;
            break;
        }
    }
    return status;
}