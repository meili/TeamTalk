//
//  base_socket.h
//  my-code
//
//  Created by shiwei on 14-7-14.
//  Copyright (c) 2014年 shiwei. All rights reserved.
//

#ifndef _BASE_SOCKET_HPP
#define _BASE_SOCKET_HPP

#if (defined(_WIN32) || defined(_WIN64))

#else
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "../type/base_type.h"


#include <string.h>

typedef int32_t	S_SOCKET;
const int32_t S_SOCKET_ERROR	= -1;
const int32_t S_INVALID_SOCKET  = -1;
#define S_SHUT_RD		SHUT_RD
#define S_SHUT_WR		SHUT_WR
#define S_SHUT_RDWR	    SHUT_RDWR



#ifdef __cplusplus
extern "C"{
#endif
    
/**
 *  S_CreateSocket
 *
 *  @param nAf (IN) address family AF_INET/AF_INET6.
 *  @param nType (IN) socket type SOCK_DGRAM/SOCK_STREAM.
 *  @param nProto (IN) 0
 *
 *  @return S_INVALID_SOCKET fail, else success.
 */
static S_SOCKET S_CreateSocket(int32_t nAf, int32_t nType, int32_t nProto)
{
    return socket(nAf, nType, nProto);
}

/**
 *  S_CloseSocket
 *
 *  @param nSockFd (IN) socket fd get by HPR_CreateSocket.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_CloseSocket(S_SOCKET nSockFd)
{
#ifdef _WIN32
	return closesocket(nSockFd);
#else
    return close(nSockFd);
#endif
    
}

/**
 *  S_ShutDown shutdown socket fd.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nHow (IN) how to shut. S_SHUT_RD/S_SHUT_WR/S_SHUT_RDWR.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_ShutDown(S_SOCKET nSockFd, int32_t nHow)
{
    return shutdown(nSockFd, nHow);
}

/**
 *  S_Bind bind socket fd to address.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param szIP (IN) local ip.
 *  @param nPort (IN) local port.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_Bind(S_SOCKET nSockFd, const char* szIP, int32_t nPort)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
    inet_pton(addr.sin_family, szIP, (void *)&(addr.sin_addr));
	//addr.sin_addr.s_addr = inet_addr(szIP);
    //此处最好使用::,否则头文件引用得时候引入了std，会定位成std::bind得函数
    return ::bind(nSockFd, (sockaddr*)&addr, (int)sizeof(addr));
}
    
/**
 *  S_Connect connect remote ip port
 *
 *  @param nSockFd (IN) socket fd
 *  @param szIP (IN) remote ip
 *  @param nPort (IN) remote port
 *
 *  @return 0 success, -1 fail
 */
static int32_t S_Connect(S_SOCKET nSockFd, const char* szIP, int32_t nPort)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
    inet_pton(addr.sin_family, szIP, (void *)&(addr.sin_addr));
    return connect(nSockFd, (sockaddr*)&addr, sizeof(addr));
}

/**
 *  S_Listen listen on port (tcp).
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nNum (IN) listen queue long.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_Listen(S_SOCKET nSockFd, int32_t nNum)
{
    return listen(nSockFd, nNum);
}

/**
 *  HPR_Accept accept remote connect (tcp)
 *
 *  @param nSockFd (IN) socket fd.
 *  @param szIP (OUT) remote ip.
 *  @param nPort (OUT) remote port.
 *
 *  @return S_INVALID_SOCKET fail, else success.
 */
static S_SOCKET S_Accept(S_SOCKET nSockFd, char* szIP, int32_t* nPort)
{
    if (szIP == NULL || nPort == NULL) {
        return S_SOCKET_ERROR;
    }
    S_SOCKET fd = 0;
    char peer_ip[32] = {0};
    sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
    fd = accept(nSockFd, (sockaddr*)&peer_addr, &addr_len);
    if (fd != S_INVALID_SOCKET)
    {
        inet_ntop(AF_INET, (void *)&(peer_addr.sin_addr), peer_ip, sizeof(peer_ip));
        strcpy(szIP, peer_ip);
        *nPort = ntohs(peer_addr.sin_port);
    }
    else
    {
        strcpy(szIP, "");
        *nPort = 0;
    }
    return fd;
}

