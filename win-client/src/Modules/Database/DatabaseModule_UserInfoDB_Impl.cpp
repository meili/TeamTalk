/******************************************************************************* 
 *  @file      DatabaseModule_UserInfoDB_Impl.cpp 2014\12\30 11:26:25 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     user info db manage
 ******************************************************************************/

#include "stdafx.h"
#include "DatabaseModule_Impl.h"
#include "utility/CppSQLite3.h"
#include "utility/utilStrCodingAPI.h"
/******************************************************************************/
namespace
{
	const std::string createUserInfoSql =
		"CREATE TABLE IF NOT EXISTS userinfo"
		"("
		"    [id]                           INTEGER PRIMARY KEY,"
		"    [userId]						TEXT NOT NULL UNIQUE,"					//1
		"    [name]							TEXT DEFAULT NULL,"						//2 姓名
		"    [nickName]						TEXT DEFAULT NULL,"						//3 真名
		"    [avatarUrl]					TEXT DEFAULT NULL,"						//4.头像的url path
		"    [departmentId]					TEXT DEFAULT NULL,"						//5.部门id
		"    [departmentName]				TEXT DEFAULT NULL,"						//6.部门名称
		"    [email]						TEXT DEFAULT NULL,"						//7.邮箱
		"    [gender]						INTEGER DEFAULT 1,"						//8.用户性别,男：1 女：2 人妖/外星人：0
		"    [user_domain]					TEXT DEFAULT NULL,"						//9.用户花名拼音
		"    [telephone]					TEXT DEFAULT NULL,"						//10.电话
		"    [status]						INTEGER DEFAULT 0,"						//11:在职  3:离职
        "    [reserve1]                     TEXT DEFAULT NULL,"                     //12:个性签名
		"    [reserve2]                     INTEGER DEFAULT NULL,"
		"    [reserve3]                     INTEGER DEFAULT NULL"
		");";
	const std::string createUserInfoIndex =
		"create unique index if not exists userid_idx ON userinfo(userId);";
	const std::string getAllUsersInfoSql
		= "select * from userinfo where status<>3 limit 10000";
	const std::string getUserInfoBySIdSql
		= "select * from userinfo where userId=? limit 1";
	const std::string insertUserInfoSql
		= "INSERT OR REPLACE into userinfo(userId,name,nickName,avatarUrl,departmentId,departmentName,email,gender,user_domain,telephone,status,reserve1) "
		"VALUES(?,?,?,?,?,?,?,?,?,?,?,?);";
	const std::string updateUserInfoBySIdSql
		= "update userinfo set name=?,nickName=?,avatarUrl=?,departmentId=?,departmentName=?,email=?,gender=?,user_domain=?,telephone=?,status=?,reserve1=? where userId=?";
	const std::string BeginInsert
		= "BEGIN TRANSACTION;";
	const std::string EndInsert
		= "COMMIT TRANSACTION;";
	const std::string RollBack
		= "ROLLBACK TRANSACTION";
}

