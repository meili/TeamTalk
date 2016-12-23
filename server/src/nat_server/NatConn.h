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
    string	local_ip;	// ������IP
    uint16_t	port;	// �˿�
    uint32_t	uid;	// �û�id
    uint32_t	rid;	// ����id
    uint32_t	create_time;// ���뷿���ʱ��
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

// void init_natconn_timer_callback(); // UDP����timer����
/*
// Client��¼ʱ����������͵���Ϣ
struct stLoginMessage
{
	char userName[20]; // ������û���
	int userid;//cahr password[10];
};

// Clientע��ʱ���͵���Ϣ
struct stLogoutMessage
{
	char userName[20];
};

// Client���������������һ��Client(userName)���Լ�������UDP����Ϣ
struct stP2PTranslate
{
	char userName[20];
};

// Client����������͵���Ϣ��ʽ
struct stMessage
{
	int iMessageType;
	union _message
	{
		stLoginMessage loginmember;	// ��½
		stLogoutMessage logoutmember;	// �˳�
		stP2PTranslate translatemessage;// �����
	}message;
};
*/
#endif /* NATCONN_H_ */