/**
 *  S_SetNonBlock set enable/disable nonblock.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param bNoBlock enable/disable nonblock.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetNoBlock(S_SOCKET nSockFd, BOOL bNoBlock)
{
#ifdef _WIN32
	int32_t ret = ioctlsocket(nSockFd, FIONBIO, &bNoBlock);
#else
    int32_t ret = 0;
    if (bNoBlock)
    {
        ret = fcntl(nSockFd, F_SETFL, O_NONBLOCK | fcntl(nSockFd, F_GETFL));
    }
    else
    {
        ret = fcntl(nSockFd, F_SETFL, ~O_NONBLOCK & fcntl(nSockFd, F_GETFL));
    }
#endif
    return ret;
}

/**
 *  S_SetTTL set ttl
 *
 *  @param nSockFd (IN) socket fd
 *  @param nTTL (IN) ttl
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetTTL(S_SOCKET nSockFd, int32_t nTTL)
{
    return setsockopt(nSockFd, IPPROTO_IP, IP_TTL, (char *)&nTTL, sizeof(nTTL));
}

/**
 *  S_GetTTL get ttl
 *
 *  @param nSockFd (IN) socket fd
 *  @param nTTL (OUT) ttl
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_GetTTL(S_SOCKET nSockFd, int32_t* nTTL)
{
    socklen_t len = sizeof(int32_t);
    return getsockopt(nSockFd, IPPROTO_IP, IP_TTL, nTTL, &len);
}

/**
 *  S_SetTOS set tos
 *
 *  @param nSockFd (IN)  socket fd
 *  @param nTOS (IN)
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetTOS(S_SOCKET nSockFd, int32_t nTOS)
{
    return setsockopt(nSockFd, IPPROTO_IP, IP_TOS, (char *)&nTOS, sizeof(nTOS));
}

/**
 *  S_GetTOS get tos
 *
 *  @param nSockFd (IN) socket fd
 *  @param nTOS (OUT)
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_GetTOS(S_SOCKET nSockFd, int32_t* nTOS)
{
    socklen_t len = sizeof(int32_t);
    return getsockopt(nSockFd, IPPROTO_IP, IP_TOS, nTOS, &len);
}

/**
 *  S_SetReuseAddr set enable/disable reuse address.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param bReuse  (IN) enable/disable reuse address.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetReuseAddr(S_SOCKET nSockFd, BOOL bReuse)
{
    return setsockopt(nSockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(bReuse));
}

/**
 *  S_SetSendTimeOut set socket fd I/O timeout.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nSendTimeOut (IN) snd timeout, unit: sec
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetSendTimeOut(S_SOCKET nSockFd, int32_t nSendTimeOut)
{
#if (defined(_WIN32) || defined(_WIN64))
	nSendTimeOut *= 1000;
#endif
    struct timeval timeout = {nSendTimeOut, 0};
	socklen_t len = sizeof(timeout);
	return setsockopt(nSockFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
}

/**
 *  S_SetRecvTimeOut set socket fd I/O timeout.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nRecvTimeOut (IN) rcv timeout, uint: sec
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_SetRecvTimeOut(S_SOCKET nSockFd, int32_t nRecvTimeOut)
{
#if (defined(_WIN32) || defined(_WIN64))
	nRecvTimeOut *= 1000;
#endif
    struct timeval timeout = {nRecvTimeOut, 0};
	socklen_t len = sizeof(timeout);
	return setsockopt(nSockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
}

/**
 *   S_SetSendBuffSize set socket snd buffsize.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nSendBuffSize (IN) snd buffer size.
 *
 *  @return 0 succes, -1 fail.
 */
static int32_t S_SetSendBuffSize(S_SOCKET nSockFd, int32_t nSendBuffSize)
{
    return setsockopt(nSockFd, SOL_SOCKET, SO_SNDBUF, &nSendBuffSize, sizeof(nSendBuffSize));
}

/**
 *  S_SetRecvBuffSize set socket rcv buffsize.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nRecvBuffSize (IN) rcv buffer size.
 *
 *  @return 0 succes, -1 fail.
 */
static int32_t S_SetRecvBuffSize(S_SOCKET nSockFd, int32_t nRecvBuffSize)
{
    return setsockopt(nSockFd, SOL_SOCKET, SO_RCVBUF, &nRecvBuffSize, sizeof(nRecvBuffSize));
}

/**
 *  S_GetSendBuffSize get socket snd buffsize.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nSendBuffSize (OUT) snd buffer size.
 *
 *  @return 0 succes, -1 fail.
 */
static int32_t S_GetSendBuffSize(S_SOCKET nSockFd, int32_t* nSendBuffSize)
{
    socklen_t len = sizeof(int32_t);
    return getsockopt(nSockFd, SOL_SOCKET, SO_SNDBUF, nSendBuffSize, &len);
}

