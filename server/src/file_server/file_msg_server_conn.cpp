//
//  file_conn.cpp
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#include "file_server/file_msg_server_conn.h"

#include "base/pb/protocol/IM.Server.pb.h"
#include "base/pb/protocol/IM.Other.pb.h"

#include "base/im_conn_util.h"

#include "file_server/config_util.h"
#include "file_server/transfer_task.h"
#include "file_server/transfer_task_manager.h"

using namespace IM::BaseDefine;

static ConnMap_t g_file_msg_server_conn_map; // connection with others, on connect insert...

void FileMsgServerConnCallback(void* callback_data, uint8_t msg, uint32_t handle, void* param) {
    if (msg == NETLIB_MSG_CONNECT) {
        FileMsgServerConn* conn = new FileMsgServerConn();
        conn->OnConnect(handle);
    } else {
        log("!!!error msg: %d ", msg);
    }
}

void FileMsgServerConnTimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
    uint64_t cur_time = get_tick_count();
    for (ConnMap_t::iterator it = g_file_msg_server_conn_map.begin(); it != g_file_msg_server_conn_map.end(); ) {
        ConnMap_t::iterator it_old = it;
        it++;
        
        FileMsgServerConn* conn = (FileMsgServerConn*)it_old->second;
        conn->OnTimer(cur_time);
    }
}

void InitializeFileMsgServerConn() {
    netlib_register_timer(FileMsgServerConnTimerCallback, NULL, 1000);
}

FileMsgServerConn::FileMsgServerConn()
    : connected_(false) {
}

FileMsgServerConn::~FileMsgServerConn() {
}

void FileMsgServerConn::Close() {
    log("Close client, handle %d", m_handle);
    
    connected_ = false;
    
    if (m_handle != NETLIB_INVALID_HANDLE) {
        netlib_close(m_handle);
        g_file_msg_server_conn_map.erase(m_handle);
    }
    
    ReleaseRef();
}

void FileMsgServerConn::OnConnect(net_handle_t handle) {
    m_handle = handle;
    
    g_file_msg_server_conn_map.insert(make_pair(handle, this));
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_file_msg_server_conn_map);
}

void FileMsgServerConn::OnClose() {
    log("Client onclose: handle=%d", m_handle);
    Close();
}

void FileMsgServerConn::OnTimer(uint64_t curr_tick) {
    if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
        log("Msg server timeout");
        Close();
    }
}

void FileMsgServerConn::OnWrite() {
    CImConn::OnWrite();
}

void FileMsgServerConn::HandlePdu(CImPdu* pdu) {
    switch (pdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
            _HandleHeartBeat(pdu);
            break;
            
        case CID_OTHER_FILE_TRANSFER_REQ:
            _HandleMsgFileTransferReq(pdu);
            break ;
        case CID_OTHER_FILE_SERVER_IP_REQ:
            _HandleGetServerAddressReq(pdu);
            break;
            
        default:
            log("No such cmd id = %u", pdu->GetCommandId());
            break;
    }
}

void FileMsgServerConn::_HandleHeartBeat(CImPdu *pdu) {
    SendPdu(pdu);
}

void FileMsgServerConn::_HandleMsgFileTransferReq(CImPdu* pdu) {
    IM::Server::IMFileTransferReq transfer_req;
    CHECK_PB_PARSE_MSG(transfer_req.ParseFromArray(pdu->GetBodyData(), pdu->GetBodyLength()));
    
    
    uint32_t from_id = transfer_req.from_user_id();
    uint32_t to_id = transfer_req.to_user_id();
    
    IM::Server::IMFileTransferRsp transfer_rsp;
    transfer_rsp.set_result_code(1);
    transfer_rsp.set_from_user_id(from_id);
    transfer_rsp.set_to_user_id(to_id);
    transfer_rsp.set_file_name(transfer_req.file_name());
    transfer_rsp.set_file_size(transfer_req.file_size());
    transfer_rsp.set_task_id("");
    transfer_rsp.set_trans_mode(transfer_req.trans_mode());
    transfer_rsp.set_attach_data(transfer_req.attach_data());

    
    bool rv = false;
    do {
        std::string task_id = GenerateUUID();
        if (task_id.empty()) {
            log("Create task id failed");
            break;
        }
        log("trams_mode=%d, task_id=%s, from_id=%d, to_id=%d, file_name=%s, file_size=%d", transfer_req.trans_mode(), task_id.c_str(), from_id, to_id, transfer_req.file_name().c_str(), transfer_req.file_size());
        
        BaseTransferTask* transfer_task = TransferTaskManager::GetInstance()->NewTransferTask(
                                                                                              transfer_req.trans_mode(),
                                                                                              task_id,
                                                                                              from_id,
                                                                                              to_id,
                                                                                              transfer_req.file_name(),
                                                                                              transfer_req.file_size());
        
        if (transfer_task == NULL) {
            // 创建未成功
            // close connection with msg svr
            // need_close = true;
            log("Create task failed");
            break;
        }
        
        transfer_rsp.set_result_code(0);
        transfer_rsp.set_task_id(task_id);
        rv = true;
        // need_seq_no = false;
        
        log("Create task succeed, task id %s, task type %d, from user %d, to user %d", task_id.c_str(), transfer_req.trans_mode(), from_id, to_id);
    } while (0);
    
    ::SendMessageLite(this, SID_OTHER, CID_OTHER_FILE_TRANSFER_RSP, pdu->GetSeqNum(), &transfer_rsp);
    
    if (!rv) {
        // 未创建成功，关闭连接
        Close();
    }
}

void FileMsgServerConn::_HandleGetServerAddressReq(CImPdu* pPdu) {
    IM::Server::IMFileServerIPRsp msg;
    
    const std::list<IM::BaseDefine::IpAddr>& addrs = ConfigUtil::GetInstance()->GetAddressList();
    
    for (std::list<IM::BaseDefine::IpAddr>::const_iterator it=addrs.begin(); it!=addrs.end(); ++it) {
        IM::BaseDefine::IpAddr* addr = msg.add_ip_addr_list();
        *addr = *it;
        log("Upload file_client_conn addr info, ip=%s, port=%d", addr->ip().c_str(), addr->port());
    }
    
    SendMessageLite(this, SID_OTHER, CID_OTHER_FILE_SERVER_IP_RSP, pPdu->GetSeqNum(), &msg);
}
