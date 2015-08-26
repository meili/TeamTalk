/******************************************************************************* 
 *  @file      IUserListModule_Impl.cpp 2014\8\6 15:28:35 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "UserListModule_Impl.h"
#include "utility/utilStrCodingAPI.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IDatabaseModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMiscModule.h"
#include "cxImage/cxImage/ximage.h"
#include "ProtocolBuffer/IM.Buddy.pb.h"
#include "network/ImCore.h"
#include "network/TTPBHeader.h"
#include "../Session/Operation/DownloadAvatarHttpOperation.h"
#include <random>

/******************************************************************************/
namespace module
{
	module::IUserListModule* getUserListModule()
	{
		static UserListModule_Impl module;
		return &module;
	}
}

// -----------------------------------------------------------------------------
//  IUserListModule_Impl: Public, Constructor

UserListModule_Impl::UserListModule_Impl()
{
}

// -----------------------------------------------------------------------------
//  IUserListModule_Impl: Public, Destructor

void UserListModule_Impl::onPacket(imcore::TTPBHeader& header, std::string& pbBody)
{
	switch (header.getCommandId())
	{
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE:
		_recentlistResponse(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_STATUS_NOTIFY:
		_userStatusNotify(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_USER_INFO_RESPONSE:
		_usersInfoResponse(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_REMOVE_SESSION_RES:
		_removeSessionResponse(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_ALL_USER_RESPONSE:
		_allUserlistResponse(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_USERS_STATUS_RESPONSE:
		_usersLineStatusResponse(pbBody);
		break;
	case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_CHANGE_AVATAR_RESPONSE:
		_changeAvatarResponse(pbBody);
		break;
	case  IM::BaseDefine::CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
		_removeSessionNotify(pbBody);
		break;
	case IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_RESPONSE:
		_departmentResponse(pbBody);
		break;
    case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_AVATAR_CHANGED_NOTIFY:
        _avatarChangeNotify(pbBody);
        break;
    case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_CHANGE_SIGN_INFO_RESPONSE:
        _changeSignInfoResponse(pbBody);
        break;
    case IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
        _signInfoChangedNotify(pbBody);
        break;
	default:
		LOG__(ERR, _T("Unknow commandID:%d"), header.getCommandId());
		return;
	}
}

void UserListModule_Impl::_allUserlistResponse(IN string& pbBody)
{
	IM::Buddy::IMAllUserRsp imAllUserRsp;
	if (!imAllUserRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	const UInt32 time = module::getSysConfigModule()->getUserInfoLatestUpdateTime();
	LOG__(APP, _T("IMAllUserRsp,latest_update_time = %d,Local_update_time = %d"), imAllUserRsp.latest_update_time(), time);
	if (imAllUserRsp.latest_update_time() != time)//比较latest_update_time
	{
		LOG__(APP, _T("IMAllUserRsp,user_list_size = %d"), imAllUserRsp.user_list_size());
		for (int n = 0; n < imAllUserRsp.user_list_size(); ++n)
		{
			const IM::BaseDefine::UserInfo& userInfo = imAllUserRsp.user_list(n);
			std::string sId = util::uint32ToString(userInfo.user_id());

			module::UserInfoEntity userInfoEntity;
			userInfoEntity.sId = sId;
			userInfoEntity.avatarUrl = userInfo.avatar_url();
			userInfoEntity.csName = util::stringToCString(userInfo.user_real_name());
			userInfoEntity.csNickName = util::stringToCString(userInfo.user_nick_name());
			userInfoEntity.department = util::uint32ToString(userInfo.department_id());
			userInfoEntity.email = userInfo.email();
			userInfoEntity.gender = userInfo.user_gender();
			userInfoEntity.user_domain = userInfo.user_domain();
			userInfoEntity.telephone = userInfo.user_tel();
			userInfoEntity.status = userInfo.status();
            userInfoEntity.signature = userInfo.sign_info();
			if (userInfoEntity.sId != module::getSysConfigModule()->userID())
			{
				CAutoLock lock(&m_lock);
				m_mapUsers[userInfoEntity.sId] = userInfoEntity;
			}
			_pushUserIdToDepartment(userInfoEntity.sId, userInfoEntity.department);//生成部门信息
		}
		module::getDatabaseModule()->sqlBatchInsertUserInfos(m_mapUsers);//保存用户信息
		module::getSysConfigModule()->saveUserInfoLatestUpdateTime(imAllUserRsp.latest_update_time());//保存最后一次更新时间
	}

	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		//todo incremental updating processing —enhancement
		_tcpGetAllUsersOnlineStatus();
		_downloadAllUserAvatarImg();//更新用户的头像
	});

	module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_DEPARTMENTLIST);

}
void UserListModule_Impl::_usersLineStatusResponse(IN string& pbBody)
{
	IM::Buddy::IMUsersStatRsp imUsersStatRsp;
	if (!imUsersStatRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
	}
	const UInt32 nSize = imUsersStatRsp.user_stat_list_size();
	LOG__(APP, _T("IMUsersStatRsp,Size = %d"), nSize);
	CAutoLock lock(&m_lock);
	for (UInt32 i = 0; i < nSize; ++i)
	{
		IM::BaseDefine::UserStat userStat = imUsersStatRsp.user_stat_list(i);
		std::string sId = util::uint32ToString(userStat.user_id());
		//更新在线状态
		module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sId);
		if (iter != m_mapUsers.end())
		{
			iter->second.onlineState = userStat.status();
		}
	}
	module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_ALLUSERLINESTATE);
}

void UserListModule_Impl::_changeAvatarResponse(IN string& pbBody)
{
	IM::Buddy::IMChangeAvatarRsp imChangeAvatarRsp;
	if (!imChangeAvatarRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	if (0 != imChangeAvatarRsp.result_code())
	{
		LOG__(ERR, _T("result_code isn't succeed:%d"), imChangeAvatarRsp.result_code());
		return;
	}
	std::string sid = util::uint32ToString(imChangeAvatarRsp.user_id());
	LOG__(APP, _T("IMChangeAvatarRsp,sid = %s"), util::stringToCString(sid));
}
void UserListModule_Impl::_removeSessionNotify(IN std::string& pbBody)
{
	IM::Buddy::IMRemoveSessionNotify imRemoveSessionNotify;
	if (!imRemoveSessionNotify.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	UInt32 id = imRemoveSessionNotify.session_id();
	std::string sessionId = util::uint32ToString(id);
	if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == imRemoveSessionNotify.session_type())
	{
		sessionId = module::getGroupListModule()->makeGroupSId(sessionId);
	}
	else if (IM::BaseDefine::SessionType::SESSION_TYPE_SINGLE == imRemoveSessionNotify.session_type())
	{

	}
	else
	{
		LOG__(ERR, _T("Unkonwn session_type:%d, sessionId:%d")
			, imRemoveSessionNotify.session_type(),id);
	}

	//TODO::通知删除会话
}

void UserListModule_Impl::_departmentResponse(IN std::string& pbBody)
{
	IM::Buddy::IMDepartmentRsp imDepartmentRsp;
	if (!imDepartmentRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	const UInt32 time = module::getSysConfigModule()->getDepartmentInfoLatestUpdateTime();
	LOG__(APP, _T("IMDepartmentRsp,latest_update_time = %d,Local_update_time = %d"), imDepartmentRsp.latest_update_time(), time);
	if (imDepartmentRsp.latest_update_time() != time)//比较latest_update_time
	{
		LOG__(APP, _T("IMDepartmentRsp,user_list_size = %d"), imDepartmentRsp.dept_list_size());
		for (int n = 0; n < imDepartmentRsp.dept_list_size(); ++n)
		{
			const IM::BaseDefine::DepartInfo& departInfo = imDepartmentRsp.dept_list(n);
			module::DepartmentEntity departmentEntity;
			departmentEntity.dId = util::uint32ToString(departInfo.dept_id());
			departmentEntity.priority = departInfo.priority();
			departmentEntity.parentDepartId = util::uint32ToString(departInfo.parent_dept_id());
			departmentEntity.name = util::stringToCString(departInfo.dept_name());
			departmentEntity.status = departInfo.dept_status();
			{
				CAutoLock lock(&m_lock);
				module::DepartmentMap::iterator itMap = m_mapDepartment.find(departmentEntity.dId);
				if (itMap == m_mapDepartment.end() || itMap->second.members.empty())
				{
					m_mapDepartment[departmentEntity.dId] = departmentEntity;
				}
				else
				{
					departmentEntity.members = itMap->second.members;
					itMap->second = departmentEntity;
				}				
			}
			module::getDatabaseModule()->sqlBatchInsertDepartmentInfos(m_mapDepartment);//保存部门信息
			module::getSysConfigModule()->saveDepartmentInfoLatestUpdateTime(time);//保存最后一次更新时间
		}
	}
}

void UserListModule_Impl::_recentlistResponse(IN string& pbBody)//最近联系人群信息
{	
	IM::Buddy::IMRecentContactSessionRsp imRecentContactSessionRsp;
	if (!imRecentContactSessionRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	const int nSize = imRecentContactSessionRsp.contact_session_list_size();
	LOG__(APP, _T("IMRecentContactSessionRsp,size=%d"), nSize);
	std::vector<module::SessionEntity> vecSessionEntities;
	
	for (int i = 0; i < nSize; i++)
	{
		const IM::BaseDefine::ContactSessionInfo& contactSessionInfo = imRecentContactSessionRsp.contact_session_list(i);
		module::SessionEntity recentSessionInfo;
		recentSessionInfo.sessionID = util::uint32ToString(contactSessionInfo.session_id());
		recentSessionInfo.sessionType = contactSessionInfo.session_type();
		recentSessionInfo.sessionStatus = contactSessionInfo.session_status();
		if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == contactSessionInfo.session_type())
		{
			recentSessionInfo.sessionID = module::getGroupListModule()->makeGroupSId(recentSessionInfo.sessionID);
		}
		recentSessionInfo.updatedTime = contactSessionInfo.updated_time();
		recentSessionInfo.latestmsgId = contactSessionInfo.latest_msg_id();
		recentSessionInfo.latestMsgContent = contactSessionInfo.latest_msg_data();
		recentSessionInfo.latestMsgFromId = util::uint32ToString(contactSessionInfo.latest_msg_from_user_id());
		const UInt32 Globaltime = module::getSessionModule()->getGlobalUpdateTime();
		LOG__(DEBG, _T("sessionID = %s,updatedTime = %d"), util::stringToCString(recentSessionInfo.sessionID), recentSessionInfo.updatedTime);
		if (recentSessionInfo.updatedTime > Globaltime)
		{
			module::getSessionModule()->setGlobalUpdateTime(recentSessionInfo.updatedTime);
            //如果是讨论组的话，需要更新讨论组的信息
            if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == recentSessionInfo.sessionType)
            {
                LOG__(APP, _T("discs group update:%d"), contactSessionInfo.session_id());
                module::GroupInfoEntity groupInfo;
                if (module::getGroupListModule()->getGroupInfoBySId(recentSessionInfo.sessionID, groupInfo)
                    && groupInfo.type == 2)
                {
                    module::getGroupListModule()->tcpGetGroupInfo(util::uint32ToString(contactSessionInfo.session_id()));
                }
            }
		}
		module::getSessionModule()->setSessionEntity(recentSessionInfo);
		vecSessionEntities.push_back(recentSessionInfo);
	}
	module::getDatabaseModule()->sqlBatchInsertRecentSessionInfos(vecSessionEntities);
	module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_RECENTLIST);
}
void UserListModule_Impl::_userStatusNotify(IN string& pbBody)
{	
	IM::Buddy::IMUserStatNotify imUserStatNotify;
	if (!imUserStatNotify.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
	}
	IM::BaseDefine::UserStat userStat = imUserStatNotify.user_stat();
	std::string sid = util::uint32ToString(userStat.user_id());
	LOG__(DEBG, _T("IMUserStatNotify,sid=%s,status=%d"), util::stringToCString(sid), userStat.status());//扰乱日志，暂时不打印
	module::UserInfoEntityMap::iterator itUser = m_mapUsers.find(sid);
	if (itUser != m_mapUsers.end())
	{
		CAutoLock lock(&m_lock);
		itUser->second.onlineState = userStat.status();
		module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_USERLINESTATE,sid);
	}
}
void UserListModule_Impl::_usersInfoResponse(IN string& pbBody)
{	
	IM::Buddy::IMUsersInfoRsp imUsersInfoRsp;
	if (!imUsersInfoRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	const int nSize = imUsersInfoRsp.user_info_list_size();
	LOG__(APP, _T("IMUsersInfoRsp,size=%d"),nSize);
	for (int n = 0; n < nSize; ++n)
	{
		const IM::BaseDefine::UserInfo& userInfo = imUsersInfoRsp.user_info_list(n);
		module::UserInfoEntity userInfoEntity;
		std::string sId = util::uint32ToString(userInfo.user_id());
		getUserInfoBySId(sId,userInfoEntity);
		userInfoEntity.sId = sId;
		userInfoEntity.avatarUrl = userInfo.avatar_url();
		userInfoEntity.csName = util::stringToCString(userInfo.user_real_name());
		userInfoEntity.csNickName = util::stringToCString(userInfo.user_nick_name());
		userInfoEntity.department = util::uint32ToString(userInfo.department_id());
		userInfoEntity.email = userInfo.email();
		userInfoEntity.gender = userInfo.user_gender();
		userInfoEntity.user_domain = userInfo.user_domain();
		userInfoEntity.telephone = userInfo.user_tel();
		userInfoEntity.status = userInfo.status();
        userInfoEntity.signature = userInfo.sign_info();//个性签名
		if (userInfoEntity.sId != module::getSysConfigModule()->userID())
		{
			CAutoLock lock(&m_lock);
			m_mapUsers[userInfoEntity.sId] = userInfoEntity;
		}
		_downloadAvatarImgBySId(sId);//更新用户的头像
		_pushUserIdToDepartment(userInfoEntity.sId, userInfoEntity.department);//生成部门信息
	}
	module::getDatabaseModule()->sqlBatchInsertUserInfos(m_mapUsers);
	_tcpGetAllUsersOnlineStatus();
	module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_DEPARTMENTLIST);
}