/**
 *  S_GetRecvBuffSize get socket rcv buffsize.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nRecvBuffSize (OUT) rcv buffer size.
 *
 *  @return 0 succes, -1 fail.
 */
static int32_t S_GetRecvBuffSize(S_SOCKET nSockFd, int32_t* nRecvBuffSize)
{
    socklen_t len = sizeof(int32_t);
    return getsockopt(nSockFd, SOL_SOCKET, SO_RCVBUF, nRecvBuffSize, &len);

}

/**
 *  S_LingerOn disable linger (tcp)
 *
 *  @param nSockFd (IN) socket fd.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_LingerOff(S_SOCKET nSockFd)
{
    struct linger so_linger;
    so_linger.l_onoff = FALSE;    //system default setting
    so_linger.l_linger = 0;
    return setsockopt(nSockFd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
}

//?
/**
 *  S_LingerOn enable linger (tcp)
 *
 *  @param nSockFd (IN) socket fd.
 *  @param nTimeOutSec (IN) linger time.
 *
 *  @return 0 success, -1 fail.
 */
static int32_t S_LingerOn(S_SOCKET nSockFd, uint16_t nTimeOutSec)
{
    struct linger so_linger;
#if (defined(_WIN32) || defined(_WIN64))
	nTimeOutSec *= 1000;
#endif
    so_linger.l_onoff = TRUE;
    so_linger.l_linger = nTimeOutSec;
    return setsockopt(nSockFd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
}

//?
/**
 *  S_SetNoDelay enable/disabel nagle algorithm (tcp)
 *
 *  @param nSockFd  (IN) socket
 *  @param bNoDelay (IN) enable/disabel nagle algorithm
 *
 *  @return 0 success, -1 fail
 */
static int32_t S_SetNoDelay(S_SOCKET nSockFd, BOOL bNoDelay)
{
    return setsockopt(nSockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));

}

/**
 *  S_Send send dada to socket fd (tcp).
 *
 *  @param nSockFd (IN) socket fd.
 *  @param pBuf (IN) data pointer to send data.
 *  @param nBufLen (IN) data count want to send.
 *
 *  @return count of data sent, -1 for error.
 */
static int32_t S_Send(S_SOCKET nSockFd, void* pBuf, int32_t nBufLen)
{
    return send(nSockFd, pBuf, nBufLen, 0);
}

/**
 *  S_Sendn send n byte dada to socket fd (tcp).
 *
 *  @param nSockFd  (IN) socket fd.
 *  @param pBuf pBuf (OUT) data pointer to recv data.
 *  @param nBufSize (IN) data length want to recv.
 *
 *  @return count of data received, -1 for error.
 */
static int32_t S_Recv(S_SOCKET nSockFd, void* pBuf, int32_t nBufSize)
{
    return recv(nSockFd, pBuf, nBufSize, 0);
}

/**
 *  S_SendTo sendto (udp)
 *
 *  @param nSockFd (IN) socket fd.
 *  @param pBuf (IN) data pointer.
 *  @param nBufLen (IN) max data buffer size
 *  @param szIP (IN) remote ip.
 *  @param nPort (IN) remote port.
 *
 *  @return count of data sent, -1 for errror.
 */
static int32_t S_SendTo(S_SOCKET nSockFd, void* pBuf, int32_t nBufLen, const char* szIP, int32_t nPort)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
    inet_pton(addr.sin_family, szIP, (void *)&(addr.sin_addr));
	//addr.sin_addr.s_addr = inet_addr(szIP);
    return sendto(nSockFd, pBuf, nBufLen, 0, (sockaddr*)&addr, sizeof(addr));
}

/**
 *  S_RecvFrom recvfrom (udp)
 *
 *  @param nSockFd (IN) socket fd
 *  @param pBuf (OUT) data pointer.
 *  @param nBufSize (IN) max data buffer size.
 *  @param szIP (OUT) remote ip.
 *  @param nPort (OUT) remote port.
 *
 *  @return count of data received, -1 for error.
 */
static int32_t S_RecvFrom(S_SOCKET nSockFd, void* pBuf, int32_t nBufSize, char* szIP, int32_t* nPort)
{
    if (szIP == NULL || nPort == NULL) {
        return S_SOCKET_ERROR;
    }
    S_SOCKET fd = 0;
    char peer_ip[32] = {0};
    sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
    int32_t ret = recvfrom(nSockFd, pBuf, nBufSize, 0, (sockaddr*)&peer_addr, &addr_len);
    if (ret >= 0)
    {
        inet_ntop(AF_INET, (void *)&(peer_addr.sin_addr), peer_ip, sizeof(peer_ip));
        strcpy(szIP, peer_ip);
        *nPort = ntohs(peer_addr.sin_port);
    }
    else
    {
        strcpy(szIP, "");
        *nPort = 0;
    }
    return ret;
}

