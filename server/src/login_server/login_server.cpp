/*
 * login_server.cpp
 *
 *  Created on: 2013-6-21
 *      Author: ziteng@mogujie.com
 */

#include "LoginConn.h"
#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "HttpConn.h"
#include "ipparser.h"

IpParser* pIpParser = NULL;
string strMsfsUrl;
string strDiscovery;//发现获取地址

// 8008端口
void client_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		CLoginConn* pConn = new CLoginConn();
		pConn->OnConnect2(handle, LOGIN_CONN_TYPE_CLIENT);  
	}
	else
	{
		log("!!!error msg: %d ", msg);
	}
}

// this callback will be replaced by imconn_callback() in OnConnect()   8100端口
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    log("msg_server come in");

	if (msg == NETLIB_MSG_CONNECT)
	{
		CLoginConn* pConn = new CLoginConn();
		pConn->OnConnect2(handle, LOGIN_CONN_TYPE_MSG_SERV);
	}
	else
	{
		log("!!!error msg: %d ", msg);
	}
}

// 8080端口
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    if (msg == NETLIB_MSG_CONNECT)
    {
        CHttpConn* pConn = new CHttpConn();
        pConn->OnConnect(handle);
    }
    else
    {
        log("!!!error msg: %d ", msg);
    }
}

int main(int argc, char* argv[])
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("Server Version: LoginServer/%s\n", VERSION);
		printf("Server Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}
	
	// signal(SIGPIPE, SIG_IGN) (转) http://www.cnblogs.com/wainiwann/p/3899176.html 
	// 信号(signals)和槽(slots)  
	//		clinet 系统会发出一个SIGPIPE信号给进程，告诉进程这个连接已经断开 SIGPIPE信号的默认执行动作是terminate(终止、退出),所以client会退出
	// 若不想客户端退出可以把SIGPIPE设为SIG_IGN  这时SIGPIPE交给了系统处理。 
	//		server 服务器采用了fork的话，要收集垃圾进程，防止僵尸进程的产生，可以这样处理： 
	// signal(SIGCHLD,SIG_IGN); 交给系统init去回收。 
	// 这里子进程就不会产生僵尸进程了。 
	signal(SIGPIPE, SIG_IGN);  // 调用了signal(SIGPIPE, SIG_IGN), 这样产生  SIGPIPE 信号时就不会中止程序，直接把这个信号忽略掉。

	CConfigFileReader config_file("loginserver.conf");

    char* client_listen_ip = config_file.GetConfigName("ClientListenIP");
    char* str_client_port = config_file.GetConfigName("ClientPort");
    char* http_listen_ip = config_file.GetConfigName("HttpListenIP");
    char* str_http_port = config_file.GetConfigName("HttpPort");
	char* msg_server_listen_ip = config_file.GetConfigName("MsgServerListenIP");
	char* str_msg_server_port = config_file.GetConfigName("MsgServerPort");
    char* str_msfs_url = config_file.GetConfigName("msfs");
    char* str_discovery = config_file.GetConfigName("discovery");

	if (!msg_server_listen_ip || !str_msg_server_port || !http_listen_ip
        || !str_http_port || !str_msfs_url || !str_discovery) {
		log("config item missing, exit... ");
		return -1;
	}

	uint16_t client_port = atoi(str_client_port);
	uint16_t msg_server_port = atoi(str_msg_server_port);
    uint16_t http_port = atoi(str_http_port);
    strMsfsUrl = str_msfs_url;
    strDiscovery = str_discovery;
    
    
    pIpParser = new IpParser();
    
	// 网络SOCKET初始化
	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;
	
	// 启动端口8100 8000 8080 监听
	CStrExplode client_listen_ip_list(client_listen_ip, ';');
	for (uint32_t i = 0; i < client_listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen(client_listen_ip_list.GetItem(i), client_port, client_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}

	CStrExplode msg_server_listen_ip_list(msg_server_listen_ip, ';');
	for (uint32_t i = 0; i < msg_server_listen_ip_list.GetItemCnt(); i++) {
		// sokcet bind listen
		ret = netlib_listen(msg_server_listen_ip_list.GetItem(i), msg_server_port, msg_serv_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}
    
    CStrExplode http_listen_ip_list(http_listen_ip, ';');
    for (uint32_t i = 0; i < http_listen_ip_list.GetItemCnt(); i++) {
        ret = netlib_listen(http_listen_ip_list.GetItem(i), http_port, http_callback, NULL);
        if (ret == NETLIB_ERROR)
            return ret;
    }
    

	printf("server start listen on:\nFor client %s:%d\nFor MsgServer: %s:%d\nFor http:%s:%d\n",
				client_listen_ip, client_port, msg_server_listen_ip, msg_server_port, http_listen_ip, http_port);

	init_login_conn();
    init_http_conn();  // 一秒发一个，发心跳，两分钟之后关闭
			/*pdu.SetPBMsg(&msg);
            pdu.SetServiceId(SID_OTHER);
            pdu.SetCommandId(CID_OTHER_HEARTBEAT);
			SendPdu(&pdu);
			*/
	printf("now enter the event loop...\n");
    
    writePid();

	netlib_eventloop();

	return 0;
}
