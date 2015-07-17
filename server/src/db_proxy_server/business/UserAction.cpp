/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：UserAction.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#include <list>
#include <map>

#include "../ProxyConn.h"
#include "../DBPool.h"
#include "../SyncCenter.h"
#include "public_define.h"
#include "UserModel.h"
#include "IM.Buddy.pb.h"
#include "IM.BaseDefine.pb.h"


namespace DB_PROXY {

    void getUserInfo(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Buddy::IMUsersInfoReq msg;
        IM::Buddy::IMUsersInfoRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t from_user_id = msg.user_id();
            uint32_t userCount = msg.user_id_list_size();
            
            std::list<uint32_t> idList;
            for(uint32_t i = 0; i < userCount;++i) {
                idList.push_back(msg.user_id_list(i));
            }
            std::list<IM::BaseDefine::UserInfo> lsUser;
            CUserModel::getInstance()->getUsers(idList, lsUser);
            msgResp.set_user_id(from_user_id);
            for(list<IM::BaseDefine::UserInfo>::iterator it=lsUser.begin();
                it!=lsUser.end(); ++it)
            {
                IM::BaseDefine::UserInfo* pUser = msgResp.add_user_info_list();
    //            *pUser = *it;
                pUser->set_user_id(it->user_id());
                pUser->set_user_gender(it->user_gender());
                pUser->set_user_nick_name(it->user_nick_name());
                pUser->set_avatar_url(it->avatar_url());
                pUser->set_department_id(it->department_id());
                pUser->set_email(it->email());
                pUser->set_user_real_name(it->user_real_name());
                pUser->set_user_tel(it->user_tel());
                pUser->set_user_domain(it->user_domain());
                pUser->set_status(it->status());
            }
            log("userId=%u, userCnt=%u", from_user_id, userCount);
            
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
            pPduRes->SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_USER_INFO_RESPONSE);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
        }
        else
        {
            log("parse pb failed");
        }
    }
    
    void getChangedUser(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Buddy::IMAllUserReq msg;
        IM::Buddy::IMAllUserRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t nReqId = msg.user_id();
            uint32_t nLastTime = msg.latest_update_time();
            
            list<IM::BaseDefine::UserInfo> lsUsers;
            
            list<uint32_t> lsIds;
            CUserModel::getInstance()->getChangedId(nLastTime, lsIds);
            CUserModel::getInstance()->getUsers(lsIds, lsUsers);
            
            msgResp.set_user_id(nReqId);
            msgResp.set_latest_update_time(nLastTime);
            for (list<IM::BaseDefine::UserInfo>::iterator it=lsUsers.begin();
                 it!=lsUsers.end(); ++it) {
                IM::BaseDefine::UserInfo* pUser = msgResp.add_user_list();
                //            *pUser = *it;
                pUser->set_user_id(it->user_id());
                pUser->set_user_gender(it->user_gender());
                pUser->set_user_nick_name(it->user_nick_name());
                pUser->set_avatar_url(it->avatar_url());
                pUser->set_department_id(it->department_id());
                pUser->set_email(it->email());
                pUser->set_user_real_name(it->user_real_name());
                pUser->set_user_tel(it->user_tel());
                pUser->set_user_domain(it->user_domain());
                pUser->set_status(it->status());
            }
            log("userId=%u, last_time=%u, userCnt=%u", nReqId, nLastTime, msgResp.user_list_size());
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
            pPduRes->SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_ALL_USER_RESPONSE);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
        }
        else
        {
            log("parse pb failed");
        }
    }
};

