#include "io_loop.h"
#include "socket_io_define.h"

CIOLoop::CIOLoop(void)
{
	m_bCloseRequest = FALSE;
}

CIOLoop::~CIOLoop(void)
{
}

/**	@fn	void CIOLoop::Start()
*	@brief 
*	@return	
*/
void CIOLoop::Start()
{
	if (!m_thread)
	{
		m_waker.Start();
		m_bCloseRequest = FALSE;
        m_thread.Start(RunThread, this);
	}
}

/**	@fn	void CIOLoop::Stop()
*	@brief 
*	@return	
*/
void CIOLoop::Stop()
{
	m_bCloseRequest = TRUE;
	m_waker.Wake();
	if (m_thread)
	{
        m_thread.Wait();
		m_MapIOStreamBySocket.clear();
	}
	m_waker.Stop();
}

/**	@fn	void CIOLoop::Run()
*	@brief 
*	@return	
*/
void CIOLoop::Run()
{
	fd_set fd_read, fd_write, fd_error;
	while (m_bCloseRequest == FALSE)
	{
		int nMaxfd = 0;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_error);
		FD_SET(m_waker.GetWakeSocket(), &fd_read);
		if(m_waker.GetWakeSocket() > nMaxfd) 
			nMaxfd = m_waker.GetWakeSocket();
		m_MapMutex.Lock();
		map<S_SOCKET, CBaseIOStream*> mapTmp = m_MapIOStreamBySocket;
		m_MapMutex.Unlock();
		map<S_SOCKET, CBaseIOStream*>::iterator it = mapTmp.begin();
		for (; it != mapTmp.end(); it++)
		{
			CBaseIOStream* pIOStream = it->second;
			if (pIOStream->CheckConnect() == FALSE)
			{
				//如果是要检查TCP CLIENT是否连接，则不设置可读
				FD_SET(it->first, &fd_read);
				if(it->first > nMaxfd) 
					nMaxfd = it->first;
			}
			if (pIOStream->CheckWrite() == TRUE)
			{
				//设置可写
				FD_SET(it->first, &fd_write);
				if(it->first > nMaxfd) 
					nMaxfd = it->first;
				//设置错误信号，用于windows的tcp connect超时检查，同时查看是否有其他触发错误
				FD_SET(it->first, &fd_error);
			}
		}
		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		int nRet = select(nMaxfd + 1, &fd_read, &fd_write, &fd_error, NULL);
		if (nRet > 0)
		{
			if (FD_ISSET(m_waker.GetWakeSocket(), &fd_read))
			{
				//stop
				m_waker.Recv();
			}
			map<S_SOCKET, CBaseIOStream*>::iterator it1 = mapTmp.begin();
			for (; it1 != mapTmp.end(); it1++)
			{
				//read
				if (FD_ISSET(it1->first, &fd_read))
				{
					CBaseIOStream* pIOStream = _GetHandlerBySock(it1->first);
					if (pIOStream != NULL)
					{
						if (pIOStream->GetSockType() == SOCK_TCP_SERVER)
						{
							pIOStream->OnAccept();
						}
						else
						{
							pIOStream->OnRecv();
						}	
					}
				}//read
				//write
				if (FD_ISSET(it1->first, &fd_write))
				{
					CBaseIOStream* pIOStream = _GetHandlerBySock(it1->first);
					if (pIOStream != NULL)	
					{
						if (pIOStream->GetSockType() == SOCK_TCP_CLIENT && pIOStream->CheckConnect())
						{
#if (defined(_WIN32) || defined(_WIN64))
							pIOStream->OnConnect(TRUE);
//#elif defined(__linux__) //mac???
#else
							//这个是unix的处理方式，经测试linux也适用,mac同理
							int32_t nError, nCode;
							socklen_t nLen; 
							nLen = sizeof(nError);     
							nCode = getsockopt(pIOStream->GetSocket(), SOL_SOCKET, SO_ERROR, &nError,
                                               &nLen);
							if (nCode < 0 || nError) 
							{     
								//连接失败
								//linux的超时失败是也是根据这个可以判断
								SOCKET_IO_WARN("socket connect failed, nCode: %d, nError: %d.", nCode,
                                               nError);
								pIOStream->OnConnect(FALSE);
							}
							else
							{
								//连接成功
								//SOCKET_IO_WARN("socket connect successed.", nCode, nError);
								pIOStream->OnConnect(TRUE);
							}
#endif
						}
						pIOStream->SendBufferAsync();
					}
				}//wirte
				//error
				if (FD_ISSET(it1->first, &fd_error))
				{
					CBaseIOStream* pIOStream = _GetHandlerBySock(it1->first);
					if (pIOStream != NULL)
					{
						//windows的超时判断是利用err_fds来判断
						//对于不存在的IP(即linux会报111错误),或者IP存在，端口不存在(即linux会报110错误)
						//都是超时错误
						if (pIOStream->CheckConnect() == TRUE)
						{
							SOCKET_IO_WARN("socket connect time out, remote ip: %s, port: %d.",
                                           pIOStream->GetRemoteIP(), pIOStream->GetRemotePort());
							pIOStream->OnConnect(FALSE);
						}
						else
						{
							SOCKET_IO_WARN("err_fds, %d.", (int32_t)pIOStream->GetSockType());
						}
					}
				}//error
			}//for
		}// nRet > 0
		else if (0 == nRet)
		{
			//Time Out
			//map<HPR_SOCK_T, CBaseIOStream*>::iterator it1 = mapTmp.begin();
			//for (; it1 != mapTmp.end(); it1++)
			//{
			//	//write
			//	CBaseIOStream* pIOStream = _GetHandlerBySock(it1->first);
			//	if (pIOStream != NULL)
			//	{
			//		if (pIOStream->GetSockType() == SOCK_TCP_CLIENT && pIOStream->CheckConnect())
			//		{
			//		}
			//	}
			//}
		}
		else
		{
			//TODO
			//Error
			SOCKET_IO_ERROR("socket select error");
            break;
		}
	}
}

