/**	@file tcp_session_async.h
 *	@note 
 *	@brief TCP Server accept到后的socket对象，非阻塞模式
 *
 *	@author		shiwei
 *	@date		2014/05/05
 *
 *	@note 这里填写本文件的详细功能描述和注解
 *	@note 历史记录：
 *	@note V1.0.0  创建文件
 */
#ifndef _TCP_SESSION_ASYNC_H
#define _TCP_SESSION_ASYNC_H

#include "socket_io.h"
#include "base_io_stream.h"
#include "../bufferloop/buffer_loop.hpp"
#include <queue>
#include "../sigslot/sigslot.h"
using namespace std;
using namespace sigslot;



class SOCKET_IO_DECLARE_CLASS CTCPSessionAsync :
	public CBaseIOStream
{
public:
	CTCPSessionAsync(CIOLoop* pIO);
	CTCPSessionAsync(CIOLoop* pIO, S_SOCKET nSock);
	~CTCPSessionAsync(void);

	virtual void SetSocket(S_SOCKET nSock);
	int32_t GetSendQueueSize() {
		m_sendqueuemutex.Lock();
		int32_t nSize = (int32_t)m_sendqueue.size();
		m_sendqueuemutex.Unlock();
		return nSize; 
	}

	void OnRecv();

	virtual int32_t SendMsgAsync(const char* szBuf, int32_t nBufSize);
	virtual void Close();
    virtual void ShutDown();

	virtual int32_t SendBufferAsync();
	virtual BOOL CheckWrite();

	/*uint32_t nsockid, const char* szBuf, int32_t nBufSize, const char* szIP, int32_t nPort*/
	sigslot::signal5<uint32_t, const char*, int32_t, const char*, int32_t > DoRecv;
	
    /*uint32_t nsockid, int32_t nErrorCode*/
	sigslot::signal2<uint32_t, int32_t> DoException;
	
    /*uint32_t nsockid*/
	sigslot::signal1<uint32_t> DoClose;
protected:
    BOOL _GetWaitForCloseStatus() { return m_bWaitForClose; }
    void _SetWaitForClose(BOOL bStatus) { m_bWaitForClose = bStatus; }

    virtual void _Close();
    void _ClearSendBuffer();
protected:
    BOOL m_bWaitForClose;
    
	queue<CSimpleBuffer*> m_sendqueue;		//待发送队列，只有非阻塞的TCP socket才会用到
	CBaseMutex m_sendqueuemutex;

};

#endif