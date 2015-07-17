/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�SessionModel.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��16��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __SESSIONMODEL_H__
#define __SESSIONMODEL_H__

#include "ImPduBase.h"
#include "IM.BaseDefine.pb.h"

class CSessionModel
{
public:
    static CSessionModel* getInstance();
    ~CSessionModel() {}

    void getRecentSession(uint32_t userId, uint32_t lastTime, list<IM::BaseDefine::ContactSessionInfo>& lsContact);
    uint32_t getSessionId(uint32_t nUserId, uint32_t nPeerId, uint32_t nType, bool isAll);
    bool updateSession(uint32_t nSessionId, uint32_t nUpdateTime);
    bool removeSession(uint32_t nSessionId);
    uint32_t addSession(uint32_t nUserId, uint32_t nPeerId, uint32_t nType);
    
private:
    CSessionModel() {};
    void fillSessionMsg(uint32_t nUserId, list<IM::BaseDefine::ContactSessionInfo>& lsContact);
private:
    static CSessionModel* m_pInstance;
};

#endif /*defined(__SESSIONMODEL_H__) */
