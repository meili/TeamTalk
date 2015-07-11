/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�Login.cpp
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#include <list>
#include "../ProxyConn.h"
#include "../HttpClient.h"
#include "../SyncCenter.h"
#include "Login.h"
#include "TokenValidator.h"
#include "json/json.h"
#include "Common.h"
#include "IM.Server.pb.h"
#include "Base64.h"
#include "InterLogin.h"
#include "ExterLogin.h"

CInterLoginStrategy g_loginStrategy;

hash_map<string, list<uint32_t> > g_hmLimits;
CLock g_cLimitLock;
namespace DB_PROXY {
    
    
void doLogin(CImPdu* pPdu, uint32_t conn_uuid)
{
    
    CImPdu* pPduResp = new CImPdu;
    
    IM::Server::IMValidateReq msg;
    IM::Server::IMValidateRsp msgResp;
    
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        
        string strDomain = msg.user_name();
        string strPass = msg.password();
        
        msgResp.set_user_name(strDomain);
        msgResp.set_attach_data(msg.attach_data());
        
        do
        {
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            uint32_t tmNow = time(NULL);
            
            //������30���ӵĴ���ʱ����¼
            /*
             ����������ﻹ�Ƿ�������������ӵ�ʱ���أ�
             �������ÿ�ζ�Ҫ����������һ������ܵ���ʧ��
             ���ں��棬���ܻ����30����֮ǰ��10�δ�ģ����Ǳ����ǶԵľ�û�취�ٷ����ˡ�
             */
            auto itTime=lsErrorTime.begin();
            for(; itTime!=lsErrorTime.end();++itTime)
            {
                if(tmNow - *itTime > 30*60)
                {
                    break;
                }
            }
            if(itTime != lsErrorTime.end())
            {
                lsErrorTime.erase(itTime, lsErrorTime.end());
            }
            
            // �ж�30�����������������Ƿ����10
            if(lsErrorTime.size() > 10)
            {
                itTime = lsErrorTime.begin();
                if(tmNow - *itTime <= 30*60)
                {
                    msgResp.set_result_code(6);
                    msgResp.set_result_string("�û���/����������̫��");
                    pPduResp->SetPBMsg(&msgResp);
                    pPduResp->SetSeqNum(pPdu->GetSeqNum());
                    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
                    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);
                    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
                    return ;
                }
            }
        } while(false);
        
        log("%s request login.", strDomain.c_str());
        
        
        
        IM::BaseDefine::UserInfo cUser;
        
        if(g_loginStrategy.doLogin(strDomain, strPass, cUser))
        {
            IM::BaseDefine::UserInfo* pUser = msgResp.mutable_user_info();
            pUser->set_user_id(cUser.user_id());
            pUser->set_user_gender(cUser.user_gender());
            pUser->set_department_id(cUser.department_id());
            pUser->set_user_nick_name(cUser.user_nick_name());
            pUser->set_user_domain(cUser.user_domain());
            pUser->set_avatar_url(cUser.avatar_url());
            pUser->set_email(cUser.email());
            pUser->set_user_tel(cUser.user_tel());
            pUser->set_user_real_name(cUser.user_real_name());
            pUser->set_status(0);
            msgResp.set_result_code(0);
            msgResp.set_result_string("�ɹ�");
            
            //�����½�ɹ������������������
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.clear();
        }
        else
        {
            //������󣬼�¼һ�ε�½ʧ��
            uint32_t tmCurrent = time(NULL);
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.push_front(tmCurrent);
            
            log("get result false");
            msgResp.set_result_code(1);
            msgResp.set_result_string("�û���/�������");
        }
    }
    else
    {
        msgResp.set_result_code(2);
        msgResp.set_result_string("������ڲ�����");
    }
    
    
    pPduResp->SetPBMsg(&msgResp);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
}

};

