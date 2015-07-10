/*
 * GroupChat.cpp
 *
 *  Created on: 2014-1-3
 *      Author: ziteng@mogujie.com
 */

#include "GroupChat.h"
#include "MsgConn.h"
#include "DBServConn.h"
#include "RouteServConn.h"

#include "AttachData.h"
#include "ImUser.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"
#include "IM.Message.pb.h"
#include <set>
using namespace IM::BaseDefine;

CGroupChat* CGroupChat::s_group_chat_instance = NULL;

CGroupChat* CGroupChat::GetInstance()
{
	if (!s_group_chat_instance) {
		s_group_chat_instance = new CGroupChat();
	}

	return s_group_chat_instance;
}

void CGroupChat::HandleClientGroupNormalRequest(CImPdu* pPdu, CMsgConn* pFromConn)
{
    IM::Group::IMNormalGroupListReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    uint32_t user_id = pFromConn->GetUserId();
    log("HandleClientGroupNormalRequest, user_id=%u. ", user_id);
    CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
    
    CDBServConn* pDBConn = get_db_serv_conn();
    if (pDBConn)
    {
        msg.set_user_id(user_id);
        msg.set_attach_data((uchar_t*)attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
        pDBConn->SendPdu(pPdu);
    }
    else
    {
        log("no db connection. ");
        IM::Group::IMNormalGroupListRsp msg2;
        msg.set_user_id(user_id);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_NORMAL_LIST_RESPONSE);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        pFromConn->SendPdu(&pdu);
    }
}

void CGroupChat::HandleGroupNormalResponse(CImPdu* pPdu)
{
    IM::Group::IMNormalGroupListRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t user_id = msg.user_id();
    uint32_t group_cnt = msg.group_version_list_size();
    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());

    log("HandleGroupNormalResponse, user_id=%u, group_cnt=%u. ", user_id, group_cnt);

    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, attach_data.GetHandle());
    if (pConn)
    {
        pConn->SendPdu(pPdu);
    }
}

void CGroupChat::HandleClientGroupInfoRequest(CImPdu *pPdu, CMsgConn* pFromConn)
{
    IM::Group::IMGroupInfoListReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    uint32_t user_id = pFromConn->GetUserId();
    uint32_t group_cnt = msg.group_version_list_size();
    log("HandleClientGroupInfoRequest, user_id=%u, group_cnt=%u. ", user_id, group_cnt);
    CPduAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0, NULL);
    
    CDBServConn* pDBConn = get_db_serv_conn();
    if (pDBConn)
    {
        msg.set_user_id(user_id);
        msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
        pDBConn->SendPdu(pPdu);
    }
    else
    {
        log("no db connection. ");
        IM::Group::IMGroupInfoListRsp msg2;
        msg2.set_user_id(user_id);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_INFO_RESPONSE);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        pFromConn->SendPdu(&pdu);
    }
}

void CGroupChat::HandleGroupInfoResponse(CImPdu* pPdu)
{
    IM::Group::IMGroupInfoListRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t user_id = msg.user_id();
    uint32_t group_cnt = msg.group_info_list_size();
    CPduAttachData pduAttachData((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    
    log("HandleGroupInfoResponse, user_id=%u, group_cnt=%u. ", user_id, group_cnt);

    //此处是查询成员时使用，主要用于群消息从数据库获得msg_id后进行发送,一般此时group_cnt = 1
    if (pduAttachData.GetPduLength() > 0 && group_cnt > 0)
    {
        IM::BaseDefine::GroupInfo group_info = msg.group_info_list(0);
        uint32_t group_id = group_info.group_id();
        log("GroupInfoRequest is send by server, group_id=%u ", group_id);
        
        std::set<uint32_t> group_member_set;
        for (uint32_t i = 0; i < group_info.group_member_list_size(); i++)
        {
            uint32_t member_user_id = group_info.group_member_list(i);
            group_member_set.insert(member_user_id);
        }
        if (group_member_set.find(user_id) == group_member_set.end())
        {
            log("user_id=%u is not in group, group_id=%u. ", user_id, group_id);
            return;
        }
        
        IM::Message::IMMsgData msg2;
        CHECK_PB_PARSE_MSG(msg2.ParseFromArray(pduAttachData.GetPdu(), pduAttachData.GetPduLength()));
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_MSG);
        pdu.SetCommandId(CID_MSG_DATA);
        
        //Push相关
        IM::Server::IMGroupGetShieldReq msg3;
        msg3.set_group_id(group_id);
        msg3.set_attach_data(pdu.GetBodyData(), pdu.GetBodyLength());
        for (uint32_t i = 0; i < group_info.group_member_list_size(); i++)
        {
            uint32_t member_user_id = group_info.group_member_list(i);
            
            msg3.add_user_id(member_user_id);
            
            CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(member_user_id);
            if (pToImUser)
            {
                CMsgConn* pFromConn = NULL;
                if( member_user_id == user_id )
                {
                    uint32_t reqHandle = pduAttachData.GetHandle();
                    if(reqHandle != 0)
                        pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, reqHandle);
                }
                
                pToImUser->BroadcastData(pdu.GetBuffer(), pdu.GetLength(), pFromConn);
            }
        }
        
        CImPdu pdu2;
        pdu2.SetPBMsg(&msg3);
        pdu2.SetServiceId(SID_OTHER);
        pdu2.SetCommandId(CID_OTHER_GET_SHIELD_REQ);
        CDBServConn* pDbConn = get_db_serv_conn();
        if (pDbConn)
        {
            pDbConn->SendPdu(&pdu2);
        }
    }
    else if (pduAttachData.GetPduLength() == 0)
    {
        //正常获取群信息的返回
        CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, pduAttachData.GetHandle());
        if (pConn)
        {
            msg.clear_attach_data();
            pPdu->SetPBMsg(&msg);
            pConn->SendPdu(pPdu);
        }
    }
}

