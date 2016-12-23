#include "BaseSocket.h"
#include "EventDispatch.h"

typedef hash_map<net_handle_t, CBaseSocket*> SocketMap;
SocketMap	g_socket_map;

void AddBaseSocket(CBaseSocket* pSocket)
{
	g_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

void RemoveBaseSocket(CBaseSocket* pSocket)
{
	g_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

CBaseSocket* FindBaseSocket(net_handle_t fd)
{
	CBaseSocket* pSocket = NULL;
	SocketMap::iterator iter = g_socket_map.find(fd);
	if (iter != g_socket_map.end())
	{
		pSocket = iter->second;
		pSocket->AddRef();
	}

	return pSocket;
}

//////////////////////////////

CBaseSocket::CBaseSocket()
{
	//log("CBaseSocket::CBaseSocket\n");
	m_socket = INVALID_SOCKET;
	m_state = SOCKET_STATE_IDLE;
}

CBaseSocket::~CBaseSocket()
{
	//log("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

int CBaseSocket::Listen(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
	m_local_ip = server_ip;
	m_local_port = port;
	m_callback = callback;
	m_callback_data = callback_data;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket failed, err_code=%d\n", _GetErrorCode());
		return NETLIB_ERROR;
	}

	_SetReuseAddr(m_socket);
	_SetNonblock(m_socket);

	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == SOCKET_ERROR)
	{
		log("bind failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_ERROR;
	}

	ret = listen(m_socket, 64);
	if (ret == SOCKET_ERROR)
	{
		log("listen failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_ERROR;
	}

	m_state = SOCKET_STATE_LISTENING;

	log("CBaseSocket::Listen on %s:%d", server_ip, port);

	AddBaseSocket(this);
	CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_READ | SOCKET_EXCEP);
	return NETLIB_OK;
}

net_handle_t CBaseSocket::Connect(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
	log("CBaseSocket::Connect, server_ip=%s, port=%d", server_ip, port);

	m_remote_ip = server_ip;
	m_remote_port = port;
	m_callback = callback;
	m_callback_data = callback_data;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		log("socket failed, err_code=%d", _GetErrorCode());
		return NETLIB_INVALID_HANDLE;
	}

	_SetNonblock(m_socket);
	_SetNoDelay(m_socket);
	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
	int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if ( (ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode())) )
	{	
		log("connect failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}
	m_state = SOCKET_STATE_CONNECTING;
	AddBaseSocket(this);
	CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_ALL);
	
	return (net_handle_t)m_socket;
}

int CBaseSocket::Send(void* buf, int len)
{
	if (m_state != SOCKET_STATE_CONNECTED)
		return NETLIB_ERROR;

	int ret = send(m_socket, (char*)buf, len, 0);
	if (ret == SOCKET_ERROR)
	{
		int err_code = _GetErrorCode();
		if (_IsBlock(err_code))
		{
#if ((defined _WIN32) || (defined __APPLE__))
			CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE);
#endif
			ret = 0;
			//log("socket send block fd=%d", m_socket);
		}
		else
		{
			log("!!!send failed, error code: %d", err_code);
		}
	}

	return ret;
}

int CBaseSocket::Recv(void* buf, int len)
{
	return recv(m_socket, (char*)buf, len, 0);
}

int CBaseSocket::Close()
{
	log("basesocket close \n");
	CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_ALL);
	RemoveBaseSocket(this);
	closesocket(m_socket);
	ReleaseRef();

	return 0;
}

//xie 2016-06-02 start//////////////////////////////
void CBaseSocket::OnUDPClose()
{
	log("basesocket onudpclose \n");
	m_state = SOCKET_STATE_CLOSING;
	m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}

void CBaseSocket::OnUDPRead()
{
	u_long avail = 0;
	if ( (ioctlsocket(m_socket, FIONREAD, &avail) == SOCKET_ERROR) || (avail == 0) )
	{
		printf("onUdpRead errno=%d",errno);
		m_callback(m_callback_data, NETLIB_MSG_CLOSE_UDP, (net_handle_t)m_socket, NULL);
	}
	else
	{
		m_callback(m_callback_data, NETLIB_MSG_READ_UDP, (net_handle_t)m_socket, NULL);
	}
}
void CBaseSocket::OnUDPWrite()
{		
	m_callback(m_callback_data, NETLIB_MSG_WRITE_UDP, (net_handle_t)m_socket, NULL);
}

