#ifndef __NETLIB_H__
#define __NETLIB_H__

#include "ostype.h"

#define NETLIB_OPT_SET_CALLBACK			1	
#define NETLIB_OPT_SET_CALLBACK_DATA	2
#define NETLIB_OPT_GET_REMOTE_IP		3
#define NETLIB_OPT_GET_REMOTE_PORT		4
#define NETLIB_OPT_GET_LOCAL_IP			5
#define NETLIB_OPT_GET_LOCAL_PORT		6
#define NETLIB_OPT_SET_SEND_BUF_SIZE	7
#define NETLIB_OPT_SET_RECV_BUF_SIZE	8

#define NETLIB_MAX_SOCKET_BUF_SIZE		(128 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

int netlib_init();

int netlib_destroy();

int netlib_listen(	
		const char*	server_ip, 
		uint16_t	port,
		callback_t	callback,
		void*		callback_data);

net_handle_t netlib_connect(
		const char*	server_ip,
		uint16_t	port,
		callback_t	callback,
		void*		callback_data);

int netlib_send(net_handle_t handle, void* buf, int len);

int netlib_recv(net_handle_t handle, void* buf, int len);

int netlib_close(net_handle_t handle);

int netlib_option(net_handle_t handle, int opt, void* optval);

int netlib_register_timer(callback_t callback, void* user_data, uint64_t interval);

int netlib_delete_timer(callback_t callback, void* user_data);

int netlib_add_loop(callback_t callback, void* user_data);

void netlib_eventloop(uint32_t wait_timeout = 100);
void netlib_eventloop_UDP(uint32_t wait_timeout = 100);

void netlib_stop_event();

bool netlib_is_running();

// add by xieqq
int netlib_listen_udp_bind(
		const char*	server_ip,  // ÓÃ htonl(INADDR_ANY)´úÌæ inet_addr(ip)
		uint16_t	port,
		callback_t	callback,
		void*		callback_data);

#ifdef __cplusplus
}
#endif

#endif
