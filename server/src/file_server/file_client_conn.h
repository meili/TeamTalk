//
//  file_client_conn.h
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#ifndef FILE_SERVER_FILE_CLIENT_CONN_H_
#define FILE_SERVER_FILE_CLIENT_CONN_H_

#include "base/imconn.h"
#include "file_server/transfer_task.h"

// TODO
// 异常情况处理
// 断线、服务器重启等
class FileClientConn : public CImConn {
public:
    FileClientConn()
        : auth_(false),
          user_id_(0),
          transfer_task_(NULL) {
    }
    
    virtual ~FileClientConn() { }
    
    virtual void Close();
    
    // void Close2();
    
    virtual void OnConnect(net_handle_t handle);
    virtual void OnClose();
    virtual void OnTimer(uint64_t curr_tick);
    
    virtual void OnWrite();
    virtual void HandlePdu(CImPdu* pdu);
    
    void ClearTransferTask() {
        user_id_ = 0;
        transfer_task_ = NULL;
    }
    
private:
    void _HandleHeartBeat(CImPdu* pdu);
    
    void _HandleClientFileLoginReq(CImPdu* pdu);
    void _HandleClientFileStates(CImPdu* pdu);
    void _HandleClientFilePullFileReq(CImPdu* pdu);
    void _HandleClientFilePullFileRsp(CImPdu *pdu);
    
    int _StatesNotify(int state, const std::string& task_id, uint32_t user_id, CImConn* conn);
    
    // bool _IsAuth() const { return auth_; }
    
    /// yunfan add 2014.8.18
private:
    int _PreUpload(const char* task_id);
    //  int _DoUpload(const char* task_id);
    /// yunan add end
    
    bool		auth_;
    
    uint32_t	user_id_;
    // 当前设计每个连接对应一次任务，故可预先缓存
    BaseTransferTask* transfer_task_;
};

void InitializeFileClientConn();
void FileClientConnCallback(void* callback_data, uint8_t msg, uint32_t handle, void* param);


#endif /* defined(FILE_SERVER_FILE_CLIENT_CONN_H_) */
