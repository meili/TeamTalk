/******************************************************************************* 
 *  @file      DatabaseModule_Impl.cpp 2014\8\3 10:43:17 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DatabaseModule_Impl.h"
#include "utility/utilStrCodingAPI.h"
#include "utility/CppSQLite3.h"
#include "utility/utilCommonAPI.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISysConfigModule.h"
/******************************************************************************/
namespace module
{
	IDatabaseModule* getDatabaseModule()
	{
		static DatabaseModule_Impl module;
		return &module;
	}
}
// -----------------------------------------------------------------------------
//  DatabaseModule_Impl: Public, Constructor

DatabaseModule_Impl::DatabaseModule_Impl()
:m_pSqliteDB(new CppSQLite3DB())
,m_pSqliteGlobalDB(new CppSQLite3DB())
{
	_startup();
}

// -----------------------------------------------------------------------------
//  DatabaseModule_Impl: Public, Destructor

DatabaseModule_Impl::~DatabaseModule_Impl()
{
	_closeDB();
	delete m_pSqliteDB;
	m_pSqliteDB = 0;

	delete m_pSqliteGlobalDB;
	m_pSqliteGlobalDB = 0;
}

BOOL DatabaseModule_Impl::_openDB()
{
	assert(m_pSqliteDB);
	assert(m_pSqliteGlobalDB);
	try
	{
		m_pSqliteDB->open(m_sDBPath.c_str());
		m_pSqliteGlobalDB->open(m_sGlobalDBPath.c_str());

		_execImageCreateTableDML();		//create image table
		_execUserInfoCreateTableDML();	//create user info table
		_execDepartmentInfoCreateTableDML();//
		_execGroupInfoCreateTableDML();	//create group info table
		_execImMessageCreateTableDML();	//create immessage table
		_execRecentSessionInfoCreateTableDML(); //create recent session table
		_execFileTransferHistoryTableDML(); //create file transfer history table
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage());
		LOG__(ERR, _T("open database failed,error msg:%s")
			, csErrMsg);
//#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
//#endif	
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("open database failed"));
		return FALSE;
	}

	LOG__(APP, _T("open database done"));
	return TRUE;
}

void DatabaseModule_Impl::_closeDB()
{
	try
	{
		m_pSqliteDB->close();
		m_pSqliteGlobalDB->close();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage());
		LOG__(ERR, _T("close database failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
	}
	catch (...)
	{
		LOG__(ERR, _T("close database failed"));
	}
	LOG__(APP, _T("close database done"));
}

BOOL DatabaseModule_Impl::_startup()
{
	try
	{
		CString dbPathTemp = module::getMiscModule()->getTTCommonAppdataUserDir();
		util::createAllDirectories(dbPathTemp);
		module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
		if (pCfg->csUserId.IsEmpty())
			return FALSE;
		CString dbPath = dbPathTemp + _T("\\") + pCfg->csUserId + _T(".db");
		dbPathTemp = module::getMiscModule()->getTTCommonAppdata();
		CString globalDBPath = dbPathTemp + _T("\\global.db");
		m_sDBPath = util::cStringToString(dbPath);
		m_sGlobalDBPath = util::cStringToString(globalDBPath);
		if (!_openDB())
		{
			LOG__(ERR, _T("open db failed"));
			return FALSE;
		}
	}
	catch (...)
	{
		LOG__(ERR, _T("unknown exception"));
		return FALSE;
	}

	return TRUE;
}




/******************************************************************************/