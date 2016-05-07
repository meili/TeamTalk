//
//  im_conn_util.h
//  TeamTalk
//
//  Created by benqi on 15/8/5.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#ifndef BASE_IM_CONN_UTIL_H_
#define BASE_IM_CONN_UTIL_H_

#include "base/ostype.h"

namespace google { namespace protobuf {
    class MessageLite;
}}

class CImConn;

int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, uint16_t error, const ::google::protobuf::MessageLite* message);

#endif /* defined(BASE_IM_CONN_UTIL_H_) */
