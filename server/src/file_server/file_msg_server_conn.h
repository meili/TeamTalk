//
//  file_conn.h
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#ifndef FILE_SERVER_FILE_MSG_SERVER_CONN_H_
#define FILE_SERVER_FILE_MSG_SERVER_CONN_H_

#include "base/imconn.h"
#include "file_server/file_server_util.h"

typedef map<std::string, transfer_task_t*> TaskMap_t; // on client connect

class FileMsgServerConn : public CImConn {
public:
    FileMsgServerConn();
    virtual ~FileMsgServerConn();
    
    virtual void Close();
    
    virtual void OnConnect(net_handle_t handle);
    
    virtual void OnClose();
    virtual void OnTimer(uint64_t curr_tick);
    
    virtual void OnWrite();
    virtual void HandlePdu(CImPdu* pdu);
    
private:
    void _HandleHeartBeat(CImPdu* pdu);
    void _HandleMsgFileTransferReq(CImPdu* pdu);
    void _HandleGetServerAddressReq(CImPdu* pdu);
    
    bool connected_;
};


void InitializeFileMsgServerConn();
void FileMsgServerConnCallback(void* callback_data, uint8_t msg, uint32_t handle, void* param);



#endif /* defined(FILE_SERVER_FILE_MSG_SERVER_CONN_H_) */
