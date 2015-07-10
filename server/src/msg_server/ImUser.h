/*
 * ImUser.h
 *
 *  Created on: 2014年4月16日
 *      Author: ziteng
 */

#ifndef IMUSER_H_
#define IMUSER_H_

#include "imconn.h"
#include "public_define.h"
#define MAX_ONLINE_FRIEND_CNT		100	//通知好友状态通知的最多个数

class CMsgConn;

class CImUser
{
public:
    CImUser(string user_name);
    ~CImUser();
    
    void SetUserId(uint32_t user_id) { m_user_id = user_id; }
    uint32_t GetUserId() { return m_user_id; }
    string GetLoginName() { return m_login_name; }
    void SetNickName(string nick_name) { m_nick_name = nick_name; }
    string GetNickName() { return m_nick_name; }
    bool IsValidate() { return m_bValidate; }
    void SetValidated() { m_bValidate = true; }
    uint32_t GetPCLoginStatus() { return m_pc_login_status; }
    void SetPCLoginStatus(uint32_t pc_login_status) { m_pc_login_status = pc_login_status; }
    
    
    user_conn_t GetUserConn();
    
    bool IsMsgConnEmpty() { return m_conn_map.empty(); }
    void AddMsgConn(uint32_t handle, CMsgConn* pMsgConn) { m_conn_map[handle] = pMsgConn; }
    void DelMsgConn(uint32_t handle) { m_conn_map.erase(handle); }
    CMsgConn* GetMsgConn(uint32_t handle);
    void ValidateMsgConn(uint32_t handle, CMsgConn* pMsgConn);
    
    void AddUnValidateMsgConn(CMsgConn* pMsgConn) { m_unvalidate_conn_set.insert(pMsgConn); }
    void DelUnValidateMsgConn(CMsgConn* pMsgConn) { m_unvalidate_conn_set.erase(pMsgConn); }
    CMsgConn* GetUnValidateMsgConn(uint32_t handle);
    
    map<uint32_t, CMsgConn*>& GetMsgConnMap() { return m_conn_map; }

    void BroadcastPdu(CImPdu* pPdu, CMsgConn* pFromConn = NULL);
    void BroadcastPduWithOutMobile(CImPdu* pPdu, CMsgConn* pFromConn = NULL);
    void BroadcastPduToMobile(CImPdu* pPdu, CMsgConn* pFromConn = NULL);
    void BroadcastClientMsgData(CImPdu* pPdu, uint32_t msg_id, CMsgConn* pFromConn = NULL, uint32_t from_id = 0);
    void BroadcastData(void* buff, uint32_t len, CMsgConn* pFromConn = NULL);
        
    void HandleKickUser(CMsgConn* pConn, uint32_t reason);
    
    bool KickOutSameClientType(uint32_t client_type, uint32_t reason, CMsgConn* pFromConn = NULL);
    
    uint32_t GetClientTypeFlag();
private:
    uint32_t		m_user_id;
    string			m_login_name;            /* 登录名 */
    string          m_nick_name;            /* 花名 */
    bool 			m_user_updated;
    uint32_t        m_pc_login_status;  // pc client login状态，1: on 0: off
    
    bool 			m_bValidate;
    
    map<uint32_t /* handle */, CMsgConn*>	m_conn_map;
    set<CMsgConn*> m_unvalidate_conn_set;
};

typedef map<uint32_t /* user_id */, CImUser*> ImUserMap_t;
typedef map<string /* 登录名 */, CImUser*> ImUserMapByName_t;

class CImUserManager
{
public:
    CImUserManager() {}
    ~CImUserManager();
    
    static CImUserManager* GetInstance();
    CImUser* GetImUserById(uint32_t user_id);
    CImUser* GetImUserByLoginName(string login_name);
    
    CMsgConn* GetMsgConnByHandle(uint32_t user_id, uint32_t handle);
    bool AddImUserByLoginName(string login_name, CImUser* pUser);
    void RemoveImUserByLoginName(string login_name);
    
    bool AddImUserById(uint32_t user_id, CImUser* pUser);
    void RemoveImUserById(uint32_t user_id);
    
    void RemoveImUser(CImUser* pUser);
    
    void RemoveAll();
    void GetOnlineUserInfo(list<user_stat_t>* online_user_info);
    void GetUserConnCnt(list<user_conn_t>* user_conn_list, uint32_t& total_conn_cnt);
    
    void BroadcastPdu(CImPdu* pdu, uint32_t client_type_flag);
private:
    ImUserMap_t m_im_user_map;
    ImUserMapByName_t m_im_user_map_by_name;
};

void get_online_user_info(list<user_stat_t>* online_user_info);


#endif /* IMUSER_H_ */
