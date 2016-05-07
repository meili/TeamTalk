//
//  transfer_task.cpp
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#include "file_server/transfer_task.h"

#include <uuid/uuid.h>

#include "base/util.h"
#include "base/pb/protocol/IM.BaseDefine.pb.h"

// static char g_current_save_path[BUFSIZ];

using namespace IM::BaseDefine;

std::string GenerateUUID() {
    std::string rv;
    
    uuid_t uid = {0};
    uuid_generate(uid);
    if (!uuid_is_null(uid)) {
        char str_uuid[64] = {0};
        uuid_unparse(uid, str_uuid);
        rv = str_uuid;
    }
    
    return rv;
}

const char* GetCurrentOfflinePath() {
    static const char* g_current_save_path = NULL;
    
    if (g_current_save_path == NULL) {
        static char s_tmp[BUFSIZ];
        char work_path[BUFSIZ];
        if(!getcwd(work_path, BUFSIZ)) {
            log("getcwd %s failed", work_path);
        } else {
            snprintf(s_tmp, BUFSIZ, "%s/offline_file", work_path);
        }
        
        log("save offline files to %s", s_tmp);
        
        int ret = mkdir(s_tmp, 0755);
        if ( (ret != 0) && (errno != EEXIST) ) {
            log("!!!Mkdir %s failed to save offline files", s_tmp);
        }
        
        g_current_save_path = s_tmp;
    }
    return g_current_save_path;
}

static FILE* OpenByRead(const std::string& task_id, uint32_t user_id) {
    FILE* fp = NULL;
    if (task_id.length()>=2) {
        char save_path[BUFSIZ];
        snprintf(save_path, BUFSIZ, "%s/%s/%s", GetCurrentOfflinePath(), task_id.substr(0, 2).c_str() , task_id.c_str());
        fp = fopen(save_path, "rb");  // save fp
        if (!fp) {
            log("Open file %s for read failed", save_path);
        }
    }
    return fp;
}

static FILE* OpenByWrite(const std::string& task_id, uint32_t user_id) {
    FILE* fp = NULL;
    if (task_id.length()>=2) {

        char save_path[BUFSIZ];

        snprintf(save_path, BUFSIZ, "%s/%s", GetCurrentOfflinePath(), task_id.substr(0, 2).c_str());
        int ret = mkdir(save_path, 0755);
        if ( (ret != 0) && (errno != EEXIST) ) {
            log("Mkdir failed for path: %s", save_path);
        } else {
            // save as g_current_save_path/to_id_url/task_id
            strncat(save_path, "/", BUFSIZ);
            strncat(save_path, task_id.c_str(), BUFSIZ);
            
            fp = fopen(save_path, "ab+");
            if (!fp) {
                log("Open file for write failed");
                //break;
            }
        }
    }
    
    return fp;
}


//----------------------------------------------------------------------------
BaseTransferTask::BaseTransferTask(const std::string& task_id, uint32_t from_user_id, uint32_t to_user_id, const std::string& file_name, uint32_t file_size)
    : task_id_(task_id),
      from_user_id_(from_user_id),
      to_user_id_(to_user_id),
      file_name_(file_name),
      file_size_(file_size),
      state_(kTransferTaskStateReady) {
    
    create_time_ = time(NULL);
          
    from_conn_ = NULL;
    to_conn_ = NULL;
          
    // last_update_time_ = get_tick_count();
}

void BaseTransferTask::SetLastUpdateTime() {
    // last_update_time_ = get_tick_count();
    
    create_time_ = time(NULL);
}

//----------------------------------------------------------------------------
uint32_t OnlineTransferTask::GetTransMode() const {
    return IM::BaseDefine::FILE_TYPE_ONLINE;
}

