/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/
#include "ClientConn.h"
#include "playsound.h"
#include "Common.h"


ClientConn::ClientConn():
m_bOpen(false)
{
    m_pSeqAlloctor = CSeqAlloctor::getInstance();
}

ClientConn::~ClientConn()
{

}

net_handle_t ClientConn::connect(const string& strIp, uint16_t nPort, const string& strName, const string& strPass)
{
	
	m_handle = netlib_connect(strIp.c_str(), nPort, imconn_callback, &m_connMap);
	m_connMap[m_handle] = this;
	return  m_handle;
}



void ClientConn::OnConfirm()
{   
    if(m_pCallback)
    {
        m_pCallback->onConnect();
    }
}

void ClientConn::OnClose()
{
    log("onclose from handle=%d\n", m_handle);
    Close();
}

void ClientConn::OnTimer(uint64_t curr_tick)
{
    if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
        CImPdu cPdu;
        IM::Other::IMHeartBeat msg;
        cPdu.SetPBMsg(&msg);
        cPdu.SetServiceId(IM::BaseDefine::SID_OTHER);
        cPdu.SetCommandId(IM::BaseDefine::CID_OTHER_HEARTBEAT);
        uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
        cPdu.SetSeqNum(nSeqNo);
        SendPdu(&cPdu);
    }
    
    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
        log("conn to msg_server timeout\n");
        Close();
    }
}


