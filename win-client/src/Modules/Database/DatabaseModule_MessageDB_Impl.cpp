/******************************************************************************* 
 *  @file      DatabaseModule_MessageDB_Impl.cpp 2015\1\6 14:53:10 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DatabaseModule_Impl.h"
#include "utility/CppSQLite3.h"
#include "Modules/MessageEntity.h"
#include "Modules/ITcpClientModule.h"
#include "json/writer.h"
#include "json/reader.h"

/******************************************************************************/
namespace
{
	const std::string createImMessageSql =
		"CREATE TABLE IF NOT EXISTS immessage"
		"("
		"    [id]                           INTEGER PRIMARY KEY,"
		"    [msgId]						INTEGER NOT NULL,"			//1
		"    [sessionid]                    TEXT NOT NULL,"				//2
		"    [talkerid]						TEXT DEFAULT NULL,"			//3
		"    [content]                      TEXT NOT NULL,"				//4
		"    [rendertype]                   INTEGER DEFAULT 2 NOT NULL,"//5
		"    [sessiontype]                  INTEGER DEFAULT 1 NOT NULL,"//6
		"    [msgtime]                      INTEGER DEFAULT 0 NOT NULL,"//7
		"    [createtime]                   INTEGER DEFAULT 0 NOT NULL,"//8
		"    [reserve1]                     TEXT DEFAULT NULL,"			
		"    [reserve2]                     INTEGER DEFAULT NULL,"		
		"    [reserve3]                     INTEGER DEFAULT NULL"		
		");";

	const std::string createImMessageIndex =
		"CREATE UNIQUE INDEX IF NOT EXISTS sessionid_idx ON immessage(msgId DESC,sessionid);";
	const std::string insertMessageSql
		= "INSERT OR IGNORE INTO immessage(msgId,sessionId,talkerid,content,rendertype,sessiontype,msgtime,createtime,reserve1,reserve2,reserve3) "
		"VALUES(?,?,?,?,?,?,?,?,?,?,?);";
	const std::string getMessageBySId_Msg_Sql
		= "select * from immessage where sessionId=? and msgId <= ? order by msgId desc limit ?";
	const std::string BeginInsert
		= "BEGIN TRANSACTION;";
	const std::string EndInsert
		= "COMMIT TRANSACTION;";
	const std::string RollBack
		= "ROLLBACK TRANSACTION";
}

void _msgToTrace(const MessageEntity& msg)
{
	CTime time(msg.msgTime);
	CString csTime = time.Format(_T("%Y-%m-%d %H:%M:%S"));

	LOG__(ERR,
		_T("msgType = %d,msgRenderType = %d,msgSessionType = %d,msgTime = %s,content = %s,imageId = %s,sessionId = %s"),
		msg.msgType,
		msg.msgRenderType,
		msg.msgSessionType,
		csTime,
		util::stringToCString(msg.content),
		util::stringToCString(msg.imageId),
		util::stringToCString(msg.sessionId));
}