bool OnlineTransferTask::ChangePullState(uint32_t user_id, int file_role) {
    // 在线文件传输，初始状态：kTransferTaskStateReady
    //  状态转换流程 kTransferTaskStateReady
    //        --> kTransferTaskStateWaitingSender或kTransferTaskStateWaitingReceiver
    //        --> kTransferTaskStateWaitingTransfer
    //
    
    bool rv = false;
    
    do {
        rv = CheckByUserIDAndFileRole(user_id, file_role);
        if (!rv) {
            //
            log("Check error! user_id=%d, file_role=%d", user_id, file_role);
            break;
        }
        
        if (state_ != kTransferTaskStateReady && state_ != kTransferTaskStateWaitingSender && state_ != kTransferTaskStateWaitingReceiver) {
            log("Invalid state, valid state is kTransferTaskStateReady or kTransferTaskStateWaitingSender or kTransferTaskStateWaitingReceiver, but state is %d", state_);
            break;
        }
        
        if (state_ == kTransferTaskStateReady) {
            // 第一个用户进来
            // 如果是sender，则-->kTransferTaskStateWaitingReceiver
            // 如果是receiver，则-->kTransferTaskStateWaitingSender
            if (file_role == CLIENT_REALTIME_SENDER) {
                state_ = kTransferTaskStateWaitingReceiver;
            } else {
                state_ = kTransferTaskStateWaitingSender;
            }
        } else {
            if (state_ == kTransferTaskStateWaitingReceiver) {
                // 此时必须是receiver
                // 需要检查是否是receiver
                if (file_role != CLIENT_REALTIME_RECVER) {
                    log("Invalid user, user_id = %d, but to_user_id_ = %d", user_id, to_user_id_);
                    break;
                }
            } else if (state_ == kTransferTaskStateWaitingSender) {
                // 此时必须是sender
                // 需要检查是否是sender
                if (file_role != CLIENT_REALTIME_SENDER) {
                    log("Invalid user, user_id = %d, but to_user_id_ = %d", user_id, to_user_id_);
                    break;
                }
            }
            
            state_ = kTransferTaskStateWaitingTransfer;
            
        }
        
        SetLastUpdateTime();
        rv = true;
    } while (0);
    
    return rv;
}

bool OnlineTransferTask::CheckByUserIDAndFileRole(uint32_t user_id, int file_role) const {
    // 在线文件传输
    // 1. file_role必须是CLIENT_REALTIME_SENDER或CLIENT_REALTIME_RECEIVER
    // 2. CLIENT_REALTIME_SENDER则user_id==from_user_id_
    // 3. CLIENT_REALTIME_RECVER则user_id==to_user_id_
    
    bool rv = false;
    
    if (file_role == CLIENT_REALTIME_SENDER){
        if (CheckFromUserID(user_id)) {
            rv = true;
        }
    } else if (file_role == CLIENT_REALTIME_RECVER) {
        if (CheckToUserID(user_id)) {
            rv = true;
        }
    }
    
    return rv;
}

int OnlineTransferTask::DoRecvData(uint32_t user_id, uint32_t offset, const char* data, uint32_t data_size) {
    int rv = -1;
    
    do {
        // 检查是否发送者
        if (!CheckFromUserID(user_id)) {
            log("Check error! user_id=%d, from_user_id=%d, to_user_id", user_id, from_user_id_, to_user_id_);
            break;
        }

        // 检查状态
        if (state_ != kTransferTaskStateWaitingTransfer && state_ != kTransferTaskStateTransfering) {
            log("Check state_! user_id=%d, state=%d, but state need kTransferTaskStateWaitingTransfer or kTransferTaskStateTransfering", user_id, state_);
            break;
        }

        // todo
        // 检查文件大小

        if (state_ == kTransferTaskStateWaitingTransfer) {
            state_ = kTransferTaskStateTransfering;
        }
        
        SetLastUpdateTime();

        rv = 0;
    } while (0);
    
    return rv;
}

