/*
 * MsgConn.h
 *
 *  Created on: 2013-7-5
 *      Author: ziteng@mogujie.com
 */

#ifndef MSGCONN_H_
#define MSGCONN_H_

#include "imconn.h"


#define KICK_FROM_ROUTE_SERVER 		1
#define MAX_ONLINE_FRIEND_CNT		100	//通知好友状态通知的最多个数

typedef struct {
	uint32_t msg_id;
	uint32_t from_id;
	uint64_t timestamp;
} msg_ack_t;

class CImUser;

class CMsgConn : public CImConn
{
public:
	CMsgConn();
	virtual ~CMsgConn();

    string GetLoginName() { return m_login_name; }
    uint32_t GetUserId() { return m_user_id; }
    void SetUserId(uint32_t user_id) { m_user_id = user_id; }
    uint32_t GetHandle() { return m_handle; }
    uint16_t GetPduVersion() { return m_pdu_version; }
    uint32_t GetClientType() { return m_client_type; }
    uint32_t GetClientTypeFlag();
    void SetOpen() { m_bOpen = true; }
    bool IsOpen() { return m_bOpen; }
    void SetKickOff() { m_bKickOff = true; }
    bool IsKickOff() { return m_bKickOff; }
    void SetOnlineStatus(uint32_t status) { m_online_status = status; }
    uint32_t GetOnlineStatus() { return m_online_status; }
    
    void SendUserStatusUpdate(uint32_t user_status);

	virtual void Close(bool kick_user = false);

	virtual void OnConnect(net_handle_t handle);
	virtual void OnClose();
	virtual inline void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);

	void AddToSendList(uint32_t msg_id, uint32_t from_id);
	void DelFromSendList(uint32_t msg_id, uint32_t from_id);
private:
    void _HandleHeartBeat(CImPdu* pPdu);
	void _HandleLoginRequest(CImPdu* pPdu);
    void _HandleLoginOutRequest(CImPdu* pPdu);
    void _HandleClientRecentContactSessionRequest(CImPdu* pPdu);
	void _HandleClientMsgData(CImPdu* pPdu);
	void _HandleClientMsgDataAck(CImPdu* pPdu);
	void _HandleClientTimeRequest(CImPdu* pPdu);
    void _HandleClientGetMsgListRequest(CImPdu* pPdu);
    void _HandleClientGetMsgByMsgIdRequest(CImPdu* pPdu);
	void _HandleClientUnreadMsgCntRequest(CImPdu* pPdu);
	void _HandleClientMsgReadAck(CImPdu* pPdu);
    void _HandleClientGetLatestMsgIDReq(CImPdu* pPdu);
	void _HandleClientP2PCmdMsg(CImPdu* pPdu);
	void _HandleClientUserInfoRequest(CImPdu* pPdu);
	void _HandleClientUsersStatusRequest(CImPdu* pPdu);
	void _HandleClientRemoveSessionRequest(CImPdu* pPdu);
	void _HandleClientAllUserRequest(CImPdu* pPdu);
    void _HandleChangeAvatarRequest(CImPdu* pPdu);
    void _HandleChangeSignInfoRequest(CImPdu* pPdu);

    void _HandleClientDeviceToken(CImPdu* pPdu);
    void _HandleKickPCClient(CImPdu* pPdu);
    void _HandleClientDepartmentRequest(CImPdu* pPdu);
    void _SendFriendStatusNotify(uint32_t status);
    void _HandlePushShieldRequest(CImPdu* pPdu);
    void _HandleQueryPushShieldRequest(CImPdu* pPdu);
private:
    string          m_login_name;        //登录名拼音
    uint32_t        m_user_id;
    bool			m_bOpen;	// only DB validate passed will be set to true;
    bool            m_bKickOff;
    uint64_t		m_login_time;
    
    uint32_t		m_last_seq_no;
    
    uint16_t		m_pdu_version;
    
    string 			m_client_version;	// e.g MAC/2.2, or WIN/2.2
    
    list<msg_ack_t>	m_send_msg_list;
    
    uint32_t		m_msg_cnt_per_sec;
    
    uint32_t        m_client_type;        //客户端登录方式
    
    uint32_t        m_online_status;      //在线状态 1-online, 2-off-line, 3-leave
};

void init_msg_conn();

#endif /* MSGCONN_H_ */