void CGroupChat::HandleGroupMessage(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_group_id = msg.to_session_id();
	string msg_data = msg.msg_data();
    uint32_t msg_id = msg.msg_id();
    if (msg_id == 0) {
        log("HandleGroupMsg, write db failed, %u->%u. ", from_user_id, to_group_id);
        return;
    }
    uint8_t msg_type = msg.msg_type();
    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());

    log("HandleGroupMsg, %u->%u, msg id=%u. ", from_user_id, to_group_id, msg_id);

    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_user_id,
                                        attach_data.GetHandle());
    if (pFromConn)
    {
        //接收反馈
        IM::Message::IMMsgDataAck msg2;
        msg2.set_user_id(from_user_id);
        msg2.set_session_id(to_group_id);
        msg2.set_msg_id(msg_id);
        msg2.set_session_type(::IM::BaseDefine::SESSION_TYPE_GROUP);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_MSG);
        pdu.SetCommandId(CID_MSG_DATA_ACK);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        pFromConn->SendPdu(&pdu);
    }
    
    CRouteServConn* pRouteConn = get_route_serv_conn();
    if (pRouteConn)
    {
        pRouteConn->SendPdu(pPdu);
    }
    
    // 服务器没有群的信息，向DB服务器请求群信息，并带上消息作为附件，返回时在发送该消息给其他群成员
    //IM::BaseDefine::GroupVersionInfo group_version_info;
    CPduAttachData pduAttachData(ATTACH_TYPE_HANDLE_AND_PDU, attach_data.GetHandle(), pPdu->GetBodyLength(), pPdu->GetBodyData());
    
    IM::Group::IMGroupInfoListReq msg3;
    msg3.set_user_id(from_user_id);
    IM::BaseDefine::GroupVersionInfo* group_version_info = msg3.add_group_version_list();
    group_version_info->set_group_id(to_group_id);
    group_version_info->set_version(0);
    msg3.set_attach_data(pduAttachData.GetBuffer(), pduAttachData.GetLength());
    CImPdu pdu;
    pdu.SetPBMsg(&msg3);
    pdu.SetServiceId(SID_GROUP);
    pdu.SetCommandId(CID_GROUP_INFO_REQUEST);
    CDBServConn* pDbConn = get_db_serv_conn();
    if(pDbConn)
    {
        pDbConn->SendPdu(&pdu);
    }
}

void CGroupChat::HandleGroupMessageBroadcast(CImPdu *pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t from_user_id = msg.from_user_id();
    uint32_t to_group_id = msg.to_session_id();
    string msg_data = msg.msg_data();
    uint32_t msg_id = msg.msg_id();
    log("HandleGroupMessageBroadcast, %u->%u, msg id=%u. ", from_user_id, to_group_id, msg_id);
    
    // 服务器没有群的信息，向DB服务器请求群信息，并带上消息作为附件，返回时在发送该消息给其他群成员
    //IM::BaseDefine::GroupVersionInfo group_version_info;
    CPduAttachData pduAttachData(ATTACH_TYPE_HANDLE_AND_PDU, 0, pPdu->GetBodyLength(), pPdu->GetBodyData());
    
    IM::Group::IMGroupInfoListReq msg2;
    msg2.set_user_id(from_user_id);
    IM::BaseDefine::GroupVersionInfo* group_version_info = msg2.add_group_version_list();
    group_version_info->set_group_id(to_group_id);
    group_version_info->set_version(0);
    msg2.set_attach_data(pduAttachData.GetBuffer(), pduAttachData.GetLength());
    CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_GROUP);
    pdu.SetCommandId(CID_GROUP_INFO_REQUEST);
    CDBServConn* pDbConn = get_db_serv_conn();
    if(pDbConn)
    {
        pDbConn->SendPdu(&pdu);
    }
}

