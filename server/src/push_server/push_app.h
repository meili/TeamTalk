//
//  push_app.h
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__push_app__
#define __my_push_server__push_app__

#include <stdio.h>
#include "type/base_type.h"
#include "socket/epoll_io_loop.h"

class CPushApp
{
public:
    CPushApp();
    virtual ~CPushApp();
    
    static CPushApp* GetInstance();
    
    BOOL Init();
    BOOL UnInit();
    BOOL Start();
    BOOL Stop();
    
    CEpollIOLoop& GetIOLoop() { return m_io; }
private:
    BOOL    m_bInit;
    CEpollIOLoop m_io;
};


#endif /* defined(__my_push_server__push_app__) */
