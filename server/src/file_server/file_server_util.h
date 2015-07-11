//
//  file_server_util.h
//  file_server
//
//  Created by yunfan on 2-9-28.
//  Copyright (c) 2014Äê yunfan. All rights reserved.
//


#pragma once


#ifndef __FILE_SERVER_UTIL_H__
#define __FILE_SERVER_TUIL_H__

#include <pthread.h>
#include <uuid/uuid.h>

#include <imconn.h>

typedef struct file_header_t
{
    char task_id[128];
    char from_user_id[64];
    char to_user_id[64];
    char create_time[128];
    char file_name[512];
    char file_size[64];
    char file_type[64];

    file_header_t () {
        memset(task_id, 0, 128);
        memset(from_user_id, 0, 64);
        memset(to_user_id, 0, 64);
        memset(create_time, 0, 64);
        memset(file_name, 0, 512);
        memset(file_size, 0, 64);
        memset(file_type, 0, 64);
    }

    void set_task_id(const char* p) {
        strncpy(task_id, p, 128 < strlen(p) ? 128 : strlen(p));
    }

    void set_from_user_id(const char* p) {
        strncpy(from_user_id, p, 64 < strlen(p) ? 64 : strlen(p));
    } // idtourl

    void set_from_user_id(uint32_t id) {
        strcpy(from_user_id, idtourl(id));
    } // warn! idtourl

    void set_to_user_id(const char* p) {
        strncpy(to_user_id, p, 64 < strlen(p) ? 64 : strlen(p));
    }

    void set_to_user_id(uint32_t id) {
        strcpy(to_user_id, idtourl(id));
    }

    void set_create_time(time_t t) {
        sprintf(create_time, "%ld", t);
    }

    void set_file_name(const char* p) {
        sprintf(file_name, p, 512 < strlen(p) ? 512 : strlen(p));
    }

    void set_file_size(uint32_t size) {
        sprintf(file_size, "%d", size);
    }
}file_header; // offline file head struct

typedef struct upload_package_t {
    uint32_t index;
    uint32_t offset;
    uint32_t size;
    char*    data;

    upload_package_t () {
        index = 0;
        offset = 0;
        size = 0;
        data = NULL;
    }

    upload_package_t(uint32_t val1, uint32_t val2, uint32_t val3) {
        index = val1;
        offset = val2;
        size = val3;
        data = NULL; //new char[val3];
 /*       if (!data) {
            data = NULL;
        } else {
            memset(data, 0 ,val3);
        } */
    }
}upload_package;

typedef struct transfer_task_t{
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

    pthread_rwlock_t task_lock;
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

    ~transfer_task_t() {
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



int generate_id(char* id);


#endif
