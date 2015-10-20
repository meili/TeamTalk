/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Client.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
================================================================*/
#include "Client.h"
#include "HttpClient.h"
#include "Common.h"
#include "json/json.h"
#include "ClientConn.h"
#include "security.h"

static ClientConn*  g_pConn = NULL;

static bool         g_bLogined = false;

CClient::CClient(const string& strName, const string& strPass, const string strDomain):
m_strName(strName),
m_strPass(strPass),
m_strLoginDomain(strDomain),
m_nLastGetUser(0),
m_nLastGetDepartMent(0),
m_nLastGetSession(0)
{
}

CClient::~CClient()
{
}

void CClient::TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    if (g_bLogined) {
        uint64_t cur_time = get_tick_count();
        g_pConn->OnTimer(cur_time);
    }
}



void CClient::onError(uint32_t nSeqNo, uint32_t nCmd,const string& strMsg)
{
    g_imlog.Error("get error:%d, msg:%s", nCmd, strMsg.c_str());
}

void CClient::connect()
{
    CHttpClient httpClient;
    string strUrl = m_strLoginDomain + "/msg_server";
    string strResp;
    CURLcode nRet = httpClient.Get(strUrl, strResp);
    if(nRet != CURLE_OK)
    {
        printf("login falied. access url:%s error\n", strUrl.c_str());
        PROMPTION;
        return;
    }
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strResp, value))
    {
        printf("login falied. parse response error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }
    string strPriorIp, strBackupIp;
    string strPort;
    uint16_t nPort;
    try {
	uint32_t nRet = value["code"].asUInt();
        if(nRet != 0)
        {
            string strMsg = value["msg"].asString();
            printf("login falied. errorMsg:%s\n", strMsg.c_str());
            PROMPTION;
            return;
        }
        strPriorIp = value["priorIP"].asString();
	strBackupIp = value["backupIp"].asString();
	strPort = value["port"].asString();
	nPort = atoi(strPort.c_str());
    } catch (std::runtime_error msg) {
        printf("login falied. get json error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }
    
    g_pConn = new ClientConn();
    m_nHandle = g_pConn->connect(strPriorIp.c_str(), nPort, m_strName, m_strPass);
    g_pConn->setcallback(this);
    if(m_nHandle != INVALID_SOCKET)
    {
        netlib_register_timer(CClient::TimerCallback, NULL, 1000);
    }
    else
    {
        printf("invalid socket handle\n");
    }
}

void CClient::onConnect()
{
    login(m_strName, m_strPass);
}


void CClient::close()
{
    g_pConn->Close();
}

void CClient::onClose()
{
    
}

uint32_t CClient::login(const string& strName, const string& strPass)
{
    return g_pConn->login(strName, strPass);
}

void CClient::onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser)
{
    if(nResultCode != 0)
    {
        printf("login failed.errorCode=%u, msg=%s\n",nResultCode, strMsg.c_str());
        return;
    }
    if(pUser)
    {
		printf("%s logined.\n" , pUser->user_real_name().c_str() );
        m_cSelfInfo = *pUser;
        g_bLogined = true;
    }
    else
    {
        printf("pUser is null\n");
    }
}

uint32_t CClient::getChangedUser()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUser(nUserId, m_nLastGetUser);
}

uint32_t CClient::getDepartMentInfo()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	return g_pConn->getDepartMentInfo(nUserId , m_nLastGetDepartMent);

}

void CClient::onGetDepartMentInfo(uint32_t nSeqNo , uint32_t nUserId , uint32_t latest_update_time , const list<IM::BaseDefine::DepartInfo> &lsDepartInfo)
{
	char szBuf[256] = {0};
	m_nLastGetDepartMent = time(NULL);
	time_t timer = (time_t)latest_update_time;
	strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));

	printf("onGetDepartMentInfo time:%s , size:%d\n" , szBuf , lsDepartInfo.size());
	for(const auto &r:lsDepartInfo)
	{
		printf("dept_id:%d , priority:%d , dept_name:%s , parent_dept_id:%d , dept_status: %d\n" , 
			r.dept_id() , r.priority() ,  r.dept_name().c_str() , r.parent_dept_id() , r.dept_status());
	}
}

