#include "tcp_client_async.h"
#include "socket_io_define.h"
#include "io_loop.h"

CTCPClientAsync::CTCPClientAsync(CIOLoop* pIO) : CBaseIOStream(pIO)
{
    _InitSocket();
}

CTCPClientAsync::~CTCPClientAsync(void)
{
	ShutDown();
}

void CTCPClientAsync::_InitSocket()
{
    _SetWaitForClose(FALSE);
    m_socket = S_CreateSocket(AF_INET, SOCK_STREAM, 0);
    SetSockType(SOCK_TCP_CLIENT);
    S_SetNoBlock(m_socket, TRUE);
    SetCheckConnect(TRUE);
}


/**	@fn	int32_t CTCPClientAsync::ConnectAsync(const char* szIP, HPR_INT32 nPort)
*	@brief 
*	@param[in] szIP 
*	@param[in] nPort 
*	@return	返回值为0
*/
int32_t CTCPClientAsync::ConnectAsync( const char* szIP, int nPort )
{
	SetRemoteIP(szIP);
	SetRemotePort(nPort);
    //windows:这种做法对select没有用
    //linux:这个做法也没有效果
    //S_SetSendTimeOut(GetSocket(), 10);
    
    SOCKET_IO_INFO("connect remote ip: %s, port: %d.", szIP, nPort);
	if (S_Connect(GetSocket(), szIP, nPort) != 0)
	{
#if (defined(_WIN32) || defined(_WIN64))
		int32_t nError = ::GetLastError();
		if (nError != WSAEWOULDBLOCK)
#else
		int32_t nError = errno;
		if (nError != EINPROGRESS)
#endif
		{
			//连接出错
			SOCKET_IO_ERROR("connect error, errno: %d.", nError);
			DoException(GetSocketID(), SOCKET_IO_TCP_CONNECT_FAILED);
		}
		else
		{
			//连接建立已启动，但是还未成功
		}
        //对于tcp client来说，在此处在加入io中
        m_pio->Add_Handler(this);
        //对于windows/linux 的select来说，connect需要激活可写事件
        //对于epoll来说，激活的是EPOLLERR?
        m_pio->Add_WriteEvent(this);
	}
	else
	{
        //对于tcp client来说，在此处在加入io中
        m_pio->Add_Handler(this);
        //对于windows/linux 的select来说，connect需要激活可写事件
        //对于epoll来说，激活的是EPOLLERR?
        m_pio->Add_WriteEvent(this);
        
		//连接成功，一般连接的是本地端口可能会立即成功
		OnConnect(TRUE);
	}
	return 0;
}

/**
 *  重连接口
 *
 *  @return
 */
int32_t CTCPClientAsync::ReConnectAsync()
{
    int32_t nErrorCode = 0;
    if (S_INVALID_SOCKET == GetSocket())
    {
        _InitSocket();
        nErrorCode = ConnectAsync(GetRemoteIP(), GetRemotePort());
    }
    return nErrorCode;
}

/**	@fn	int32_t CTCPClientAsync::SendBufferAsync()
*	@brief 将待发送队列中的数据发送出去
*	@return	
*/
int32_t CTCPClientAsync::SendBufferAsync()
{
	int32_t nErrorCode = 0;
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
		//将未成功的数据重新放置buffer loop中，待下次发送
		int32_t nSize = 0;
        pBufferLoop->Read(NULL, nRet);
        SOCKET_IO_DEBUG("send tcp data, send size: %d, less than %d.", nRet, pBufferLoop->GetWriteOffset());
	}
	else
	{
		m_sendqueuemutex.Lock();
        delete pBufferLoop;
		m_sendqueue.pop();
		m_sendqueuemutex.Unlock();
	}
	return nErrorCode;
}

/**	@fn	int32_t CTCPClientAsync::SendMsgAsync(const char* szBuf, int32_t nBufSize)
*	@brief 
*	@param[in] szBuf 
*	@param[in] nBufSize 
*	@return	
*/
int32_t CTCPClientAsync::SendMsgAsync(const char* szBuf, int32_t nBufSize )
{
	int32_t nErrorCode = 0;
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
			SOCKET_IO_DEBUG("send tcp data, buffer is blocking.");
		}
		else
		{
			SOCKET_IO_ERROR("send tcp data error, errno: %d.", nError);
			DoException(GetSocketID(), SOCKET_IO_TCP_SEND_FAILED);
		}
	}
	else if (nRet == 0)
	{
		nErrorCode = 0;
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
		SOCKET_IO_DEBUG("send tcp data, send size: %d, less than %d.", nRet, nBufSize);
	}
	return nErrorCode;
}

/**	@fn	BOOL CTCPClientAsync::_Close()
 *	@brief 关闭socket
 *	@return
 */
void CTCPClientAsync::_Close()
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

/**	@fn	void CTCPClientAsync::Close()
*	@brief 
*	@return	
*/
void CTCPClientAsync::Close()
{
    _SetWaitForClose(TRUE);
    m_sendqueuemutex.Lock();
    if (m_sendqueue.size() == 0) {
        _Close();
    }
    m_sendqueuemutex.Unlock();
}

/**	@fn	BOOL CTCPClientAsync::ShutDown()
 *	@brief 立刻关闭socket
 *	@return
 */
void CTCPClientAsync::ShutDown()
{
    _SetWaitForClose(TRUE);
    _Close();
}

/**	@fn	void CTCPClientAsync::OnConnect(BOOL bConnected)
*	@brief 
*	@return	
*/
void CTCPClientAsync::OnConnect(BOOL bConnected)
{
	//无论是否连接成功，都认为已经判断结束
	SetCheckConnect(FALSE);
	//连接完毕，则删除写/错误事件的注册,改成读事件
	m_pio->Remove_WriteEvent(this);
	if (TRUE == bConnected)
	{
		SOCKET_IO_INFO("socket connect successed, remote ip: %s, port: %d.", GetRemoteIP(),
                       GetRemotePort());
		DoConnect(GetSocketID());
	}
	else
	{
		SOCKET_IO_ERROR("socket connect failed, remote ip: %s, port: %d.", GetRemoteIP(), GetRemotePort());
		DoException(GetSocketID(), SOCKET_IO_TCP_CONNECT_FAILED);
	}
}

/**	@fn	void CTCPClientAsync::OnRecv()
*	@brief 
*	@return	
*/
void CTCPClientAsync::OnRecv()
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
			//用select/epoll/iocp的方式应该不会有这个情况出现
			SOCKET_IO_DEBUG("recv tcp data error, buffer is blocking.");
		}
	}
}

/**	@fn	BOOL CTCPClientAsync::CheckWrite()
*	@brief 判断SOCKET是否需要设置可写
*	@return	
*/
BOOL CTCPClientAsync::CheckWrite()
{
	if (GetSendQueueSize() != 0)
	{
		return TRUE;
	}
	return CheckConnect();
}

void CTCPClientAsync::_ClearSendBuffer()
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