void UserListModule_Impl::_removeSessionResponse(IN string& pbBody)
{
	IM::Buddy::IMRemoveSessionRsp imRemoveSessionRsp;
	if (!imRemoveSessionRsp.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	std::string sid = util::uint32ToString(imRemoveSessionRsp.session_id());
	LOG__(APP, _T("IMRemoveSessionRsp sessionId =%s"), util::stringToCString(sid));
	if (0 != imRemoveSessionRsp.result_code())
	{
		LOG__(ERR, _T("result_code is not succeed:%d"), imRemoveSessionRsp.result_code());
		return;
	}
	
	if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == imRemoveSessionRsp.session_type())//群
	{
		
	}
	else if (IM::BaseDefine::SessionType::SESSION_TYPE_SINGLE == imRemoveSessionRsp.session_type())//人
	{

	}
}

const module::DepartmentMap& UserListModule_Impl::getAllDepartments()
{
	CAutoLock lock(&m_lock);
	return m_mapDepartment;
}
void UserListModule_Impl::getAllUsersInfo(module::UserInfoEntityMap& MapUsers)const
{
	MapUsers = m_mapUsers;
}
BOOL UserListModule_Impl::getUserInfoBySId(IN std::string sid, OUT module::UserInfoEntity& userInfo)
{
	CAutoLock lock(&m_lock);
	//据说map在find之前需要判空
	if (m_mapUsers.empty())
		return FALSE;
	module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sid);
	if (iter == m_mapUsers.end())
		return FALSE;

	userInfo = iter->second;
	return TRUE;
}

