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

static ConnMap_t g_nat_conn_map;

/*CNatConn* FindNatConnByHandle(uint32_t conn_handle)
{
    CNatConn* pConn = NULL;
	printf("find handler %d \n", conn_handle);
    NatConnMap_t::iterator it = g_nat_conn_map.find(conn_handle);
    if (it != g_nat_conn_map.end()) {
        pConn = it->second;
    }

    return pConn;
}
*/
void nat_conn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	printf("nat_conn_callback %d handle = %d\n",*((uint32_t*)&callback_data), handle);
	//NOTUSED_ARG(uParam);
	//NOTUSED_ARG(pParam);
	if (msg == NETLIB_MSG_CONNECT)
	{
		CNatConn* pConn = new CNatConn();
		pConn->OnConnect(handle);// UDP不需要connect、执行OnConnect是设回调值~	UDP_Bind就会执行到这里
	}
	else
	{
		printf("!!!error msg: %d \n", msg);

		log("!!!error msg: %d \n", msg);// UDP
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

	//netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)nat_conn_callback);
	//netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_nat_conn_map);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback); // 数据接收imconn_callback
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_nat_conn_map);

	//imconn_callback(g_nat_conn_map, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);

}
/*
// 读
void CNatConn::OnReadUDP()
{
	sockaddr_in sender; // 发送端的地址 从哪发来的

	// stMessage recvbuf; // 用 protobuf定义的IMAudioReq
	printf("1111111111\n");
	//IM::Message::IMAudioReq recvbuf; // IMAudioReq 是个类 class
	//printf("22222222222=%d\n", sizeof(IM::Message::IMAudioReq));

	//IM::Server::IMUserStatusUpdate msg;
    //CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	char recvbuf[128]={0};
	memset(recvbuf,0,128); 
	printf("333333333333333\n");
	socklen_t dwSender = sizeof(sender);
	printf("4444444444444444\n");
	//for(;;){
		//	int ret = recvfrom(m_sock_handle, (char *)&recvbuf, sizeof(IM::Message::IMAudioReq), 0, (sockaddr *)&sender, &dwSender);
		//if(ret <= 0)
		//{
		//	printf("recv error");
		//	return;
		//}
	// recv到发生EAGAIN为
	//}
	//printf("OnUDPRead m_sock_handle=%d", m_sock_handle);
	// UDP包固定大小
	int ret = recvfrom(m_sock_handle, (char *)&recvbuf, 128, 0, (sockaddr *)&sender, &dwSender);
	if(ret <= 0)
	{	// 读取要参照 void CImConn::OnRead()
		printf("recv error");
		return;
	}
	// else
	//{ 
	//	printf("recv success:%s",recvbuf);
	//}
	printf("Receive message from: %s :%ld:%d:%s\n",inet_ntoa(sender.sin_addr),ntohs(sender.sin_port),ret,recvbuf);
	
	
	//HandlePdu(&recvbuf);

	//int nsend = sendto(m_sock_handle, (const char*)&recvbuf,128, 0, (const sockaddr*)&sender, sizeof(sender));
    //    printf("send message %d\n",nsend);    
}

// 写
void CNatConn::OnWriteUDP()
{
	// 写入字段拼接要参数void CImConn::OnWrite()
}
*/

//void CNatConn::HandlePdu(IM::Message::IMAudioReq* recvbuf)
//{
void CNatConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
		case CID_MSG_AUDIO_UDP_REQUEST:
			printf("receive CID_MSG_AUDIO_UDP_REQUEST \n");
			_HandleClientAudioData(pPdu);	
			break;
		case CID_MSG_DATA:
			printf("receive CID_MSG_DATA \n");
			_HandleClientMsgData(pPdu);
			break;
	//switch (recvbuf->msg_type()) { // 登录 登出 打洞
        /*case CID_OTHER_HEARTBEAT:
            // do not take any action, heart beat only update m_last_recv_tick break;
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
	//	log("CNatConn::HandlePdu, wrong cmd id: %d ", recvbuf->msg_type());
		break;
	}
}

void CNatConn::_HandleClientAudioData(CImPdu* pPdu)
{
    IM::Message::IMAudioReq audioReq; // 音频请求

	//required uint32 from_user_id = 1;		// 用户id
	//required uint32 to_room_id = 2;			// 要加入的房间id
	//required uint32 msg_id = 3;			// 消息ID （加入还是退出） 0 加入 1退出
	//required uint32 create_time = 4;		// 消息时间
	//required IM.BaseDefine.MsgType msg_type = 5;	// 消息类型
	//required IM.BaseDefine.ClientType client_type = 6;  // 客户端类型
	// MSG_TYPE_SINGLE_AUDIO_MEET
	CHECK_PB_PARSE_MSG(audioReq.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	printf("recv ok\n");	
	// 测试看收的到吗，能不解析
//	printf("from_user_id = %d, to_room_id = %d,msg_id = %d", audioReq.from_user_id, audioReq.to_room_id, audioReq.msg_id);
	
}

void CNatConn::_HandleClientMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	if (msg.msg_data().length() == 0) {
//		log("discard an empty message, uid=%u ", GetUserId());
		return;
	}
/*
	if (m_msg_cnt_per_sec >= MAX_MSG_CNT_PER_SECOND) {
		log("!!!too much msg cnt in one second, uid=%u ", GetUserId());
		return;
	}
    
    if (msg.from_user_id() == msg.to_session_id() && CHECK_MSG_TYPE_SINGLE(msg.msg_type()))
    {
        log("!!!from_user_id == to_user_id. ");
        return;
    }

	m_msg_cnt_per_sec++;

	uint32_t to_session_id = msg.to_session_id();
    uint32_t msg_id = msg.msg_id();
	uint8_t msg_type = msg.msg_type();
    string msg_data = msg.msg_data();

	if (g_log_msg_toggle) {
		log("HandleClientMsgData, %d->%d, msg_type=%u, msg_id=%u. ", GetUserId(), to_session_id, msg_type, msg_id);
	}

	uint32_t cur_time = time(NULL);
    CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_from_user_id(GetUserId());
    msg.set_create_time(cur_time);
    msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
    pPdu->SetPBMsg(&msg);
*/
	// send to DB storage server
//	CDBServConn* pDbConn = get_db_serv_conn();
//	if (pDbConn) {
//		pDbConn->SendPdu(pPdu);
//	}
}
