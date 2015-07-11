/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：DepartAction.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月13日
*   描    述：
*
================================================================*/
#include "DepartAction.h"
#include "DepartModel.h"
#include "IM.Buddy.pb.h"
#include "../ProxyConn.h"

namespace DB_PROXY{
    void getChgedDepart(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::Buddy::IMDepartmentReq msg;
        IM::Buddy::IMDepartmentRsp msgResp;
        if (msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
            
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t nUserId = msg.user_id();
            uint32_t nLastUpdate = msg.latest_update_time();
            list<uint32_t> lsChangedIds;
            CDepartModel::getInstance()->getChgedDeptId(nLastUpdate, lsChangedIds);
            list<IM::BaseDefine::DepartInfo> lsDeparts;
            CDepartModel::getInstance()->getDepts(lsChangedIds, lsDeparts);
            
            msgResp.set_user_id(nUserId);
            msgResp.set_latest_update_time(nLastUpdate);
            for(auto it=lsDeparts.begin(); it!=lsDeparts.end(); ++it)
            {
                IM::BaseDefine::DepartInfo* pDeptInfo = msgResp.add_dept_list();
                pDeptInfo->set_dept_id(it->dept_id());
                pDeptInfo->set_priority(it->priority());
                pDeptInfo->set_dept_name(it->dept_name());
                pDeptInfo->set_parent_dept_id(it->parent_dept_id());
                pDeptInfo->set_dept_status(it->dept_status());
            }
            log("userId=%u, last_update=%u, cnt=%u", nUserId, nLastUpdate, lsDeparts.size());
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
            pPduRes->SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_RESPONSE);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
            
        }
        else
        {
            log("parse pb failed");
        }
    }
}