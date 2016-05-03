/*
 * RouteConn.cpp
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#include "netlib.h"
#include "RouteConn.h"
#include "UserInfo.h"
#include "IM.Buddy.pb.h"
#include "IM.Group.pb.h"
#include "IM.Message.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "public_define.h"
using namespace IM::BaseDefine;

//typedef hash_map<uint32_t /* user_id */, UserStat_t> UserStatMap_t;
static ConnMap_t g_route_conn_map;
typedef hash_map<uint32_t, CUserInfo*> UserInfoMap_t;
static UserInfoMap_t g_user_map;

CUserInfo* GetUserInfo(uint32_t user_id)
{
    CUserInfo* pUser = NULL;
    UserInfoMap_t::iterator it = g_user_map.find(user_id);
    if (it != g_user_map.end()) {
        pUser = it->second;
    }
    
    return pUser;
}
 
void route_serv_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t cur_time = get_tick_count();
	for (ConnMap_t::iterator it = g_route_conn_map.begin(); it != g_route_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		CRouteConn* pConn = (CRouteConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
}

void init_routeconn_timer_callback()
{
	netlib_register_timer(route_serv_timer_callback, NULL, 1000);
}

CRouteConn::CRouteConn()
{
	m_bMaster = false;
}

CRouteConn::~CRouteConn()
{

}

void CRouteConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_route_conn_map.erase(m_handle);
	}

	// remove all user info from this MessageServer
    
    UserInfoMap_t::iterator it_old;
    for (UserInfoMap_t::iterator it = g_user_map.begin(); it != g_user_map.end(); )
    {
        it_old = it;
        it++;
        
        CUserInfo* pUser = it_old->second;
        pUser->RemoveRouteConn(this);
        if (pUser->GetRouteConnCount() == 0)
        {
            delete pUser;
            pUser = NULL;
            g_user_map.erase(it_old);
        }
    }

	ReleaseRef();
}

void CRouteConn::OnConnect(net_handle_t handle)
{
	m_handle = handle;

	g_route_conn_map.insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_route_conn_map);
}

void CRouteConn::OnClose()
{
	log("MsgServer onclose: handle=%d ", m_handle);
	Close();
}

void CRouteConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL)
    {
        IM::Other::IMHeartBeat msg;
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_HEARTBEAT);
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("message server timeout ");
		Close();
	}
}

void CRouteConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            // do not take any action, heart beat only update m_last_recv_tick
            break;
        case CID_OTHER_ONLINE_USER_INFO:
            _HandleOnlineUserInfo( pPdu );
            break;
        case CID_OTHER_USER_STATUS_UPDATE:
            _HandleUserStatusUpdate( pPdu );
            break;
        case CID_OTHER_ROLE_SET:
            _HandleRoleSet( pPdu );
            break;
        case CID_BUDDY_LIST_USERS_STATUS_REQUEST:
            _HandleUsersStatusRequest( pPdu );
            break;
        case CID_MSG_DATA:
        case CID_SWITCH_P2P_CMD:
        case CID_MSG_READ_NOTIFY:
        case CID_OTHER_SERVER_KICK_USER:
        case CID_GROUP_CHANGE_MEMBER_NOTIFY:
        case CID_FILE_NOTIFY:
        case CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
            _BroadcastMsg(pPdu, this);
            break;
        case CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
            _BroadcastMsg(pPdu);
            break;
        
	default:
		log("CRouteConn::HandlePdu, wrong cmd id: %d ", pPdu->GetCommandId());
		break;
	}
}

void CRouteConn::_HandleOnlineUserInfo(CImPdu* pPdu)
{
    IM::Server::IMOnlineUserInfo msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_count = msg.user_stat_list_size();

	log("HandleOnlineUserInfo, user_cnt=%u ", user_count);

	for (uint32_t i = 0; i < user_count; i++) {
        IM::BaseDefine::ServerUserStat server_user_stat = msg.user_stat_list(i);
		_UpdateUserStatus(server_user_stat.user_id(), server_user_stat.status(), server_user_stat.client_type());
	}
}

void CRouteConn::_HandleUserStatusUpdate(CImPdu* pPdu)
{
    IM::Server::IMUserStatusUpdate msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_status = msg.user_status();
	uint32_t user_id = msg.user_id();
    uint32_t client_type = msg.client_type();
	log("HandleUserStatusUpdate, status=%u, uid=%u, client_type=%u ", user_status, user_id, client_type);

	_UpdateUserStatus(user_id, user_status, client_type);
    
    //用于通知客户端,同一用户在pc端的登录情况
    CUserInfo* pUser = GetUserInfo(user_id);
    if (pUser)
    {
        IM::Server::IMServerPCLoginStatusNotify msg2;
        msg2.set_user_id(user_id);
        if (user_status == IM::BaseDefine::USER_STATUS_OFFLINE)
        {
            msg2.set_login_status(IM_PC_LOGIN_STATUS_OFF);
        }
        else
        {
            msg2.set_login_status(IM_PC_LOGIN_STATUS_ON);
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_LOGIN_STATUS_NOTIFY);
        
        if (user_status == USER_STATUS_OFFLINE)
        {
            //pc端下线且无pc端存在，则给msg_server发送一个通知
            if (CHECK_CLIENT_TYPE_PC(client_type) && !pUser->IsPCClientLogin())
            {
                _BroadcastMsg(&pdu);
            }
        }
        else
        {
            //只要pc端在线，则不管上线的是pc还是移动端，都通知msg_server
            if (pUser->IsPCClientLogin())
            {
                _BroadcastMsg(&pdu);
            }
        }
    }
    
    //状态更新的是pc client端，则通知给所有其他人
    if (CHECK_CLIENT_TYPE_PC(client_type))
    {
        IM::Buddy::IMUserStatNotify msg3;
        IM::BaseDefine::UserStat* user_stat = msg3.mutable_user_stat();
        user_stat->set_user_id(user_id);
        user_stat->set_status((IM::BaseDefine::UserStatType)user_status);
        CImPdu pdu2;
        pdu2.SetPBMsg(&msg3);
        pdu2.SetServiceId(SID_BUDDY_LIST);
        pdu2.SetCommandId(CID_BUDDY_LIST_STATUS_NOTIFY);
        
        //用户存在
        if (pUser)
        {
            //如果是pc客户端离线，但是仍然存在pc客户端，则不发送离线通知
            //此种情况一般是pc客户端多点登录时引起
            if (USER_STATUS_OFFLINE == user_status && pUser->IsPCClientLogin())
            {
                return;
            }
            else
            {
                _BroadcastMsg(&pdu2);
            }
        }
        else//该用户不存在了，则表示是离线状态
        {
            _BroadcastMsg(&pdu2);
        }
    }
}

