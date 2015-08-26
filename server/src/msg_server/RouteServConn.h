/*
 * RouteServConn.h
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#ifndef ROUTESERVCONN_H_
#define ROUTESERVCONN_H_

#include "imconn.h"
#include "ServInfo.h"

class CRouteServConn : public CImConn
{
public:
	CRouteServConn();
	virtual ~CRouteServConn();

	bool IsOpen() { return m_bOpen; }
	uint64_t GetConnectTime() { return m_connect_time; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	void _HandleKickUser(CImPdu* pPdu);
	void _HandleStatusNotify(CImPdu* pPdu);
    void _HandleMsgReadNotify(CImPdu* pPdu);
	void _HandleMsgData(CImPdu* pPdu);
	void _HandleP2PMsg(CImPdu* pPdu);
	void _HandleUsersStatusResponse(CImPdu* pPdu);
    void _HandlePCLoginStatusNotify(CImPdu* pPdu);
    void _HandleRemoveSessionNotify(CImPdu* pPdu);
    void _HandleSignInfoChangedNotify(CImPdu* pPdu);
private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
	uint64_t	m_connect_time;
};

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count);
bool is_route_server_available();
void send_to_all_route_server(CImPdu* pPdu);
CRouteServConn* get_route_serv_conn();


#endif /* ROUTESERVCONN_H_ */
