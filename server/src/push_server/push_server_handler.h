//
//  push_server_handler.h
//  my_push_server
//
//  Created by luoning on 14-11-11.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__push_server_handler__
#define __my_push_server__push_server_handler__

#include <stdio.h>
#include "socket/base_handler.hpp"
class CPushServerHandler : public CBaseHandler
{
public:
    CPushServerHandler() {}
    virtual ~CPushServerHandler() {}
    
    void OnAccept(uint32_t nsockid, S_SOCKET sock, const char* szIP, int32_t nPort);
    void OnClose(uint32_t nsockid);
private:
};

#endif /* defined(__my_push_server__push_server_handler__) */
