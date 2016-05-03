/*
 * RouteServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "RouteServConn.h"
#include "MsgConn.h"
#include "LoginServConn.h"
#include "DBServConn.h"
#include "FileHandler.h"
#include "GroupChat.h"
#include "ImUser.h"
#include "PushServConn.h"
#include "AttachData.h"
#include "FileServConn.h"
#include "IM.Buddy.pb.h"
#include "IM.Message.pb.h"
#include "IM.Other.pb.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "IM.File.pb.h"
using namespace IM::BaseDefine;

static ConnMap_t g_route_server_conn_map;

static serv_info_t* g_route_server_list;
static uint32_t g_route_server_count;
static CRouteServConn* g_master_rs_conn = NULL;
static CFileHandler* s_file_handler = NULL;
static CGroupChat* s_group_chat = NULL;

void route_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CRouteServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_route_server_conn_map.begin(); it != g_route_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CRouteServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	// reconnect RouteServer
	serv_check_reconnect<CRouteServConn>(g_route_server_list, g_route_server_count);
}

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_route_server_list = server_list;
	g_route_server_count = server_count;

	serv_init<CRouteServConn>(g_route_server_list, g_route_server_count);

	netlib_register_timer(route_server_conn_timer_callback, NULL, 1000);
	s_file_handler = CFileHandler::getInstance();
	s_group_chat = CGroupChat::GetInstance();
}

bool is_route_server_available()
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_route_server(CImPdu* pPdu)
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			pConn->SendPdu(pPdu);
		}
	}
}

// get the oldest route server connection
CRouteServConn* get_route_serv_conn()
{
	return g_master_rs_conn;
}

void update_master_route_serv_conn()
{
	uint64_t oldest_connect_time = (uint64_t)-1;
	CRouteServConn* pOldestConn = NULL;

	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time) ){
			pOldestConn = pConn;
			oldest_connect_time = pConn->GetConnectTime();
		}
	}

	g_master_rs_conn =  pOldestConn;

	if (g_master_rs_conn) {
        IM::Server::IMRoleSet msg;
        msg.set_master(1);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_ROLE_SET);
		g_master_rs_conn->SendPdu(&pdu);
	}
}


CRouteServConn::CRouteServConn()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CRouteServConn::~CRouteServConn()
{

}

void CRouteServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	log("Connecting to RouteServer %s:%d ", server_ip, server_port);

	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_route_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_route_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CRouteServConn::Close()
{
	serv_reset<CRouteServConn>(g_route_server_list, g_route_server_count, m_serv_idx);

	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_route_server_conn_map.erase(m_handle);
	}

	ReleaseRef();

	if (g_master_rs_conn == this) {
		update_master_route_serv_conn();
	}
}

void CRouteServConn::OnConfirm()
{
	log("connect to route server success ");
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_route_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

	if (g_master_rs_conn == NULL) {
		update_master_route_serv_conn();
	}

	list<user_stat_t> online_user_list;
    CImUserManager::GetInstance()->GetOnlineUserInfo(&online_user_list);
    IM::Server::IMOnlineUserInfo msg;
    for (list<user_stat_t>::iterator it = online_user_list.begin(); it != online_user_list.end(); it++) {
        user_stat_t user_stat = *it;
        IM::BaseDefine::ServerUserStat* server_user_stat = msg.add_user_stat_list();
        server_user_stat->set_user_id(user_stat.user_id);
        server_user_stat->set_status((::IM::BaseDefine::UserStatType)user_stat.status);
        server_user_stat->set_client_type((::IM::BaseDefine::ClientType)user_stat.client_type);

    }
    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_ONLINE_USER_INFO);
	SendPdu(&pdu);
}

void CRouteServConn::OnClose()
{
	log("onclose from route server handle=%d ", m_handle);
	Close();
}

void CRouteServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
        IM::Other::IMHeartBeat msg;
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_HEARTBEAT);
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to route server timeout ");
		Close();
	}
}

void CRouteServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            break;
        case CID_OTHER_SERVER_KICK_USER:
            _HandleKickUser( pPdu );
            break;
        case CID_BUDDY_LIST_STATUS_NOTIFY:
            _HandleStatusNotify( pPdu );
            break;
        case CID_BUDDY_LIST_USERS_STATUS_RESPONSE:
            _HandleUsersStatusResponse( pPdu );
            break;
        case CID_MSG_READ_NOTIFY:
            _HandleMsgReadNotify(pPdu);
            break;
        case CID_MSG_DATA:
            _HandleMsgData(pPdu);
            break;
        case CID_SWITCH_P2P_CMD:
            _HandleP2PMsg(pPdu );
            break;
        case CID_OTHER_LOGIN_STATUS_NOTIFY:
            _HandlePCLoginStatusNotify(pPdu);
            break;
        case CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
            _HandleRemoveSessionNotify(pPdu);
            break;
        case CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
            _HandleSignInfoChangedNotify(pPdu);
        case CID_GROUP_CHANGE_MEMBER_NOTIFY:
            s_group_chat->HandleGroupChangeMemberBroadcast(pPdu);
            break;
        case CID_FILE_NOTIFY:
            s_file_handler->HandleFileNotify(pPdu);
            break;
            break;
        default:
            log("unknown cmd id=%d ", pPdu->GetCommandId());
            break;
	}
}

void CRouteServConn::_HandleKickUser(CImPdu* pPdu)
{
    IM::Server::IMServerKickUser msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_id = msg.user_id();
    uint32_t client_type = msg.client_type();
    uint32_t reason = msg.reason();
	log("HandleKickUser, user_id=%u, client_type=%u, reason=%u. ", user_id, client_type, reason);

    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
	if (pUser) {
		pUser->KickOutSameClientType(client_type, reason);
	}
}

// friend online/off-line notify
void CRouteServConn::_HandleStatusNotify(CImPdu* pPdu)
{
    IM::Buddy::IMUserStatNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    IM::BaseDefine::UserStat user_stat = msg.user_stat();

	log("HandleFriendStatusNotify, user_id=%u, status=%u ", user_stat.user_id(), user_stat.status());

	// send friend online message to client
    CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_PC);
}

void CRouteServConn::_HandleMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    if (CHECK_MSG_TYPE_GROUP(msg.msg_type())) {
        s_group_chat->HandleGroupMessageBroadcast(pPdu);
        return;
    }
	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_session_id();
    uint32_t msg_id = msg.msg_id();
	log("HandleMsgData, %u->%u, msg_id=%u. ", from_user_id, to_user_id, msg_id);
    
    
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
    if (pFromImUser)
    {
        pFromImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
    }
    
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
	if (pToImUser)
    {
		pToImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
	}
}

void CRouteServConn::_HandleMsgReadNotify(CImPdu *pPdu)
{
    IM::Message::IMMsgDataReadNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t req_id = msg.user_id();
    uint32_t session_id = msg.session_id();
    uint32_t msg_id = msg.msg_id();
    uint32_t session_type = msg.session_type();
    
    log("HandleMsgReadNotify, user_id=%u, session_id=%u, session_type=%u, msg_id=%u. ", req_id, session_id, session_type, msg_id);
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(req_id);
    if (pUser)
    {
        pUser->BroadcastPdu(pPdu);
    }
}

void CRouteServConn::_HandleP2PMsg(CImPdu* pPdu)
{
    IM::SwitchService::IMP2PCmdMsg msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_user_id();

	log("HandleP2PMsg, %u->%u ", from_user_id, to_user_id);
    
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    
 	if (pFromImUser) {
 		pFromImUser->BroadcastPdu(pPdu);
	}
    
 	if (pToImUser) {
 		pToImUser->BroadcastPdu(pPdu);
	}
}

void CRouteServConn::_HandleUsersStatusResponse(CImPdu* pPdu)
{
    IM::Buddy::IMUsersStatRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_id = msg.user_id();
	uint32_t result_count = msg.user_stat_list_size();
	log("HandleUsersStatusResp, user_id=%u, query_count=%u ", user_id, result_count);
    
    CPduAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    if (attach_data.GetType() == ATTACH_TYPE_HANDLE)
    {
        uint32_t handle = attach_data.GetHandle();
        CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
        if (pConn) {
            msg.clear_attach_data();
            pPdu->SetPBMsg(&msg);
            pConn->SendPdu(pPdu);
        }
    }
    else if (attach_data.GetType() == ATTACH_TYPE_PDU_FOR_PUSH)
    {
        IM::BaseDefine::UserStat user_stat = msg.user_stat_list(0);
        IM::Server::IMPushToUserReq msg2;
        CHECK_PB_PARSE_MSG(msg2.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
        IM::BaseDefine::UserTokenInfo* user_token = msg2.mutable_user_token_list(0);

        //pc client登录，则为勿打扰式推送
        if (user_stat.status() == IM::BaseDefine::USER_STATUS_ONLINE)
        {
            user_token->set_push_type(IM_PUSH_TYPE_SILENT);
            log("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
        }
        else
        {
            user_token->set_push_type(IM_PUSH_TYPE_NORMAL);
            log("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_PUSH_TO_USER_REQ);
        
        CPushServConn* PushConn = get_push_serv_conn();
        if (PushConn)
        {
            PushConn->SendPdu(&pdu);
        }
    }
    else if (attach_data.GetType() == ATTACH_TYPE_HANDLE_AND_PDU_FOR_FILE)
    {
        IM::BaseDefine::UserStat user_stat = msg.user_stat_list(0);
        IM::Server::IMFileTransferReq msg3;
        CHECK_PB_PARSE_MSG(msg3.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
        uint32_t handle = attach_data.GetHandle();
        
        IM::BaseDefine::TransferFileType trans_mode = IM::BaseDefine::FILE_TYPE_OFFLINE;
        if (user_stat.status() == IM::BaseDefine::USER_STATUS_ONLINE)
        {
            trans_mode = IM::BaseDefine::FILE_TYPE_ONLINE;
        }
        msg3.set_trans_mode(trans_mode);
        CImPdu pdu;
        pdu.SetPBMsg(&msg3);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_FILE_TRANSFER_REQ);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        CFileServConn* pConn = get_random_file_serv_conn();
        if (pConn) {
            pConn->SendPdu(&pdu);
        }
        else
        {
            log("no file server ");
            IM::File::IMFileRsp msg4;
            msg4.set_result_code(1);
            msg4.set_from_user_id(msg3.from_user_id());
            msg4.set_to_user_id(msg3.to_user_id());
            msg4.set_file_name(msg3.file_name());
            msg4.set_task_id("");
            msg4.set_trans_mode(msg3.trans_mode());
            CImPdu pdu2;
            pdu2.SetPBMsg(&msg4);
            pdu2.SetServiceId(SID_FILE);
            pdu2.SetCommandId(CID_FILE_RESPONSE);
            pdu2.SetSeqNum(pPdu->GetSeqNum());
            CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(msg3.from_user_id(),handle);
            if (pMsgConn)
            {
                pMsgConn->SendPdu(&pdu2);
            }
        }
    }
}

void CRouteServConn::_HandleRemoveSessionNotify(CImPdu *pPdu)
{
    IM::Buddy::IMRemoveSessionNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t user_id = msg.user_id();
    uint32_t session_id = msg.session_id();
    log("HandleRemoveSessionNotify, user_id=%u, session_id=%u ", user_id, session_id);
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser)
    {
        pUser->BroadcastPdu(pPdu);
    }
}

void CRouteServConn::_HandlePCLoginStatusNotify(CImPdu *pPdu)
{
    IM::Server::IMServerPCLoginStatusNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t user_id = msg.user_id();
    uint32_t login_status = msg.login_status();
    log("HandlePCLoginStatusNotify, user_id=%u, login_status=%u ", user_id, login_status);
    
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser)
    {
        pUser->SetPCLoginStatus(login_status);
        IM::Buddy::IMPCLoginStatusNotify msg2;
        msg2.set_user_id(user_id);
        if (IM_PC_LOGIN_STATUS_ON == login_status)
        {
            msg2.set_login_stat(::IM::BaseDefine::USER_STATUS_ONLINE);
        }
        else
        {
            msg2.set_login_stat(::IM::BaseDefine::USER_STATUS_OFFLINE);
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
        pdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_PC_LOGIN_STATUS_NOTIFY);
        pUser->BroadcastPduToMobile(&pdu);
    }
}

void CRouteServConn::_HandleSignInfoChangedNotify(CImPdu* pPdu) {
        IM::Buddy::IMSignInfoChangedNotify msg;
        CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
        log("HandleSignInfoChangedNotify, changed_user_id=%u, sign_info=%s ", msg.changed_user_id(), msg.sign_info().c_str());
    
        // send friend online message to client
        CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_BOTH);
}
