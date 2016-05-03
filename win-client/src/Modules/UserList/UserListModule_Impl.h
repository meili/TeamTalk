/*******************************************************************************
 *  @file      IUserListModule_Impl.h 2014\8\6 15:27:11 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef IUSERLISTMODULE_IMPL_9768C185_67AE_45BB_B840_F0A66E6A7044_H__
#define IUSERLISTMODULE_IMPL_9768C185_67AE_45BB_B840_F0A66E6A7044_H__

#include "network/Lock.h"
#include "Modules/IUserListModule.h"
#include <string>
/******************************************************************************/

/**
 * The class <code>IUserListModule_Impl</code> 
 *
 */
class UserListModule_Impl final : public module::IUserListModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    UserListModule_Impl();
    /**
     * Destructor
     */
    virtual ~UserListModule_Impl() = default;
    //@}
	virtual void onPacket(imcore::TTPBHeader& header, std::string& pbBody);

public:
	virtual BOOL startup();
	virtual const module::DepartmentMap& getAllDepartments();
	virtual void getAllUsersInfo(module::UserInfoEntityMap& MapUsers)const;
	virtual BOOL getUserInfoBySId(IN std::string sid, OUT module::UserInfoEntity& userInfo);
	
	virtual void removeAllListInfo();
	virtual UInt8 getMyLineStatus();

	virtual void tcpGetUserOnlieStatus(IN const std::string& sId);
	virtual void tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId);//要分批获取
	virtual void tcpGetUserInfo(IN const std::string& sId);
	virtual void tcpGetUsersInfo(IN const module::UserInfoEntityVec& VecUnKnowUserInfo);
    virtual void tcpChangeMySignInfo(IN const std::string sSignInfo);

	virtual CString getDefaultAvatartPath();
	virtual BOOL getMyInfo(OUT module::UserInfoEntity& myInfo);
	virtual BOOL createUserInfo(IN const module::UserInfoEntity& info);

	virtual void getSearchUserNameListByShortName(IN const CString& sShortName, OUT	module::UserInfoEntityVec& nameList);
	virtual void getSearchDepartmentsByShortName(IN const CString& sShortName, OUT	std::vector<std::string>& departmentList);
	void onCallbackOperation(std::shared_ptr<void> param);

	virtual std::string randomGetUser(void);//彩蛋：随机获取一个用户ID
private:
	/**@name 服务器端拆包*/
	//@{
	void _recentlistResponse(IN std::string& pbBody);//最近联系人列表
	void _userStatusNotify(IN std::string& pbBody);
	void _usersInfoResponse(IN std::string& pbBody);//用户信息
	void _removeSessionResponse(IN std::string& pbBody);//移除会话
	void _allUserlistResponse(IN std::string& pbBody);//所有人信息
	void _usersLineStatusResponse(IN std::string& pbBody);//用户在线状态
	void _changeAvatarResponse(IN std::string& pbBody);//修改头像通知
	void _removeSessionNotify(IN std::string& pbBody);//移除会话通知
	void _departmentResponse(IN std::string& pbBody);//部门请求返回

    void _changeSignInfoResponse(IN std::string& pbBody);//修改签名返回
    void _avatarChangeNotify(IN std::string& pbBody);//修改头像通知
    void _signInfoChangedNotify(IN std::string& pbBody);//修改签名通知
	//@}

	/**@name 工具函数*/
	//@{
	void _pushUserIdToDepartment(const std::string& sId, const std::string& dId);
	void _downloadAllUserAvatarImg();
	BOOL _downloadAvatarImgBySId(IN const std::string& sId);
	void _tcpGetAllUsersOnlineStatus(void);
	void _tcpGetUserOnlieStatus(IN const module::UserInfoEntityVec& VecId);
	void _tcpGetUserInfoList(IN module::UserInfoEntityVec VecUnKnowUserInfo);
	std::string _getGrayLocalPathFromFilename(std::string& finename);
	//@}	
private:
	CLock								m_lock;
	module::DepartmentMap				m_mapDepartment;						//部门信息
	module::UserInfoEntityMap           m_mapUsers;								//所以用户的信息
	UInt32								m_tcpGetUserFriendInfoListTime;			//trick,请求用户信息的次数，一次请求所有人有问题。
	UInt32								m_tcpGetUserFriendInfoListBackTime;		//请求用户信息的次数，一次请求所有人有问题。返回的次数
};
/******************************************************************************/
#endif// IUSERLISTMODULE_IMPL_9768C185_67AE_45BB_B840_F0A66E6A7044_H__