void CGroupChat::HandleClientGroupCreateRequest(CImPdu* pPdu, CMsgConn* pFromConn)
{
    IM::Group::IMGroupCreateReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t req_user_id = pFromConn->GetUserId();
    string group_name = msg.group_name();
    uint32_t group_type = msg.group_type();
    if (group_type == IM::BaseDefine::GROUP_TYPE_NORMAL) {
        log("HandleClientGroupCreateRequest, create normal group failed, req_id=%u, group_name=%s. ", req_user_id, group_name.c_str());
        return;
    }
	string group_avatar = msg.group_avatar();
	uint32_t user_cnt = msg.member_id_list_size();
	log("HandleClientGroupCreateRequest, req_id=%u, group_name=%s, avatar_url=%s, user_cnt=%u ",
			req_user_id, group_name.c_str(), group_avatar.c_str(), user_cnt);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
        msg.set_user_id(req_user_id);
        msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
		pDbConn->SendPdu(pPdu);
	} else {
		log("no DB connection ");
        IM::Group::IMGroupCreateRsp msg2;
        msg2.set_user_id(req_user_id);
        msg2.set_result_code(1);
        msg2.set_group_name(group_name);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_CREATE_RESPONSE);
		pdu.SetSeqNum(pPdu->GetSeqNum());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupCreateResponse(CImPdu* pPdu)
{
    IM::Group::IMGroupCreateRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_id = msg.user_id();
    uint32_t result = msg.result_code();
	uint32_t group_id = msg.group_id();
	string group_name = msg.group_name();
	uint32_t user_cnt = msg.user_id_list_size();
	log("HandleGroupCreateResponse, req_id=%u, result=%u, group_id=%u, group_name=%s, member_cnt=%u. ", user_id, result, group_id, group_name.c_str(), user_cnt);

    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    
    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id,
                                                                    attach_data.GetHandle());
    if (pFromConn)
    {
        msg.clear_attach_data();
        pPdu->SetPBMsg(&msg);
        pFromConn->SendPdu(pPdu);
    }
    //创建的通知暂时取消，因为有消息的时候客户端也会去拉取
}

void CGroupChat::HandleClientGroupChangeMemberRequest(CImPdu* pPdu, CMsgConn* pFromConn)
{
    IM::Group::IMGroupChangeMemberReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t change_type = msg.change_type();
	uint32_t req_user_id = pFromConn->GetUserId();
	uint32_t group_id = msg.group_id();
	uint32_t user_cnt = msg.member_id_list_size();
	log("HandleClientChangeMemberReq, change_type=%u, req_id=%u, group_id=%u, user_cnt=%u ",
			change_type, req_user_id, group_id, user_cnt);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {

        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
        msg.set_user_id(req_user_id);
        msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
		pDbConn->SendPdu(pPdu);
	} else {
		log("no DB connection ");
        IM::Group::IMGroupChangeMemberRsp msg2;
        msg2.set_user_id(req_user_id);
        msg2.set_change_type((IM::BaseDefine::GroupModifyType)change_type);
        msg2.set_result_code(1);
        msg2.set_group_id(group_id);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_CHANGE_MEMBER_RESPONSE);
		pdu.SetSeqNum(pPdu->GetSeqNum());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupChangeMemberResponse(CImPdu* pPdu)
{
    IM::Group::IMGroupChangeMemberRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t change_type = msg.change_type();
	uint32_t user_id = msg.user_id();
	uint32_t result = msg.result_code();
	uint32_t group_id = msg.group_id();
	uint32_t chg_user_cnt = msg.chg_user_id_list_size();
    uint32_t cur_user_cnt = msg.cur_user_id_list_size();
	log("HandleChangeMemberResp, change_type=%u, req_id=%u, group_id=%u, result=%u, chg_usr_cnt=%u, cur_user_cnt=%u. ",
			change_type, user_id, group_id, result, chg_user_cnt, cur_user_cnt);

    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id,
                                                                attach_data.GetHandle());
    if (pFromConn) {
        msg.clear_attach_data();
        pPdu->SetPBMsg(&msg);
        pFromConn->SendPdu(pPdu);
    }
   
	if (!result) {
        IM::Group::IMGroupChangeMemberNotify msg2;
        msg2.set_user_id(user_id);
        msg2.set_change_type((::IM::BaseDefine::GroupModifyType)change_type);
        msg2.set_group_id(group_id);
        for (uint32_t i = 0; i < chg_user_cnt; i++) {
            msg2.add_chg_user_id_list(msg.chg_user_id_list(i));
        }
        for (uint32_t i = 0; i < cur_user_cnt; i++) {
            msg2.add_cur_user_id_list(msg.cur_user_id_list(i));
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_CHANGE_MEMBER_NOTIFY);
		CRouteServConn* pRouteConn = get_route_serv_conn();
		if (pRouteConn) {
			pRouteConn->SendPdu(&pdu);
		}

        for (uint32_t i = 0; i < chg_user_cnt; i++)
        {
            uint32_t to_user_id = msg.chg_user_id_list(i);
            _SendPduToUser(&pdu, to_user_id, pFromConn);
        }
        for (uint32_t i = 0; i < cur_user_cnt; i++) {
            uint32_t to_user_id = msg.cur_user_id_list(i);
            _SendPduToUser(&pdu, to_user_id, pFromConn);
        }
    }
}