void UserListModule_Impl::removeAllListInfo()
{
	m_mapUsers.clear();
}

UInt8 UserListModule_Impl::getMyLineStatus()
{
	module::UserInfoEntity myInfo;
	getMyInfo(myInfo);
	if (myInfo.isOnlne())//TODO:离开的状态也是要考虑网络状态的
	{
		return IM::BaseDefine::USER_STATUS_ONLINE;
	}
	else
	{
		return IM::BaseDefine::USER_STATUS_OFFLINE;
	}
}

void UserListModule_Impl::tcpGetUserOnlieStatus(IN const std::string& sId)
{
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		UInt32 userId = util::stringToInt32(sId);
		IM::Buddy::IMUsersStatReq imUsersStatReq;
		imUsersStatReq.set_user_id(module::getSysConfigModule()->userId());
		imUsersStatReq.add_user_id_list(userId);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
			, IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_USERS_STATUS_REQUEST
			, &imUsersStatReq);
	}
	);
}
void UserListModule_Impl::_tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId)
{
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		IM::Buddy::IMUsersStatReq imUsersStatReq;
		for (auto sId : VecId)
		{
			UInt32 userId = util::stringToInt32(sId);
			imUsersStatReq.add_user_id_list(userId);
		}
		imUsersStatReq.set_user_id(module::getSysConfigModule()->userId());
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
			, IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_USERS_STATUS_REQUEST
			, &imUsersStatReq);
	}
	);
}
void UserListModule_Impl::tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId)
{
	if (VecId.empty())
	{
		return;
	}

	const UInt32 nMax = 300;		//大佛：分批获取,一次性获取太多会失败
	int nTime = static_cast<int>(VecId.size() / nMax);
	if (nTime > 0)
	{
		std::vector<std::string> MainList = static_cast<std::vector<std::string>>(VecId);
		std::vector<std::string>::iterator it = MainList.begin();
		for (int n = 0; n < nTime; ++n)
		{
			std::vector<std::string> VecId(nMax);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + n*nMax, it + (n + 1)*nMax, itSend);
			_tcpGetUserOnlieStatus(VecId);
		}
		UInt32 nLeft = VecId.size() % nMax;
		if (nLeft)
		{
			std::vector<std::string> VecId(nLeft);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + nTime*nMax, it + nTime*nMax + nLeft, itSend);
			_tcpGetUserOnlieStatus(VecId);
		}
	}
	else
	{
		_tcpGetUserOnlieStatus(VecId);
	}
}