//xie 2016-06-02 end//////////////////////////////

void CBaseSocket::OnRead()
{
	if (m_state == SOCKET_STATE_LISTENING)
	{
		_AcceptNewSocket();
	}
	else  // UDP的时候的状态 SOCKET_STATE_UDP_BIND 
	{
		u_long avail = 0;
		if ( (ioctlsocket(m_socket, FIONREAD, &avail) == SOCKET_ERROR) || (avail == 0) )
		{
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
		}
		else
		{
			m_callback(m_callback_data, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);
		}
	}
}

void CBaseSocket::OnWrite()
{
#if ((defined _WIN32) || (defined __APPLE__))
	CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_WRITE);
#endif
	printf("CBaseSocket::OnWrite m_state = %d", m_state);

	if (m_state == SOCKET_STATE_CONNECTING) // 客户端连服务端成功状态会是SOCKET_STATE_CONNECTING
	{
		int error = 0;
		socklen_t len = sizeof(error);
#ifdef _WIN32

		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
#endif
		if (error) {
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
		} else {
			log("CBaseSocket::OnWrite SOCKET_STATE_CONNECTED");
			m_state = SOCKET_STATE_CONNECTED;
			m_callback(m_callback_data, NETLIB_MSG_CONFIRM, (net_handle_t)m_socket, NULL);
		}
	}
	else
	{
		log("CBaseSocket::OnWrite m_state = %d", m_state);

		m_callback(m_callback_data, NETLIB_MSG_WRITE, (net_handle_t)m_socket, NULL);
	}
}

void CBaseSocket::OnClose()
{
	log("basesocket onclose \n");
	m_state = SOCKET_STATE_CLOSING;
	m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}

