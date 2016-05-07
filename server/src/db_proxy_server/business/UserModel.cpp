/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：UserModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月05日
*   描    述：
*
================================================================*/
#include "UserModel.h"
#include "../DBPool.h"
#include "../CachePool.h"
#include "Common.h"
#include "SyncCenter.h"


CUserModel* CUserModel::m_pInstance = NULL;

CUserModel::CUserModel()
{

}

CUserModel::~CUserModel()
{
    
}

CUserModel* CUserModel::getInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new CUserModel();
    }
    return m_pInstance;
}

void CUserModel::getChangedId(uint32_t& nLastTime, list<uint32_t> &lsIds)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql ;
        if(nLastTime == 0)
        {
            strSql = "select id, updated from IMUser where status != 3";
        }
        else
        {
            strSql = "select id, updated from IMUser where updated>=" + int2string(nLastTime);
        }
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nUpdated = pResultSet->GetInt("updated");
        	 if(nLastTime < nUpdated)
                {
                    nLastTime = nUpdated;
                }
                lsIds.push_back(nId);
  		}
            delete pResultSet;
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

void CUserModel::getUsers(list<uint32_t> lsIds, list<IM::BaseDefine::UserInfo> &lsUsers)
{
    if (lsIds.empty()) {
        log("list is empty");
        return;
    }
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strClause;
        bool bFirst = true;
        for (auto it = lsIds.begin(); it!=lsIds.end(); ++it)
        {
            if(bFirst)
            {
                bFirst = false;
                strClause += int2string(*it);
            }
            else
            {
                strClause += ("," + int2string(*it));
            }
        }
        string  strSql = "select * from IMUser where id in (" + strClause + ")";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::UserInfo cUser;
                cUser.set_user_id(pResultSet->GetInt("id"));
                cUser.set_user_gender(pResultSet->GetInt("sex"));
                cUser.set_user_nick_name(pResultSet->GetString("nick"));
                cUser.set_user_domain(pResultSet->GetString("domain"));
                cUser.set_user_real_name(pResultSet->GetString("name"));
                cUser.set_user_tel(pResultSet->GetString("phone"));
                cUser.set_email(pResultSet->GetString("email"));
                cUser.set_avatar_url(pResultSet->GetString("avatar"));
		cUser.set_sign_info(pResultSet->GetString("sign_info"));
             
                cUser.set_department_id(pResultSet->GetInt("departId"));
  		 cUser.set_department_id(pResultSet->GetInt("departId"));
                cUser.set_status(pResultSet->GetInt("status"));
                lsUsers.push_back(cUser);
            }
            delete pResultSet;
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

