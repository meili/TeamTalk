/*
 * FileHandler.cpp
 *
 *  Created on: 2013-12-17
 *      Author: ziteng@mogujie.com
 */

#include "FileHandler.h"
#include "MsgConn.h"
#include "RouteServConn.h"
#include "DBServConn.h"
#include "IM.File.pb.h"
#include "IM.Server.pb.h"
#include "IM.Buddy.pb.h"
#include "ImUser.h"
#include "AttachData.h"
#include "FileServConn.h"
using namespace IM::BaseDefine;

CFileHandler* CFileHandler::s_handler_instance = NULL;

CFileHandler* CFileHandler::getInstance()
{
	if (!s_handler_instance) {
		s_handler_instance = new CFileHandler();
	}

	return s_handler_instance;
}

void CFileHandler::HandleClientFileRequest(CMsgConn* pMsgConn, CImPdu* pPdu)
{
    IM::File::IMFileReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t from_id = pMsgConn->GetUserId();
    uint32_t to_id = msg.to_user_id();
    string file_name = msg.file_name();
    uint32_t file_size = msg.file_size();
    uint32_t trans_mode = msg.trans_mode();
    log("HandleClientFileRequest, %u->%u, fileName: %s, trans_mode: %u.", from_id, to_id, file_name.c_str(), trans_mode);
    
    CDbAttachData attach(ATTACH_TYPE_HANDLE, pMsgConn->GetHandle());
    CFileServConn* pFileConn = get_random_file_serv_conn();
    if (pFileConn)
    {
        IM::Server::IMFileTransferReq msg2;
        msg2.set_from_user_id(from_id);
        msg2.set_to_user_id(to_id);
        msg2.set_file_name(file_name);
        msg2.set_file_size(file_size);
        msg2.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
        msg2.set_attach_data(attach.GetBuffer(), attach.GetLength());
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_FILE_TRANSFER_REQ);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        
        if (IM::BaseDefine::FILE_TYPE_OFFLINE == trans_mode)
        {
            pFileConn->SendPdu(&pdu);
        }
        else //IM::BaseDefine::FILE_TYPE_ONLINE
        {
            CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(to_id);
            if (pUser && pUser->GetPCLoginStatus())//已有对应的账号pc登录状态
            {
                pFileConn->SendPdu(&pdu);
            }
            else//无对应用户的pc登录状态,向route_server查询状态
            {
                //no pc_client in this msg_server, check it from route_server
                CPduAttachData attach_data(ATTACH_TYPE_HANDLE_AND_PDU_FOR_FILE, pMsgConn->GetHandle(), pdu.GetBodyLength(), pdu.GetBodyData());
                IM::Buddy::IMUsersStatReq msg3;
                msg3.set_user_id(from_id);
                msg3.add_user_id_list(to_id);
                msg3.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
                CImPdu pdu2;
                pdu2.SetPBMsg(&msg3);
                pdu2.SetServiceId(SID_BUDDY_LIST);
                pdu2.SetCommandId(CID_BUDDY_LIST_USERS_STATUS_REQUEST);
                pdu2.SetSeqNum(pPdu->GetSeqNum());
                CRouteServConn* route_conn = get_route_serv_conn();
                if (route_conn)
                {
                    route_conn->SendPdu(&pdu2);
                }
            }
        }
    }
    else
    {
        log("HandleClientFileRequest, no file server.   ");
        IM::File::IMFileRsp msg2;
        msg2.set_result_code(1);
        msg2.set_from_user_id(from_id);
        msg2.set_to_user_id(to_id);
        msg2.set_file_name(file_name);
        msg2.set_task_id("");
        msg2.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_FILE);
        pdu.SetCommandId(CID_FILE_RESPONSE);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        pMsgConn->SendPdu(&pdu);
    }
}

void CFileHandler::HandleClientFileHasOfflineReq(CMsgConn* pMsgConn, CImPdu* pPdu)
{
    uint32_t req_user_id = pMsgConn->GetUserId();
    log("HandleClientFileHasOfflineReq, req_id=%u   ", req_user_id);
    
    CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pMsgConn->GetHandle(), 0);
    CDBServConn* pDbConn = get_db_serv_conn();
    if (pDbConn) {
        IM::File::IMFileHasOfflineReq msg;
        CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
        msg.set_user_id(req_user_id);
        msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
        pPdu->SetPBMsg(&msg);
        pDbConn->SendPdu(pPdu);
    }
    else
    {
        log("warning no DB connection available ");
        IM::File::IMFileHasOfflineRsp msg;
        msg.set_user_id(req_user_id);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_FILE);
        pdu.SetCommandId(CID_FILE_HAS_OFFLINE_RES);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        pMsgConn->SendPdu(&pdu);
    }
}