uint32_t ClientConn::login(const string &strName, const string &strPass)
{
    CImPdu cPdu;
    IM::Login::IMLoginReq msg;
    msg.set_user_name(strName);
    msg.set_password(strPass);
    msg.set_online_status(IM::BaseDefine::USER_STATUS_ONLINE);
    msg.set_client_type(IM::BaseDefine::CLIENT_TYPE_WINDOWS);
    msg.set_client_version("1.0");
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_LOGIN);
    cPdu.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_USERLOGIN);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUser(uint32_t nUserId, uint32_t nTime)
{
    CImPdu cPdu;
    IM::Buddy::IMAllUserReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_ALL_USER_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUserInfo(uint32_t nUserId, list<uint32_t>& lsUserId)
{
    CImPdu cPdu;
    IM::Buddy::IMUsersInfoReq msg;
    msg.set_user_id(nUserId);
    for (auto it=lsUserId.begin(); it!=lsUserId.end(); ++it) {
        msg.add_user_id_list(*it);
    }
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_USER_INFO_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMessage(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, const string& strMsgData)
{
    CImPdu cPdu;
    IM::Message::IMMsgData msg;
    msg.set_from_user_id(nFromId);
    msg.set_to_session_id(nToId);
    msg.set_msg_id(0);
    msg.set_create_time(time(NULL));
    msg.set_msg_type(nType);
    msg.set_msg_data(strMsgData);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUnreadMsgCnt(uint32_t nUserId)
{
    CImPdu cPdu;
    IM::Message::IMUnreadMsgCntReq msg;
    msg.set_user_id(nUserId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_UNREAD_CNT_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getGroupList(uint32_t nUserId)
{
	CImPdu cPdu;
	IM::Group::IMNormalGroupListReq msg;
	msg.set_user_id(nUserId);
	printf("%d\n" , nUserId);
	cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_GROUP);
    cPdu.SetCommandId(IM::BaseDefine::CID_GROUP_NORMAL_LIST_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getGroupInfo(uint32_t nUserId , const list<IM::BaseDefine::GroupVersionInfo> &lsGroupVersionInfo)
{
	CImPdu cPdu;
	IM::Group::IMGroupInfoListReq msg;
	msg.set_user_id(nUserId);
	
	for(const auto &vi:lsGroupVersionInfo)
	{
		IM::BaseDefine::GroupVersionInfo* pGroupVersionInfo = msg.add_group_version_list();
		pGroupVersionInfo->set_group_id(vi.group_id());
		pGroupVersionInfo->set_version(0);
	}

	cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_GROUP);
    cPdu.SetCommandId(IM::BaseDefine::CID_GROUP_INFO_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getRecentSession(uint32_t nUserId, uint32_t nLastTime)
{
    CImPdu cPdu;
    IM::Buddy::IMRecentContactSessionReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nLastTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getDepartMentInfo(uint32_t nUserId , uint32_t nLastTime)
{
	CImPdu cPdu;
    IM::Buddy::IMDepartmentReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nLastTime);
	

    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
    cPdu.SetCommandId( IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);

    return nSeqNo;

}

uint32_t ClientConn::getMsgList(uint32_t nUserId, IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    CImPdu cPdu;
    IM::Message::IMGetMsgListReq msg;
    msg.set_user_id(nUserId);
    msg.set_session_type(nType);
    msg.set_session_id(nPeerId);
    msg.set_msg_id_begin(nMsgId);
    msg.set_msg_cnt(nMsgCnt);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_LIST_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMsgAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    CImPdu cPdu;
    IM::Message::IMMsgDataAck msg;
    msg.set_user_id(nUserId);
    msg.set_session_id(nPeerId);
    msg.set_session_type(nType);
    msg.set_msg_id(nMsgId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA_ACK);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMsgReadAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    CImPdu cPdu;
    IM::Message::IMMsgDataReadAck msg;
    msg.set_user_id(nUserId);
    msg.set_session_id(nPeerId);
    msg.set_session_type(nType);
    msg.set_msg_id(nMsgId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::CID_MSG_READ_ACK);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}




void ClientConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
	}
	ReleaseRef();
}

void ClientConn::HandlePdu(CImPdu* pPdu)
{
    //printf("pdu type = %u\n", pPdu->GetPduType());
	switch (pPdu->GetCommandId()) {
        case IM::BaseDefine::CID_OTHER_HEARTBEAT:
//		printf("Heartbeat\n");
		break;
        case IM::BaseDefine::CID_LOGIN_RES_USERLOGIN:  
			_HandleLoginResponse(pPdu);
		break;
        case IM::BaseDefine::CID_BUDDY_LIST_ALL_USER_RESPONSE:
			_HandleUser(pPdu);
        break;
        case IM::BaseDefine::CID_BUDDY_LIST_USER_INFO_RESPONSE:
			_HandleUserInfo(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_DATA_ACK:
            _HandleSendMsg(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_UNREAD_CNT_RESPONSE: 
			_HandleUnreadCnt(pPdu);
            break;
        case IM::BaseDefine::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE:
			_HandleRecentSession(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_LIST_RESPONSE:
			_HandleMsgList(pPdu);
        break;
        case IM::BaseDefine::CID_MSG_DATA:
            _HandleMsgData(pPdu);
		break;
		case IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_RESPONSE:
			_HandleDepartMentInfo(pPdu);
        break;
		case IM::BaseDefine::CID_GROUP_NORMAL_LIST_RESPONSE:
			_HandleGroupList(pPdu);
		break;
		case IM::BaseDefine::CID_GROUP_INFO_RESPONSE:
			_HandleGroupInfo(pPdu);
        break;
		default:
		log("wrong msg_type=%d\n", pPdu->GetCommandId());
		break;
	}
}
void ClientConn::_HandleLoginResponse(CImPdu* pPdu)
{
    IM::Login::IMLoginRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nRet = msgResp.result_code();
        string strMsg = msgResp.result_string();
	if(nRet == 0)
        {
            m_bOpen = true;
            IM::BaseDefine::UserInfo cUser = msgResp.user_info();
            m_pCallback->onLogin(nSeqNo, nRet, strMsg, &cUser);
        }
        else
        {
            m_pCallback->onLogin(nSeqNo, nRet, strMsg);
        }
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUser(CImPdu* pPdu)
{
    IM::Buddy::IMAllUserRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_list_size();
        list<IM::BaseDefine::UserInfo> lsUsers;
        for(uint32_t i=0; i<userCnt; ++i)
        {
            IM::BaseDefine::UserInfo cUserInfo = msgResp.user_list(i);
            lsUsers.push_back(cUserInfo);
        }
        m_pCallback->onGetChangedUser(nSeqNo, lsUsers);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUserInfo(CImPdu* pPdu)
{
    IM::Buddy::IMUsersInfoRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_info_list_size();
        list<IM::BaseDefine::UserInfo> lsUser;
        for (uint32_t i=0; i<userCnt; ++i) {
            IM::BaseDefine::UserInfo userInfo = msgResp.user_info_list(i);
            lsUser.push_back(userInfo);
        }
        m_pCallback->onGetUserInfo(nSeqNo, lsUser);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleSendMsg(CImPdu* pPdu)
{
    IM::Message::IMMsgDataAck msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nSendId = msgResp.user_id();
        uint32_t nRecvId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id();
        IM::BaseDefine::SessionType nType = msgResp.session_type();
        m_pCallback->onSendMsg(nSeqNo, nSendId, nRecvId, nType, nMsgId);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleDepartMentInfo(CImPdu *pPdu)
{
	IM::Buddy::IMDepartmentRsp msgResp;
	uint32_t nSeqNo = pPdu->GetSeqNum();
	    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
		uint32_t nUserId = msgResp.user_id();
		uint32_t lastest_update_time = msgResp.latest_update_time();
        uint32_t departCnt = msgResp.dept_list_size();
        list<IM::BaseDefine::DepartInfo> lsDepartInfo;
        for (uint32_t i=0; i<departCnt; ++i) {
            IM::BaseDefine::DepartInfo departInfo = msgResp.dept_list(i);
            lsDepartInfo.push_back(departInfo);
        }
        m_pCallback->onGetDepartMentInfo(nSeqNo,  nUserId , lastest_update_time,  lsDepartInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleGroupList(CImPdu *pPdu)
{
	IM::Group::IMNormalGroupListRsp msgResp;
	uint32_t nSeqNo = pPdu->GetSeqNum();
	    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
		uint32_t nUserId = msgResp.user_id();
        list<IM::BaseDefine::GroupVersionInfo> lsGroupVersionInfo;
		int Cnt = msgResp.group_version_list_size();
        for (uint32_t i=0; i<Cnt; ++i) {
            IM::BaseDefine::GroupVersionInfo groupVersionInfo = msgResp.group_version_list(i);
            lsGroupVersionInfo.push_back(groupVersionInfo);
        }
        m_pCallback->onGroupList(nSeqNo,  nUserId , lsGroupVersionInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}


void ClientConn::_HandleGroupInfo(CImPdu *pPdu)
{
	IM::Group::IMGroupInfoListRsp msgResp;
	uint32_t nSeqNo = pPdu->GetSeqNum();
	    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
		uint32_t nUserId = msgResp.user_id();
		int Cnt = msgResp.group_info_list_size();
        list<IM::BaseDefine::GroupInfo> lsGroupInfo;
        for (uint32_t i=0; i<Cnt; ++i) {
            IM::BaseDefine::GroupInfo groupInfo = msgResp.group_info_list(i);
            lsGroupInfo.push_back(groupInfo);
        }
        m_pCallback->onGroupInfo(nSeqNo,  nUserId , lsGroupInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUnreadCnt(CImPdu* pPdu)
{
    IM::Message::IMUnreadMsgCntRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::UnreadInfo> lsUnreadInfo;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nTotalCnt = msgResp.total_cnt();
        uint32_t nCnt = msgResp.unreadinfo_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::UnreadInfo unreadInfo = msgResp.unreadinfo_list(i);
            lsUnreadInfo.push_back(unreadInfo);
        }
        m_pCallback->onGetUnreadMsgCnt(nSeqNo, nUserId, nTotalCnt, lsUnreadInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb fail");
    }
}

void ClientConn::_HandleRecentSession(CImPdu *pPdu)
{
    IM::Buddy::IMRecentContactSessionRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::ContactSessionInfo> lsSession;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nCnt = msgResp.contact_session_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::ContactSessionInfo session = msgResp.contact_session_list(i);
            lsSession.push_back(session);
        }
        m_pCallback->onGetRecentSession(nSeqNo, nUserId, lsSession);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleMsgList(CImPdu *pPdu)
{
    IM::Message::IMGetMsgListRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nUserId= msgResp.user_id();
        IM::BaseDefine::SessionType nSessionType = msgResp.session_type();
        uint32_t nPeerId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id_begin();
        uint32_t nMsgCnt = msgResp.msg_list_size();
        list<IM::BaseDefine::MsgInfo> lsMsg;
        for(uint32_t i=0; i<nMsgCnt; ++i)
        {
            IM::BaseDefine::MsgInfo msgInfo = msgResp.msg_list(i);
            lsMsg.push_back(msgInfo);
        }
        m_pCallback->onGetMsgList(nSeqNo, nUserId, nPeerId, nSessionType, nMsgId, nMsgCnt, lsMsg);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}
void ClientConn::_HandleMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        play("message.wav");
        
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_session_id();
        uint32_t nMsgId = msg.msg_id();
        IM::BaseDefine::MsgType nMsgType = msg.msg_type();
        uint32_t nCreateTime = msg.create_time();
        string strMsg(msg.msg_data().c_str(), msg.msg_data().length());
        
        IM::BaseDefine::SessionType nSessionType;
        if(nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_TEXT)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_SINGLE;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_SINGLE;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_TEXT)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_GROUP;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_AUDIO)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_GROUP;
        }
		
		if(nSessionType == IM::BaseDefine::SESSION_TYPE_SINGLE)
			sendMsgAck(nToId , nFromId , nSessionType, nMsgId);
        m_pCallback->onRecvMsg(nSeqNo, nFromId, nToId, nMsgId, nCreateTime, nMsgType, strMsg);
		sendMsgReadAck( nToId , nFromId , nSessionType, nMsgId);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}