//int32_t S_Ioctl(S_SOCKET nSockFd, int32_t nCmd, int32_t* nData);

/**
 *  S_GetSockName get host ip and port.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param szIP (OUT) host ip.
 *  @param nPort  (OUT) host port.
 *
 *  @return 0 success, -1 fail
 */
static int32_t S_GetSockName(S_SOCKET nSockFd, char* szIP, int32_t* nPort)
{
    if (szIP == NULL || nPort == NULL) {
        return S_SOCKET_ERROR;
    }
    char host_ip[32] = {0};
    struct sockaddr_in host_addr;
    int32_t addr_len = sizeof(host_addr);
    int32_t ret = getsockname(nSockFd, (struct sockaddr *)&host_addr, (socklen_t*)&addr_len);
    if (ret == 0) {
        inet_ntop(AF_INET, &host_addr.sin_addr, host_ip, sizeof(host_ip));
        strcpy(szIP, host_ip);
        *nPort = ntohs(host_addr.sin_port);
    }
    else
    {
        strcpy(szIP, "");
        *nPort = 0;
    }
    return ret;
}

/**
 *  S_GetPeerName get remote ip and port.
 *
 *  @param nSockFd (IN) socket fd.
 *  @param szIP (OUT) remote ip.
 *  @param nPort (OUT) remote port.
 *
 *  @return 0 success, -1 fail
 */
static int32_t S_GetPeerName(S_SOCKET nSockFd, char* szIP, int32_t* nPort)
{
    if (szIP == NULL || nPort == NULL) {
        return S_SOCKET_ERROR;
    }
    char peer_ip[32] = {0};
    struct sockaddr_in peer_addr;
    int32_t addr_len = sizeof(peer_addr);
    int32_t ret = getpeername(nSockFd, (struct sockaddr *)&peer_addr, (socklen_t*)&addr_len);
    if (ret == 0) {
        inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
        strcpy(szIP, peer_ip);
        *nPort = ntohs(peer_addr.sin_port);
    }
    else
    {
        strcpy(szIP, "");
        *nPort = 0;
    }
    return ret;
}

/**
 *  S_GetHostByName get host by name(domain)
 *
 *  @param szName domain
 *  @param szIP
 *
 *  @return 0 success, -1 error
 */
static int32_t S_GetHostByName(const char* szName, char* szIP)
{
    if (szName == NULL || szIP == NULL) {
        return S_SOCKET_ERROR;
    }
    struct hostent * hostent = NULL;
    struct in_addr sin_addr;
    memset(&sin_addr, 0, sizeof(in_addr));

    hostent = gethostbyname(szName);
    
    if (!hostent) {
        return S_SOCKET_ERROR;
    }
    
    char host_ip[32] = {0};
    sin_addr = *(struct in_addr*) hostent->h_addr_list[0];
    inet_ntop(AF_INET, &sin_addr, host_ip, sizeof(host_ip));
    strcpy(szIP, host_ip);
    return 0;
}
    
    
/**
 *  S_ntohl
 *
 *  @param netlong
 *
 *  @return
 */
static uint32_t S_ntohl(uint32_t netlong)
{
    return ntohl(netlong);
}

/**
 *  S_htonl
 *
 *  @param hostlong
 *
 *  @return
 */
static uint32_t S_htonl(uint32_t hostlong)
{
    return htonl(hostlong);
}

/**
 *  S_ntohs
 *
 *  @param netshort
 *
 *  @return
 */
static uint16_t S_ntohs(uint16_t netshort)
{
    return ntohs(netshort);
}

/**
 *  S_htons
 *
 *  @param hostshort
 *
 *  @return
 */
static uint16_t S_htons(uint16_t hostshort)
{
    return htons(hostshort);
}
    
/**
 *  S_ntohll
 *
 *  @param val
 *
 *  @return
 */
static uint64_t S_ntohll(uint64_t val)
{
    return (((uint64_t )htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
}

/**
 *  S_htonll
 *
 *  @param val
 *
 *  @return
 */
static uint64_t S_htonll(uint64_t val)
{
    return (((uint64_t )htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
}
    
#ifdef __cplusplus
}
#endif

#endif
