/*
 * LoginServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */


#include "LoginServConn.h"
#include "MsgConn.h"
#include "ImUser.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "ImPduBase.h"
#include "public_define.h"
using namespace IM::BaseDefine;

static ConnMap_t g_login_server_conn_map;

static serv_info_t* g_login_server_list;
static uint32_t	g_login_server_count;

static string g_msg_server_ip_addr1;
static string g_msg_server_ip_addr2;
static uint16_t g_msg_server_port;
static uint32_t g_max_conn_cnt;

void login_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CLoginServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_login_server_conn_map.begin(); it != g_login_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CLoginServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	// reconnect LoginServer
	serv_check_reconnect<CLoginServConn>(g_login_server_list, g_login_server_count);
}

void init_login_serv_conn(serv_info_t* server_list, uint32_t server_count, const char* msg_server_ip_addr1,
		const char* msg_server_ip_addr2, uint16_t msg_server_port, uint32_t max_conn_cnt)
{
	g_login_server_list = server_list;
	g_login_server_count = server_count;

	serv_init<CLoginServConn>(g_login_server_list, g_login_server_count);

	g_msg_server_ip_addr1 = msg_server_ip_addr1;
	g_msg_server_ip_addr2 = msg_server_ip_addr2;
	g_msg_server_port = msg_server_port;
	g_max_conn_cnt = max_conn_cnt;

	netlib_register_timer(login_server_conn_timer_callback, NULL, 1000);
}

// if there is one LoginServer available, return true
bool is_login_server_available()
{
	CLoginServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_login_server_count; i++) {
		pConn = (CLoginServConn*)g_login_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_login_server(CImPdu* pPdu)
{
	CLoginServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_login_server_count; i++) {
		pConn = (CLoginServConn*)g_login_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			pConn->SendPdu(pPdu);
		}
	}
}


CLoginServConn::CLoginServConn()
{
	m_bOpen = false;
}

CLoginServConn::~CLoginServConn()
{

}

void CLoginServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx)
{
	log("Connecting to LoginServer %s:%d ", server_ip, server_port);
	m_serv_idx = serv_idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_login_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_login_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CLoginServConn::Close()
{
	serv_reset<CLoginServConn>(g_login_server_list, g_login_server_count, m_serv_idx);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_login_server_conn_map.erase(m_handle);
	}

	ReleaseRef();
}

void CLoginServConn::OnConfirm()
{
	log("connect to login server success ");
	m_bOpen = true;
	g_login_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

	uint32_t cur_conn_cnt = 0;
	uint32_t shop_user_cnt = 0;
    
    list<user_conn_t> user_conn_list;
    CImUserManager::GetInstance()->GetUserConnCnt(&user_conn_list, cur_conn_cnt);
	char hostname[256] = {0};
	gethostname(hostname, 256);
    IM::Server::IMMsgServInfo msg;
    msg.set_ip1(g_msg_server_ip_addr1);
    msg.set_ip2(g_msg_server_ip_addr2);
    msg.set_port(g_msg_server_port);
    msg.set_max_conn_cnt(g_max_conn_cnt);
    msg.set_cur_conn_cnt(cur_conn_cnt);
    msg.set_host_name(hostname);
    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_MSG_SERV_INFO);
	SendPdu(&pdu);
}

void CLoginServConn::OnClose()
{
	log("login server conn onclose, from handle=%d ", m_handle);
	Close();
}

void CLoginServConn::OnTimer(uint64_t curr_tick)
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
		log("conn to login server timeout ");
		Close();
	}
}

void CLoginServConn::HandlePdu(CImPdu* pPdu)
{
	//printf("recv pdu_type=%d ", pPdu->GetPduType());
}