int OnlineTransferTask::DoPullFileRequest(uint32_t user_id, uint32_t offset, uint32_t data_size, std::string* data) {
    int rv = -1;
    
    // 在线
    do {
        // 1. 检查状态
        if (state_ != kTransferTaskStateWaitingTransfer && state_ != kTransferTaskStateTransfering) {
            log("Check state_! user_id=%d, state=%d, but state need kTransferTaskStateWaitingTransfer or kTransferTaskStateTransfering", user_id, state_);
            break;
        }
        
        if (state_ == kTransferTaskStateWaitingTransfer) {
            state_ = kTransferTaskStateTransfering;
        }
   
        SetLastUpdateTime();
        rv = 0;
        
    } while (0);

    return rv;
}

//----------------------------------------------------------------------------
OfflineTransferTask* OfflineTransferTask::LoadFromDisk(const std::string& task_id, uint32_t user_id) {
    OfflineTransferTask* offline = NULL;
    
    FILE* fp = OpenByRead(task_id, user_id);
    if (fp) {
        OfflineFileHeader file_header;
        size_t size = fread(&file_header, 1, sizeof(file_header), fp);
        if (size==sizeof(file_header)) {
            fseek(fp, 0L, SEEK_END);
            size_t file_size = static_cast<size_t>(ftell(fp))-size;
            if (file_size == file_header.get_file_size()) {
                offline = new OfflineTransferTask(file_header.get_task_id(),
                                                  file_header.get_from_user_id(),
                                                  file_header.get_to_user_id(),
                                                  file_header.get_file_name(),
                                                  file_header.get_file_size());
                if (offline) {
                    offline->set_state(kTransferTaskStateWaitingDownload);
                }
            } else {
                log("Offile file size by task_id=%s, user_id=%u, header_file_size=%u, disk_file_size=%u", task_id.c_str(), user_id, file_header.get_file_size(), file_size);
            }
        } else {
            log("Read file_header error by task_id=%s, user_id=%u", task_id.c_str(), user_id);
        }
        fclose(fp);
    }
    
    return offline;
}


uint32_t OfflineTransferTask::GetTransMode() const {
    return IM::BaseDefine::FILE_TYPE_OFFLINE;
}

bool OfflineTransferTask::ChangePullState(uint32_t user_id, int file_role) {
    // 离线文件传输
    // 1. 如果是发送者，状态转换 kTransferTaskStateReady－->kTransferTaskStateWaitingUpload
    // 2. 如果是接收者，状态转换 kTransferTaskStateUploadEnd --> kTransferTaskStateWaitingDownload

//    if (CheckFromUserID(user_id)) {
//        // 如果是发送者
//        // 当前状态必须为kTransferTaskStateReady
//        if () {
//
//        }
//    } else {
//        // 如果是接收者
//    }
    
    bool rv = false;
    
    do {
        rv = CheckByUserIDAndFileRole(user_id, file_role);
        if (!rv) {
            //
            log("Check error! user_id=%d, file_role=%d", user_id, file_role);
            break;
        }
        
        if (state_ != kTransferTaskStateReady &&
                state_ != kTransferTaskStateUploadEnd &&
                state_ != kTransferTaskStateWaitingDownload) {
            
            log("Invalid state, valid state is kTransferTaskStateReady or kTransferTaskStateUploadEnd, but state is %d", state_);
            break;
        }
        
        if (state_ == kTransferTaskStateReady) {
            // 第一个用户进来，必须是CLIENT_OFFLINE_UPLOAD
            // 必须是kTransferTaskStateReady，则-->kTransferTaskStateWaitingUpload
            if (CLIENT_OFFLINE_UPLOAD == file_role) {
                state_ = kTransferTaskStateWaitingUpload;
            } else {
                log("Offline upload: file_role is CLIENT_OFFLINE_UPLOAD but file_role = %d", file_role);
                break;
                // state_ = kTransferTaskStateWaitingSender;
            }
        } else {
            if (file_role == CLIENT_OFFLINE_DOWNLOAD) {
                state_ = kTransferTaskStateWaitingDownload;
            } else {
                log("Offline upload: file_role is CLIENT_OFFLINE_DOWNLOAD but file_role = %d", file_role);
                break;
            }
        }
        
        SetLastUpdateTime();
        rv = true;
    } while (0);
    
    return rv;
}

