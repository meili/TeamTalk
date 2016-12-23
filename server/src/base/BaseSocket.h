/*
 *  a wrap for non-block socket class for Windows, LINUX and MacOS X platform
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ostype.h"
#include "util.h"
/*
// ostype.h中定义过了
enum  
{
	SOCKET_STATE_IDLE,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_CLOSING,
	SOCKET_STATE_UDP_BIND
};*/

class CBaseSocket : public CRefObject
{
public:
	CBaseSocket();
	
	virtual ~CBaseSocket();

	SOCKET GetSocket() { return m_socket; }
	void SetSocket(SOCKET fd) { m_socket = fd; }
	void SetState(uint8_t state) { m_state = state; }

	void SetCallback(callback_t callback) { m_callback = callback; }
	void SetCallbackData(void* data) { m_callback_data = data; }
	void SetRemoteIP(char* ip) { m_remote_ip = ip; }
	void SetRemotePort(uint16_t port) { m_remote_port = port; }
	void SetSendBufSize(uint32_t send_size);
	void SetRecvBufSize(uint32_t recv_size);

	const char*	GetRemoteIP() { return m_remote_ip.c_str(); }
	uint16_t	GetRemotePort() { return m_remote_port; }
	const char*	GetLocalIP() { return m_local_ip.c_str(); }
	uint16_t	GetLocalPort() { return m_local_port; }
public:
	int Listen(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	net_handle_t Connect(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	int Send(void* buf, int len);

	int Recv(void* buf, int len);

	int Close();
//add by xieqq 2016-05-11  udp socket////////////////////////////
	int UDP_Bind(const char* server_ip, uint16_t port,  callback_t callback, void* callback_data);
//add by xieqq 2016-05-11 end///////////////////////////////////
public:	
	void OnRead();
	void OnWrite();
	void OnClose();

	void OnUDPRead();
	void OnUDPWrite();
	void OnUDPClose();
private:	
	int _GetErrorCode();
	bool _IsBlock(int error_code);

	void _SetNonblock(SOCKET fd);
	void _SetReuseAddr(SOCKET fd);
	void _SetNoDelay(SOCKET fd);
	void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr);

	void _AcceptNewSocket();
//add by xieqq 2016-05-11  udp socket////////////////////////////
	void _SetAddr(const uint16_t port, sockaddr_in* pAddr);
//add by xieqq 2016-05-11 end///////////////////////////////////

private:
	string			m_remote_ip;
	uint16_t		m_remote_port;
	string			m_local_ip;
	uint16_t		m_local_port;

	callback_t		m_callback;
	void*			m_callback_data;

	uint8_t			m_state;
	SOCKET			m_socket;
};

CBaseSocket* FindBaseSocket(net_handle_t fd);

#endif
