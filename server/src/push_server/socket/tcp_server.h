/**	@file tcp_server.h
 *	@note 
 *	@brief 主要用于TCP Server端的对象
 *
 *	@author		shiwei
 *	@date		2014/05/05
 *
 *	@note 这里填写本文件的详细功能描述和注解
 *	@note 历史记录：
 *	@note V1.0.0  创建文件
 */
#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include "socket_io.h"
#include "base_io_stream.h"
#include "../sigslot/sigslot.h"
using namespace sigslot;
class SOCKET_IO_DECLARE_CLASS CTCPServer :
	public CBaseIOStream
{
public:
	CTCPServer(CIOLoop* pIO);
	~CTCPServer(void);

	void OnAccept();
	void Listen();
	void Close();

	/*uint32_t nsockid, S_SOCKET nSock, const char* szIP, int32_t nPort*/
	sigslot::signal4<uint32_t, S_SOCKET, const char*, int32_t > DoAccept;
    
	/*uint32_t nsockid*/
	sigslot::signal1<uint32_t> DoClose;
	
private:
};

#endif