/**	@fn	void* CIOLoop::RunThread(void* ptr)
*	@brief 
*	@param[in] ptr 
*	@return	
*/
void* CIOLoop::RunThread( void* ptr )
{
	CIOLoop* pApp = (CIOLoop*)ptr;
	pApp->Run();
	return NULL;
}

/**	@fn	void CIOLoop::Add_Handler(CBaseIOStream* piostream)
*	@brief 
*	@param[in] piostream 
*	@return	
*/
void CIOLoop::Add_Handler( CBaseIOStream* piostream )
{
	m_MapMutex.Lock();
	m_MapIOStreamBySocket[piostream->GetSocket()] = piostream;
	m_waker.Wake();
	m_MapMutex.Unlock();
}

/**	@fn	void CIOLoop::Remove_Handler(CBaseIOStream* piostream)
*	@brief 
*	@param[in] piostream 
*	@return	
*/
void CIOLoop::Remove_Handler( CBaseIOStream* piostream )
{
	m_MapMutex.Lock();
	m_MapIOStreamBySocket.erase(piostream->GetSocket());
	m_waker.Wake();
	m_MapMutex.Unlock();
}

/**	@fn	CBaseIOStream* CIOLoop::_GetHandlerBySock(S_SOCKET sock)
*	@brief 
*	@param[in] sock 
*	@return	
*/
CBaseIOStream* CIOLoop::_GetHandlerBySock( S_SOCKET sock )
{
	CBaseIOStream* pIOStream = NULL;
	map<S_SOCKET, CBaseIOStream*>::iterator it = m_MapIOStreamBySocket.find(sock);
	if (it != m_MapIOStreamBySocket.end())
	{
		pIOStream = it->second;
	}
	return pIOStream;
}
