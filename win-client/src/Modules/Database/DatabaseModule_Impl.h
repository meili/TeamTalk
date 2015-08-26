/*******************************************************************************
 *  @file      DatabaseModule_Impl.h 2014\8\3 10:43:14 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__
#define DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__

#include "Modules/IDatabaseModule.h"
/******************************************************************************/
class CppSQLite3DB;

/**
 * The class <code>DatabaseModule_Impl</code> 
 *
 */
class DatabaseModule_Impl : public module::IDatabaseModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    DatabaseModule_Impl();
    /**
     * Destructor
     */
    virtual ~DatabaseModule_Impl();
    //@}

public:
	/**@name 图片存储相关*/
	//@{
	virtual BOOL sqlInsertImImageEntity(const module::ImImageEntity& entity);
	virtual BOOL sqlGetImImageEntityByHashcode(UInt32 hashcode, module::ImImageEntity& entity);
	virtual BOOL sqlUpdateImImageEntity(UInt32 hashcode, module::ImImageEntity& entity);
	//@}

	/**@name 最近联系会话信息*/
	//@{
	virtual BOOL sqlGetRecentSessionInfoByGId(IN std::string& sId, OUT module::SessionEntity& sessionInfo);
	virtual BOOL sqlGetAllRecentSessionInfo(OUT std::vector<module::SessionEntity>& sessionList);
	virtual BOOL sqlInsertRecentSessionInfoEntity(IN const module::SessionEntity& sessionInfo);
	virtual BOOL sqlDeleteRecentSessionInfoEntity(IN const std::string& sessionId);
	virtual BOOL sqlUpdateRecentSessionInfoEntity(IN const module::SessionEntity& sessionInfo);
	virtual BOOL sqlBatchInsertRecentSessionInfos(IN std::vector<module::SessionEntity>& sessionList);
	//@}

	/**@name 用户信息相关*/
	//@{
	virtual BOOL sqlGetAllUsersInfo(OUT std::vector<module::UserInfoEntity>& userList);
	virtual BOOL sqlGetUserInfoBySId(IN std::string& sId,OUT module::UserInfoEntity& userInfo);
	virtual BOOL sqlInsertUserInfoEntity(IN const module::UserInfoEntity& userInfo);
	virtual BOOL sqlUpdateUserInfoEntity(std::string& sId, IN const module::UserInfoEntity& userInfo);
	virtual BOOL sqlBatchInsertUserInfos(IN module::UserInfoEntityMap& mapUserInfos);
	//@}

	/**@name 部门信息相关*/
	//@{
	virtual BOOL sqlGetAllDepartmentInfo(OUT std::vector<module::DepartmentEntity>& departmentList);
	virtual BOOL sqlGetDepartmentBySId(IN std::string& sId, OUT module::DepartmentEntity& departmentInfo);
	virtual BOOL sqlInsertDepartmentInfoEntity(IN const module::DepartmentEntity& departmentInfo);
	virtual BOOL sqlUpdateDepartmentInfoEntity(std::string& sId, IN const module::DepartmentEntity& departmentInfo);
	virtual BOOL sqlBatchInsertDepartmentInfos(IN module::DepartmentMap& mapDepartmentInfos);
	//@}

	/**@name 群信息相关*/
	//@{
	virtual BOOL sqlGetGroupInfoByGId(IN std::string& gId, OUT module::GroupInfoEntity& groupInfo);
	virtual BOOL sqlGetAllGroupInfo(OUT std::vector<module::GroupInfoEntity>& groupList);
	virtual BOOL sqlInsertOrReplaceGroupInfoEntity(IN const module::GroupInfoEntity& groupInfo);
	virtual BOOL sqlDeleteGroupInfoEntity(IN const std::string& groupId);
	virtual BOOL sqlUpdateGroupInfoEntity(std::string& sId, IN const module::GroupInfoEntity& groupInfo);
	virtual BOOL sqlBatchInsertGroupInfos(IN module::GroupInfoMap& mapGroupInfos);
	//@}

	/**@name 消息相关*/
	//@{
	virtual BOOL sqlInsertMessage(IN MessageEntity& msg);
	virtual BOOL sqlBatchInsertMessage(IN std::list<MessageEntity>& msgList);
	virtual BOOL sqlGetHistoryMessage(IN const std::string& sId,const IN UInt32 msgId, IN UInt32 nMsgCount
		, OUT std::vector<MessageEntity>& msgList);
	//@}

	/**@name 文件传输相关*/
	//@{
	virtual BOOL sqlInsertFileTransferHistory(IN TransferFileEntity& fileInfo);
	virtual BOOL sqlGetFileTransferHistory(OUT std::vector<TransferFileEntity>& fileList);
	//@}
	
private:
	BOOL _startup();
	BOOL _openDB();
	void _closeDB();
	BOOL _execImageCreateTableDML();
	BOOL _execUserInfoCreateTableDML();
	BOOL _execDepartmentInfoCreateTableDML();
	BOOL _execGroupInfoCreateTableDML();
	BOOL _execImMessageCreateTableDML();
	BOOL _execRecentSessionInfoCreateTableDML();
	BOOL _execFileTransferHistoryTableDML();
	std::string _makeJsonForGroupMembers(IN std::list<std::string> groupMemeberList);
	void _parseJsonForGroupMembers(IN std::string strJson, OUT std::list<std::string>& groupMemeberList);

private:
	CppSQLite3DB*				m_pSqliteDB;			//current login account db
	CppSQLite3DB*				m_pSqliteGlobalDB;		//global db
	std::string					m_sDBPath;
	std::string					m_sGlobalDBPath;
};
/******************************************************************************/
#endif// DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__
