/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：DepartModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月12日
*   描    述：
*
================================================================*/
#include "DepartModel.h"
#include "../DBPool.h"

CDepartModel* CDepartModel::m_pInstance = NULL;

CDepartModel* CDepartModel::getInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CDepartModel();
    }
    return m_pInstance;
}

void CDepartModel::getChgedDeptId(uint32_t& nLastTime, list<uint32_t>& lsChangedIds)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select id, updated from IMDepart where updated > " + int2string(nLastTime);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                uint32_t id = pResultSet->GetInt("id");
                uint32_t nUpdated = pResultSet->GetInt("updated");
                if(nLastTime < nUpdated)
                {
                    nLastTime = nUpdated;
                }
                lsChangedIds.push_back(id);
            }
            delete  pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave.");
    }
}

void CDepartModel::getDepts(list<uint32_t>& lsDeptIds, list<IM::BaseDefine::DepartInfo>& lsDepts)
{
    if(lsDeptIds.empty())
    {
        log("list is empty");
        return;
    }
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strClause;
        bool bFirst = true;
        for (auto it=lsDeptIds.begin(); it!=lsDeptIds.end(); ++it) {
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
        string strSql = "select * from IMDepart where id in ( " + strClause + " )";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                IM::BaseDefine::DepartInfo cDept;
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nParentId = pResultSet->GetInt("parentId");
                string strDeptName = pResultSet->GetString("departName");
                uint32_t nStatus = pResultSet->GetInt("status");
                uint32_t nPriority = pResultSet->GetInt("priority");
                if(IM::BaseDefine::DepartmentStatusType_IsValid(nStatus))
                {
                    cDept.set_dept_id(nId);
                    cDept.set_parent_dept_id(nParentId);
                    cDept.set_dept_name(strDeptName);
                    cDept.set_dept_status(IM::BaseDefine::DepartmentStatusType(nStatus));
                    cDept.set_priority(nPriority);
                    lsDepts.push_back(cDept);
                }
            }
            delete  pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

void CDepartModel::getDept(uint32_t nDeptId, IM::BaseDefine::DepartInfo& cDept)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMDepart where id = " + int2string(nDeptId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nParentId = pResultSet->GetInt("parentId");
                string strDeptName = pResultSet->GetString("departName");
                uint32_t nStatus = pResultSet->GetInt("status");
                uint32_t nPriority = pResultSet->GetInt("priority");
                if(IM::BaseDefine::DepartmentStatusType_IsValid(nStatus))
                {
                    cDept.set_dept_id(nId);
                    cDept.set_parent_dept_id(nParentId);
                    cDept.set_dept_name(strDeptName);
                    cDept.set_dept_status(IM::BaseDefine::DepartmentStatusType(nStatus));
                    cDept.set_priority(nPriority);
                }
            }
            delete  pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}