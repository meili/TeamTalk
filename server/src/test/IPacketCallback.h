/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：IPacketCallback.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __IPACKETCALLBACK_H__
#define __IPACKETCALLBACK_H__
#include <list>
#include "IM.BaseDefine.pb.h"

using namespace std;

class IPacketCallback
{
public:
    IPacketCallback() {};
    virtual ~IPacketCallback() {};
    virtual void onConnect() = 0;
    virtual void onClose() = 0;
    virtual void onError(uint32_t nSeqNo, uint32_t nCmd, const string& strMsg) = 0;
    virtual void onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser = NULL) = 0;
    virtual void onGetChangedUser(uint32_t nSeqNo,const list<IM::BaseDefine::UserInfo>& lsUser) = 0;
    virtual void onGetUserInfo(uint32_t nSeqNo,const list<IM::BaseDefine::UserInfo>& lsUser) = 0;
    virtual void onSendMsg(uint32_t nSeqNo, uint32_t nUserId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId) = 0;
    virtual void onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt) = 0;
    virtual void onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo>& lsSession) = 0;
    virtual void onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo>& lsMsg) = 0;
    virtual void onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string& strMsgData) = 0;
	virtual void onGetDepartMentInfo(uint32_t nSeqNo , uint32_t nUserId , uint32_t latest_update_time , const list<IM::BaseDefine::DepartInfo> &lsDepartInfo) = 0;
	virtual void onGroupList(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo) = 0;
	virtual void onGroupInfo(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupInfo> &lsGroupInfo) = 0;
};

#endif /*defined(__IPACKETCALLBACK_H__) */