void CGroupChat::HandleGroupChangeMemberBroadcast(CImPdu* pPdu)
{
    IM::Group::IMGroupChangeMemberNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t change_type = msg.change_type();
	uint32_t group_id = msg.group_id();
	uint32_t chg_user_cnt = msg.chg_user_id_list_size();
    uint32_t cur_user_cnt = msg.cur_user_id_list_size();
	log("HandleChangeMemberBroadcast, change_type=%u, group_id=%u, chg_user_cnt=%u, cur_user_cnt=%u. ", change_type, group_id, chg_user_cnt, cur_user_cnt);

    for (uint32_t i = 0; i < chg_user_cnt; i++)
    {
        uint32_t to_user_id = msg.chg_user_id_list(i);
        _SendPduToUser(pPdu, to_user_id);
    }
    for (uint32_t i = 0; i < cur_user_cnt; i++) {
        uint32_t to_user_id = msg.cur_user_id_list(i);
        _SendPduToUser(pPdu, to_user_id);
    }
}

void CGroupChat::HandleClientGroupShieldGroupRequest(CImPdu *pPdu, CMsgConn *pFromConn)
{
    IM::Group::IMGroupShieldReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t shield_status = msg.shield_status();
    uint32_t group_id = msg.group_id();
    uint32_t user_id = pFromConn->GetUserId();
    log("HandleClientGroupShieldGroupRequest, user_id: %u, group_id: %u, shield_status: %u. ",
        user_id, group_id, shield_status);
    
    CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
        msg.set_user_id(user_id);
        msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
		pDbConn->SendPdu(pPdu);
        
	} else {
        log("no DB connection ");
        IM::Group::IMGroupShieldRsp msg2;
        msg2.set_user_id(user_id);
        msg2.set_result_code(1);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_GROUP);
        pdu.SetCommandId(CID_GROUP_SHIELD_GROUP_RESPONSE);
		pdu.SetSeqNum(pPdu->GetSeqNum());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupShieldGroupResponse(CImPdu *pPdu)
{
    IM::Group::IMGroupShieldRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t result = msg.result_code();
    uint32_t user_id = msg.user_id();
    uint32_t group_id = msg.group_id();
    log("HandleGroupShieldGroupResponse, result: %u, user_id: %u, group_id: %u. ", result,
        user_id, group_id);
    
    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id,
                                        attach_data.GetHandle());
    if (pMsgConn) {
        msg.clear_attach_data();
        pPdu->SetPBMsg(&msg);
		pMsgConn->SendPdu(pPdu);
    }
}

void CGroupChat::HandleGroupGetShieldByGroupResponse(CImPdu *pPdu)
{
    IM::Server::IMGroupGetShieldRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t shield_status_list_cnt = msg.shield_status_list_size();
    
    log("HandleGroupGetShieldByGroupResponse, shield_status_list_cnt: %u. ",
        shield_status_list_cnt);
    
    IM::Server::IMGetDeviceTokenReq msg2;
    msg2.set_attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    for (uint32_t i = 0; i < shield_status_list_cnt; i++)
    {
        IM::BaseDefine::ShieldStatus shield_status = msg.shield_status_list(i);
        if (shield_status.shield_status() == 0)
        {
            msg2.add_user_id(shield_status.user_id());
        }
        else
        {
            log("user_id: %u shield group, group id: %u. ", shield_status.user_id(), shield_status.group_id());
        }
    }
    CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_GET_DEVICE_TOKEN_REQ);
    CDBServConn* pDbConn = get_db_serv_conn();
    if (pDbConn) {
        pDbConn->SendPdu(&pdu);
    }
}

void CGroupChat::_SendPduToUser(CImPdu* pPdu, uint32_t user_id, CMsgConn* pReqConn)
{
    if(!pPdu) {
        return;
    }
    CImUser* pToUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if(pToUser) {
        pToUser->BroadcastPdu(pPdu, pReqConn);
    }
}
