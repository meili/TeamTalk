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
	printf("CNatConn _Close_: handle=%d ", m_handle);

    g_nat_conn_map.erase(m_handle);
    netlib_close(m_handle);

	ReleaseRef();
}

void CNatConn::OnClose()
{
	printf("CNatConn onclose: handle=%d ", m_handle);
	Close();
}

void CNatConn::OnConnect(net_handle_t handle)
{
	printf("CNatConn OnConnect insert handle %d \n", handle);

	//m_sock_handle = handle;
	m_handle = handle;// 用父类的m_handle要不recvform未赋值

	g_nat_conn_map.insert(make_pair(handle, this)); // handler=m_socket

	//netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)nat_conn_callback);
	//netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_nat_conn_map);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback); // CImConn类的: 数据接收imconn_callback
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_nat_conn_map);
}
/*

*/
void CNatConn::HandlePdu_UDP(CImPdu* pPdu, sockaddr_in sender)
{
	switch (pPdu->GetCommandId()) {
		case CID_MSG_AUDIO_UDP_REQUEST:
	//		printf("receive CID_MSG_AUDIO_UDP_REQUEST \n");
			_HandleClientAudioData(pPdu, sender);	
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

// 根据人查找 // 房间号 ip port
//map<uint32_t, user_serv_info_t*> g_user_info;
// 房间ID,找出有多少人在这个房间 (map 嵌套)
typedef map<uint32_t, user_serv_info_t*>  user_map;
typedef map<uint32_t, user_map*> room_map;
//hash_map<uint32_t, map<uint32_t, user_serv_info_t*>*> g_user_room_info;
room_map g_user_room_info;

void CNatConn::_HandleClientAudioData(CImPdu* pPdu, sockaddr_in sender)
{	// 收到消息要连哪个房间的处理

    IM::Message::IMAudioReq audioReq; // 音频请求

	//required uint32 from_user_id = 1;		// 用户id
	//required uint32 to_room_id = 2;		// 要加入的房间id
	//required uint32 msg_id = 3;			// 消息ID （加入还是退出） 0 加入 1退出
	//required uint32 create_time = 4;		// 消息时间
	//required IM.BaseDefine.MsgType msg_type = 5;	// 消息类型
	//required IM.BaseDefine.ClientType client_type = 6;  // 客户端类型	
	// MSG_TYPE_SINGLE_AUDIO_MEET

	CHECK_PB_PARSE_MSG(audioReq.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	printf("recv ok\n");	
	// 测试看收的到吗，能不解析
	printf("from_user_id = %d, to_room_id = %d,msg_id = %d, ip = %s, port = %d\n", audioReq.from_user_id(), audioReq.to_room_id(), 
		audioReq.msg_id(), inet_ntoa(sender.sin_addr),ntohs(sender.sin_port));
	// 两人id 相加房间号
	// 根据房间ID去找 (退出机制要完善，一段时间后不在房间的清掉？)
	room_map::iterator it = g_user_room_info.find(audioReq.to_room_id());
	if (it == g_user_room_info.end()) {
		// 如果没找到，插入
		// map<uint32_t, user_serv_info_t*>* t_user_info = new map<uint32_t, user_serv_info_t*>; // 记得delete
		user_map* t_user_info = new user_map;
		user_serv_info_t* pMsgServInfo = new user_serv_info_t;
		
		pMsgServInfo->local_ip = audioReq.local_ip(); // 局域网IP

		pMsgServInfo->ip_addr =inet_ntoa(sender.sin_addr);//ntohl(sender.sin_addr.s_addr);//.S_un.S_addr);//msg.ip1();	// int
		pMsgServInfo->port = ntohs(sender.sin_port);//msg.ip2();	//		int
		pMsgServInfo->uid = (audioReq.from_user_id());	// 用户ID
		pMsgServInfo->rid = audioReq.to_room_id();	// 房ID
		pMsgServInfo->create_time = time(NULL); // 当前时间
		// 加人
		t_user_info->insert(make_pair(audioReq.from_user_id(), pMsgServInfo));

		// 把人加入房间
		g_user_room_info.insert(make_pair(audioReq.to_room_id(), t_user_info));
	} else {
		// 找到room，is in update or insert
		user_map* p_user_info = it->second;
		user_map::iterator it_user = p_user_info->find(audioReq.from_user_id());
		if(it_user != p_user_info->end()){
			// 己经加入这个房间
			// 更新
			
		} else {
			user_serv_info_t* pMsgServInfo = new user_serv_info_t;

			pMsgServInfo->local_ip = audioReq.local_ip(); // 局域网IP

			pMsgServInfo->ip_addr =inet_ntoa(sender.sin_addr);//ntohl(sender.sin_addr.s_addr);//.S_un.S_addr);//msg.ip1();	// int
			pMsgServInfo->port = ntohs(sender.sin_port);//msg.ip2();	//		int
			pMsgServInfo->uid = audioReq.from_user_id();	// 用户ID
			pMsgServInfo->rid = audioReq.to_room_id();	// 房ID
			pMsgServInfo->create_time = time(NULL); // 当前时间
			// 加人
			p_user_info->insert(make_pair(audioReq.from_user_id(), pMsgServInfo));		
		}

		// 遍历发送
		for (user_map::iterator it_send = p_user_info->begin(); it_send != p_user_info->end(); ) {
			//printf("0.1\n");
			user_map::iterator it_old = it_send;
			//printf("0.5\n");
			it_send++;
			// 
			user_serv_info_t* p_user_serv_info = it_old->second;
			printf("1_ %s\n", p_user_serv_info->ip_addr.c_str());
			for (user_map::iterator it_send2 = p_user_info->begin(); it_send2 != p_user_info->end(); ) {
				user_map::iterator it_old2 = it_send2;
				it_send2++;

				user_serv_info_t* p_user_serv_info2 = it_old2->second;
				
				//printf("2_ %s _ %d\n",p_user_serv_info2->ip_addr.c_str(), p_user_serv_info2->port);				

				if(p_user_serv_info->uid == p_user_serv_info2->uid){
					printf("3\n");
					continue;	// 自己不用给自己发
				} else 
				{
					printf("4\n");
					// 消息发送
					CImPdu pdu;
					
					IM::Message::IMAudioRsp msgARsp;
						
					msgARsp.set_from_user_id(p_user_serv_info2->uid);// 用户ID
					msgARsp.set_to_room_id(p_user_serv_info2->rid);	// 房间ID
					msgARsp.set_count_in_room(p_user_info->size()); // 房间里的人数 
					/*if(p_user_serv_info->ip_addr == p_user_serv_info2->ip_addr)
					{	
						printf("ip equale = local ip %s\n",p_user_serv_info2->local_ip.c_str());
						// 两个客户端的外网IP相等，证明可能在一个局域网内，可以直连
						IM::BaseDefine::UserIpAddr *user_ip_addr= msgARsp.mutable_user_list();// new IM::BaseDefine::UserIpAddr;
						user_ip_addr->set_user_id(p_user_serv_info2->uid);
						user_ip_addr->set_ip(p_user_serv_info2->local_ip.c_str());
						user_ip_addr->set_port(8132);						
					}
					else*/ 
					{
						IM::BaseDefine::UserIpAddr *user_ip_addr= msgARsp.mutable_user_list();// new IM::BaseDefine::UserIpAddr;
						user_ip_addr->set_user_id(p_user_serv_info2->uid);
						user_ip_addr->set_ip(p_user_serv_info2->ip_addr.c_str());
						user_ip_addr->set_port(p_user_serv_info2->port);
				//		msgARsp.set_allocated_user_list(user_ip_addr);
					}
					pdu.SetPBMsg(&msgARsp);

					pdu.SetServiceId(IM::BaseDefine::SID_MSG);						// service 消息ID
					pdu.SetCommandId(CID_MSG_AUDIO_UDP_RESPONSE);	// 音频返回请求
					pdu.SetSeqNum(pPdu->GetSeqNum());				// 返回值，证明把ip、port给服务器了
					//pdu.SetPBMsg(&msgARsp);//	
					// 回复
					sockaddr_in remote;
					remote.sin_family=AF_INET;
					remote.sin_port= htons(p_user_serv_info->port); 
					remote.sin_addr.s_addr = inet_addr(p_user_serv_info->ip_addr.c_str());
					//pAddr->sin_addr.s_addr = inet_addr(ip);
			//pMsgServInfo->ip_addr =ntohl(sender.sin_addr.s_addr);//.S_un.S_addr);//msg.ip1();	// int
			// inet_ntoa(sender.sin_addr)  string
					printf("sendto ip = %s, port = %d, get ip = %s, port2 = %d\n",
						p_user_serv_info->ip_addr.c_str(),
						p_user_serv_info->port,
						p_user_serv_info2->ip_addr.c_str(),
						p_user_serv_info2->port);
					SendPduUDP(&pdu, remote);				 
			//		delete user_ip_addr;
				}
			}
			
		}
		/*	
			// 消息发送
			// sendto
			CImPdu pdu;
			pdu.SetPBMsg(&msgARsp);
			pdu.SetServiceId(SID_MSG);						// service 消息ID
			pdu.SetCommandId(CID_MSG_AUDIO_UDP_REQUEST);	// 音频返回请求
			pdu.SetSeqNum(pPdu->GetSeqNum());				// 返回值，证明把ip、port给服务器了
			SendPdu(&pdu);
		*/
	}
	
}

void CNatConn::_HandleClientMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	if (msg.msg_data().length() == 0) {
//		log("discard an empty message, uid=%u ", GetUserId());
		return;
	}

	// 

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