void UserListModule_Impl::_tcpGetAllUsersOnlineStatus(void)
{
	if (m_mapUsers.empty())
	{
		return;
	}
	module::UserInfoEntityVec allUserVec;
	for (auto& kvp : m_mapUsers)
	{
		//if (kvp.second.sId != module::getSysConfigModule()->userID())//自己的状态自己维护
		{
			allUserVec.push_back(kvp.second.sId);
		}
	}
	tcpGetUserOnlieStatus(allUserVec);
}



void UserListModule_Impl::tcpGetUserInfo(IN const std::string& sId)
{
	if (sId.empty())
	{
		return;
	}
	module::UserInfoEntityVec VecUserInfo;
	VecUserInfo.push_back(sId);
	_tcpGetUserInfoList(VecUserInfo);
}

void UserListModule_Impl::tcpGetUsersInfo(IN const module::UserInfoEntityVec& VecUnKnowUserInfo)
{
	////一次最多取200，多了取不到了，亲。 ---- 大佛
	//const UInt32 nMax = 200;
	//m_tcpGetUserFriendInfoListBackTime = 0;
	//m_tcpGetUserFriendInfoListTime = static_cast<int>(VecUnKnowUserInfo.size() / nMax);
	//if (m_tcpGetUserFriendInfoListTime > 0)
	//{
	//	std::vector<std::string> MainList = static_cast<std::vector<std::string>>(VecUnKnowUserInfo);
	//	std::vector<std::string>::iterator it = MainList.begin();
	//	for (UInt32 n = 0; n < m_tcpGetUserFriendInfoListTime; ++n)
	//	{
	//		std::vector<std::string> VecId(nMax);
	//		std::vector<std::string>::iterator itSend = VecId.begin();
	//		std::copy(it + n*nMax, it + (n + 1)*nMax, itSend);
	//		_tcpGetUserInfoList(VecId);
	//	}
	//	UInt32 nLeft = VecUnKnowUserInfo.size() % nMax;
	//	if (nLeft)
	//	{
	//		std::vector<std::string> VecId(nLeft);
	//		std::vector<std::string>::iterator itSend = VecId.begin();
	//		std::copy(it + m_tcpGetUserFriendInfoListTime*nMax, it + m_tcpGetUserFriendInfoListTime*nMax + nLeft, itSend);
	//		_tcpGetUserInfoList(VecId);
	//	}
	//}
	//else
	//{
	//	std::vector<std::string> VecId;
	//	VecId = VecUnKnowUserInfo;
	//	_tcpGetUserInfoList(VecId);
	//}
}

