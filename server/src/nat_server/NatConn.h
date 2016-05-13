/*
 * NatConn.h
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#ifndef __NATCONN_H__
#define __NATCONN_H__

#include "netlib.h"
#include "util.h"

class CNatConn : public CImConn
{
public:
	CNatConn();
	virtual ~CNatConn();

	virtual void Close();

	virtual void OnConnect(net_handle_t handle);
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);

private:
	void _HandleOnlineUserInfo(CImPdu* pPdu);
	void _HandleUserStatusUpdate(CImPdu* pPdu);
	void _HandleRoleSet(CImPdu* pPdu);
	void _HandleUsersStatusRequest(CImPdu* pPdu);
	void _HandleMsgReadNotify(CImPdu* pPdu);
	void _HandleKickUser(CImPdu* pPdu);
    
	void _DispatchFriend(uint32_t friend_cnt, uint32_t* friend_id_list);

	void _BroadcastMsg(CImPdu* pPdu, CNatConn* pFromConn = NULL);
    
private:
    void _UpdateUserStatus(uint32_t user_id, uint32_t status, uint32_t client_type);
    void _SendPduToUser(uint32_t user_id, CImPdu* pPdu, bool bAll);
    
private:
	bool			m_bMaster;

protected:
	net_handle_t	m_sock_handle;
};


typedef hash_map<uint32_t, CNatConn*> NatConnMap_t;

CNatConn* FindHttpConnByHandle(uint32_t handle);


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

#endif /* NATCONN_H_ */
