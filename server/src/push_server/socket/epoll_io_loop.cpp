#include "epoll_io_loop.h"
#include "socket_io_define.h"
CEpollIOLoop::CEpollIOLoop(void)
{
	m_eid = 0;
	m_epollszie = EPOLL_SIZE;
    m_bCloseRequest = FALSE;
}

CEpollIOLoop::~CEpollIOLoop(void)
{
}

#if defined(__linux__)
/**	@fn	void CEpollIOLoop::Start(int32_t nEpollSize)
*	@brief 
*	@param[in] nEpollSize 默认的epoll监听size，默认为EPOLL_SIZE 
*	@return	
*/
void CEpollIOLoop::Start(int32_t nEpollSize)
{
	if (!m_thread)
	{
		_SetEpollSize(nEpollSize);
		m_waker.Start();
		//m_eid = epoll_create(nEpollSize);
        m_eid = epoll_create1(EPOLL_CLOEXEC);
		m_bCloseRequest = FALSE;
        m_thread.Start(RunThread, this);
	}
}

/**	@fn	void CEpollIOLoop::Stop()
*	@brief 
*	@return	
*/
void CEpollIOLoop::Stop()
{
	m_bCloseRequest = TRUE;
	m_waker.Wake();
	if (m_thread)
	{
        m_thread.Wait();
        m_MapIOStreamBySocket.clear();
	}
	m_waker.Stop();
	close(m_eid);
}

/**	@fn	void CEpollIOLoop::Run()
*	@brief 
*	@return	
*/
void CEpollIOLoop::Run()
{
	struct epoll_event ev;
	ev.data.fd=m_waker.GetWakeSocket();
	//设置要处理的事件类型
	ev.events=EPOLLIN;
	epoll_ctl(m_eid, EPOLL_CTL_ADD, m_waker.GetWakeSocket(), &ev);

	while (TRUE)
	{
		struct epoll_event* events = new epoll_event[_GetEpollSize()];
		int32_t nfds = epoll_wait(m_eid, events, _GetEpollSize(), -1);
		if (nfds <= 0)
			continue;
		for (int32_t i = 0; i < nfds; i++)
		{
			S_SOCKET sock = events[i].data.fd;
			if (sock == m_waker.GetWakeSocket())
			{
				m_waker.Recv();
			}
            if (events[i].events & EPOLLIN)
			{
                SOCKET_IO_DEBUG("socket read event.");
				CBaseIOStream* pIOStream = _GetHandlerBySock(sock);
				if (pIOStream != NULL)
				{
					if (pIOStream->GetSockType() == SOCK_TCP_SERVER)
					{
						pIOStream->OnAccept();
					}
					else
					{
						pIOStream->OnRecv();
                        SOCKET_IO_TRACE("socket recv data, sock id: %d.", pIOStream->GetSocketID());
					}
				}
				else
				{
					//调试的时候可能数据还没读，但是对象已经没了，需要清掉
					//epoll也会在调用close(sock)后会自己清除。
					epoll_ctl(m_eid, EPOLL_CTL_DEL, sock, &events[i]);
				}
			}//EPOLLIN
            if (events[i].events & EPOLLOUT)
			{
                SOCKET_IO_DEBUG("socket write event.");
				CBaseIOStream* pIOStream = _GetHandlerBySock(sock);
				if (pIOStream != NULL)
				{
					if (pIOStream->GetSockType() == SOCK_TCP_CLIENT && pIOStream->CheckConnect())
					{
						//连接成功
						pIOStream->OnConnect(TRUE);
					}
					pIOStream->SendBufferAsync();
				}
			}//EPOLLOUT
            if (events[i].events & EPOLLERR)
			{
                SOCKET_IO_DEBUG("socket error event.");
				CBaseIOStream* pIOStream = _GetHandlerBySock(sock);
                if (pIOStream != NULL)
                {
                    if (pIOStream->GetSockType() == SOCK_TCP_CLIENT && pIOStream->CheckConnect())
                    {
                        int32_t nError, nCode;
                        socklen_t nLen;
                        nLen = sizeof(nError);
                        nCode = getsockopt(pIOStream->GetSocket(), SOL_SOCKET, SO_ERROR, &nError, &nLen);
                        if (nCode < 0 || nError)
                        {
                            //连接失败
                            SOCKET_IO_WARN("socket connect failed, nCode: %d, nError: %d.", nCode, nError);
                            pIOStream->OnConnect(FALSE);
                        }
                    }
                    else
                    {
                        int32_t nError, nCode;
                        socklen_t nLen;
                        nLen = sizeof(nError);
                        nCode = getsockopt(pIOStream->GetSocket(), SOL_SOCKET, SO_ERROR, &nError, &nLen);
                        //连接失败
                        SOCKET_IO_WARN("socket error event, nCode: %d, nError: %d.", nCode, nError);
                        pIOStream->ShutDown();
                    }
                }
			}//EPOLLERR
		}
        if (events) {
            delete []events;
            events = NULL;
        }
	}
}

