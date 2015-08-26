//
//  transfer_task.h
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#ifndef FILE_SERVER_TRANSFER_TASK_H_
#define FILE_SERVER_TRANSFER_TASK_H_

#include "base/util.h"

#include "file_server/offline_file_util.h"

class CImConn;


// 状态机
enum TransferTaskState {
    kTransferTaskStateInvalid = 0,              // 非法状态
    
    kTransferTaskStateReady = 1,                // 已经准备好了
    
    kTransferTaskStateWaitingSender = 2,        // 等待发起者
    kTransferTaskStateWaitingReceiver = 3,      // 等待接收者
    kTransferTaskStateWaitingTransfer = 4,   // 全部已经准备好了
    kTransferTaskStateTransfering = 5,          // 正在传输中
    kTransferTaskStateTransferDone = 6,          // 正在传输中
    
    kTransferTaskStateWaitingUpload = 7,        // 等待上传
    kTransferTaskStateUploading = 8,            // 正在上传中
    kTransferTaskStateUploadEnd = 9,            // 正在上传中
    
    kTransferTaskStateWaitingDownload = 10,      // 等待下载
    kTransferTaskStateDownloading = 11,          // 正在下载中
    kTransferTaskStateDownloadEnd = 12,          // 正在下载中

    kTransferTaskStateError = 13,               // 传输失败
};

struct OfflineFileHeader {
    OfflineFileHeader () {
        task_id[0] = '\0';
        from_user_id[0] = '\0';
        to_user_id[0] = '\0';
        create_time[0] = '\0';
        file_name[0] = '\0';
        file_size[0] = '\0';
        //file_type[0] = '\0';
    }
    
    void set_task_id(std::string& _task_id) {
        strncpy(task_id, _task_id.c_str(), 128 < _task_id.length() ? 128 : _task_id.length());
    }
    
    void set_from_user_id(uint32_t id) {
        sprintf(from_user_id, "%u", id);
    }
    
    void set_to_user_id(uint32_t id) {
        sprintf(to_user_id, "%u", id);
    }
    
    void set_create_time(time_t t) {
        sprintf(create_time, "%ld", t);
    }
    
    void set_file_name(const char* p) {
        sprintf(file_name, p, 512 < strlen(p) ? 512 : strlen(p));
    }
    
    void set_file_size(uint32_t size) {
        sprintf(file_size, "%u", size);
    }
    
    std::string get_task_id() const {
        return task_id;
    }
    
    uint32_t get_from_user_id() const {
        return string2int(std::string(from_user_id));
    }

    uint32_t get_to_user_id() const {
        return string2int(std::string(to_user_id));
    }

    uint32_t get_create_time() const {
        return string2int(std::string(create_time));
    }
    
    std::string get_file_name() const {
        return file_name;
    }

    uint32_t get_file_size() const {
        return string2int(std::string(file_size));
    }

    char task_id[128];
    char from_user_id[64];
    char to_user_id[64];
    char create_time[128];
    char file_name[512];
    char file_size[64];
    // char file_type[64];
};


//----------------------------------------------------------------------------
class BaseTransferTask {
public:
    BaseTransferTask(const std::string& task_id, uint32_t from_user_id, uint32_t to_user_id, const std::string& file_name, uint32_t file_size);
    virtual ~BaseTransferTask() { }
    
    virtual uint32_t GetTransMode() const = 0;
    
    inline const std::string& task_id() const { return task_id_; }
    inline uint32_t from_user_id() const { return from_user_id_; }
    inline uint32_t to_user_id() const { return  to_user_id_; }
    inline uint32_t file_size() const { return file_size_; }
    inline const std::string& file_name() const { return file_name_; }
    inline time_t create_time() const { return  create_time_; }
    inline void set_state(int state) { state_ = state; }
    inline int state() const { return state_; }
    
    uint32_t GetOpponent(uint32_t user_id) const {
        return (user_id == from_user_id_ ? user_id : from_user_id_);
    }
    
     CImConn* GetOpponentConn(uint32_t user_id) const {
        return (user_id == from_user_id_ ? to_conn_ : from_conn_);
     }

    
     CImConn* GetFromConn() {
        return from_conn_;
     }
     CImConn* GetToConn() {
        return to_conn_;
     }

     CImConn* GetConnByUserID(uint32_t user_id) const {
        if (from_user_id_ == user_id) {
            return from_conn_;
        } else if (to_user_id_ == user_id) {
            return to_conn_;
        } else {
            return NULL;
        }
     }
    
