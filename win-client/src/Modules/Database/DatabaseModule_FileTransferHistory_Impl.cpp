/******************************************************************************* 
 *  @file      DatabaseModule_FileTransferHistory_Impl.cpp 2015\1\13 9:26:59 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DatabaseModule_Impl.h"
#include "../FileTransfer/TransferManager.h"
#include "json/writer.h"
#include "json/reader.h"
#include "utility/CppSQLite3.h"
/******************************************************************************/
namespace
{
	const std::string createFileTransferHistorySql =
		"CREATE TABLE IF NOT EXISTS fileTransferMsg"
		"("
		"    [id]                           INTEGER PRIMARY KEY,"
		"    [taskid]						TEXT NOT NULL,"
		"    [fromid]						TEXT DEFAULT NULL,"
		"    [filename]						TEXT DEFAULT NULL,"
		"    [reserve1]                     TEXT DEFAULT NULL,"
		"    [reserve2]                     INTEGER DEFAULT NULL,"
		"    [reserve3]                     INTEGER DEFAULT NULL,"
		"    [savepath]                     TEXT NOT NULL,"
		"    [filesize]                     INTEGER DEFAULT 0 NOT NULL,"
		"    [finishtime]                   INTEGER DEFAULT 0 NOT NULL"
		");";

	const std::string insertFileTransferHistorySql
		= "INSERT INTO fileTransferMsg(taskid, fromid,filename,reserve1,reserve2,reserve3,savepath,filesize,finishtime) "
		"VALUES(?, ? ,? ,? ,?,?,?,?,?);";
	const std::string getFileTransferHistoryBySIdSql
		= "select * from fileTransferMsg order by id desc limit ?";
}

BOOL DatabaseModule_Impl::sqlInsertFileTransferHistory(IN TransferFileEntity& fileInfo)
{
    if (fileInfo.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD
        || fileInfo.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER)
    {
        LOG__(DEBG, _T("fileInfo.nClientMode not fixed"));
        return FALSE;
    }
    try
    {
        CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertFileTransferHistorySql.c_str());
        stmt.bind(1, fileInfo.sTaskID.c_str());
        stmt.bind(2, fileInfo.sFromID.c_str());
        std::string filename = util::cStringToString(fileInfo.getRealFileName());
        stmt.bind(3, filename.c_str());
        std::string savePath = util::cStringToString(fileInfo.getSaveFilePath());
        stmt.bind(7, savePath.c_str());
        stmt.bind(8, (Int32)fileInfo.nFileSize);
        stmt.bind(9, time(0));
        stmt.execDML();
    }

    catch (CppSQLite3Exception& sqliteException)
    {
#ifdef _DEBUG
        MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
        CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
        LOG__(ERR, _T("failed,error msg:%s"), csErrMsg);
        return FALSE;
    }
    catch (...)
    {
        LOG__(ERR, _T("unknown exception"));
        return FALSE;
    }

    return TRUE;
}

BOOL DatabaseModule_Impl::sqlGetFileTransferHistory(OUT std::vector<TransferFileEntity>& fileList)
{
    try
    {
        CppSQLite3Statement stmt;
        stmt = m_pSqliteDB->compileStatement(getFileTransferHistoryBySIdSql.c_str());
        stmt.bind(1, 20);

        CppSQLite3Query query = stmt.execQuery();
        while (!query.eof())
        {
            TransferFileEntity fileInfo;
            fileInfo.sTaskID = query.getStringField(1);
            fileInfo.sFromID = query.getStringField(2);
            fileInfo.sFileName = query.getStringField(3);
            CString strSavePath = util::stringToCString(query.getStringField(7));
            fileInfo.setSaveFilePath(strSavePath);
            fileInfo.nFileSize = query.getIntField(8);
            fileInfo.time = query.getIntField(9);
            fileList.push_back(fileInfo);
            query.nextRow();
        }
    }
    catch (CppSQLite3Exception& sqliteException)
    {
#ifdef _DEBUG
        MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
        CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
        LOG__(ERR, _T("db failed,error msg:%s"),
            csErrMsg);
        return FALSE;
    }
    catch (...)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL DatabaseModule_Impl::_execFileTransferHistoryTableDML()
{
	try
	{
		m_pSqliteDB->execDML(createFileTransferHistorySql.c_str());
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("execFileTransferHistoryDML failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif	
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("execFileTransferHistoryDML failed"));
		return FALSE;
	}

	return TRUE;
}
/******************************************************************************/