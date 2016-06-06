#include "EventDispatch.h"
#include "BaseSocket.h"

#define MIN_TIMER_DURATION	100	// 100 miliseconds

CEventDispatch* CEventDispatch::m_pEventDispatch = NULL;

CEventDispatch::CEventDispatch()
{
    running = false;
#ifdef _WIN32
	FD_ZERO(&m_read_set);
	FD_ZERO(&m_write_set);
	FD_ZERO(&m_excep_set);
#elif __APPLE__
	m_kqfd = kqueue();
	if (m_kqfd == -1)
	{
		log("kqueue failed");
	}
#else
	m_epfd = epoll_create(1024);
	if (m_epfd == -1)
	{
		log("epoll_create failed");
	}
#endif
}

CEventDispatch::~CEventDispatch()
{
#ifdef _WIN32

#elif __APPLE__
	close(m_kqfd);
#else
	close(m_epfd);
#endif
}

void CEventDispatch::AddTimer(callback_t callback, void* user_data, uint64_t interval)
{
	list<TimerItem*>::iterator it;
	for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
	{
		TimerItem* pItem = *it;
		if (pItem->callback == callback && pItem->user_data == user_data)
		{
			pItem->interval = interval;
			pItem->next_tick = get_tick_count() + interval;
			return;
		}
	}

	TimerItem* pItem = new TimerItem;
	pItem->callback = callback;
	pItem->user_data = user_data;
	pItem->interval = interval;
	pItem->next_tick = get_tick_count() + interval;
	m_timer_list.push_back(pItem);
}

void CEventDispatch::RemoveTimer(callback_t callback, void* user_data)
{
	list<TimerItem*>::iterator it;
	for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
	{
		TimerItem* pItem = *it;
		if (pItem->callback == callback && pItem->user_data == user_data)
		{
			m_timer_list.erase(it);
			delete pItem;
			return;
		}
	}
}

void CEventDispatch::_CheckTimer()
{
	uint64_t curr_tick = get_tick_count();
	list<TimerItem*>::iterator it;

	for (it = m_timer_list.begin(); it != m_timer_list.end(); )
	{
		TimerItem* pItem = *it;
		it++;		// iterator maybe deleted in the callback, so we should increment it before callback
		if (curr_tick >= pItem->next_tick)
		{
			pItem->next_tick += pItem->interval;
			pItem->callback(pItem->user_data, NETLIB_MSG_TIMER, 0, NULL);
		}
	}
}

void CEventDispatch::AddLoop(callback_t callback, void* user_data)
{
    TimerItem* pItem = new TimerItem;
    pItem->callback = callback;
    pItem->user_data = user_data;
    m_loop_list.push_back(pItem);
}

void CEventDispatch::_CheckLoop()
{
    for (list<TimerItem*>::iterator it = m_loop_list.begin(); it != m_loop_list.end(); it++) {
        TimerItem* pItem = *it;
        pItem->callback(pItem->user_data, NETLIB_MSG_LOOP, 0, NULL);
    }
}

CEventDispatch* CEventDispatch::Instance()
{
	if (m_pEventDispatch == NULL)
	{
		m_pEventDispatch = new CEventDispatch();
	}

	return m_pEventDispatch;
}

#ifdef _WIN32
// windows用select
void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event)
{
	CAutoLock func_lock(&m_lock);

	if ((socket_event & SOCKET_READ) != 0)
	{
		FD_SET(fd, &m_read_set);
	}
		
	if ((socket_event & SOCKET_WRITE) != 0)
	{
		FD_SET(fd, &m_write_set);
	}

	if ((socket_event & SOCKET_EXCEP) != 0)
	{
		FD_SET(fd, &m_excep_set);
	}
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	CAutoLock func_lock(&m_lock);

	if ((socket_event & SOCKET_READ) != 0)
	{
		FD_CLR(fd, &m_read_set);
	}

	if ((socket_event & SOCKET_WRITE) != 0)
	{
		FD_CLR(fd, &m_write_set);
	}

	if ((socket_event & SOCKET_EXCEP) != 0)
	{
		FD_CLR(fd, &m_excep_set);
	}
}

void CEventDispatch::StartDispatch(uint32_t wait_timeout)
{
	fd_set read_set, write_set, excep_set;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = wait_timeout * 1000;	// 10 millisecond

    if(running)
        return;
    running = true;
    
    while (running)
	{
		_CheckTimer();
        _CheckLoop();

		if (!m_read_set.fd_count && !m_write_set.fd_count && !m_excep_set.fd_count)
		{
			Sleep(MIN_TIMER_DURATION);
			continue;
		}

		m_lock.lock();
		memcpy(&read_set, &m_read_set, sizeof(fd_set));
		memcpy(&write_set, &m_write_set, sizeof(fd_set));
		memcpy(&excep_set, &m_excep_set, sizeof(fd_set));
		m_lock.unlock();

		int nfds = select(0, &read_set, &write_set, &excep_set, &timeout);

		if (nfds == SOCKET_ERROR)
		{
			log("select failed, error code: %d", GetLastError());
			Sleep(MIN_TIMER_DURATION);
			continue;			// select again
		}

		if (nfds == 0)
		{
			continue;
		}

		for (u_int i = 0; i < read_set.fd_count; i++)
		{
			//log("select return read count=%d\n", read_set.fd_count);
			SOCKET fd = read_set.fd_array[i];
			CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
			if (pSocket)
			{
				pSocket->OnRead();
				pSocket->ReleaseRef();
			}
		}

		for (u_int i = 0; i < write_set.fd_count; i++)
		{
			//log("select return write count=%d\n", write_set.fd_count);
			SOCKET fd = write_set.fd_array[i];
			CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
			if (pSocket)
			{
				pSocket->OnWrite();
				pSocket->ReleaseRef();
			}
		}

		for (u_int i = 0; i < excep_set.fd_count; i++)
		{
			//log("select return exception count=%d\n", excep_set.fd_count);
			SOCKET fd = excep_set.fd_array[i];
			CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
			if (pSocket)
			{
				pSocket->OnClose();
				pSocket->ReleaseRef();
			}
		}

	}
}

