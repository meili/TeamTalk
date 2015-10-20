/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Client.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <iostream>
#include "ostype.h"
#include "IM.BaseDefine.pb.h"
#include "IPacketCallback.h"
using namespace std;

typedef hash_map<uint32_t, IM::BaseDefine::UserInfo*> CMapId2User_t;
typedef hash_map<string, IM::BaseDefine::UserInfo*> CMapNick2User_t;
class CClient:public IPacketCallback
{
public:
    CClient(const string& strName, const string& strPass, const string strDomain="access.tt.mogujie.org");
    ~CClient();
public:
    string getName(){ return m_strName; }
    string getDomain() { return m_strLoginDomain; }
    static void TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);
    CMapNick2User_t& getNick2UserMap() { return m_mapNick2UserInfo; }
    
public:
    void connect();
    void close();
    uint32_t login(const string& strName, const string& strPass);
    uint32_t getChangedUser();
    uint32_t getUserInfo(list<uint32_t>& lsUserId);
    uint32_t sendMsg(uint32_t nToId,IM::BaseDefine::MsgType nType, const string& strMsg);
    uint32_t getUnreadMsgCnt();
    uint32_t getRecentSession();
    uint32_t getMsgList(IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt);
    uint32_t sendReadAck();
	uint32_t getDepartMentInfo();
	uint32_t getGroupList();
	uint32_t getGroupInfo(const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo);
public:
    virtual void onError(uint32_t nSeqNo, uint32_t nCmd, const string& strMsg);
    virtual void onConnect();
    virtual void onClose();
    virtual void onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser = NULL);
    virtual void onGetChangedUser(uint32_t nSeqNo,const list<IM::BaseDefine::UserInfo>& lsUser);
    virtual void onGetUserInfo(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo>& lsUser);
    virtual void onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId);
    virtual void onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt);
    virtual void onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo>& lsSession);
    virtual void onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo>& lsMsg);
    virtual void onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string& strMsgData);
	virtual void onGetDepartMentInfo(uint32_t nSeqNo , uint32_t nUserId , uint32_t latest_update_time , const list<IM::BaseDefine::DepartInfo> &lsDepartInfo);
	virtual void onGroupList(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo);
	virtual void onGroupInfo(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupInfo> &lsGroupInfo);
private:
    string          m_strName;
    string          m_strPass;
    string          m_strLoginDomain;
    uint32_t        m_nLastGetUser;
	uint32_t		m_nLastGetDepartMent;
    uint32_t        m_nLastGetSession;
    net_handle_t    m_nHandle;
    IM::BaseDefine::UserInfo    m_cSelfInfo;
    
    
    
    CMapId2User_t m_mapId2UserInfo;
    CMapNick2User_t m_mapNick2UserInfo;
};


#endif /*defined(__CLIENT_H__) */
