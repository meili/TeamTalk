/*
 * NatConn.cpp
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#include "netlib.h"
#include "NatConn.h"
#include "UserInfo.h"
#include "IM.Buddy.pb.h"
#include "IM.Group.pb.h"
#include "IM.Message.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "public_define.h"
using namespace IM::BaseDefine;

static NatConnMap_t g_nat_conn_map;

CNatConn* FindNatConnByHandle(uint32_t conn_handle)
{
    CNatConn* pConn = NULL;
	printf("find handler %d \n", conn_handle);
    NatConnMap_t::iterator it = g_nat_conn_map.find(conn_handle);
    if (it != g_nat_conn_map.end()) {
        pConn = it->second;
    }

    return pConn;
}

void nat_conn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	printf("nat_conn_callback %d\n",*((uint32_t*)&callback_data));
	//NOTUSED_ARG(uParam);
	NOTUSED_ARG(pParam);

	// convert void* to uint32_t, oops
	uint32_t conn_handle = *((uint32_t*)(&callback_data));
    CNatConn* pConn = FindNatConnByHandle(conn_handle);
    if (!pConn) {
	printf("CNatConn is null   not find NatConnByHandle \n");
        return;
    }

	switch (msg)
	{
	case NETLIB_MSG_READ:
		printf("nat_conn_callback read\n");
		pConn->OnUDPRead();
		break;
	case NETLIB_MSG_WRITE:
		printf("nat_conn_callback Write\n");
		pConn->OnUDPWrite();
		break;
	case NETLIB_MSG_CLOSE:
		printf("nat_conn_callback Close\n");
		pConn->OnClose();
		break;
	default:		
		printf("nat_conn_callback default\n");
		log("!!!httpconn_callback error msg: %d\n ", msg);
		break;
	}
}

//void init_natconn_timer_callback()
//{
//	netlib_register_timer(nat_conn_callback, NULL, 1000);
//}
CNatConn::CNatConn()
{
}

CNatConn::~CNatConn()
{

}

void CNatConn::Close()
{	
    //m_state = CONN_STATE_CLOSED;

    g_nat_conn_map.erase(m_sock_handle);
    netlib_close(m_sock_handle);

	ReleaseRef();
}

void CNatConn::OnClose()
{
	log("MsgServer onclose: handle=%d ", m_sock_handle);
	Close();
}

void CNatConn::OnConnect(net_handle_t handle)
{
	printf("CNatConn OnConnect insert handle %d \n", handle);

	m_sock_handle = handle;

	g_nat_conn_map.insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)nat_conn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_nat_conn_map);
}

// 读
void CNatConn::OnUDPRead()
{
	sockaddr_in sender; // 发送端的地址 从哪发来的

	// stMessage recvbuf; // 用 protobuf定义的IMAudioReq

	IM::Message::IMAudioReq recvbuf;

	memset(&recvbuf,0,sizeof(IM::Message::IMAudioReq));
	
	socklen_t dwSender = sizeof(sender);
	//for(;;){
		//	int ret = recvfrom(m_sock_handle, (char *)&recvbuf, sizeof(IM::Message::IMAudioReq), 0, (sockaddr *)&sender, &dwSender);
		//if(ret <= 0)
		//{
		//	printf("recv error");
		//	return;
		//}
	// recv到发生EAGAIN为
	//}
	// UDP包固定大小
	int ret = recvfrom(m_sock_handle, (char *)&recvbuf, sizeof(IM::Message::IMAudioReq), 0, (sockaddr *)&sender, &dwSender);
	if(ret <= 0)
	{
		printf("recv error");
		return;
	} else 
		printf("recv success:%s",(char *)&recvbuf);

	sendto(m_sock_handle, (const char*)&recvbuf,sizeof(IM::Message::IMAudioReq), 0, (const sockaddr*)&sender, sizeof(sender));
            
	//HandlePdu(&recvbuf);
}

// 写
void CNatConn::OnUDPWrite()
{
}


void CNatConn::HandlePdu(IM::Message::IMAudioReq* recvbuf)
{
	switch (recvbuf->msg_type()) { // 登录 登出 打洞
        /*case CID_OTHER_HEARTBEAT:
            // do not take any action, heart beat only update m_last_recv_tick
            break;
        case CID_OTHER_ONLINE_USER_INFO:
            _HandleOnlineUserInfo( pPdu );
            break;
        case CID_OTHER_USER_STATUS_UPDATE:
            _HandleUserStatusUpdate( pPdu );
            break;
        case CID_OTHER_ROLE_SET:
            _HandleRoleSet( pPdu );
            break;
        case CID_BUDDY_LIST_USERS_STATUS_REQUEST:
            _HandleUsersStatusRequest( pPdu );
            break;
        case CID_MSG_DATA:
        case CID_SWITCH_P2P_CMD:
        case CID_MSG_READ_NOTIFY:
        case CID_OTHER_SERVER_KICK_USER:
        case CID_GROUP_CHANGE_MEMBER_NOTIFY:
        case CID_FILE_NOTIFY:
        case CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
            _BroadcastMsg(pPdu, this);
            break;
        case CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
            _BroadcastMsg(pPdu);
            break;
        */
	default:
		log("CNatConn::HandlePdu, wrong cmd id: %d ", recvbuf->msg_type());
		break;
	}
}