void CEventDispatch::StopDispatch()
{
    running = false;
}

#elif __APPLE__

void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event)
{
	struct kevent ke;

	if ((socket_event & SOCKET_READ) != 0)
	{
		EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
		kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
	}

	if ((socket_event & SOCKET_WRITE) != 0)
	{
		EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
		kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
	}
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	struct kevent ke;

	if ((socket_event & SOCKET_READ) != 0)
	{
		EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
	}

	if ((socket_event & SOCKET_WRITE) != 0)
	{
		EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
	}
}

void CEventDispatch::StartDispatch(uint32_t wait_timeout)
{
	struct kevent events[1024];
	int nfds = 0;
	struct timespec timeout;
	timeout.tv_sec = 0;
	timeout.tv_nsec = wait_timeout * 1000000;

    if(running)
        return;
    running = true;
    
    while (running)
	{
		nfds = kevent(m_kqfd, NULL, 0, events, 1024, &timeout);

		for (int i = 0; i < nfds; i++)
		{
			int ev_fd = events[i].ident;
			CBaseSocket* pSocket = FindBaseSocket(ev_fd);
			if (!pSocket)
				continue;

			if (events[i].filter == EVFILT_READ)
			{
				//log("OnRead, socket=%d\n", ev_fd);
				pSocket->OnRead();
			}

			if (events[i].filter == EVFILT_WRITE)
			{
				//log("OnWrite, socket=%d\n", ev_fd);
				pSocket->OnWrite();
			}

			pSocket->ReleaseRef();
		}

		_CheckTimer();
        _CheckLoop();
	}
}

void CEventDispatch::StopDispatch()
{
    running = false;
}

#else

void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0)
	{
		log("epoll_ctl() failed, errno=%d", errno);
	}
}

void CEventDispatch::AddUDPEvent(SOCKET fd, uint8_t socket_event)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0)
	{
		log("epoll_ctl() failed, errno=%d", errno);
	}
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) != 0)
	{
		log("epoll_ctl failed, errno=%d", errno);
	}
}


// unix用epoll
void CEventDispatch::StartDispatch(uint32_t wait_timeout)
{
	struct epoll_event events[1024];
	int nfds = 0;

    if(running)
        return;
    running = true;
    
	while (running)
	{
		nfds = epoll_wait(m_epfd, events, 1024, wait_timeout);
		if (nfds == -1){
			printf("epoll_wait error");
			break;
		} else {
			printf("epoll_wait %d \n",nfds);
		}

		for (int i = 0; i < nfds; i++)
		{
			log("epoll_wait nfds %d", nfds);

			int ev_fd = events[i].data.fd;
			CBaseSocket* pSocket = FindBaseSocket(ev_fd);
			if (!pSocket){
				log("epoll_wait FindBaseSocket null");
				continue;
			}
            
            //Commit by zhfu @2015-02-28
            #ifdef EPOLLRDHUP
            if (events[i].events & EPOLLRDHUP)
            {
                log("On Peer Close, socket=%d", ev_fd);
                pSocket->OnClose();
            }
            #endif
            // Commit End

			if (events[i].events & EPOLLIN)
			{
				log("OnRead, socket=%d\n", ev_fd);
				pSocket->OnRead();
			}

			if (events[i].events & EPOLLOUT)
			{
				log("OnWrite, socket=%d\n", ev_fd);
				pSocket->OnWrite();
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				log("OnClose, socket=%d\n", ev_fd);
				pSocket->OnClose();
			}

			pSocket->ReleaseRef();// udp这个释放后会有问题 
		}

		_CheckTimer();
        _CheckLoop();
	}
}
//xieqq 2016-05-12///////////////////
// 基本同上面的，不用_CheckTimer(); _CheckLoop();
void CEventDispatch::StartDispatchUDP(uint32_t wait_timeout)
{
	struct epoll_event events[1024];
	int nfds = 0;

    if(running)
        return;
    running = true;
    
	while (running)
	{
		nfds = epoll_wait(m_epfd, events, 1024, wait_timeout);
		for (int i = 0; i < nfds; i++)
		{
			int ev_fd = events[i].data.fd;
			CBaseSocket* pSocket = FindBaseSocket(ev_fd);
			if (!pSocket){
				printf("pSocket null EventDispatch 451");
				continue;
            }
            //Commit by zhfu @2015-02-28
            #ifdef EPOLLRDHUP
            if (events[i].events & EPOLLRDHUP)
            {
                printf("EventDispatch On Peer Close, socket=%d", ev_fd);
                pSocket->OnUDPClose();
            }
            #endif
            // Commit End

			if (events[i].events & EPOLLIN)
			{
				printf("EventDispatch OnRead, socket=%d\n", ev_fd);
				pSocket->OnUDPRead();
			}

			if (events[i].events & EPOLLOUT)
			{
				printf("EventDispatch OnWrite, socket=%d\n", ev_fd);
				pSocket->OnUDPWrite();
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				printf("OnClose, socket=%d\n", ev_fd);
				pSocket->OnUDPClose();
			}
			printf("psocket releaseref() %d \n", events[i]);
			pSocket->ReleaseRef(); // 
		}

		//_CheckTimer();
        //_CheckLoop();
	}
}
/////////////////////////////////////


void CEventDispatch::StopDispatch()
{
    running = false;
}


#endif
