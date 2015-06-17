#include "tcp_session_async.h"
#include "io_loop.h"
#include "socket_io_define.h"

CTCPSessionAsync::CTCPSessionAsync(CIOLoop* pIO) : CBaseIOStream(pIO)
{
    m_bWaitForClose = FALSE;
	m_socket = S_INVALID_SOCKET;
	SetSockType(SOCK_TCP_SESSION);
}

/**	@fn	 CTCPSessionAsync::CTCPSessionAsync(CIOLoop* pIO, S_SOCKET nSock)
*	@brief 
*	@param[in] pIO 
*	@param[in] nSock 
*	@return	
*/
CTCPSessionAsync::CTCPSessionAsync( CIOLoop* pIO, S_SOCKET nSock ) : CBaseIOStream(pIO)
{
	SetSockType(SOCK_TCP_SESSION);
	SetSocket(nSock);
}

CTCPSessionAsync::~CTCPSessionAsync(void)
{
	ShutDown();
}

/**	@fn	void CTCPSessionAsync::SetSocket(S_SOCKET nSock)
*	@brief 
*	@param[in] nSock 
*	@return	
*/
void CTCPSessionAsync::SetSocket( S_SOCKET nSock )
{
	m_socket = nSock;
    S_SetNoBlock(GetSocket(), TRUE);
	m_pio->Add_Handler(this);
}


/**	@fn	void CTCPSessionAsync::OnRecv()
*	@brief 
*	@return	
*/
void CTCPSessionAsync::OnRecv()
{
	char szBuf[TCP_RECV_SIZE] = {0};
	int32_t nRet = S_Recv(GetSocket(), szBuf, TCP_RECV_SIZE);
	if (nRet > 0)
	{
		int32_t nBufSize = nRet;
        char szIP[32] = {0};
        int32_t nPort = 0;
        S_GetPeerName(GetSocket(), szIP, &nPort);
		DoRecv(GetSocketID(), szBuf, nBufSize, szIP, nPort);
	}
	else if (nRet == 0)		
	{
		//对方关闭socket
        SOCKET_IO_WARN("recv tcp data error, peer closed.");
		DoException(GetSocketID(), SOCKET_IO_TCP_RECV_FAILED);
	}
	else
	{
#if (defined(_WIN32) || defined(_WIN64))
		int32_t nErrorCode = ::GetLastError();
		if (nErrorCode != WSAEWOULDBLOCK)
#else
		int32_t nErrorCode = errno;
		if (nErrorCode != EAGAIN)
#endif
		{
			SOCKET_IO_ERROR("recv tcp data error, errno: %d.", nErrorCode);
			DoException(GetSocketID(), SOCKET_IO_TCP_RECV_FAILED);
		}
		else
		{
			//用select/epoll/iocp的方式应该很少会有这个情况出现
			SOCKET_IO_DEBUG("recv tcp data error, buffer is blocking.");
		}
	}
}

/**	@fn	int32_t CTCPSessionAsync::SendBufferAsync()
*	@brief 
*	@return	
*/
int32_t CTCPSessionAsync::SendBufferAsync()
{
	int32_t nErrorCode = SOCKET_IO_RESULT_OK;
	m_sendqueuemutex.Lock();
	if (m_sendqueue.size() == 0)
	{
		//待发送队列中为空，则删除写事件的注册,改成读事件
		m_pio->Remove_WriteEvent(this);
		m_sendqueuemutex.Unlock();
        if (_GetWaitForCloseStatus() == TRUE)
        {
            //待发送内容发送完毕，则关闭链接
            _Close();
        }
		return nErrorCode;
	}
	CSimpleBuffer* pBufferLoop = m_sendqueue.front();
	m_sendqueuemutex.Unlock();
	int32_t nRet = S_Send(GetSocket(), (void*)pBufferLoop->GetBuffer(), pBufferLoop->GetWriteOffset());
	if ( nRet < 0)
	{
#if (defined(_WIN32) || defined(_WIN64))
		int32_t nError = ::GetLastError();
		if (WSAEWOULDBLOCK == nError)
#else
		int32_t nError = errno;
		if (EAGAIN == nError)
#endif
		{
			SOCKET_IO_DEBUG("send tcp data, buffer is blocking.")
		}
		else
		{
            _ClearSendBuffer();
			SOCKET_IO_ERROR("send tcp data error, errno: %d.", nError);
			DoException(GetSocketID(), SOCKET_IO_TCP_SEND_FAILED);
		}
	}
	else if (nRet == 0)
	{
        _ClearSendBuffer();
		SOCKET_IO_WARN("send tcp data error, peer closed.");
		DoException(GetSocketID(), SOCKET_IO_TCP_SEND_FAILED);
	}
	else if (nRet != pBufferLoop->GetWriteOffset())
	{
		int32_t nSize = 0;
		//将未成功的数据重新放置buffer loop中，待下次发送
        pBufferLoop->Read(NULL, nRet);
        SOCKET_IO_INFO("send tcp data, send size: %d, less than %d.", nRet, pBufferLoop->GetWriteOffset());
	}
	else
	{
		m_sendqueuemutex.Lock();
        delete pBufferLoop;
        pBufferLoop = NULL;
		m_sendqueue.pop();
		m_sendqueuemutex.Unlock();
	}
	return nErrorCode;
}

