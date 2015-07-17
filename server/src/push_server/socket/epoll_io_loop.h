/**	@file epoll_io_loop.h
 *	@note 
 *	@brief CEPOLLIOLoop类的头文件。
 *
 *	@author		shiwei
 *	@date		2014/03/20
 *
 *	@note 该类使用epoll的LT模式完成网络IO的读写
 *	@note 历史记录：
 *	@note V1.0.0  创建文件
 */
#ifndef _EPOLL_IO_LOOP_H
#define _EPOLL_IO_LOOP_H

#include "socket_io.h"
#include "../thread/base_thread.hpp"
#include "base_socket.hpp"
#include "../type/base_type.h"
#include "base_io_stream.h"
#include "io_loop.h"

#include <map>
#include <vector>

#if defined(__linux__)
#include <sys/epoll.h>  
#endif

using namespace std;

#define EPOLL_SIZE  256


class SOCKET_IO_DECLARE_CLASS CEpollIOLoop : public CIOLoop
{
public:
	CEpollIOLoop(void);
	virtual ~CEpollIOLoop(void);

#if defined(__linux__)
	virtual void Start(int32_t nEpollSize = EPOLL_SIZE);
	virtual void Stop();
	virtual void Run();

	virtual void Add_Handler( CBaseIOStream* piostream );
	virtual void Remove_Handler(CBaseIOStream* piostream);
	virtual void Add_WriteEvent(CBaseIOStream* piostream);
	virtual void Remove_WriteEvent(CBaseIOStream* piostream);
private:
	void _SetEpollSize(int32_t nSize) { m_epollszie = nSize; }
	int _GetEpollSize() { return m_epollszie; }
#endif

private:
	int32_t m_eid;
	int32_t m_epollszie;

};
#endif