void UserListModule_Impl::_tcpGetUserInfoList(IN module::UserInfoEntityVec VecUnKnowUserInfo)
{
	if (VecUnKnowUserInfo.empty())
		return;

	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		IM::Buddy::IMUsersInfoReq imUsersInfoReq;
		imUsersInfoReq.set_user_id(module::getSysConfigModule()->userId());
		for (auto sid:VecUnKnowUserInfo)
		{
			UInt32 userID = util::stringToInt32(sid);
			imUsersInfoReq.add_user_id_list(userID);
		}

		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
			, IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_USER_INFO_REQUEST
			, &imUsersInfoReq);
	}
	);
}

CString UserListModule_Impl::getDefaultAvatartPath()
{
	return module::getMiscModule()->getDefaultAvatar() + _T("default.png");
}

BOOL UserListModule_Impl::getMyInfo(OUT module::UserInfoEntity& myInfo)
{
	return getUserInfoBySId(module::getSysConfigModule()->userID(), myInfo);
}

void UserListModule_Impl::_pushUserIdToDepartment(const std::string& sId, const std::string& dId)
{
	CAutoLock lock(&m_lock);
	module::DepartmentMap::iterator itDepartment = m_mapDepartment.find(dId);
	if (itDepartment != m_mapDepartment.end())//该部门已存在
	{
		module::DepartmentEntity& entity = itDepartment->second;
		entity.members.push_back(sId);
	}
	else//若该部门不存在
	{
		module::DepartmentEntity entity;
		entity.dId = dId;
		entity.members.push_back(sId);
		m_mapDepartment.insert(std::make_pair(dId, entity));
	}
}