void CClient::onGetChangedUser(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
	
	printf("onGetChangedUser  %d:\n" , lsUser.size());
	auto begin = lsUser.begin();
	while(begin != lsUser.end()){
		printf("id:%d name:%s\n" ,begin->user_id() , begin->user_real_name().c_str());
		++begin;
	}
	
	m_nLastGetUser = time(NULL);
	
    for(auto it=lsUser.begin(); it!=lsUser.end(); ++it)
    {
        IM::BaseDefine::UserInfo* pUserInfo = new IM::BaseDefine::UserInfo();
		bool bDeteled = false;
        *pUserInfo = *it;
        uint32_t nUserId = pUserInfo->user_id();
        string strNick = pUserInfo->user_nick_name();
        if(it->status() != 3)
        {
            auto it1 = m_mapId2UserInfo.find(nUserId);
            if(it1 == m_mapId2UserInfo.end())
            {
                m_mapId2UserInfo.insert(pair<uint32_t, IM::BaseDefine::UserInfo*>(nUserId, pUserInfo));
            }
            else
            {
                delete it1->second;
                bDeteled = true;
				m_mapId2UserInfo[nUserId] = pUserInfo;
			}
            
		
            auto it2 = m_mapNick2UserInfo.find(strNick);
            if(it2 == m_mapNick2UserInfo.end())
            {
                m_mapNick2UserInfo.insert(pair<string, IM::BaseDefine::UserInfo*>(strNick, pUserInfo));
            }
            else
            {
				if(!bDeteled)
                {
					delete it2->second;
				}
                m_mapNick2UserInfo[strNick] = pUserInfo;
            }
        }
    }
}

uint32_t CClient::getUserInfo(list<uint32_t>& lsUserId)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUserInfo(nUserId, lsUserId);
}

void CClient::onGetUserInfo(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
	printf("onGetUserInfo  %d:\n" , lsUser.size());
	auto begin = lsUser.begin();
	while(begin != lsUser.end()){
		printf("id:%d name:%s\n" ,begin->user_id() , begin->user_real_name().c_str());
		++begin;
	}
}

uint32_t CClient::sendMsg(uint32_t nToId, IM::BaseDefine::MsgType nType, const string &strMsg)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->sendMessage(nFromId, nToId, nType, strMsg);
}

void CClient::onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    printf("send msg succes. seqNo:%u, msgId:%u\n", nSeqNo, nMsgId);
}


uint32_t CClient::getUnreadMsgCnt()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUnreadMsgCnt(nUserId);
}

void CClient::onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt)
{
    printf("unreadmsg total:%d  unread:%d\n" , nTotalCnt , lsUnreadCnt.size());

	auto it = lsUnreadCnt.begin();
	while(it != lsUnreadCnt.end()){
			char *desMsg=0;
			uint32_t noutlen = 0 ;
			int retCode = DecryptMsg(it->latest_msg_data().c_str() , it->latest_msg_data().length() , &desMsg,noutlen);

		printf("session_id:%d,session_type:%d,unread_cnt:%d,msg_id:%d,msg_type:%d ,from_user_id:%d,msg_data:%s",
			it->session_id() , it->session_type() , it->unread_cnt() , it->latest_msg_id()   ,
			it->latest_msg_type() , it->latest_msg_from_user_id() , desMsg);
		Free(desMsg);
	
		getMsgList(it->session_type() , it->session_id() , it->latest_msg_id() , it->unread_cnt());
		++it;
	}
}

uint32_t CClient::getGroupInfo(const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo)
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	return g_pConn->getGroupInfo(nUserId , lsGroupVersionInfo);
}

uint32_t CClient::getGroupList()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	return g_pConn->getGroupList(nUserId);
}