bool CUserModel::getUser(uint32_t nUserId, DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMUser where id="+int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                cUser.nId = pResultSet->GetInt("id");
                cUser.nSex = pResultSet->GetInt("sex");
                cUser.strNick = pResultSet->GetString("nick");
                cUser.strDomain = pResultSet->GetString("domain");
                cUser.strName = pResultSet->GetString("name");
                cUser.strTel = pResultSet->GetString("phone");
                cUser.strEmail = pResultSet->GetString("email");
                cUser.strAvatar = pResultSet->GetString("avatar");
                cUser.sign_info = pResultSet->GetString("sign_info");
                cUser.nDeptId = pResultSet->GetInt("departId");
                cUser.nStatus = pResultSet->GetInt("status");
                bRet = true;
            }
            delete pResultSet;
        }
        else
        {
            log("no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
    return bRet;
}


bool CUserModel::updateUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
    if (pDBConn)
    {
        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMUser set `sex`=" + int2string(cUser.nSex)+ ", `nick`='" + cUser.strNick +"', `domain`='"+ cUser.strDomain + "', `name`='" + cUser.strName + "', `phone`='" + cUser.strTel + "', `email`='" + cUser.strEmail+ "', `avatar`='" + cUser.strAvatar + "', `sign_info`='" + cUser.sign_info +"', `departId`='" + int2string(cUser.nDeptId) + "', `status`=" + int2string(cUser.nStatus) + ", `updated`="+int2string(nNow) + " where id="+int2string(cUser.nId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("updateUser: update failed:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
    return bRet;
}

bool CUserModel::insertUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
    if (pDBConn)
    {
        string strSql = "insert into IMUser(`id`,`sex`,`nick`,`domain`,`name`,`phone`,`email`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?)";
        CPrepareStatement* stmt = new CPrepareStatement();
        if (stmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t nNow = (uint32_t) time(NULL);
            uint32_t index = 0;
            uint32_t nGender = cUser.nSex;
            uint32_t nStatus = cUser.nStatus;
            stmt->SetParam(index++, cUser.nId);
            stmt->SetParam(index++, nGender);
            stmt->SetParam(index++, cUser.strNick);
            stmt->SetParam(index++, cUser.strDomain);
            stmt->SetParam(index++, cUser.strName);
            stmt->SetParam(index++, cUser.strTel);
            stmt->SetParam(index++, cUser.strEmail);
            stmt->SetParam(index++, cUser.strAvatar);
            
            stmt->SetParam(index++, cUser.sign_info);
            stmt->SetParam(index++, cUser.nDeptId);
            stmt->SetParam(index++, nStatus);
            stmt->SetParam(index++, nNow);
            stmt->SetParam(index++, nNow);
            bRet = stmt->ExecuteUpdate();
            
            if (!bRet)
            {
                log("insert user failed: %s", strSql.c_str());
            }
        }
        delete stmt;
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
    return bRet;
}

void CUserModel::clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType)
{
    if(IM::BaseDefine::SessionType_IsValid(nSessionType))
    {
        CacheManager* pCacheManager = CacheManager::getInstance();
        CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
        if (pCacheConn)
        {
            // Clear P2P msg Counter
            if(nSessionType == IM::BaseDefine::SESSION_TYPE_SINGLE)
            {
                int nRet = pCacheConn->hdel("unread_" + int2string(nUserId), int2string(nPeerId));
                if(!nRet)
                {
                    log("hdel failed %d->%d", nPeerId, nUserId);
                }
            }
            // Clear Group msg Counter
            else if(nSessionType == IM::BaseDefine::SESSION_TYPE_GROUP)
            {
                string strGroupKey = int2string(nPeerId) + GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX;
                map<string, string> mapGroupCount;
                bool bRet = pCacheConn->hgetAll(strGroupKey, mapGroupCount);
                if(bRet)
                {
                    string strUserKey = int2string(nUserId) + "_" + int2string(nPeerId) + GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX;
                    string strReply = pCacheConn->hmset(strUserKey, mapGroupCount);
                    if(strReply.empty()) {
                        log("hmset %s failed !", strUserKey.c_str());
                    }
                }
                else
                {
                    log("hgetall %s failed!", strGroupKey.c_str());
                }
                
            }
            pCacheManager->RelCacheConn(pCacheConn);
        }
        else
        {
            log("no cache connection for unread");
        }
    }
    else{
        log("invalid sessionType. userId=%u, fromId=%u, sessionType=%u", nUserId, nPeerId, nSessionType);
    }
}

void CUserModel::setCallReport(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::ClientType nClientType)
{
    if(IM::BaseDefine::ClientType_IsValid(nClientType))
    {
        CDBManager* pDBManager = CDBManager::getInstance();
        CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
        if(pDBConn)
        {
            string strSql = "insert into IMCallLog(`userId`, `peerId`, `clientType`,`created`,`updated`) values(?,?,?,?,?)";
            CPrepareStatement* stmt = new CPrepareStatement();
            if (stmt->Init(pDBConn->GetMysql(), strSql))
            {
                uint32_t nNow = (uint32_t) time(NULL);
                uint32_t index = 0;
                uint32_t nClient = (uint32_t) nClientType;
                stmt->SetParam(index++, nUserId);
                stmt->SetParam(index++, nPeerId);
                stmt->SetParam(index++, nClient);
                stmt->SetParam(index++, nNow);
                stmt->SetParam(index++, nNow);
                bool bRet = stmt->ExecuteUpdate();
                
                if (!bRet)
                {
                    log("insert report failed: %s", strSql.c_str());
                }
            }
            delete stmt;
            pDBManager->RelDBConn(pDBConn);
        }
        else
        {
            log("no db connection for teamtalk_master");
        }
        
    }
    else
    {
        log("invalid clienttype. userId=%u, peerId=%u, clientType=%u", nUserId, nPeerId, nClientType);
    }
}