     void SetConnByUserID(uint32_t user_id, CImConn* conn) {
        if (from_user_id_ == user_id) {
            from_conn_ = conn;
        } else if (to_user_id_ == user_id) {
            to_conn_ = conn;
        }
     }
    
    inline bool CheckFromUserID(uint32_t user_id) const {
        return from_user_id_ == user_id;
    }

    inline bool CheckToUserID(uint32_t user_id) const {
        return to_user_id_ == user_id;
    }

    inline bool CheckUserID(uint32_t user_id) const {
        return user_id == from_user_id_ || user_id == to_user_id_;
    }
    
    bool IsWaitTranfering() const {
        bool rv = false;
        if (state_ == kTransferTaskStateWaitingTransfer || state_ == kTransferTaskStateWaitingUpload || kTransferTaskStateWaitingDownload) {
            rv = true;
        }
        return rv;
    }
    
    void SetLastUpdateTime();
    
    // int StatesNotify(int state, const std::string& task_id, uint32_t user_id);
    
    // 检查状态
    virtual bool ChangePullState(uint32_t user_id, int file_role) {
        return false;
    }
    
    // 检查输入是否合法
    virtual bool CheckByUserIDAndFileRole(uint32_t user_id, int file_role) const { return false; }
    
    virtual int DoRecvData(uint32_t user_id, uint32_t offset, const char* data, uint32_t data_size) { return -1; }
    
    virtual int DoPullFileRequest(uint32_t user_id, uint32_t offset, uint32_t data_size, std::string* data) { return -1; }

protected:
    // uint32_t    transfer_mode; // FILE_TYPE_ONLINE realtime, FILE_TYPE_OFFLINE offline / mobile
    std::string task_id_; // uuid_unparse char[37]
    uint32_t    from_user_id_;
    uint32_t    to_user_id_; // if offline or mobile, null
    std::string file_name_;
    uint32_t    file_size_;
    time_t      create_time_;

    int         state_;         // 传输状态

    CImConn*    from_conn_;
    CImConn*    to_conn_;
    
    // uint64_t    last_update_time_;
};

typedef map<std::string, BaseTransferTask*> TransferTaskMap;
typedef map<CImConn*, BaseTransferTask*> TransferTaskConnkMap;

//----------------------------------------------------------------------------
class OnlineTransferTask : public BaseTransferTask {
public:
    OnlineTransferTask(const std::string& task_id, uint32_t from_user_id, uint32_t to_user_id, const std::string& file_name, uint32_t file_size)
        : BaseTransferTask(task_id, from_user_id, to_user_id, file_name, file_size) {
        mac_seq_num_ = 0;
    }
    
    virtual ~OnlineTransferTask() { }
    
    virtual uint32_t GetTransMode() const;
    
    
    virtual bool ChangePullState(uint32_t user_id, int file_role);
    virtual bool CheckByUserIDAndFileRole(uint32_t user_id, int file_role) const;
    
    virtual int DoRecvData(uint32_t user_id, uint32_t offset, const char* data, uint32_t data_size);
    virtual int DoPullFileRequest(uint32_t user_id, uint32_t offset, uint32_t data_size, std::string* data);
    
    void SetSeqNum(uint32_t seq_num) {
        mac_seq_num_ = seq_num;
    }
    
    uint32_t GetSeqNum() const {
        return mac_seq_num_;
    }
private:
    // mac客户端需要保证seq_num，但客户端目前机制无法处理在线文件传输的seq_num，故服务端纪录并设置seq_num
    uint32_t mac_seq_num_;
};

//----------------------------------------------------------------------------
#define SEGMENT_SIZE 32768

class OfflineTransferTask : public BaseTransferTask {
public:
    OfflineTransferTask(const std::string& task_id, uint32_t from_user_id, uint32_t to_user_id, const std::string& file_name, uint32_t file_size)
        : BaseTransferTask(task_id, from_user_id, to_user_id, file_name, file_size) {
//        file_header_ = NULL;
        fp_ = NULL;
        transfered_idx_ = 0;
            
        sengment_size_ = SetMaxSegmentSize(file_size);
    }

    virtual ~OfflineTransferTask() {
//        if (file_header_) {
//            delete file_header_;
//            file_header_ = NULL;
//        }
        if (fp_) {
            fclose(fp_);
            fp_ =NULL;
        }
    }
    
    static OfflineTransferTask* LoadFromDisk(const std::string& task_id, uint32_t user_id);
    
    
    virtual uint32_t GetTransMode() const;
    
    virtual bool ChangePullState(uint32_t user_id, int file_role);
    virtual bool CheckByUserIDAndFileRole(uint32_t user_id, int file_role) const;
    