void UserListModule_Impl::getSearchUserNameListByShortName(IN const CString& sShortName, OUT module::UserInfoEntityVec& nameList)
{
	for (auto& kvp : m_mapUsers)
	{
		module::UserInfoEntity& userInfo = kvp.second;

		CString RealName = userInfo.getRealName();

		if (util::isIncludeChinese(util::cStringToString(sShortName, CP_ACP)))//检索中文
		{
			if (RealName.Find(sShortName) != -1)
			{
				nameList.push_back(userInfo.sId);
			}
		}
		else//检索字母
		{
			CString strLowName = sShortName;
			strLowName.MakeLower();
			CString firstPY = util::HZ2FirstPY(util::cStringToString(RealName, CP_ACP));
			if (firstPY.Find(strLowName) != -1)//先检索简拼
			{
				nameList.push_back(userInfo.sId);
			}
			else
			{
				//CString allPY = util::HZ2AllPY(RealName);//再检索全拼
				CString allPY = util::stringToCString(userInfo.user_domain);
				if (allPY.Find(strLowName) != -1)
				{
					nameList.push_back(userInfo.sId);
				}
			}
		}

	}
}
void UserListModule_Impl::getSearchDepartmentsByShortName(IN const CString& sShortName,
	OUT std::vector<std::string>& departmentList)
{
	CString strUPName = sShortName;
	strUPName.MakeUpper();
	for (auto kvD:m_mapDepartment)
	{
		module::DepartmentEntity&  departMent = kvD.second;
		CString groupName = departMent.name;
		groupName.MakeUpper();
		//if (groupName.CompareNoCase(sShortName))
		if (-1 != groupName.Find(strUPName))
		{
			departmentList.push_back(departMent.dId);
		}
	}
}

BOOL UserListModule_Impl::_downloadAvatarImgBySId(IN const std::string& sId)
{
	CAutoLock lock(&m_lock);
	module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sId);
	if (iter == m_mapUsers.end())
		return FALSE;

	module::UserInfoEntity& user = iter->second;
	std::string strFormat = AVATAR_FORMAT_48X48;
	if (module::getSysConfigModule()->userID() == user.sId)
		strFormat = AVATAR_FORMAT_64X64;
	
	if (!user.avatarUrl.empty() && user.avatarUrl.find("avatar_default.jpg") == std::string::npos)
	{
		UInt32 hashcode = util::hash_BKDR((user.avatarUrl + strFormat).c_str());
		module::ImImageEntity imageEntity;
		module::getDatabaseModule()->sqlGetImImageEntityByHashcode(hashcode, imageEntity);
		CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(imageEntity.filename);
		if (!imageEntity.filename.empty() && PathFileExists(csLocalPath))
		{
			//本地磁盘存在
			user.avatarLocalPath = util::cStringToString(csLocalPath);
			user.avatarGrayLocalPath = _getGrayLocalPathFromFilename(imageEntity.filename);
		}
		else
		{
			//不存在则去服务器下载
			DownloadAvatarHttpOperation* pOper = new DownloadAvatarHttpOperation(user.sId, user.avatarUrl, TRUE
				, strFormat, BIND_CALLBACK_1(UserListModule_Impl::onCallbackOperation));
			module::getHttpPoolModule()->pushHttpOperation(pOper);
		}
	}

	return TRUE;
}

void UserListModule_Impl::_downloadAllUserAvatarImg()
{
	for (auto& kv : m_mapUsers)
	{
		_downloadAvatarImgBySId(kv.first);
	}
}