void CRouteConn::_HandleRoleSet(CImPdu* pPdu)
{
    IM::Server::IMRoleSet msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t master = msg.master();

	log("HandleRoleSet, master=%u, handle=%u ", master, m_handle);
	if (master == 1) {
		m_bMaster = true;
	} else {
		m_bMaster = false;
	}
}

void CRouteConn::_HandleUsersStatusRequest(CImPdu* pPdu)
{
    IM::Buddy::IMUsersStatReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t request_id = msg.user_id();
	uint32_t query_count = msg.user_id_list_size();
	log("HandleUserStatusReq, req_id=%u, query_count=%u ", request_id, query_count);

    IM::Buddy::IMUsersStatRsp msg2;
    msg2.set_user_id(request_id);
    msg2.set_attach_data(msg.attach_data());
    list<user_stat_t> result_list;
	user_stat_t status;
    for(uint32_t i = 0; i < query_count; i++)
	{
        IM::BaseDefine::UserStat* user_stat = msg2.add_user_stat_list();
        uint32_t user_id = msg.user_id_list(i);
        user_stat->set_user_id(user_id);
        CUserInfo* pUser = GetUserInfo(user_id);
        if (pUser) {
            user_stat->set_status((::IM::BaseDefine::UserStatType) pUser->GetStatus()) ;
        }
		else
		{
            user_stat->set_status(USER_STATUS_OFFLINE) ;
		}
	}

	// send back query user status
    CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_BUDDY_LIST);
    pdu.SetCommandId(CID_BUDDY_LIST_USERS_STATUS_RESPONSE);
	pdu.SetSeqNum(pPdu->GetSeqNum());
	SendPdu(&pdu);
}

/*
 * update user status info, the logic seems complex
 */
void CRouteConn::_UpdateUserStatus(uint32_t user_id, uint32_t status, uint32_t client_type)
{
    CUserInfo* pUser = GetUserInfo(user_id);
    if (pUser) {
        if (pUser->FindRouteConn(this))
        {
            if (status == USER_STATUS_OFFLINE)
            {
                pUser->RemoveClientType(client_type);
                if (pUser->IsMsgConnNULL())
                {
                    pUser->RemoveRouteConn(this);
                    if (pUser->GetRouteConnCount() == 0) {
                        delete pUser;
                        pUser = NULL;
                        g_user_map.erase(user_id);
                    }
                }
            }
            else
            {
                pUser->AddClientType(client_type);
            }
        }
        else
        {
            if (status != USER_STATUS_OFFLINE)
            {
                pUser->AddRouteConn(this);
                pUser->AddClientType(client_type);
            }
        }
    }
    else
    {
        if (status != USER_STATUS_OFFLINE) {
            CUserInfo* pUserInfo = new CUserInfo();
            if (pUserInfo != NULL) {
                pUserInfo->AddRouteConn(this);
                pUserInfo->AddClientType(client_type);
                g_user_map.insert(make_pair(user_id, pUserInfo));
            }
            else
            {
                log("new UserInfo failed. ");
            }
        }
    }
}

void CRouteConn::_BroadcastMsg(CImPdu* pPdu, CRouteConn* pFromConn)
{
	ConnMap_t::iterator it;
	for (it = g_route_conn_map.begin(); it != g_route_conn_map.end(); it++) {
		CRouteConn* pRouteConn = (CRouteConn*)it->second;
		if (pRouteConn != pFromConn) {
			pRouteConn->SendPdu(pPdu);
		}
	}
}


void CRouteConn::_SendPduToUser(uint32_t user_id, CImPdu* pPdu, bool bAll)
{
    CUserInfo* pUser = GetUserInfo(user_id);
    if (pUser)
    {
        set<CRouteConn*>* pUserSet = pUser->GetRouteConn();
        for (set<CRouteConn*>::iterator it = pUserSet->begin(); it != pUserSet->end(); it++)
        {
            CRouteConn* pToConn = *it;
            if (bAll || pToConn != this)
            {
                pToConn->Send(pPdu->GetBuffer(), pPdu->GetLength());
            }
        }
    }
}