/**	@fn	int32_t CTCPSessionAsync::SendMsgAsync(const char* szBuf, int32_t nBufSize)
*	@brief 
*	@param[in] szBuf 
*	@param[in] nBufSize 
*	@return	
*/
int32_t CTCPSessionAsync::SendMsgAsync( const char* szBuf, int32_t nBufSize )
{
	int32_t nErrorCode = SOCKET_IO_RESULT_OK;
    
    m_sendqueuemutex.Lock();
    if (m_sendqueue.size() != 0)
    {
        if (_GetWaitForCloseStatus() == TRUE)
        {
            SOCKET_IO_DEBUG("send tcp data error, socket will be closed.");
        }
        else
        {
            if (m_sendqueue.size() >= MAX_SEND_QUEUE_SIZE) {
                SOCKET_IO_WARN("send tcp data error, buffer is overload.");
            }
            else
            {
                SOCKET_IO_DEBUG("send tcp data, push data to buffer.");
                CSimpleBuffer* pBufferLoop = new CSimpleBuffer();
                pBufferLoop->Write(szBuf, nBufSize);
                m_sendqueue.push(pBufferLoop);
                //m_pio->Add_WriteEvent(this);
            }
        }
        m_sendqueuemutex.Unlock();
        return nErrorCode;
    }
    m_sendqueuemutex.Unlock();
    
	int32_t nRet = S_Send(GetSocket(), (void*)szBuf, nBufSize);
	if ( nRet < 0)
	{
#if (defined(_WIN32) || defined(_WIN64))
		int32_t nError = ::GetLastError();
		if (WSAEWOULDBLOCK == nError)
#else
		int32_t nError = errno;
		if (EAGAIN == nError)
#endif
		{
			CSimpleBuffer* pBufferLoop = new CSimpleBuffer();
			pBufferLoop->Write(szBuf, nBufSize);
			m_sendqueuemutex.Lock();
			m_sendqueue.push(pBufferLoop);
			m_sendqueuemutex.Unlock();
			//有数据放入待发送队列，则注册为写事件
			m_pio->Add_WriteEvent(this);
			SOCKET_IO_INFO("send tcp data, buffer is blocking.")
		}
		else
		{
			SOCKET_IO_ERROR("send tcp data error, errno: %d.", nError);
			DoException(GetSocketID(), SOCKET_IO_TCP_SEND_FAILED);
		}
	}
	else if (nRet == 0)
	{
		SOCKET_IO_WARN("send tcp data error, peer closed.");
		DoException(GetSocketID(), SOCKET_IO_TCP_SEND_FAILED);
	}
	else if (nRet != nBufSize)
	{
		int32_t nRest = nBufSize - nRet;
		CSimpleBuffer* pBufferLoop = new CSimpleBuffer();
		pBufferLoop->Write(szBuf + nRet, nRest);
		m_sendqueuemutex.Lock();
		m_sendqueue.push(pBufferLoop);
		m_sendqueuemutex.Unlock();
		//有数据放入待发送队列，则注册为写事件
		m_pio->Add_WriteEvent(this);
		SOCKET_IO_INFO("send tcp data, send size: %d, less than %d.", nRet, nBufSize);
	}
	return nErrorCode;
}

/**	@fn	BOOL CTCPSessionAsync::_Close()
 *	@brief 关闭socket
 *	@return
 */
void CTCPSessionAsync::_Close()
{
    if (GetSocket() != S_INVALID_SOCKET)
    {
        if (m_pio)
        {
            m_pio->Remove_Handler(this);
        }
        S_CloseSocket(GetSocket());
        SOCKET_IO_WARN("close socket, sock %d, real sock: %d.", GetSocketID(), GetSocket());
        m_socket = S_INVALID_SOCKET;
        DoClose(GetSocketID());
        _ClearSendBuffer();
    }
}

/**	@fn	void CTCPSessionAsync::Close()
 *	@brief
 *	@return
 */
void CTCPSessionAsync::Close()
{
    _SetWaitForClose(TRUE);
    m_sendqueuemutex.Lock();
    if (m_sendqueue.size() == 0) {
        _Close();
    }
    m_sendqueuemutex.Unlock();
}

/**	@fn	BOOL CTCPSessionAsync::ShutDown()
 *	@brief 立刻关闭socket
 *	@return
 */
void CTCPSessionAsync::ShutDown()
{
    _SetWaitForClose(TRUE);
    _Close();
}

/**	@fn	BOOL CTCPSessionAsync::CheckWrite()
*	@brief 判断SOCKET是否需要设置可写
*	@return	
*/
BOOL CTCPSessionAsync::CheckWrite()
{
	if (GetSendQueueSize() != 0)
	{
		return TRUE;
	}
	return FALSE;
}

void CTCPSessionAsync::_ClearSendBuffer()
{
    m_sendqueuemutex.Lock();
    while (m_sendqueue.size() > 0)
    {
        CSimpleBuffer* pBufferLoop = m_sendqueue.front();
        delete pBufferLoop;
        m_sendqueue.pop();
    }
    m_sendqueuemutex.Unlock();
}

