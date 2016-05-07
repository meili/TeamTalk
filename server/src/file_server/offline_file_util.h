//
//  file_util.h
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#ifndef FILE_SERVER_OFFLINE_FILE_UTIL_H_
#define FILE_SERVER_OFFLINE_FILE_UTIL_H_

//#include "base/ostype.h"
#include <string.h>

#include "base/UtilPdu.h"

struct FileHeader {
    FileHeader () {
        task_id[0] = '\0';
        from_user_id[0] = '\0';
        to_user_id[0] = '\0';
        create_time[0] = '\0';
        file_name[0] = '\0';
        file_size[0] = '\0';
        file_type[0] = '\0';
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
    
    char task_id[128];
    char from_user_id[64];
    char to_user_id[64];
    char create_time[128];
    char file_name[512];
    char file_size[64];
    char file_type[64];
};


struct UploadPackage {
    UploadPackage () {
        index = 0;
        offset = 0;
        size = 0;
        data = NULL;
    }
    
    UploadPackage(uint32_t val1, uint32_t val2, uint32_t val3) {
        index = val1;
        offset = val2;
        size = val3;
        data = NULL;
    }
    
    uint32_t index;
    uint32_t offset;
    uint32_t size;
    char*    data;

};

#endif /* defined(FILE_SERVER_OFFLINE_FILE_UTIL_H_) */