/**	@fn	void CEpollIOLoop::Add_Handler(CBaseIOStream* piostream)
*	@brief 
*	@param[in] piostream 
*	@return	
*/
void CEpollIOLoop::Add_Handler( CBaseIOStream* piostream )
{
	m_MapMutex.Lock();
	m_MapIOStreamBySocket[piostream->GetSocket()] = piostream;
	struct epoll_event ev;
	ev.data.fd = piostream->GetSocket();
	ev.events = EPOLLIN;
	epoll_ctl(m_eid, EPOLL_CTL_ADD, piostream->GetSocket(), &ev);
	m_waker.Wake();
	m_MapMutex.Unlock();
}

/**	@fn	void CEpollIOLoop::Remove_Handler(CBaseIOStream* piostream)
*	@brief 
*	@param[in] piostream 
*	@return	
*/
void CEpollIOLoop::Remove_Handler( CBaseIOStream* piostream )
{
	m_MapMutex.Lock();
	//关闭socket的时候，epoll会自动从集合中删除该socket?
	struct epoll_event ev;
	ev.data.fd=piostream->GetSocket();
	epoll_ctl(m_eid, EPOLL_CTL_DEL, piostream->GetSocket(), &ev);
	m_MapIOStreamBySocket.erase(piostream->GetSocket());
	m_waker.Wake();
	m_MapMutex.Unlock();
}

/**	@fn	void CEpollIOLoop::Add_WriteEvent(CBaseIOStream* piostream)
*	@brief 注册写事件，如果对于tcp client来说，如果是用于判断connect是否成功，则需要注册错误事件
*	@param[in] piostream 
*	@return	
*/
void CEpollIOLoop::Add_WriteEvent( CBaseIOStream* piostream )
{
	if (NULL == piostream)
	{
		return;
	}
	m_MapMutex.Lock();
	if (m_MapIOStreamBySocket.find(piostream->GetSocket()) != m_MapIOStreamBySocket.end())
	{
		
		struct epoll_event ev;
		ev.data.fd=piostream->GetSocket();
		if (piostream->GetSockType() == SOCK_TCP_CLIENT && piostream->CheckConnect())
		{
            SOCKET_IO_DEBUG("add write event for check connect.");
			//用于判断是否connect成功
			//对于111(Connection refused)(即连接一个不存在的IP)错误或者110(Connection timed out)
			//(即连接一个IP存在，PORT未开放)来说,没有定义EPOLLERR,
			//也会触发这个错误事件，保险起见，还是设置ERR事件
			//连接成功利用可写可以判断，错误则用ERR判断
			ev.events=EPOLLOUT | EPOLLERR;
			epoll_ctl(m_eid, EPOLL_CTL_MOD, piostream->GetSocket(), &ev);
		}
		else
		{
            SOCKET_IO_DEBUG("add write event.");
			//可写事件
			ev.events=EPOLLIN | EPOLLOUT | EPOLLERR;
			epoll_ctl(m_eid, EPOLL_CTL_MOD, piostream->GetSocket(), &ev);
		}
		m_waker.Wake();
	}
	m_MapMutex.Unlock();
	return;
}

/**	@fn	void CEpollIOLoop::Remove_WriteEvent(CBaseIOStream* piostream)
*	@brief 删除写事件变成读事件
*	@param[in] piostream 
*	@return	
*/
void CEpollIOLoop::Remove_WriteEvent( CBaseIOStream* piostream )
{
	if (NULL == piostream)
	{
		return;
	}
	m_MapMutex.Lock();
	if (m_MapIOStreamBySocket.find(piostream->GetSocket()) != m_MapIOStreamBySocket.end())
	{
		struct epoll_event ev;
		ev.data.fd=piostream->GetSocket();
		ev.events=EPOLLIN | EPOLLERR;
		epoll_ctl(m_eid, EPOLL_CTL_MOD, piostream->GetSocket(), &ev);
		m_waker.Wake();
	}
	m_MapMutex.Unlock();
	return;
}

#endif