BOOL DatabaseModule_Impl::sqlGetHistoryMessage(IN const std::string& sId, IN const UInt32 msgId
											 , IN UInt32 nMsgCount, OUT std::vector<MessageEntity>& msgList)
{
	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pSqliteDB->compileStatement(getMessageBySId_Msg_Sql.c_str());
		stmt.bind(1, sId.c_str());
		stmt.bind(2, (int)msgId);
		stmt.bind(3, (int)nMsgCount);

		CppSQLite3Query query = stmt.execQuery();
		while (!query.eof())
		{
			MessageEntity msg;
			msg.msgType = MESSAGE_TYPE_HISTORY;
			msg.msgId = query.getIntField(1);
			msg.msgRenderType = query.getIntField(5);
			//对语音消息做个特殊处理，content存储的是json格式字符串
			if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
			{
				std::string jsonAudioContent = query.getStringField(4);
				Json::Reader reader;
				Json::Value root;
				if (reader.parse(jsonAudioContent, root))
				{
					msg.msgAudioTime = (root.get("msgAudioTime", "")).asUInt();
					msg.content = (root.get("msgAudioId", "")).asString();
					msg.msgAudioReaded = 1;//历史语音消息默认为已读
				}
			}
			else
			{
				msg.content = query.getStringField(4);
			}
			msg.msgSessionType = query.getIntField(6);
			msg.msgTime = query.getIntField(7);
			msg.talkerSid = query.getStringField(3);
			msg.msgAudioReaded = TRUE;
			msgList.push_back(msg);
			//msgList.insert(msgList.begin(), msg);//需要
			query.nextRow();
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("get history message failed,error msg:%s"),
			csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("get history message unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::_execImMessageCreateTableDML()
{
	try
	{
		m_pSqliteDB->execDML(createImMessageSql.c_str());
		m_pSqliteDB->execDML(createImMessageIndex.c_str());
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

BOOL DatabaseModule_Impl::sqlInsertMessage(IN MessageEntity& msg)
{
	if (module::TCPCLIENT_STATE_DISCONNECT == module::getTcpClientModule()->getTcpClientNetState()
		|| MESSAGE_RENDERTYPE_SYSTEMTIPS == msg.msgRenderType)
	{
		return FALSE;
	}
	if (msg.msgId <= 0)
	{
		std::string msgDecrptyCnt;
		DECRYPT_MSG(msg.content, msgDecrptyCnt);
		LOG__(ERR, _T("msgid <= 0, msgid:%d msg_content:%s Don't save to DB!")
			, msg.msgId, util::stringToCString(msgDecrptyCnt));
		return FALSE;
	}

	try
	{
		CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertMessageSql.c_str());
		stmt.bind(1, (Int32)msg.msgId);
		stmt.bind(2, msg.sessionId.c_str());
		stmt.bind(3, msg.talkerSid.c_str());
		//对语音消息做个特殊处理，content存储的是json格式字符串
		if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
		{
			Json::Value root;
			root["msgAudioTime"] = msg.msgAudioTime;
			root["msgAudioId"] = msg.content;
			Json::FastWriter fstWrite;
			std::string audioContent = fstWrite.write(root);
			stmt.bind(4, audioContent.c_str());
		}
		else
		{
			stmt.bind(4, msg.content.c_str());
		}
		stmt.bind(5, msg.msgRenderType);
		stmt.bind(6, msg.msgSessionType);
		stmt.bind(7, (Int32)msg.msgTime);
		stmt.bind(8, time(0));
		stmt.execDML();
	}

	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		//MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		LOG__(ERR, _T("db failed,error msg:%s"), csErrMsg);
		_msgToTrace(msg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("db unknown exception"));
		_msgToTrace(msg);
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlBatchInsertMessage(IN std::list<MessageEntity>& msgList)
{
	if (module::TCPCLIENT_STATE_DISCONNECT == module::getTcpClientModule()->getTcpClientNetState())
	{
		LOG__(ERR, _T("TCPCLIENT_STATE_DISCONNECT"));
		return FALSE;
	}
	if (msgList.empty())
	{
		LOG__(ERR, _T("msgList is empty!"));
		return FALSE;
	}
	MessageEntity msg;
	try
	{
		CppSQLite3Statement stmtBegin = m_pSqliteDB->compileStatement(BeginInsert.c_str());
		stmtBegin.execDML();

		std::list<MessageEntity>::iterator iter = msgList.begin();
		for (; iter != msgList.end(); ++iter)
		{
			MessageEntity msg = *iter;
			if (msg.msgId <= 0)//非法的msg消息不用存储
			{
				std::string msgDecrptyCnt;
				if (msg.msgId != 0)
				{
					DECRYPT_MSG(msg.content, msgDecrptyCnt);
					LOG__(ERR, _T("msgid <= 0, msgid:%d msg_content:%s Don't save to DB!")
						, msg.msgId, util::stringToCString(msgDecrptyCnt));
				}
				else
				{
					LOG__(ERR, _T("msgid <= 0, msgid:%d msg_content:%s Don't save to DB!")
						, msg.msgId, msg.content);
				}
				continue;
			}
			CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertMessageSql.c_str());
			stmt.bind(1, (Int32)msg.msgId);
			stmt.bind(2, msg.sessionId.c_str());
			stmt.bind(3, msg.talkerSid.c_str());
			//对语音消息做个特殊处理，content存储的是json格式字符串
			if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
			{
				Json::Value root;
				root["msgAudioTime"] = msg.msgAudioTime;
				root["msgAudioId"] = msg.content;
				Json::FastWriter fstWrite;
				std::string audioContent = fstWrite.write(root);
				stmt.bind(4, audioContent.c_str());
			}
			else
			{
				stmt.bind(4, msg.content.c_str());
			}
			stmt.bind(5, msg.msgRenderType);
			stmt.bind(6, msg.msgSessionType);
			stmt.bind(7, (Int32)msg.msgTime);
			stmt.bind(8, time(0));
			stmt.execDML();
		}

		CppSQLite3Statement stmtEnd = m_pSqliteDB->compileStatement(EndInsert.c_str());
		stmtEnd.execDML();
	}
	catch (CppSQLite3Exception& e)
	{
		CString csErrMsg = util::stringToCString(e.errorMessage());
		LOG__(ERR, _T("db failed,error msg:%s"), csErrMsg);
		CppSQLite3Statement stmtRollback = m_pSqliteDB->compileStatement(RollBack.c_str());
		stmtRollback.execDML();
		_msgToTrace(msg);
		return FALSE;
	}
	catch (...)
	{
		LOG__(ERR, _T("db unknown exception"));
		_msgToTrace(msg);
		return FALSE;
	}

	return TRUE;
}
/******************************************************************************/