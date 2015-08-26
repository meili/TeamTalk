/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
*
================================================================*/
#include "FileModel.h"
#include "../DBPool.h"

CFileModel* CFileModel::m_pInstance = NULL;

CFileModel::CFileModel()
{
    
}

CFileModel::~CFileModel()
{
    
}

CFileModel* CFileModel::getInstance()
{
    if (m_pInstance == NULL) {
        m_pInstance = new CFileModel();
    }
    return m_pInstance;
}

void CFileModel::getOfflineFile(uint32_t userId, list<IM::BaseDefine::OfflineFileInfo>& lsOffline)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMTransmitFile where toId="+int2string(userId) + " and status=0 order by created";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::OfflineFileInfo offlineFile;
                offlineFile.set_from_user_id(pResultSet->GetInt("fromId"));
                offlineFile.set_task_id(pResultSet->GetString("taskId"));
                offlineFile.set_file_name(pResultSet->GetString("fileName"));
                offlineFile.set_file_size(pResultSet->GetInt("size"));
                lsOffline.push_back(offlineFile);
            }
            delete pResultSet;
        }
        else
        {
            log("no result for:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_slave");
    }
}

void CFileModel::addOfflineFile(uint32_t fromId, uint32_t toId, string& taskId, string& fileName, uint32_t fileSize)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
    if (pDBConn)
    {
        string strSql = "insert into IMTransmitFile (`fromId`,`toId`,`fileName`,`size`,`taskId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?)";
        
        // 必须在释放连接前delete CPrepareStatement对象，否则有可能多个线程操作mysql对象，会crash
        CPrepareStatement* pStmt = new CPrepareStatement();
        if (pStmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t status = 0;
            uint32_t nCreated = (uint32_t)time(NULL);
            
            uint32_t index = 0;
            pStmt->SetParam(index++, fromId);
            pStmt->SetParam(index++, toId);
            pStmt->SetParam(index++, fileName);
            pStmt->SetParam(index++, fileSize);
            pStmt->SetParam(index++, taskId);
            pStmt->SetParam(index++, status);
            pStmt->SetParam(index++, nCreated);
            pStmt->SetParam(index++, nCreated);
            
            bool bRet = pStmt->ExecuteUpdate();
            
            if (!bRet)
            {
                log("insert message failed: %s", strSql.c_str());
            }
        }
        delete pStmt;
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
}

void CFileModel::delOfflineFile(uint32_t fromId, uint32_t toId, string& taskId)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
    if (pDBConn)
    {
        string strSql = "delete from IMTransmitFile where  fromId=" + int2string(fromId) + " and toId="+int2string(toId) + " and taskId='" + taskId + "'";
        if(pDBConn->ExecuteUpdate(strSql.c_str()))
        {
            log("delete offline file success.%d->%d:%s", fromId, toId, taskId.c_str());
        }
        else
        {
            log("delete offline file failed.%d->%d:%s", fromId, toId, taskId.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for teamtalk_master");
    }
}