uint32_t CClient::getRecentSession()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getRecentSession(nUserId, m_nLastGetSession);
}

void CClient::onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo> &lsSession)
{
    printf("GetRecentSession %d:\n" , lsSession.size());

	m_nLastGetSession = time(NULL);

	 list<IM::BaseDefine::GroupVersionInfo> lsGroupVersionInfo;

	for( const auto &k:lsSession){

		char *desMsg=0;
		uint32_t noutlen = 0 ;
		int retCode = DecryptMsg(k.latest_msg_data().c_str() , k.latest_msg_data().length(), &desMsg,noutlen);

		char szBuf[256] = {0};
		time_t timer = (time_t)k.updated_time();
		strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));
		printf("session_id:%d , session_type:%d , session_status:%d , update_time:%s , msg_id:%d , msg_type:%d , msg_from_user_id:%d , msg_data:%s\n" , 
		k.session_id() , k.session_type() , k.session_status() ,szBuf , k.latest_msg_id() , k.latest_msg_type() , k.latest_msg_from_user_id(), desMsg);
		
		if(k.session_type() == IM::BaseDefine::SESSION_TYPE_GROUP)
		{
			IM::BaseDefine::GroupVersionInfo groupVerInfo;
			groupVerInfo.set_group_id(k.session_id());
			groupVerInfo.set_version(0);
			lsGroupVersionInfo.push_back(groupVerInfo);
		}
		Free(desMsg);

	}

	if(!lsGroupVersionInfo.empty())
		getGroupInfo(lsGroupVersionInfo);
}

uint32_t CClient::getMsgList(IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getMsgList(nUserId, nType, nPeerId, nMsgId, nMsgCnt);
}

void CClient::onGroupList(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo)
{
	printf("GroupList %d\n" , lsGroupVersionInfo.size());
	for(const auto &k:lsGroupVersionInfo)
	{
		printf("group_id : %d , version : %d\n" , k.group_id() , k.version());
	}

	getGroupInfo(lsGroupVersionInfo);
}

void CClient::onGroupInfo(uint32_t nSeqNo , uint32_t nUserId , const list<IM::BaseDefine::GroupInfo> &lsGroupInfo)
{
	printf("GroupInfo %d\n" , lsGroupInfo.size());
	for(const auto &k:lsGroupInfo)
	{
		printf("group_id:%d version:%d group_name:%s group_avatar:%s group_creator_id:%d group_type:%d shield_status:%d \n" ,
			k.group_id() , k.version() , k.group_name().c_str() , k.group_avatar().c_str() , k.group_creator_id(),
			k.group_type() , k.shield_status() );

		for(const auto &v:k.group_member_list())
		{
			printf("\t%d" , v);
		}
		printf("\n");
	}
}
void CClient::onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo> &lsMsg)
{
    printf("GetMsgList session_id:%d , msg_id:%d, msg_cnt:%d:\n" , nPeerId , nMsgId , nMsgCnt);
	for(const auto &m:lsMsg)
	{
		char *desMsg=0;
		uint32_t noutlen = 0 ;
		int retCode = DecryptMsg(m.msg_data().c_str() , m.msg_data().length() , &desMsg,noutlen);

		char szBuf[256] = {0};
		time_t timer = (time_t)m.create_time();
		strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));
		printf("time:%s fromid:%d  , type:%d , msg:%s\n", szBuf , m.from_session_id() , m.msg_type() , desMsg);
		Free(desMsg);  
	}
}

void CClient::onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string &strMsgData)
{
	char *desMsg=0;
	uint32_t noutlen = 0 ;
	int retCode = DecryptMsg(strMsgData.c_str() , strMsgData.length() , &desMsg,noutlen);

	char szBuf[256] = {0};
    time_t timer = (time_t)nCreateTime;
    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));
      

    printf("time:%s fromid:%d , toid:%d , type:%d , msg:%s\n" , szBuf , nFromId , nToId ,nMsgType, desMsg);  
	Free(desMsg);  
}
