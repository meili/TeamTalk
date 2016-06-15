/*
 * NatConn.h
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#ifndef __NATCONN_H__
#define __NATCONN_H__

#include <sys/types.h>
#include <sys/socket.h>

#include "netlib.h"
#include "util.h"
#include "imconn.h"
#include "IM.Message.pb.h"

typedef struct  {
    string	ip_addr;	// IP
    string	local_ip;	// 局哉网IP
    uint16_t	port;	// 端口
    uint32_t	uid;	// 用户id
    uint32_t	rid;	// 房间id
    uint32_t	create_time;// 加入房间的时间
} user_serv_info_t;

class CNatConn : public CImConn
{
public:
	CNatConn();
	virtual ~CNatConn();

	virtual void Close();

	virtual void OnConnect(net_handle_t handle);
	virtual void OnClose();

//	virtual void HandlePdu(CImPdu* pPdu);

	virtual void HandlePdu_UDP(CImPdu* pPdu, sockaddr_in sender);

	void _HandleClientAudioData(CImPdu* pPdu, sockaddr_in sender);
	void _HandleClientMsgData(CImPdu* pPdu);


protected:
	uint32_t        m_state;

	//net_handle_t	m_sock_handle;
};


//typedef hash_map<uint32_t, CNatConn*> NatConnMap_t;

void nat_conn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

//CNatConn* FindNatConnByHandle(uint32_t handle);

// void init_natconn_timer_callback(); // UDP不用timer心跳
/*
// Client登录时向服务器发送的消息
struct stLoginMessage
{
	char userName[20]; // 请求的用户名
	int userid;//cahr password[10];
};

// Client注销时发送的消息
struct stLogoutMessage
{
	char userName[20];
};

// Client向服务器请求另外一个Client(userName)向自己方向发送UDP打洞消息
struct stP2PTranslate
{
	char userName[20];
};

// Client向服务器发送的消息格式
struct stMessage
{
	int iMessageType;
	union _message
	{
		stLoginMessage loginmember;	// 登陆
		stLogoutMessage logoutmember;	// 退出
		stP2PTranslate translatemessage;// 请求打洞
	}message;
};
*/
#endif /* NATCONN_H_ */
