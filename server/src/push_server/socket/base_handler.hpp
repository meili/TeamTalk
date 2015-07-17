//
//  base_handler.hpp
//  my-code
//
//  Created by shiwei on 14-10-13.
//  Copyright (c) 2014Äê shiwei. All rights reserved.
//

#ifndef _BASE_HANDLER_HPP
#define _BASE_HANDLER_HPP

#include "../sigslot/sigslot.h"
#include "../type/base_type.h"
#include "base_socket.hpp"
#include <string>
using namespace std;
using namespace sigslot;
class CBaseHandler : public has_slots<>
{
public:
	CBaseHandler(void) {}
	~CBaseHandler(void) {}

    virtual void OnAccept(uint32_t nsockid, S_SOCKET sock, const char* szIP, int32_t nPort) {}
    
    virtual void OnException(uint32_t nsockid, int32_t nErrorCode) {}
    
    virtual void OnClose(uint32_t nsockid) {}
	
    virtual void OnConnect(uint32_t nsockid) {}
	
    virtual void OnRecvData(const char* szBuf, int32_t nBufSize) {}
    
    void OnRecv(uint32_t nsockid, const char* szBuf, int32_t nBufSize, const char* szIP,
                int32_t nPort)
    {
        m_nSockID = nsockid;
        m_strRemoteIP = szIP;
        m_nRemotePort = nPort;
        OnRecvData(szBuf, nBufSize);
    }
    
protected:
    uint32_t _GetSockID() { return m_nSockID; }
    string _GetRemoteIP() const { return m_strRemoteIP; }
    uint32_t _GetRemotePort() { return m_nRemotePort; }
    
protected:
    uint32_t m_nSockID;
    string m_strRemoteIP;
    uint32_t m_nRemotePort;
};


#endif