bool CUserModel::updateUserSignInfo(uint32_t user_id, const string& sign_info) {
   
    if (sign_info.length() > 128) {
        log("updateUserSignInfo: sign_info.length()>128.\n");
        return false;
    }
    bool rv = false;
    CDBManager* db_manager = CDBManager::getInstance();
    CDBConn* db_conn = db_manager->GetDBConn("teamtalk_master");
    if (db_conn) {
        uint32_t now = (uint32_t)time(NULL);
        string str_sql = "update IMUser set `sign_info`='" + sign_info + "', `updated`=" + int2string(now) + " where id="+int2string(user_id);
        rv = db_conn->ExecuteUpdate(str_sql.c_str());
        if(!rv) {
            log("updateUserSignInfo: update failed:%s", str_sql.c_str());
        }else{
                CSyncCenter::getInstance()->updateTotalUpdate(now);
           
        }
        db_manager->RelDBConn(db_conn);
        } else {
            log("updateUserSignInfo: no db connection for teamtalk_master");
            }
    return rv;
    }

bool CUserModel::getUserSingInfo(uint32_t user_id, string* sign_info) {
    bool rv = false;
    CDBManager* db_manager = CDBManager::getInstance();
    CDBConn* db_conn = db_manager->GetDBConn("teamtalk_slave");
    if (db_conn) {
        string str_sql = "select sign_info from IMUser where id="+int2string(user_id);
        CResultSet* result_set = db_conn->ExecuteQuery(str_sql.c_str());
        if(result_set) {
            if (result_set->Next()) {
                *sign_info = result_set->GetString("sign_info");
                rv = true;
                }
            delete result_set;
            } else {
                        log("no result set for sql:%s", str_sql.c_str());
                   }
                db_manager->RelDBConn(db_conn);
        } else {
                    log("no db connection for teamtalk_slave");
               }
    return rv;
   }

bool CUserModel::updatePushShield(uint32_t user_id, uint32_t shield_status) {
    bool rv = false;
    
    CDBManager* db_manager = CDBManager::getInstance();
    CDBConn* db_conn = db_manager->GetDBConn("teamtalk_master");
    if (db_conn) {
        uint32_t now = (uint32_t)time(NULL);
        string str_sql = "update IMUser set `push_shield_status`="+ int2string(shield_status) + ", `updated`=" + int2string(now) + " where id="+int2string(user_id);
        rv = db_conn->ExecuteUpdate(str_sql.c_str());
        if(!rv) {
            log("updatePushShield: update failed:%s", str_sql.c_str());
        }
        db_manager->RelDBConn(db_conn);
    } else {
        log("updatePushShield: no db connection for teamtalk_master");
    }
    
    return rv;
}

bool CUserModel::getPushShield(uint32_t user_id, uint32_t* shield_status) {
    bool rv = false;
    
    CDBManager* db_manager = CDBManager::getInstance();
    CDBConn* db_conn = db_manager->GetDBConn("teamtalk_slave");
    if (db_conn) {
        string str_sql = "select push_shield_status from IMUser where id="+int2string(user_id);
        CResultSet* result_set = db_conn->ExecuteQuery(str_sql.c_str());
        if(result_set) {
            if (result_set->Next()) {
                *shield_status = result_set->GetInt("push_shield_status");
                rv = true;
            }
            delete result_set;
        } else {
            log("getPushShield: no result set for sql:%s", str_sql.c_str());
        }
        db_manager->RelDBConn(db_conn);
    } else {
        log("getPushShield: no db connection for teamtalk_slave");
    }
    
    return rv;
}

