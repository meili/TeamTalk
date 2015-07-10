//
//  PushServConn.cpp
//  im-server-TT
//
//  Created by luoning on 14-9-15.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "PushServConn.h"
#include "ImUser.h"
#include "IM.Server.pb.h"
#include "IM.Other.pb.h"
#include "IM.BaseDefine.pb.h"
using namespace IM::BaseDefine;

#define IOS_PUSH_FLASH_MAX_LENGTH    40

static ConnMap_t g_push_server_conn_map;
static CPushServConn* g_master_push_conn = NULL;

static serv_info_t* g_push_server_list = NULL;
static uint32_t		g_push_server_count = 0;			// 到PushServer的总连接数

static void push_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CPushServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();
    
	for (ConnMap_t::iterator it = g_push_server_conn_map.begin(); it != g_push_server_conn_map.end(); ) {
		it_old = it;
		it++;
        
		pConn = (CPushServConn*)it_old->second;
		if (pConn->IsOpen()) {
			pConn->OnTimer(cur_time);
		}
	}
    
	// reconnect Push Server
	// will reconnect in 4s, 8s, 16s, 32s, 64s, 4s 8s ...
	serv_check_reconnect<CPushServConn>(g_push_server_list, g_push_server_count);
}

void init_push_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_push_server_list = server_list;
	g_push_server_count = server_count;
    
	serv_init<CPushServConn>(g_push_server_list, g_push_server_count);
    
	netlib_register_timer(push_server_conn_timer_callback, NULL, 1000);
}

void build_ios_push_flash(string& flash, uint32_t msg_type, uint32_t from_id)
{
    string pic_prefix = "&$#@~^@[{:";
    string pic_suffix = ":}]&$~@#@";
    size_t pos_prefix = flash.find(pic_prefix);
    size_t pos_suffix = flash.find(pic_suffix);
    
    string comm_flash = "您收到了一条消息";
    //如果是带有图片链接的话，将消息体改变
    if (pos_prefix != string::npos && pos_suffix != string::npos && pos_prefix < pos_suffix) {
        flash = comm_flash;
    }
    else
    {
        CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(from_id);
        if (pImUser)
        {
            string nick_name = pImUser->GetNickName();
            string msg_tmp;
            if (msg_type == IM::BaseDefine::MSG_TYPE_GROUP_AUDIO )
            {
                msg_tmp.append(nick_name);
                msg_tmp.append("在群聊中发送了一条语音消息");
            }
            else if (msg_type == IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO)
            {
                msg_tmp.append(nick_name);
                msg_tmp.append("给您发送了一条语音消息");
            }
            else
            {
                msg_tmp.append(nick_name);
                msg_tmp.append(":");
                msg_tmp.append(flash);
            }
            flash = msg_tmp;
            uint32_t flash_max_length = IOS_PUSH_FLASH_MAX_LENGTH - 3;
            if (flash.length() > flash_max_length)
            {
                string flash_tmp = flash.substr(0, flash_max_length);
                flash_tmp.append("...");
                flash = flash_tmp;
            }
        }
        else
        {
            flash = comm_flash;
        }
    }
}

CPushServConn* get_push_serv_conn()
{
    CPushServConn* push_conn = NULL;
	if (g_master_push_conn && g_master_push_conn->IsOpen()) {
        push_conn = g_master_push_conn;
    }
	return push_conn;
}


CPushServConn::CPushServConn()
{
	m_bOpen = false;
}

CPushServConn::~CPushServConn()
{
    
}

void CPushServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx)
{
	log("Connecting to Push Server %s:%d ", server_ip, server_port);
    
	m_serv_idx = serv_idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_push_server_conn_map);
    
	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_push_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CPushServConn::Close()
{
	// reset server information for the next connect
	serv_reset<CPushServConn>(g_push_server_list, g_push_server_count, m_serv_idx);
    
    m_bOpen = false;
    g_master_push_conn = NULL;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_push_server_conn_map.erase(m_handle);
	}
    
	ReleaseRef();
}

void CPushServConn::OnConfirm()
{
	log("connect to push server success ");
	m_bOpen = true;
	g_push_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
    g_master_push_conn = this;
}

void CPushServConn::OnClose()
{
	log("onclose from push server handle=%d ", m_handle);
	Close();
}

void CPushServConn::OnTimer(uint64_t curr_tick)
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
		log("conn to push server timeout ");
		Close();
	}
}

void CPushServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            //log("push server heart beat. ");
            break;
        case CID_OTHER_PUSH_TO_USER_RSP:
            _HandlePushToUserResponse( pPdu );
            break;
        default:
            log("push server, wrong cmd id=%d ", pPdu->GetCommandId());
	}
}

void CPushServConn::_HandlePushToUserResponse(CImPdu *pPdu)
{
    // uint32_t result_cnt = pPdu->GetUserCnt();
    // push_result_t* push_result_list = pPdu->GetUserTokenList();
    log("HandlePushToUserResponse ");
}