std::string UserListModule_Impl::_getGrayLocalPathFromFilename(std::string& finename)
{
	CString csFileName = util::stringToCString(finename);
	CString csGrayAvatarPath = module::getMiscModule()->getDownloadDir() + PREFIX_GRAY_AVATAR + csFileName;
	//本地磁盘存在
	if (PathFileExists(csGrayAvatarPath))
	{
		return util::cStringToString(csGrayAvatarPath);
	}
	else
	{
		//不存在则对图片做灰度处理并且保存到本地
		CxImage cximage;
		CString csAvatarPath = module::getMiscModule()->getDownloadDir() + csFileName;
		bool bSucc = cximage.Load(csAvatarPath);
		if (bSucc)
		{
			cximage.GrayScale();
			cximage.Save(csAvatarPath, CXIMAGE_FORMAT_PNG);
		}
	}
	return "";
}

void UserListModule_Impl::onCallbackOperation(std::shared_ptr<void> param)
{
	DownloadImgParam* pDownParam = (DownloadImgParam*)param.get();
	if (DownloadImgParam::DOWNLOADIMG_OK == pDownParam->m_result)
	{
		module::UserInfoEntityMap::iterator userIter = m_mapUsers.find(pDownParam->m_sId);
		if (userIter != m_mapUsers.end())
		{
			CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(pDownParam->m_imgEntity.filename);
			userIter->second.avatarLocalPath = util::cStringToString(csLocalPath);
			userIter->second.avatarGrayLocalPath = _getGrayLocalPathFromFilename(pDownParam->m_imgEntity.filename);
		}
		std::tuple<std::string, std::string>* pTuple = new std::tuple<std::string, std::string>;
		*pTuple = std::tie(module::MODULE_USERLIST_PREFIX, pDownParam->m_sId);
		module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_DOWNAVATAR_SUCC, shared_ptr<void>(pTuple));
	}
}

BOOL UserListModule_Impl::createUserInfo(IN const module::UserInfoEntity& info)
{
	CAutoLock lock(&m_lock);
	module::UserInfoEntity infoTemp;
	if (getUserInfoBySId(info.sId, infoTemp))
		return FALSE;
	m_mapUsers[info.sId] = info;

	//下载头像
	_downloadAvatarImgBySId(info.sId);

	return TRUE;
}

BOOL UserListModule_Impl::startup()
{
	std::vector<module::DepartmentEntity> departmentList;
	if (!module::getDatabaseModule()->sqlGetAllDepartmentInfo(departmentList))
	{
		LOG__(ERR, _T("get all local department info failed"));
		return FALSE;
	}
	for (auto departmentInfo : departmentList)
	{
		m_mapDepartment[departmentInfo.dId] = departmentInfo;
	}

	std::vector<module::UserInfoEntity> userlist;
	if (!module::getDatabaseModule()->sqlGetAllUsersInfo(userlist))
	{
		LOG__(ERR, _T("get all local user info failed"));
		return FALSE;
	}
	LOG__(APP, _T("local db user count = %d"),userlist.size());
	for (const module::UserInfoEntity& userInfo : userlist)
	{
		if (userInfo.sId != module::getSysConfigModule()->userID())
		{
			m_mapUsers[userInfo.sId] = userInfo;
			_pushUserIdToDepartment(userInfo.sId, userInfo.department);//生成部门信息
		}
	}

	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		_tcpGetAllUsersOnlineStatus();
		_downloadAllUserAvatarImg();//更新用户的头像
	}
	);
	module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_DEPARTMENTLIST);

	return TRUE;
}

std::string UserListModule_Impl::randomGetUser(void)
{
	std::string sid;
	module::UserInfoEntity myInfo;
	module::getUserListModule()->getMyInfo(myInfo);
	module::UserInfoEntityVec allUserVec;
	CAutoLock lock(&m_lock);
	for (auto kvp : m_mapUsers)
	{
		if (kvp.second.gender != myInfo.gender)//只获取跟自己性别不同的
		{
			allUserVec.push_back(kvp.second.sId);
		}
	}
	//取随机数
	std::default_random_engine generator(time(0));
	std::uniform_int_distribution<int> dis(0, allUserVec.size()-1);
	auto dice = std::bind(dis, generator);
	UInt32 nRandon = dice();
	if (nRandon < allUserVec.size())
	{
		sid = allUserVec[nRandon];
	}
	return sid;
}


