//
//  push_server.h
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__push_server__
#define __my_push_server__push_server__

#include <stdio.h>
#include "socket/tcp_server.h"
#include "socket/epoll_io_loop.h"
#include <string>
#include "push_server_handler.h"
#include <memory>
using namespace std;
class CPushServer : public std::enable_shared_from_this<CPushServer>
{
public:
    CPushServer(CEpollIOLoop& io);
    virtual ~CPushServer();
    
    void SetListenIP(const char* listen_ip) { m_strListenIP = listen_ip; }
    string GetListenIP() { return m_strListenIP; }
    void SetPort(uint32_t port) { m_nPort = port; }
    uint32_t GetPort() { return m_nPort; }
    CEpollIOLoop& GetIOLoop() { return m_io; }

    BOOL Start();
    BOOL Stop();
    
private:
    string m_strListenIP;
    uint32_t m_nPort;
    
    CTCPServer* m_pServer;
    CEpollIOLoop& m_io;
    CPushServerHandler m_handler;
};

typedef std::shared_ptr<CPushServer> push_server_ptr;
#endif /* defined(__my_push_server__push_server__) */
