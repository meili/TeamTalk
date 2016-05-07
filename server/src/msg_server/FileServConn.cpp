//
//  FileServConn.cpp
//  public_TTServer
//
//  Created by luoning on 14-8-19.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "FileServConn.h"
#include "FileHandler.h"
#include "util.h"
#include "ImUser.h"
#include "AttachData.h"
#include "RouteServConn.h"
#include "MsgConn.h"
#include "IM.Server.pb.h"
#include "IM.Other.pb.h"
#include "IM.File.pb.h"
using namespace IM::BaseDefine;
static ConnMap_t g_file_server_conn_map;

static serv_info_t* g_file_server_list;
static uint32_t g_file_server_count;
static CFileHandler* s_file_handler = NULL;

void file_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CFileServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();
    
	for (ConnMap_t::iterator it = g_file_server_conn_map.begin(); it != g_file_server_conn_map.end();
         )
    {
        it_old = it;
        it++;
		pConn = (CFileServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
    
	// reconnect FileServer
	serv_check_reconnect<CFileServConn>(g_file_server_list, g_file_server_count);
}

void init_file_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_file_server_list = server_list;
	g_file_server_count = server_count;
    
	serv_init<CFileServConn>(g_file_server_list, g_file_server_count);
    
	netlib_register_timer(file_server_conn_timer_callback, NULL, 1000);
	s_file_handler = CFileHandler::getInstance();
}

bool is_file_server_available()
{
	CFileServConn* pConn = NULL;
    
	for (uint32_t i = 0; i < g_file_server_count; i++) {
		pConn = (CFileServConn*)g_file_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}
	return false;
}

//
CFileServConn* get_random_file_serv_conn()
{
    CFileServConn* pConn = NULL;
    CFileServConn* pConnTmp = NULL;
    if (0 == g_file_server_count) {
        return pConn;
    }
    int32_t random_num = rand() % g_file_server_count;
    pConnTmp = (CFileServConn*)g_file_server_list[random_num].serv_conn;
    if (pConnTmp && pConnTmp->IsOpen())
    {
        pConn = pConnTmp;
    }
    else
    {
        for (uint32_t i = 0; i < g_file_server_count; i++)
        {
            int j = (random_num + i + 1) % g_file_server_count;
            pConnTmp = (CFileServConn*)g_file_server_list[j].serv_conn;
            if (pConnTmp && pConnTmp->IsOpen())
            {
                pConn = pConnTmp;
                break;
            }
        }
    }
	return pConn;
}


CFileServConn::CFileServConn()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CFileServConn::~CFileServConn()
{
}

void CFileServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	log("Connecting to FileServer %s:%d ", server_ip, server_port);
    
	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_file_server_conn_map);
    
	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_file_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CFileServConn::Close()
{
	serv_reset<CFileServConn>(g_file_server_list, g_file_server_count, m_serv_idx);
    
	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_file_server_conn_map.erase(m_handle);
	}
    
	ReleaseRef();
}

void CFileServConn::OnConfirm()
{
	log("connect to file server success ");
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_file_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
    
    IM::Server::IMFileServerIPReq msg;
    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_FILE_SERVER_IP_REQ);
    SendPdu(&pdu);
}

void CFileServConn::OnClose()
{
	log("onclose from file server handle=%d ", m_handle);
	Close();
}

void CFileServConn::OnTimer(uint64_t curr_tick)
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
		log("conn to file server timeout ");
		Close();
	}
}

void CFileServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            break;
        case CID_OTHER_FILE_TRANSFER_RSP:
            _HandleFileMsgTransRsp(pPdu);
            break;
        case CID_OTHER_FILE_SERVER_IP_RSP:
            _HandleFileServerIPRsp(pPdu);
            break;
        default:
            log("unknown cmd id=%d ", pPdu->GetCommandId());
            break;
	}
}

void CFileServConn::_HandleFileMsgTransRsp(CImPdu* pPdu)
{
    IM::Server::IMFileTransferRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t result = msg.result_code();
    uint32_t from_id = msg.from_user_id();
    uint32_t to_id = msg.to_user_id();
    string file_name = msg.file_name();
    uint32_t file_size = msg.file_size();
    string task_id = msg.task_id();
    uint32_t trans_mode = msg.trans_mode();
    CDbAttachData attach((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    log("HandleFileMsgTransRsp, result: %u, from_user_id: %u, to_user_id: %u, file_name: %s, \
        task_id: %s, trans_mode: %u. ", result, from_id, to_id,
        file_name.c_str(), task_id.c_str(), trans_mode);

    const list<IM::BaseDefine::IpAddr>* ip_addr_list = GetFileServerIPList();

    IM::File::IMFileRsp msg2;
    msg2.set_result_code(result);
    msg2.set_from_user_id(from_id);
    msg2.set_to_user_id(to_id);
    msg2.set_file_name(file_name);
    msg2.set_task_id(task_id);
    msg2.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
    for (list<IM::BaseDefine::IpAddr>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
    {
        IM::BaseDefine::IpAddr ip_addr_tmp = *it;
        IM::BaseDefine::IpAddr* ip_addr = msg2.add_ip_addr_list();
        ip_addr->set_ip(ip_addr_tmp.ip());
        ip_addr->set_port(ip_addr_tmp.port());
    }
    CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_FILE);
    pdu.SetCommandId(CID_FILE_RESPONSE);
    pdu.SetSeqNum(pPdu->GetSeqNum());
    uint32_t handle = attach.GetHandle();
    
    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_id, handle);
    if (pFromConn)
    {
        pFromConn->SendPdu(&pdu);
    }
    
    if (result == 0)
    {
        IM::File::IMFileNotify msg3;
        msg3.set_from_user_id(from_id);
        msg3.set_to_user_id(to_id);
        msg3.set_file_name(file_name);
        msg3.set_file_size(file_size);
        msg3.set_task_id(task_id);
        msg3.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
        msg3.set_offline_ready(0);
        for (list<IM::BaseDefine::IpAddr>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            IM::BaseDefine::IpAddr ip_addr_tmp = *it;
            IM::BaseDefine::IpAddr* ip_addr = msg3.add_ip_addr_list();
            ip_addr->set_ip(ip_addr_tmp.ip());
            ip_addr->set_port(ip_addr_tmp.port());
        }
        CImPdu pdu2;
        pdu2.SetPBMsg(&msg3);
        pdu2.SetServiceId(SID_FILE);
        pdu2.SetCommandId(CID_FILE_NOTIFY);
        
        //send notify to target user
        CImUser* pToUser = CImUserManager::GetInstance()->GetImUserById(to_id);
        if (pToUser)
        {
            pToUser->BroadcastPduWithOutMobile(&pdu2);
        }
        
        //send to route server
        CRouteServConn* pRouteConn = get_route_serv_conn();
        if (pRouteConn) {
            pRouteConn->SendPdu(&pdu2);
        }
    }
}

void CFileServConn::_HandleFileServerIPRsp(CImPdu* pPdu)
{
    IM::Server::IMFileServerIPRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    uint32_t ip_addr_cnt = msg.ip_addr_list_size();
    
    for (uint32_t i = 0; i < ip_addr_cnt ; i++)
    {
        IM::BaseDefine::IpAddr ip_addr = msg.ip_addr_list(i);
        log("_HandleFileServerIPRsp -> %s : %d ", ip_addr.ip().c_str(), ip_addr.port());
        m_ip_list.push_back(ip_addr);
    }
}