bool OfflineTransferTask::CheckByUserIDAndFileRole(uint32_t user_id, int file_role) const {
    // 离线文件传输
    // 1. file_role必须是CLIENT_OFFLINE_UPLOAD或CLIENT_OFFLINE_DOWNLOAD
    // 2. CLIENT_OFFLINE_UPLOAD则user_id==from_user_id_
    // 3. CLIENT_OFFLINE_DOWNLOAD则user_id==to_user_id_
    
    bool rv = false;
    
    if (file_role == CLIENT_OFFLINE_UPLOAD){
        if (CheckFromUserID(user_id)) {
            rv = true;
        }
    } else if (file_role == CLIENT_OFFLINE_DOWNLOAD) {
        if (CheckToUserID(user_id)) {
            rv = true;
        }
    }
    
    return rv;
}

int OfflineTransferTask::DoRecvData(uint32_t user_id, uint32_t offset, const char* data, uint32_t data_size) {
    // 离线文件上传
    
    int rv = -1;
    
    do {
        // 检查是否发送者
        if (!CheckFromUserID(user_id)) {
            log("rsp user_id=%d, but sender_id is %d", user_id, from_user_id_);
            break;
        }
        
        // 检查状态
        if (state_ != kTransferTaskStateWaitingUpload && state_ != kTransferTaskStateUploading) {
            log("state=%d error, need kTransferTaskStateWaitingUpload or kTransferTaskStateUploading", state_);
            break;
        }
        
        // 检查offset是否有效
        if (offset != transfered_idx_*SEGMENT_SIZE) {
            break;
        }
        
        //if (data_size != GetNextSegmentBlockSize()) {
        //    break;
        //}
        // todo
        // 检查文件大小
        
        data_size = GetNextSegmentBlockSize();
        log("Ready recv data, offset=%d, data_size=%d, segment_size=%d", offset, data_size, sengment_size_);
        
        if (state_ == kTransferTaskStateWaitingUpload) {
            if (fp_ == NULL) {
                fp_ = OpenByWrite(task_id_, to_user_id_);
                if (fp_ == NULL) {
                    break;
                }
            }

            // 写文件头
            OfflineFileHeader file_header;
            memset(&file_header, 0, sizeof(file_header));
            file_header.set_create_time(time(NULL));
            file_header.set_task_id(task_id_);
            file_header.set_from_user_id(from_user_id_);
            file_header.set_to_user_id(to_user_id_);
            file_header.set_file_name("");
            file_header.set_file_size(file_size_);
            fwrite(&file_header, 1, sizeof(file_header), fp_);
            fflush(fp_);

            state_ = kTransferTaskStateUploading;
        }
        
        // 存储
        if (fp_ == NULL) {
            //
            break;
        }
        
        fwrite(data, 1, data_size, fp_);
        fflush(fp_);

        ++transfered_idx_;
        SetLastUpdateTime();

        if (transfered_idx_ == sengment_size_) {
            state_ = kTransferTaskStateUploadEnd;
            fclose(fp_);
            fp_ = NULL;
            rv = 1;
        } else {
            rv = 0;
        }
    } while (0);
    
    return rv;
}

