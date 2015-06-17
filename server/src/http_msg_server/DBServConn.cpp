/*
 * DBServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "DBServConn.h"
#include "RouteServConn.h"
#include "public_define.h"
#include "AttachData.h"
#include "HttpConn.h"
#include "HttpPdu.h"
#include "HttpQuery.h"
#include "IM.Other.pb.h"
#include "IM.Message.pb.h"
#include "IM.Buddy.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "IM.Group.pb.h"
#include <list>
using namespace std;

using namespace IM::BaseDefine;

extern hash_map<string, auth_struct*> g_hm_http_auth;
bool g_bOnSync = false;

uint64_t    g_last_recv_auth = 0;
uint32_t    g_latest_auth = 0;

namespace HTTP {
    
#define SERVER_TIMEOUT				30000
    
static ConnMap_t g_db_server_conn_map;

static serv_info_t* g_db_server_list = NULL;
static uint32_t		g_db_server_count = 0;
static uint32_t		g_db_server_login_count = 0;	// 到进行登录处理的DBServer的总连接数
    
    
    
static void db_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CDBServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_db_server_conn_map.begin(); it != g_db_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CDBServConn*)it_old->second;
		if (pConn->IsOpen()) {
			pConn->OnTimer(cur_time);
		}
	}

	// reconnect DB Storage Server
	// will reconnect in 4s, 8s, 16s, 32s, 64s, 4s 8s ...
	serv_check_reconnect<CDBServConn>(g_db_server_list, g_db_server_count);
}

void init_db_serv_conn(serv_info_t* server_list, uint32_t server_count, uint32_t concur_conn_cnt)
{
	g_db_server_list = server_list;
	g_db_server_count = server_count;

	uint32_t total_db_instance = server_count / concur_conn_cnt;

	// 必须至少配置2个BusinessServer实例, 一个用于用户登录业务，一个用于其他业务
	// 这样当其他业务量非常繁忙时，也不会影响客服端的登录验证
	// 建议配置4个实例，这样更新BusinessServer时，不会影响业务
	if (total_db_instance < 2) {
		log("DBServerIP need 2 instance at lest");
		exit(1);
	}

	g_db_server_login_count = (total_db_instance / 2) * concur_conn_cnt;
	log("DB server connection index for login business: [0, %u), for other business: [%u, %u)",
			g_db_server_login_count, g_db_server_login_count, g_db_server_count);

	serv_init<CDBServConn>(g_db_server_list, g_db_server_count);

	netlib_register_timer(db_server_conn_timer_callback, NULL, 1000);
}

// get a random db server connection in the range [start_pos, stop_pos)
static CDBServConn* get_db_server_conn_in_range(uint32_t start_pos, uint32_t stop_pos)
{
	uint32_t i = 0;
	CDBServConn* pDbConn = NULL;

	// determine if there is a valid DB server connection
	for (i = start_pos; i < stop_pos; i++) {
		pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
		if (pDbConn && pDbConn->IsOpen()) {
			break;
		}
	}

	// no valid DB server connection
	if (i == stop_pos) {
		return NULL;
	}

	// return a random valid DB server connection
	while (true) {
		int i = rand() % (stop_pos - start_pos) + start_pos;
		pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
		if (pDbConn && pDbConn->IsOpen()) {
			break;
		}
	}

	return pDbConn;
}

CDBServConn* get_db_serv_conn_for_login()
{
	// 先获取login业务的实例，没有就去获取其他业务流程的实例
	CDBServConn* pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
	if (!pDBConn) {
		pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
	}

	return pDBConn;
}

CDBServConn* get_db_serv_conn()
{
	// 先获取其他业务流程的实例，没有就去获取login业务的实例
	CDBServConn* pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
	if (!pDBConn) {
		pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
	}

	return pDBConn;
}


CDBServConn::CDBServConn()
{
	m_bOpen = false;
}

CDBServConn::~CDBServConn()
{

}

void CDBServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx)
{
	log("Connecting to DB Storage Server %s:%d", server_ip, server_port);

	m_serv_idx = serv_idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_db_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_db_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CDBServConn::Close()
{
	// reset server information for the next connect
	serv_reset<CDBServConn>(g_db_server_list, g_db_server_count, m_serv_idx);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_db_server_conn_map.erase(m_handle);
	}

	ReleaseRef();
}

void CDBServConn::OnConfirm()
{
	log("connect to db server success");
	m_bOpen = true;
	g_db_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
}

void CDBServConn::OnClose()
{
	log("onclose from db server handle=%d", m_handle);
	Close();
}

void CDBServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
        IM::Other::IMHeartBeat msg;
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(IM::BaseDefine::SID_OTHER);
        pdu.SetCommandId(IM::BaseDefine::CID_OTHER_HEARTBEAT);
		SendPdu(&pdu);
	}
	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to db server timeout");
		Close();
	}
}

void CDBServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            break;
        case CID_OTHER_STOP_RECV_PACKET:
            _HandleStopReceivePacket(pPdu);
            break;
        case CID_GROUP_CREATE_RESPONSE:
            _HandleCreateGroupRsp(pPdu);
            break;
        case CID_GROUP_CHANGE_MEMBER_RESPONSE:
            _HandleChangeMemberRsp(pPdu);
            break;
        default:
            log("db server, wrong cmd id=%d", pPdu->GetCommandId());
	}
}

void CDBServConn::_HandleStopReceivePacket(CImPdu* pPdu)
{
	log("HandleStopReceivePacket, from %s:%d",
			g_db_server_list[m_serv_idx].server_ip.c_str(), g_db_server_list[m_serv_idx].server_port);

	m_bOpen = false;
}
    
void CDBServConn::_HandleCreateGroupRsp(CImPdu *pPdu)
{
    IM::Group::IMGroupCreateRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    uint32_t user_id = msg.user_id();
    string group_name = msg.group_name();
    uint32_t result_code = msg.result_code();
    uint32_t group_id = 0;
    if (msg.has_group_id()) {
        group_id = msg.group_id();
    }
    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    uint32_t http_handle = attach_data.GetHandle();
    CHttpConn* pHttpConn = FindHttpConnByHandle(http_handle);
    if(!pHttpConn)
    {
        log("no http connection");
        return;
    }
    log("HandleCreateGroupRsp, req_id=%u, group_name=%s, result=%u", user_id, group_name.c_str(),result_code);
    
    char* response_buf = NULL;
    if (result_code != 0)
    {
        response_buf = PackSendCreateGroupResult(HTTP_ERROR_CREATE_GROUP, HTTP_ERROR_MSG[10].c_str(), group_id);
    }
    else
    {
        response_buf = PackSendCreateGroupResult(HTTP_ERROR_SUCCESS, HTTP_ERROR_MSG[0].c_str(), group_id);
    }
    pHttpConn->Send(response_buf, (uint32_t)strlen(response_buf));
    pHttpConn->Close();
    
}

void CDBServConn::_HandleChangeMemberRsp(CImPdu *pPdu)
{
    IM::Group::IMGroupChangeMemberRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t change_type = msg.change_type();
    uint32_t user_id = msg.user_id();
    uint32_t result = msg.result_code();
    uint32_t group_id = msg.group_id();
    uint32_t chg_user_cnt = msg.chg_user_id_list_size();
    uint32_t cur_user_cnt = msg.cur_user_id_list_size();
    log("HandleChangeMemberResp, change_type=%u, req_id=%u, group_id=%u, result=%u, chg_usr_cnt=%u, cur_user_cnt=%u.",
        change_type, user_id, group_id, result, chg_user_cnt, cur_user_cnt);
    CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    uint32_t http_handle = attach_data.GetHandle();
    CHttpConn* pHttpConn = FindHttpConnByHandle(http_handle);
    if(!pHttpConn)
    {
        log("no http connection.");
        return;
    }
    char* response_buf = NULL;
    if (result != 0)
    {
        response_buf = PackSendResult(HTTP_ERROR_CHANGE_MEMBER, HTTP_ERROR_MSG[11].c_str());
    }
    else
    {
        response_buf = PackSendResult(HTTP_ERROR_SUCCESS, HTTP_ERROR_MSG[0].c_str());
    }
    pHttpConn->Send(response_buf, (uint32_t)strlen(response_buf));
    pHttpConn->Close();
    
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
    }
}
    
};