void UserListModule_Impl::_changeSignInfoResponse(IN std::string& pbBody)
{
    IM::Buddy::IMChangeSignInfoRsp imChangeSignInfoRsp;
    if (!imChangeSignInfoRsp.ParseFromString(pbBody))
    {
        LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
        return;
    }
    if (0 != imChangeSignInfoRsp.result_code())
    {
        LOG__(ERR, _T("result_code isn't succeed:%d"), imChangeSignInfoRsp.result_code());
        module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_USERSIGNINFO_CHANGED
            , module::getSysConfigModule()->userID());//修改失败？
        return;
    }
    std::string sid = util::uint32ToString(imChangeSignInfoRsp.user_id());
    std::string sSignInfo = imChangeSignInfoRsp.sign_info();
    LOG__(APP, _T("IMChangeSignInfoRsp,sid = %s,SignInfo=%s")
        , util::stringToCString(sid), util::stringToCString(sSignInfo));
    module::UserInfoEntity userInfo;
    if (!getUserInfoBySId(sid, userInfo))
    {
        LOG__(DEBG, _T("Can't find the sid"));
        return;
    }
    {
        CAutoLock lock(&m_lock);
        userInfo.signature = sSignInfo;
        m_mapUsers[userInfo.sId] = userInfo;
    }
    module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_USERSIGNINFO_CHANGED, sid);
}
void UserListModule_Impl::_avatarChangeNotify(IN std::string& pbBody)//修改头像签名通知
{
    IM::Buddy::IMAvatarChangedNotify imAvatarChangedNotify;
    if (!imAvatarChangedNotify.ParseFromString(pbBody))
    {
        LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
        return;
    }
    std::string sid = util::uint32ToString(imAvatarChangedNotify.changed_user_id());
    std::string sAvatarUrl = imAvatarChangedNotify.avatar_url();
    LOG__(APP, _T("IMAvatarChangedNotify,sid = %s,sAvatarUrl=%s")
        , util::stringToCString(sid), util::stringToCString(sAvatarUrl));
    module::UserInfoEntity userInfo;
    if (!getUserInfoBySId(sid, userInfo))
    {
        LOG__(DEBG, _T("Can't find the sid"));
        return;
    }
    {
        CAutoLock lock(&m_lock);
        userInfo.avatarUrl = sAvatarUrl;
        m_mapUsers[userInfo.sId] = userInfo;
    }
    _downloadAvatarImgBySId(sid);//下载头像
}
void UserListModule_Impl::_signInfoChangedNotify(IN std::string& pbBody)//修改个性签名通知
{
    IM::Buddy::IMSignInfoChangedNotify imSignInfoChangedNotify;
    if (!imSignInfoChangedNotify.ParseFromString(pbBody))
    {
        LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
        return;
    }

    std::string sid = util::uint32ToString(imSignInfoChangedNotify.changed_user_id());
    std::string sSignInfo = imSignInfoChangedNotify.sign_info();
    LOG__(APP, _T("IMSignInfoChangedNotify,sid = %s,sSignInfo=%s")
        , util::stringToCString(sid), util::stringToCString(sSignInfo));
    module::UserInfoEntity userInfo;
    if (!getUserInfoBySId(sid, userInfo))
    {
        LOG__(DEBG, _T("Can't find the sid"));
        return;
    }
    {
        CAutoLock lock(&m_lock);
        userInfo.signature = sSignInfo;
        m_mapUsers[userInfo.sId] = userInfo;
    }
    module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_USERSIGNINFO_CHANGED, sid);
}
void UserListModule_Impl::tcpChangeMySignInfo(IN const std::string sSignInfo)
{
    if (sSignInfo.empty())//空的，是取消签名
    {
        LOG__(APP, _T("sSignInfo empty"));
    }
    imcore::IMLibCoreStartOperationWithLambda(
        [=]()
    {
        IM::Buddy::IMChangeSignInfoReq imChangeSignInfoReq;
        imChangeSignInfoReq.set_user_id(module::getSysConfigModule()->userId());
        imChangeSignInfoReq.set_sign_info(sSignInfo);

        LOG__(APP, _T("IMChangeSignInfoReq:%s"), util::stringToCString(sSignInfo));
        module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
            , IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_CHANGE_SIGN_INFO_REQUEST
            , &imChangeSignInfoReq);
    }
    );
}


/******************************************************************************/