void CFileHandler::HandleClientFileAddOfflineReq(CMsgConn* pMsgConn, CImPdu* pPdu)
{
    IM::File::IMFileAddOfflineReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t from_id = pMsgConn->GetUserId();
    uint32_t to_id = msg.to_user_id();
    string task_id = msg.task_id();
    string file_name = msg.file_name();
    uint32_t file_size = msg.file_size();
    log("HandleClientFileAddOfflineReq, %u->%u, task_id: %s, file_name: %s, size: %u  ",
        from_id, to_id, task_id.c_str(), file_name.c_str(), file_size);
    
    CDBServConn* pDbConn = get_db_serv_conn();
    if (pDbConn) {
        msg.set_from_user_id(from_id);
        pPdu->SetPBMsg(&msg);
        pDbConn->SendPdu(pPdu);
    }
    
    CFileServConn* pFileConn = get_random_file_serv_conn();
    if (pFileConn)
    {
        const list<IM::BaseDefine::IpAddr>* file_addr_list = pFileConn->GetFileServerIPList();
        
        IM::File::IMFileNotify msg2;
        msg2.set_from_user_id(from_id);
        msg2.set_to_user_id(to_id);
        msg2.set_file_name(file_name);
        msg2.set_file_size(file_size);
        msg2.set_task_id(task_id);
        msg2.set_trans_mode(IM::BaseDefine::FILE_TYPE_OFFLINE);
        msg2.set_offline_ready(1);
        for (list<IM::BaseDefine::IpAddr>::const_iterator it = file_addr_list->begin(); it != file_addr_list->end(); it++)
        {
            IM::BaseDefine::IpAddr ip_addr_tmp = *it;
            IM::BaseDefine::IpAddr* ip_addr = msg2.add_ip_addr_list();
            ip_addr->set_ip(ip_addr_tmp.ip());
            ip_addr->set_port(ip_addr_tmp.port());
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_FILE);
        pdu.SetCommandId(CID_FILE_NOTIFY);
        
        CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(to_id);
        if (pUser)
        {
            //to user is online, notify the offline file has been ready
            pUser->BroadcastPduWithOutMobile(&pdu);
        }
        CRouteServConn* pConn = get_route_serv_conn();
        if (pConn) {
            pConn->SendPdu(&pdu);
        }
    }
}

void CFileHandler::HandleClientFileDelOfflineReq(CMsgConn* pMsgConn, CImPdu* pPdu)
{
    IM::File::IMFileDelOfflineReq msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t from_id = msg.from_user_id();
    uint32_t to_id = msg.to_user_id();
    string task_id = msg.task_id();
    log("HandleClientFileDelOfflineReq, %u->%u, task_id=%s ", from_id, to_id, task_id.c_str());
    
    CDBServConn* pDbConn = get_db_serv_conn();
    if (pDbConn) {
        msg.set_from_user_id(from_id);
        pPdu->SetPBMsg(&msg);
        pDbConn->SendPdu(pPdu);
    }
}

void CFileHandler::HandleFileHasOfflineRes(CImPdu* pPdu)
{
    IM::File::IMFileHasOfflineRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t req_user_id = msg.user_id();
    uint32_t file_cnt = msg.offline_file_list_size();
    CDbAttachData attach((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    log("HandleFileHasOfflineRes, req_id=%u, file_cnt=%u ", req_user_id, file_cnt);
    
    CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id,
                                                                        attach.GetHandle());
    CFileServConn* pFileConn = get_random_file_serv_conn();
    const list<IM::BaseDefine::IpAddr>* ip_list = NULL;
    if (pFileConn)
    {
        ip_list = pFileConn->GetFileServerIPList();
        for (list<IM::BaseDefine::IpAddr>::const_iterator it = ip_list->begin(); it != ip_list->end(); it++)
        {
            IM::BaseDefine::IpAddr ip_addr_tmp = *it;
            IM::BaseDefine::IpAddr* ip_addr = msg.add_ip_addr_list();
            ip_addr->set_ip(ip_addr_tmp.ip());
            ip_addr->set_port(ip_addr_tmp.port());
        }
    }
    else
    {
        log("HandleFileHasOfflineRes, no file server. ");
    }
    if (pConn) {
        pPdu->SetPBMsg(&msg);
        pConn->SendPdu(pPdu);
    }
}

void CFileHandler::HandleFileNotify(CImPdu* pPdu)
{
    IM::File::IMFileNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    
    uint32_t from_user_id = msg.from_user_id();
    uint32_t to_user_id = msg.to_user_id();
    string file_name = msg.file_name();
    uint32_t file_size = msg.file_size();
    string task_id = msg.task_id();
    uint32_t ip_addr_cnt = msg.ip_addr_list_size();
    uint32_t trans_mode = msg.trans_mode();
    uint32_t offline_ready = msg.offline_ready();
    log("HandleFileNotify, from_id: %u, to_id: %u, file_name: %s, task_id: %s, trans_mode: %u,\
        offline_ready: %u. ", from_user_id, to_user_id, file_name.c_str(), task_id.c_str(),
        trans_mode, offline_ready);
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    if (pUser) {
        pUser->BroadcastPdu(pPdu);
    }
}