BOOL DatabaseModule_Impl::sqlUpdateUserInfoEntity(std::string& sId, IN const module::UserInfoEntity& userInfo)
{
	try
	{
		CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(updateUserInfoBySIdSql.c_str());
		stmt.bind(1, util::cStringToString(userInfo.csName).c_str());
		stmt.bind(2, util::cStringToString(userInfo.csNickName).c_str());
		stmt.bind(3, userInfo.avatarUrl.c_str());
		stmt.bind(4, userInfo.dId.c_str());
		stmt.bind(5, userInfo.department.c_str());
		stmt.bind(6, userInfo.email.c_str());
		stmt.bind(7, (int)userInfo.gender);
		stmt.bind(8, userInfo.user_domain.c_str());
		stmt.bind(9, userInfo.telephone.c_str());
		stmt.bind(10, (int)userInfo.status);
        stmt.bind(11, userInfo.signature.c_str());
		stmt.bind(12, userInfo.sId.c_str()); 
		int countUpdate = stmt.execDML();
		if (0 == countUpdate)
		{
			LOG__(ERR, _T("db update failed:%s"), util::stringToCString(userInfo.sId));
			return FALSE;
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("db failed,error msg:%s"), csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("db unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlBatchInsertUserInfos(IN module::UserInfoEntityMap& mapUserInfos)
{
	try
	{
		CppSQLite3Statement stmtBegin = m_pSqliteDB->compileStatement(BeginInsert.c_str());
		stmtBegin.execDML();

		for (auto kv : mapUserInfos)
		{
			module::UserInfoEntity& userInfo = kv.second;
			CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertUserInfoSql.c_str());
			stmt.bind(1, userInfo.sId.c_str());
			stmt.bind(2, util::cStringToString(userInfo.csName).c_str());
			stmt.bind(3, util::cStringToString(userInfo.csNickName).c_str());
			stmt.bind(4, userInfo.avatarUrl.c_str());
			stmt.bind(5, userInfo.dId.c_str());
			stmt.bind(6, userInfo.department.c_str());
			stmt.bind(7, userInfo.email.c_str());
			stmt.bind(8, (int)userInfo.gender);
			stmt.bind(9, userInfo.user_domain.c_str());
			stmt.bind(10, userInfo.telephone.c_str());
			stmt.bind(11, (int)userInfo.status);
            stmt.bind(12, userInfo.signature.c_str());
			stmt.execDML();
		}

		CppSQLite3Statement stmtEnd = m_pSqliteDB->compileStatement(EndInsert.c_str());
		stmtEnd.execDML();
	}
	catch (CppSQLite3Exception& e)
	{
		CString csErrMsg = util::stringToCString(e.errorMessage());
		LOG__(ERR, _T("batch insert failed,error msg:%s"), csErrMsg);
		CppSQLite3Statement stmtRollback = m_pSqliteDB->compileStatement(RollBack.c_str());
		stmtRollback.execDML();
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("batch insert unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlInsertUserInfoEntity(IN const module::UserInfoEntity& userInfo)
{
	try
	{
		CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertUserInfoSql.c_str());
		stmt.bind(1,userInfo.sId.c_str());
		stmt.bind(2,util::cStringToString(userInfo.csName).c_str());
		stmt.bind(3,util::cStringToString(userInfo.csNickName).c_str());
		stmt.bind(4, userInfo.avatarUrl.c_str());
		stmt.bind(5, userInfo.dId.c_str());
		stmt.bind(6, userInfo.department.c_str());
		stmt.bind(7, userInfo.email.c_str());
		stmt.bind(8, (int)userInfo.gender);
		stmt.bind(9, userInfo.user_domain.c_str());
		stmt.bind(10, userInfo.telephone.c_str());
		stmt.bind(11, (int)userInfo.status);
        stmt.bind(12, userInfo.signature.c_str());
		stmt.execDML();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("insert failed,error msg:%s"), csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("db unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlGetUserInfoBySId(IN std::string& sId, OUT module::UserInfoEntity& userInfo)
{
	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pSqliteDB->compileStatement(getUserInfoBySIdSql.c_str());
		stmt.bind(1, sId.c_str());

		CppSQLite3Query query = stmt.execQuery();
		if (!query.eof())
		{
			userInfo.sId = sId;
			userInfo.csName = util::stringToCString(query.getStringField(2));
			userInfo.csNickName = util::stringToCString(query.getStringField(3));
			userInfo.avatarUrl = query.getStringField(4);
			userInfo.dId = query.getStringField(5);
			userInfo.department = query.getStringField(6);
			userInfo.email = query.getStringField(7);
			userInfo.gender = query.getIntField(8);
			userInfo.user_domain = query.getStringField(9);
			userInfo.telephone = query.getStringField(10);
			userInfo.status = query.getIntField(11);
            userInfo.signature = query.getStringField(12);
		}
		else
		{
			return FALSE;
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
		LOG__(ERR, _T("db unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlGetAllUsersInfo(OUT std::vector<module::UserInfoEntity>& userList)
{
	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pSqliteDB->compileStatement(getAllUsersInfoSql.c_str());
		CppSQLite3Query query = stmt.execQuery();
		while (!query.eof())
		{
			module::UserInfoEntity userInfo;
			userInfo.sId = query.getStringField(1);
			userInfo.csName = util::stringToCString(query.getStringField(2));
			userInfo.csNickName = util::stringToCString(query.getStringField(3));
			userInfo.avatarUrl = query.getStringField(4);
			userInfo.dId = query.getStringField(5);
			userInfo.department = query.getStringField(6);
			userInfo.email = query.getStringField(7);
			userInfo.gender = query.getIntField(8);
			userInfo.user_domain = query.getStringField(9);
			userInfo.telephone = query.getStringField(10);
			userInfo.status = query.getIntField(11);
            userInfo.signature = query.getStringField(12);

			userList.push_back(userInfo);
			query.nextRow();
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("select failed,error msg:%s"),
			csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("db unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::_execUserInfoCreateTableDML()
{
	try
	{
		m_pSqliteDB->execDML(createUserInfoSql.c_str());
		m_pSqliteDB->execDML(createUserInfoIndex.c_str());
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("create table failed,error msg:%s")
			, csErrMsg);
	#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
	#endif	
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("create table unknown exception"));
		return FALSE;
	}
	
	return TRUE;
}

/******************************************************************************/