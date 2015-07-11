//
//  apns_client_handler.h
//  my_push_server
//
//  Created by luoning on 14-11-11.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__apns_client_handler__
#define __my_push_server__apns_client_handler__

#include <stdio.h>
#include "apns_msg.h"
#include "socket/base_handler.hpp"
class CAPNSClientHandler : public CBaseHandler
{
public:
    CAPNSClientHandler() {}
    virtual ~CAPNSClientHandler() {}
    
    virtual void OnException(uint32_t nsockid, int32_t nErrorCode);
    
    virtual void OnClose(uint32_t nsockid);
    
    virtual void OnConnect(uint32_t nsockid);
    
    virtual void OnSSLConnect(uint32_t nsockid);

    virtual void OnRecvData(const char* szBuf, int32_t nBufSize);
    
protected:
    CAPNSGateWayResMsg m_Msg;
};

#endif /* defined(__my_push_server__apns_client_handler__) */