int OfflineTransferTask::DoPullFileRequest(uint32_t user_id, uint32_t offset, uint32_t data_size, std::string* data) {
    int rv = -1;
    
    log("Recv pull file request: user_id=%d, offset=%d, data_size=%d", user_id, offset, data_size);

    do {
        // 1. 首先检查状态，必须为kTransferTaskStateWaitingDownload或kTransferTaskStateDownloading
        if (state_ != kTransferTaskStateWaitingDownload && state_ != kTransferTaskStateDownloading) {
            log("state=%d error, need kTransferTaskStateWaitingDownload or kTransferTaskStateDownloading", state_);
            break;
        }
        
        // 2. 处理kTransferTaskStateWaitingDownload
        if(state_ == kTransferTaskStateWaitingDownload) {
            if (transfered_idx_ != 0)
                transfered_idx_ = 0;
            
            if (fp_!=NULL) {
                fclose(fp_);
                fp_ = NULL;
            }

            fp_ = OpenByRead(task_id_, user_id);
            if (fp_ == NULL) {
                break;
            }
            
            //if (file_header_ == NULL) {
            //    file_header_ = new FileHeader();
            //}
 
            OfflineFileHeader file_header;
            size_t size = fread(&file_header, 1, sizeof(file_header), fp_); // read header
            if (sizeof(file_header) != size) {
                // close to ensure next time will read again
                log("read file head failed.");
                fclose(fp_); // error to get header
                fp_ = NULL;
                break;
                
            }
 
            state_ = kTransferTaskStateDownloading;
        } else {
            // 检查文件是否打开
            if (fp_ == NULL) {
                // 不可能发生
                break;
            }
        }
        
        // 检查offset是否有效
        if (offset != transfered_idx_*SEGMENT_SIZE) {
            log("Recv offset error, offser=%d, transfered_offset=%d", offset, transfered_idx_*SEGMENT_SIZE);
            break;
        }
        
        data_size = GetNextSegmentBlockSize();
        
        log("Ready send data, offset=%d, data_size=%d", offset, data_size);
        //if (data_size != GetNextSegmentBlockSize()) {
        //    log("Recv data_size error, data_size=%d, transfered_data_size=%d", data_size, GetNextSegmentBlockSize());
        //    break;
        //}

        
        // the header won't be sent to recver, because the msg svr had already notified it.
        // if the recver needs to check it, it could be helpful
        // or sometime later, the recver needs it in some way.
        
            
        // read data and send based on offset and datasize.
        char* tmpbuf = new char[data_size];
        if (NULL == tmpbuf) {
            // alloc mem failed
            log("alloc mem failed.");
            // SendPdu(&pdu);
            //t->unlock(__LINE__);
            // return;
            break;
        }
        memset(tmpbuf, 0, data_size);
        
        size_t size = fread(tmpbuf, 1, data_size, fp_);
        if (size != data_size) {
            log("Read size error, data_size=%d, but read_size=%d", data_size, size);
            delete [] tmpbuf;
            break;
            //
        }
        
        data->append(tmpbuf, data_size);
        delete [] tmpbuf;

        transfered_idx_++;
        
        SetLastUpdateTime();
        if (transfered_idx_ == sengment_size_) {
            log("pull req end.");
            state_ = kTransferTaskStateUploadEnd;
            fclose(fp_);
            fp_ = NULL;
            rv = 1;
        } else {
            rv = 0;
        }

/*      
        msg2.set_file_data(tmpbuf, size);
        msg2.set_result_code(0);
        CImPdu pdu2;
        pdu2.SetPBMsg(&msg2);
        pdu2.SetServiceId(SID_FILE);
        pdu2.SetCommandId(CID_FILE_PULL_DATA_RSP);
        pdu2.SetSeqNum(pPdu->GetSeqNum());
        pdu2.SetSeqNum(pPdu->GetSeqNum());
        SendPdu(&pdu2);
        delete[] tmpbuf;
  */
//        t->transfered_size += size; // record transfered size for next time offset

/*
        // offset file_header_t
        int iret = fseek(fp_, sizeof(FileHeader) + offset, SEEK_SET); // read after file_header_t
        if (0 != iret) {
            log("seek offset failed.");
            // SendPdu(&pdu);
            delete[] tmpbuf;
            
            //t->unlock(__LINE__);
            //return;
            // offset failed
            break;
        }
 */
        
        
/*
            t->transfered_size += size; // record transfered size for next time offset
            if (0 == size) {
                fclose(t->fp);
                t->fp = NULL;
                
                _StatesNotify(CLIENT_FILE_DONE, task_id.c_str(), user_id, this);
                Close();
                
                t->self_destroy = true;
                t->unlock(__LINE__);
                return;
            }

 */
//        rv = 0;
 //       }
        
    } while (0);

    return rv;
}


