//
//  config_util.cpp
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#include "file_server/config_util.h"

void ConfigUtil::AddAddress(const char* ip, uint16_t port) {
    IM::BaseDefine::IpAddr addr;
    addr.set_ip(ip);
    addr.set_port(port);
    addrs_.push_back(addr);
}