    virtual int DoRecvData(uint32_t user_id, uint32_t offset, const char* data, uint32_t data_size);
    virtual int DoPullFileRequest(uint32_t user_id, uint32_t offset, uint32_t data_size, std::string* data);
   
    inline int GetSegmentSize() const {
        return sengment_size_;
    }
    
    inline int GetNextSegmentBlockSize() {
        int block_size = SEGMENT_SIZE;
        if (transfered_idx_+1 == sengment_size_) {
            block_size = file_size_ - transfered_idx_*SEGMENT_SIZE;
        }
        return block_size;
    }

    uint32_t GetNextOffset() const {
        return SEGMENT_SIZE * transfered_idx_;
    }
    
private:
    // 迭代器
    inline int SetMaxSegmentSize(uint32_t file_size) {
        int seg_size = file_size / SEGMENT_SIZE;
        if (file_size_%SEGMENT_SIZE != 0) {
            seg_size = file_size / SEGMENT_SIZE + 1;
        }
        return seg_size;
    }

    // std::map<uint32_t, upload_package_t*> upload_packages;
    
    
    // uint32_t    transfered_size;    // download size
    // file_header_t*   file_head;
    
    // FileHeader* file_header_;
    FILE*       fp_;
    
    // 已经传输
    int transfered_idx_;
    int sengment_size_;
    
    // std::map<uint32_t, upload_package_t*> upload_packages;
};


/*
struct TransferTask {
    uint32_t    transfer_mode; // FILE_TYPE_ONLINE realtime, FILE_TYPE_OFFLINE offline / mobile
    string      task_id; // uuid_unparse char[37]
    uint32_t    from_user_id;
    uint32_t    to_user_id; // if offline or mobile, null
    bool        ready_to_recv; //
    bool        ready_to_send; //
    uint32_t    file_size;
    string      current_block;
    time_t      create_time;
    bool        self_destroy;
    CImConn*    from_conn;
    CImConn*    to_conn;
    
    // pthread_rwlock_t task_lock;
    std::map<uint32_t, upload_package_t*> upload_packages;
    
    FILE*       fp;
    uint32_t    transfered_size;    // download size
    file_header_t*   file_head;
    pthread_t   worker;
    
    transfer_task_t() {
        transfer_mode = 0;
        task_id = "";
        from_user_id = 0;
        to_user_id = 0;
        ready_to_recv = false;
        ready_to_send = false;
        file_size = 0;
        pthread_rwlock_init(&task_lock, NULL);
        //       upload_packages_lock = PTHREAD_RWLOCK_INITIALIZER;
        current_block = "";
        create_time = time(NULL);
        fp = NULL;
        transfered_size = 0;
        file_head = NULL;
        self_destroy = false;
        worker = (pthread_t)0;
    }
    
    ~TransferTask() {
        lock(__LINE__);
        unlock(__LINE__);
    }
    
    void lock(int n) {
        //    	log("++++++++++++++++++++++ lock %d %s\n", n, task_id.c_str());
        pthread_rwlock_wrlock(&task_lock);
    }
    
    void unlock(int n) {
        //    	log("++++++++++++++++++++++unlock %d %s\n", n, task_id.c_str());
        pthread_rwlock_unlock(&task_lock);
    }
    
    void release() {
        std::map<uint32_t, upload_package_t*>::iterator itor = upload_packages.begin();
        for ( ; itor != upload_packages.end(); ) {
            if (!itor->second) {
                upload_packages.erase(itor++);
                continue ;
            }
            if (itor->second->data) {
                delete[] itor->second->data;
                itor->second->data = NULL;
            }
            delete itor->second;
            itor->second = NULL;
            upload_packages.erase(itor++);
        }
        
        if (fp) {
            fclose(fp);
            fp = NULL;
        }
        if (file_head) {
            delete file_head;
            file_head = NULL;
        }
        
        if (worker) {
            //          pthread_exit(&worker);
        }
    }
    
    uint32_t GetOpponent(uint32_t user_id) {
        return (user_id == from_user_id ? to_user_id : from_user_id);
    }
    
    CImConn* GetOpponentConn(uint32_t user_id) {
        return (user_id == from_user_id ? to_conn : from_conn);
    }
    
    CImConn* GetConn(uint32_t user_id) {
        return (user_id == from_user_id ? from_conn : to_conn);
    }
}transfer_task;
*/

std::string GenerateUUID();
const char* GetCurrentOfflinePath();

// nt generate_id(char* id);


#endif /* defined(__im_server_mac_new__transfer_task__) */