void CBaseSocket::SetSendBufSize(uint32_t send_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &send_size, 4);
	if (ret == SOCKET_ERROR) {
		log("set SO_SNDBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &size, &len);
	log("socket=%d send_buf_size=%d", m_socket, size);
}

void CBaseSocket::SetRecvBufSize(uint32_t recv_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &recv_size, 4);
	if (ret == SOCKET_ERROR) {
		log("set SO_RCVBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &size, &len);
	log("socket=%d recv_buf_size=%d", m_socket, size);
}

int CBaseSocket::_GetErrorCode()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool CBaseSocket::_IsBlock(int error_code)
{
#ifdef _WIN32
	return ( (error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK) );
#else
	return ( (error_code == EINPROGRESS) || (error_code == EWOULDBLOCK) );
#endif
}

void CBaseSocket::_SetNonblock(SOCKET fd)
{
#ifdef _WIN32
	u_long nonblock = 1;
	int ret = ioctlsocket(fd, FIONBIO, &nonblock); // windows用完成端口  unix用epoll
#else
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
	if (ret == SOCKET_ERROR)
	{
		log("_SetNonblock failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetReuseAddr(SOCKET fd)
{
	int reuse = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	if (ret == SOCKET_ERROR)
	{
		log("_SetReuseAddr failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetNoDelay(SOCKET fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (ret == SOCKET_ERROR)
	{
		log("_SetNoDelay failed, err_code=%d", _GetErrorCode());
	}
}
void CBaseSocket::_SetAddr(const uint16_t port, sockaddr_in* pAddr)
{	
	log("_setaddr %d", port);
	memset(pAddr, 0, sizeof(sockaddr_in));
	pAddr->sin_family = AF_INET;
	pAddr->sin_port = htons(port);
	// 有的服务器是多宿主机，至少有两个网卡，那么运行在这样的服务器上的服务程序在为其socket绑定IP地址时
	// 可以把htonl(INADDR_ANY)置给s_addr，这样做的好处是不论哪个网段上的客户程序都能与该服务程序通信；
	pAddr->sin_addr.s_addr = INADDR_ANY; // htonl(INADDR_ANY); 使用INADDR_ANY 指示任意地址 	
	if (pAddr->sin_addr.s_addr == INADDR_NONE)
	{
		log("sin_addr failed, ip=INADDR_ANY");
	}
}

void CBaseSocket::_SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr)
{
	memset(pAddr, 0, sizeof(sockaddr_in));
	pAddr->sin_family = AF_INET;
	pAddr->sin_port = htons(port);
	// 有的服务器是多宿主机，至少有两个网卡，那么运行在这样的服务器上的服务程序在为其socket绑定IP地址时
	// 可以把htonl(INADDR_ANY)置给s_addr，这样做的好处是不论哪个网段上的客户程序都能与该服务程序通信；
	pAddr->sin_addr.s_addr = inet_addr(ip); // htonl(INADDR_ANY); 使用INADDR_ANY 指示任意地址 
	if (pAddr->sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(ip);
		if (host == NULL)
		{
			log("gethostbyname failed, ip=%s", ip);
			return;
		}

		pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
	}
}

void CBaseSocket::_AcceptNewSocket()
{
	SOCKET fd = 0;
	sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	char ip_str[64];
	while ( (fd = accept(m_socket, (sockaddr*)&peer_addr, &addr_len)) != INVALID_SOCKET )
	{
		CBaseSocket* pSocket = new CBaseSocket();
		uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
		uint16_t port = ntohs(peer_addr.sin_port);

		snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

		log("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

		pSocket->SetSocket(fd);
		pSocket->SetCallback(m_callback);
		pSocket->SetCallbackData(m_callback_data);
		pSocket->SetState(SOCKET_STATE_CONNECTED);
		pSocket->SetRemoteIP(ip_str);
		pSocket->SetRemotePort(port);

		_SetNoDelay(fd);
		_SetNonblock(fd);
		AddBaseSocket(pSocket);
		CEventDispatch::Instance()->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);
		m_callback(m_callback_data, NETLIB_MSG_CONNECT, (net_handle_t)fd, NULL);
	}
}

//add by xieqq 2016-05-11  udp socket////////////////////////////
int CBaseSocket::UDP_Bind(const char* server_ip, uint16_t port,  callback_t callback, void* callback_data)
{
	m_local_ip = server_ip;
	m_local_port = port;
	m_callback = callback;
	m_callback_data = callback_data;

	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket failed, err_code=%d\n", _GetErrorCode());
		return NETLIB_ERROR;
	}
	// 这通常是重启监听服务器时出现，若不设置此选项，则bind时将出错。
	_SetReuseAddr(m_socket); //  /*设置socket属性，端口可以重用*/

	_SetNonblock(m_socket); // 设置句柄为非阻塞方式

	sockaddr_in serv_addr;

	printf("socket success");

	_SetAddr(server_ip, port, &serv_addr);
	// 绑定端口
    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == SOCKET_ERROR)
	{
		log("bind failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_ERROR;
	} 

	//ret = listen(m_socket, 64); // udp不用listen
	//if (ret == SOCKET_ERROR)
	//{
	//	log("listen failed, err_code=%d", _GetErrorCode());
	//	closesocket(m_socket);
	//	return NETLIB_ERROR;
	//}

	m_state = SOCKET_STATE_UDP_BIND;

	log("CBaseSocket::UDP_Bind on any %s :%d", server_ip, port);

	AddBaseSocket(this); // g_socket_map.insert	
		
	// UDP不用连接，直接调用连接成功
	m_callback(m_callback_data, NETLIB_MSG_CONNECT, (net_handle_t)m_socket, NULL);

	CEventDispatch::Instance()->AddUDPEvent(m_socket, SOCKET_READ | SOCKET_EXCEP);// SOCKET_ALL);//SOCKET_READ | SOCKET_EXCEP);
	// udp 只是sendto recvfrom 要不要用epoll?  
	//  先用epoll试试  epoll收不到消息?
	//  就不addEvent了

	return NETLIB_OK;
}
//add by xieqq 2016-05-11 end///////////////////////////////////
