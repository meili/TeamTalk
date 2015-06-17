/******************************************************************************* 
 *  @file      GroupListModule_Impl.cpp 2014\8\6 15:30:42 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ProtocolBuffer/im.Group.pb.h"
#include "GroupListModule_Impl.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ISysConfigModule.h"
#include "utility/utilStrCodingAPI.h"
#include "utility/Multilingual.h"
#include "UI/CreateDiscussionGrpDialog.h"
#include "UI/ChangeDiscussionGrpMemberDialog.h"
#include "../Session/Operation/DownloadAvatarHttpOperation.h"
#include "network/ImCore.h"
#include "network/TTPBHeader.h"
#include <TUPLE>
/******************************************************************************/
namespace module
{
	module::IGroupListModule* getGroupListModule()
	{
		static GroupListModule_Impl module;
		return &module;
	}
}

void GroupListModule_Impl::release()
{
	delete this;
}

void GroupListModule_Impl::onPacket(imcore::TTPBHeader& header, std::string& pbBody)
{
	switch (header.getCommandId())
	{
	case IM::BaseDefine::CID_GROUP_NORMAL_LIST_RESPONSE :
		_groupNormalListResponse(pbBody);
		break;
	case IM::BaseDefine::CID_GROUP_INFO_RESPONSE:
		_groupInfoResponse(pbBody);
		break;
	case IM::BaseDefine::CID_GROUP_CREATE_RESPONSE:
		_groupCreateDiscussGroupResponse(pbBody);
		break;
	case IM::BaseDefine::CID_GROUP_CHANGE_MEMBER_RESPONSE:
		_groupChangedGroupMembersResponse(pbBody);
		break;
	case IM::BaseDefine::CID_GROUP_SHIELD_GROUP_RESPONSE:
		_groupShieldResponse(pbBody);
		break;
	case IM::BaseDefine::CID_GROUP_CHANGE_MEMBER_NOTIFY:
		_groupChangeMemberNotify(pbBody);
		break;
	default:
		break;
	}
}
void GroupListModule_Impl::getAllGroupListInfo(OUT module::GroupInfoMap& Groups)
{
	Groups = m_mapGroups;
}
BOOL GroupListModule_Impl::startup()
{
	std::vector<module::GroupInfoEntity> groupList;
	if (!module::getDatabaseModule()->sqlGetAllGroupInfo(groupList))
	{
		LOG__(ERR, _T("get all local group info failed"));
		return FALSE;
	}

	for (const module::GroupInfoEntity& groupInfo : groupList)
	{
		m_mapGroups[groupInfo.gId] = groupInfo;
	}
	_downloadAllGroupAvatarImg();//下载所有群的头像
	module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_GROUPLIST);//通知UI更新界面
	return TRUE;
}
BOOL GroupListModule_Impl::getGroupInfoBySId(IN const std::string& sID, OUT module::GroupInfoEntity& groupInfo)
{
	CAutoLock lock(&m_lock);
	module::GroupInfoMap::iterator it = m_mapGroups.find(sID);
	if (it != m_mapGroups.end())
	{
		groupInfo = it->second;
		return TRUE;
	}
	return FALSE;
}


BOOL GroupListModule_Impl::popAddedMemberVecBySId(IN const std::string& sID, OUT module::UserInfoEntityVec& AddedMemberVec)
{
	std::map<std::string, module::UserInfoEntityVec>::iterator it = m_mapAddedMember.find(sID);
	if (it != m_mapAddedMember.end())
	{
		AddedMemberVec = it->second;
		{
			CAutoLock lock(&m_lock);
			m_mapAddedMember.erase(it);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL GroupListModule_Impl::popRemovedMemberVecBySId(IN const std::string& sID, OUT module::UserInfoEntityVec& RemovedMemberVec)
{
	std::map<std::string, module::UserInfoEntityVec>::iterator it = m_mapRemovedMember.find(sID);
	if (it != m_mapRemovedMember.end())
	{
		RemovedMemberVec = it->second;
		{
			CAutoLock lock(&m_lock);
			m_mapRemovedMember.erase(it);
		}
		return TRUE;
	}
	return FALSE;
}
CString GroupListModule_Impl::getDefaultAvatartPath()
{
	return module::getMiscModule()->getDefaultAvatar() + _T("DiscussionGroups.png");
}
void GroupListModule_Impl::tcpGetGroupsInfo(IN const module::GroupVec& VecGroupId)
{
	if (VecGroupId.empty())
	{
		return;
	}
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		IM::Group::IMGroupInfoListReq imGroupInfoListReq;
		imGroupInfoListReq.set_user_id(module::getSysConfigModule()->userId());		
		for (auto groupId : VecGroupId)
		{
			UInt32 groupID = util::stringToInt32(getOriginalSId(groupId));
			LOG__(APP, _T("IMGroupInfoListReq, groupID = %d"), groupID);
			IM::BaseDefine::GroupVersionInfo* pGroupVersionInfo = imGroupInfoListReq.add_group_version_list();
			pGroupVersionInfo->set_group_id(groupID);
			pGroupVersionInfo->set_version(0);
		}
		imGroupInfoListReq.set_user_id(module::getSysConfigModule()->userId());		
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_GROUP
			, IM::BaseDefine::GroupCmdID::CID_GROUP_INFO_REQUEST
			, &imGroupInfoListReq);
	}
	);
}

void GroupListModule_Impl::tcpGetGroupInfo(IN const std::string& groupId)
{
	if (groupId.empty())
	{
		return;
	}
	module::GroupVec VecGroupId;
	VecGroupId.push_back(groupId);
	tcpGetGroupsInfo(VecGroupId);
}


void GroupListModule_Impl::tcpShieldGroup(IN const std::string& groupId, IN UInt32 shieldStatus)
{
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		IM::Group::IMGroupShieldReq imGroupShieldReq;
		imGroupShieldReq.set_user_id(module::getSysConfigModule()->userId());
		UInt32 groupID = util::stringToInt32(getOriginalSId(groupId));
		imGroupShieldReq.set_group_id(groupID);
		imGroupShieldReq.set_shield_status(shieldStatus);

		LOG__(APP, _T("IMGroupShieldReq, groupID = %d,shieldStatus = %d"), groupID, shieldStatus);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_GROUP
			, IM::BaseDefine::GroupCmdID::CID_GROUP_SHIELD_GROUP_REQUEST
			, &imGroupShieldReq);
	}
	);
}
void GroupListModule_Impl::onCreateDiscussionGrpDialog(const std::string& currentSessionId)
{
	CreateDiscussionGrpDialog* pFrame = new CreateDiscussionGrpDialog(currentSessionId);
	if (pFrame == NULL) return;
	CString strWinName = util::getMultilingual()->getStringById(_T("STRID_GROUPLISTMODULE_CREATGROUPWNDNAME"));
	pFrame->Create(NULL, strWinName, UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 800);
	pFrame->CenterWindow();
	pFrame->ShowWindow(true);
}

void GroupListModule_Impl::onChangeDiscussionGrpMemberDialog(const std::string& currentSessionId)
{
	ChangeDiscussionGrpMemberDialog* pFrame = new ChangeDiscussionGrpMemberDialog(currentSessionId);
	if (pFrame == NULL) return;
	CString strWinName = util::getMultilingual()->getStringById(_T("STRID_GROUPLISTMODULE_CHANGEGROUPNUM"));
	pFrame->Create(NULL, strWinName, UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 800);
	pFrame->CenterWindow();
	pFrame->ShowWindow(true);
}

std::string GroupListModule_Impl::makeGroupSId(const std::string& sid)
{
	if (sid.empty())
	{
		return sid;
	}
	std::string sessionID = ("group_") + sid;
	return sessionID;
}
std::string GroupListModule_Impl::getOriginalSId(const std::string& sid)
{
	std::string::size_type len = sid.size();
	if (len > 6)
	{
		std::string groupMark = sid.substr(0, 6);
		if (groupMark == std::string("group_"))
		{
			std::string OriginalSID = sid.substr(6, len);
			return OriginalSID;
		}
		else
		{
			LOG__(ERR, _T("groupMark can't find group_:%s"), util::stringToCString(sid));
		}
	}
	return sid;
}

void GroupListModule_Impl::_groupNormalListResponse(IN std::string& body)
{
	IM::Group::IMNormalGroupListRsp imNormalGrouplstResp;
	if (!imNormalGrouplstResp.ParseFromString(body))
	{
		LOG__(ERR, _T("normal group parse failed,body:%s"), util::stringToCString(body));
		return;
	}
	IM::Group::IMGroupInfoListReq imGroupInfoListReq;
	const UInt32 nSize = imNormalGrouplstResp.group_version_list_size();
	LOG__(APP, _T("IMNormalGroupListRsp,size = %d"), nSize);
	for (UInt32 i = 0; i < nSize; ++i)
	{
		IM::BaseDefine::GroupVersionInfo groupVersionInfo = imNormalGrouplstResp.group_version_list(i);
		std::string& groupId = makeGroupSId(util::uint32ToString(groupVersionInfo.group_id()));
		module::GroupInfoEntity groupinfo;
		BOOL bNeedUpdate = TRUE;
		if (getGroupInfoBySId(groupId, groupinfo))
		{
			if (groupinfo.gId == groupId
				&& groupinfo.version == groupVersionInfo.version())
			{
				bNeedUpdate = FALSE;
			}
		}
		if (bNeedUpdate)
		{
			groupinfo.gId = groupId;
			IM::BaseDefine::GroupVersionInfo* pGroupVersionInfo = imGroupInfoListReq.add_group_version_list();
			pGroupVersionInfo->set_group_id(util::stringToInt32(getOriginalSId(groupinfo.gId)));//传本地保存的群的版本号
			pGroupVersionInfo->set_version(groupinfo.version);
			{
				CAutoLock lock(&m_lock);
				m_mapGroups[groupId] = groupinfo;
			}
		}
	}

	if (0 != imGroupInfoListReq.group_version_list_size())//请求群信息
	{
		imGroupInfoListReq.set_user_id(module::getSysConfigModule()->userId());
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_GROUP
			, IM::BaseDefine::GroupCmdID::CID_GROUP_INFO_REQUEST
			, &imGroupInfoListReq);
	}
}

void GroupListModule_Impl::_groupInfoResponse(IN std::string& body)
{
	IM::Group::IMGroupInfoListRsp imGroupInfoListResp;
	if (!imGroupInfoListResp.ParseFromString(body))
	{
		LOG__(ERR, _T("group detail info list parse failed,body:%s"), util::stringToCString(body));
		return;
	}
	std::list<std::string>* pGroupIDs = new std::list<std::string>;
	const UInt32 nSize = imGroupInfoListResp.group_info_list_size();
	LOG__(APP, _T("IMGroupInfoListRsp,nSize = %d"), nSize);
	for (UInt32 i = 0; i < nSize; ++i)
	{
		IM::BaseDefine::GroupInfo groupDetailInfo = imGroupInfoListResp.group_info_list(i);
		std::string& groupId = makeGroupSId(util::uint32ToString(groupDetailInfo.group_id()));
		LOG__(APP, _T("GroupId = %s"), util::stringToCString(groupId));
		module::GroupInfoEntity groupinfo;
		BOOL bExist = getGroupInfoBySId(groupId, groupinfo);
		groupinfo.gId = groupId;
		groupinfo.version = groupDetailInfo.version();
		groupinfo.csName = util::stringToCString(groupDetailInfo.group_name());
		groupinfo.avatarUrl = groupDetailInfo.group_avatar();
		groupinfo.creatorId = util::uint32ToString(groupDetailInfo.group_creator_id());
		groupinfo.type = groupDetailInfo.group_type();
		groupinfo.shieldStatus = groupDetailInfo.shield_status();
		for (int j = 0; j < groupDetailInfo.group_member_list_size(); ++j)
		{
			std::string userId = util::uint32ToString(groupDetailInfo.group_member_list(j));
			_safePushBack(userId, groupinfo.groupMemeberList);
		}
		if (!groupinfo.groupMemeberList.empty())//可能存在无效的空群
		{
			{
				CAutoLock lock(&m_lock);
				m_mapGroups[groupId] = groupinfo;
			}
			pGroupIDs->push_back(groupId);
			if (!bExist)//被加入了陌生群，收到陌生群消息返回，需要创建会话
			{
				_creatAndSaveSessionToDB(groupinfo);
			}
		}
		else
		{
			LOG__(ERR, _T("the group member is empty,groupID:%d, name:%s"), groupDetailInfo.group_id(), groupinfo.csName);
		}
	}
	if (0 != imGroupInfoListResp.group_info_list_size()
		&& !pGroupIDs->empty())
	{
		module::getDatabaseModule()->sqlBatchInsertGroupInfos(m_mapGroups);
		module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_GROUPINFO_UPDATE, std::shared_ptr<void>(pGroupIDs));
		//下载所有群的头像
		_downloadAllGroupAvatarImg();
	}
	else
		LOG__(ERR, _T("group_info_list_size is %d,pGroupIDs size:%d")
		, imGroupInfoListResp.group_info_list_size(), pGroupIDs->size());
}

void GroupListModule_Impl::_groupCreateDiscussGroupResponse(IN std::string& body)
{
	IM::Group::IMGroupCreateRsp imGroupCreateResp;
	if (!imGroupCreateResp.ParseFromString(body))
	{
		LOG__(ERR, _T("create group response parse failed,body:%s"), util::stringToCString(body));
		return;
	}
	UInt32 resultCode = imGroupCreateResp.result_code();
	if (resultCode != 0)
	{
		LOG__(ERR, _T("create group faile,error code:%d"), resultCode);
		return;
	}

	std::string& groupId = makeGroupSId(util::uint32ToString(imGroupCreateResp.group_id()));
	LOG__(APP, _T("IMGroupCreateRsp,groupId = %s"), util::stringToCString(groupId));
	module::GroupInfoEntity groupInfo;
	getGroupInfoBySId(groupId, groupInfo);
	groupInfo.gId = groupId;
	groupInfo.csName = util::stringToCString(imGroupCreateResp.group_name());
	groupInfo.groupUpdated = static_cast<UInt32>(time(0));
	groupInfo.type = 2;
	groupInfo.creatorId = module::getSysConfigModule()->userID();
	for (int i = 0; i < imGroupCreateResp.user_id_list_size(); ++i)
	{
		std::string& userId = util::uint32ToString(imGroupCreateResp.user_id_list(i));
		_safePushBack(userId, groupInfo.groupMemeberList);
	}
	{
		CAutoLock lock(&m_lock);
		m_mapGroups[groupId] = groupInfo;
	}
	//保存群信息到sqllite
	module::getDatabaseModule()->sqlInsertOrReplaceGroupInfoEntity(groupInfo);
	_creatAndSaveSessionToDB(groupInfo);
	module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_CREATNEWGROUP, groupInfo.gId);//通知UI更新讨论组
	//下载所有群的头像
	_downloadAllGroupAvatarImg();
}
void GroupListModule_Impl::_groupChangedGroupMembersResponse(IN std::string& body)
{
	IM::Group::IMGroupChangeMemberRsp imGroupChangeMemberRsp;
	if (!imGroupChangeMemberRsp.ParseFromString(body))
	{
		LOG__(ERR, _T("ParseFromString failed,body:%s"), util::stringToCString(body));
		return;
	}

	std::string groupID = makeGroupSId(util::uint32ToString(imGroupChangeMemberRsp.group_id()));
	LOG__(APP, _T("IMGroupChangeMemberRsp,groupID = %s"), util::stringToCString(groupID));
	
	module::GroupInfoMap::iterator itGroupInfo = m_mapGroups.find(groupID);
	if (itGroupInfo != m_mapGroups.end())
	{
		module::GroupInfoEntity& groupInfo = itGroupInfo->second;
		if (IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_DEL == imGroupChangeMemberRsp.change_type())
		{
			for (int i = 0; i < imGroupChangeMemberRsp.chg_user_id_list_size(); ++i)
			{
				std::string sid = util::uint32ToString(imGroupChangeMemberRsp.chg_user_id_list(i));
				for (std::list<string>::iterator it = groupInfo.groupMemeberList.begin()
					; it != groupInfo.groupMemeberList.end(); ++it)
				{
					if (*it == sid)
					{
						LOG__(APP, _T("erase sid = %s"), util::stringToCString(sid));
						groupInfo.groupMemeberList.erase(it);
						break;
					}
				}
			}
		}
		else if (IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_ADD == imGroupChangeMemberRsp.change_type())
		{
			for (int i = 0; i < imGroupChangeMemberRsp.chg_user_id_list_size(); ++i)
			{
				std::string sid = util::uint32ToString(imGroupChangeMemberRsp.chg_user_id_list(i));
				LOG__(APP, _T("add sid = %s"), util::stringToCString(sid));
				_safePushBack(sid, groupInfo.groupMemeberList);
			}
		}
		else
		{
			LOG__(ERR, _T("Unknown Modify Type:%d"), imGroupChangeMemberRsp.group_id());
			return;
		}
		//保存群信息到sqllite
		module::getDatabaseModule()->sqlInsertOrReplaceGroupInfoEntity(groupInfo);

		//通知
		module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_MEMBER_CHANGED, groupID);//通知UI更新
	}
	else
	{
		LOG__(ERR, _T("Can't find the changed group:%d"), imGroupChangeMemberRsp.group_id());
	}
}
void GroupListModule_Impl::_groupShieldResponse(IN std::string& body)
{
	IM::Group::IMGroupShieldRsp imGroupShieldRsp;
	if (!imGroupShieldRsp.ParseFromString(body))
	{
		LOG__(ERR, _T("ParseFromString failed,body:%s"), util::stringToCString(body));
		return;
	}
	std::string groupId = util::uint32ToString(imGroupShieldRsp.group_id());
	groupId = makeGroupSId(groupId);
	UInt32 nRes = imGroupShieldRsp.result_code();
	LOG__(APP, _T("IMGroupShieldRsp, groupId:%d,result:%d"), imGroupShieldRsp.group_id(),nRes);
	if (0 == nRes)
	{
		module::GroupInfoMap::iterator itGroupInfo = m_mapGroups.find(groupId);
		if (itGroupInfo == m_mapGroups.end())
		{
			LOG__(ERR, _T("Can't find the group:%s"), util::stringToCString(groupId));
			return;
		}
		itGroupInfo->second.shieldStatus = !(itGroupInfo->second.shieldStatus);//成功的话，就是取反
		//保存群信息到sqllite
		module::getDatabaseModule()->sqlInsertOrReplaceGroupInfoEntity(itGroupInfo->second);
		module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_SHIELD_SUCCEED, groupId);//通知UI更新
	}
	else 
		LOG__(ERR, _T("group Shield failed groupId:%s,result:%d"), util::stringToCString(groupId), nRes);
}

void GroupListModule_Impl::_groupChangeMemberNotify(IN std::string& body)
{
	IM::Group::IMGroupChangeMemberNotify imGroupChangeMemberNotify;
	if (!imGroupChangeMemberNotify.ParseFromString(body))
	{
		LOG__(ERR, _T("ParseFromString failed,body:%s"), util::stringToCString(body));
		return;
	}
	std::string groupID = makeGroupSId(util::uint32ToString(imGroupChangeMemberNotify.group_id()));
	LOG__(APP, _T("IMGroupChangeMemberNotify, groupId:%s"), util::stringToCString(groupID));
	module::GroupInfoEntity groupInfo;
	if (!getGroupInfoBySId(groupID, groupInfo))
	{
		LOG__(APP, _T("can't find the group id:%s"), util::stringToCString(groupID));
		if (IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_ADD == imGroupChangeMemberNotify.change_type())//只有新增的时候需要去请求
		{
			IM::Group::IMGroupInfoListReq imGroupInfoListReq;//本地没有这个群的信息
			imGroupInfoListReq.set_user_id(module::getSysConfigModule()->userId());
			IM::BaseDefine::GroupVersionInfo* pGroupVersionInfo = imGroupInfoListReq.add_group_version_list();
			pGroupVersionInfo->set_group_id(imGroupChangeMemberNotify.group_id());
			pGroupVersionInfo->set_version(0);
			module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_GROUP
				, IM::BaseDefine::GroupCmdID::CID_GROUP_INFO_REQUEST
				, &imGroupInfoListReq);
		}
		return;
	}
	const UInt32 nChangeType = imGroupChangeMemberNotify.change_type();
	BOOL bExistMySelf = FALSE;
	if (IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_ADD == imGroupChangeMemberNotify.change_type())//增加成会员
	{
		for (int n = 0; n < imGroupChangeMemberNotify.chg_user_id_list_size();++n)
		{
			CAutoLock lock(&m_lock);
			std::string& sid = util::uint32ToString(imGroupChangeMemberNotify.chg_user_id_list(n));
			bExistMySelf = (sid == module::getSysConfigModule()->userID());
			LOG__(APP, _T("add sid = %s"), util::stringToCString(sid));
			_safePushBack(sid, m_mapGroups[groupID].groupMemeberList);
		}
		if (0 != imGroupChangeMemberNotify.chg_user_id_list_size())
		{
			//保存群信息到sqllite
            module::getDatabaseModule()->sqlInsertOrReplaceGroupInfoEntity(m_mapGroups[groupID]);
			_creatAndSaveSessionToDB(m_mapGroups[groupID]);
		}
	}
	else if (IM::BaseDefine::GroupModifyType::GROUP_MODIFY_TYPE_DEL == imGroupChangeMemberNotify.change_type())//删除成员
	{
		std::list<string>& groupMemeberList = m_mapGroups[groupID].groupMemeberList;
		for (int n = 0; n < imGroupChangeMemberNotify.chg_user_id_list_size(); ++n)
		{
			for (std::list<string>::iterator itUser = groupMemeberList.begin();
				itUser != groupMemeberList.end(); ++itUser)
			{
				std::string sid = util::uint32ToString(imGroupChangeMemberNotify.chg_user_id_list(n));
				bExistMySelf = (sid == module::getSysConfigModule()->userID());
				if (*itUser == sid)
				{
					LOG__(APP, _T("del sid = %s"), util::stringToCString(sid));
					groupMemeberList.erase(itUser);
					break;
				}
			}
		}
		if (0 != imGroupChangeMemberNotify.chg_user_id_list_size())
		{
			if (bExistMySelf)
			{
				module::getSessionModule()->deleteSessionEntity(groupID);
				module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_MYSELF_KICKED, groupID);
				return;//通知UI自己被踢出了群
			}
			else
			{
				//保存群信息到sqllite
				module::getDatabaseModule()->sqlInsertOrReplaceGroupInfoEntity(m_mapGroups[groupID]);
			}
		}
	}

	if (0 != imGroupChangeMemberNotify.chg_user_id_list_size())
	{
		module::getGroupListModule()->asynNotifyObserver(module::KEY_GROUPLIST_UPDATE_MEMBER_CHANGED, groupID);//通知UI更新
	}
}


void GroupListModule_Impl::GetSearchGroupNameListByShortName(IN const CString& sShortName, OUT module::GroupVec & gidList)
{
	CString strUpName = sShortName;
	strUpName.MakeLower();
	for (auto& kvp : m_mapGroups)
	{
		module::GroupInfoEntity& groupInfo = kvp.second;
		CString RealName = groupInfo.csName;

		if (util::isIncludeChinese(util::cStringToString(sShortName, CP_ACP)))//检索中文
		{
			if (RealName.Find(sShortName) != -1)
			{
				gidList.push_back(groupInfo.gId);
			}
		}
		else
		{
			CString firstPY = util::HZ2FirstPY(util::cStringToString(groupInfo.csName, CP_ACP));
			if (firstPY.Find(strUpName) != -1)//先检索简拼
			{
				gidList.push_back(groupInfo.gId);
			}
			else
			{
				CString allPY = util::HZ2AllPY(groupInfo.csName);//再检索全拼
				if (allPY.Find(strUpName) != -1)
				{
					gidList.push_back(groupInfo.gId);
				}
			}
		}
	}
}

void GroupListModule_Impl::_downloadAllGroupAvatarImg()
{
	for (auto& kv : m_mapGroups)
	{
		module::GroupInfoEntity& group = kv.second;
		if (!group.avatarUrl.empty() && group.avatarUrl.find("avatar_group_default.jpg") == std::string::npos)
		{
			UInt32 hashcode = util::hash_BKDR((group.avatarUrl + AVATAR_FORMAT_48X48).c_str());
			module::ImImageEntity imageEntity;
			module::getDatabaseModule()->sqlGetImImageEntityByHashcode(hashcode, imageEntity);
			CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(imageEntity.filename);
			if (!imageEntity.filename.empty()&& PathFileExists(csLocalPath))
			{
				//存在则直接赋值
				group.avatarLocalPath = imageEntity.filename;
			}
			else
			{
				//不存在则去服务器下载
				DownloadAvatarHttpOperation* pOper = new DownloadAvatarHttpOperation(group.gId, group.avatarUrl, TRUE
					, AVATAR_FORMAT_48X48, BIND_CALLBACK_1(GroupListModule_Impl::onCallbackOperation));
				module::getHttpPoolModule()->pushHttpOperation(pOper);
			}
		}
	}
}

void GroupListModule_Impl::onCallbackOperation(std::shared_ptr<void> param)
{
	DownloadImgParam* pDownParam = (DownloadImgParam*)param.get();
	if (DownloadImgParam::DOWNLOADIMG_OK == pDownParam->m_result)
	{
		module::GroupInfoMap::iterator groupIter = m_mapGroups.find(pDownParam->m_sId);
		if (groupIter != m_mapGroups.end())
		{
			groupIter->second.avatarLocalPath = pDownParam->m_imgEntity.filename;
			//提示UI更新
			std::tuple<std::string, std::string>* pTuple = new std::tuple<std::string, std::string>;
			*pTuple = std::tie(module::MODULE_GROUPLIST_PREFIX, pDownParam->m_sId);
			module::getUserListModule()->asynNotifyObserver(module::KEY_USERLIST_DOWNAVATAR_SUCC, shared_ptr<void>(pTuple));
		}
	}
}

void GroupListModule_Impl::_creatAndSaveSessionToDB(IN module::GroupInfoEntity& groupInfo)
{
	module::SessionEntity recentSessionInfo;
	if (!module::getSessionModule()->getSessionEntityBySId(groupInfo.gId, recentSessionInfo))
	{
		//创建session，并保存到db
		recentSessionInfo.sessionID = groupInfo.gId;
		recentSessionInfo.sessionType = IM::BaseDefine::SessionType::SESSION_TYPE_GROUP;
		recentSessionInfo.updatedTime = static_cast<UInt32>(time(0));
		recentSessionInfo.latestmsgId = 0;
		recentSessionInfo.latestMsgContent = std::string("");
		recentSessionInfo.latestMsgFromId = std::string("");
	}
	module::getSessionModule()->setSessionEntity(recentSessionInfo);
	module::getDatabaseModule()->sqlInsertRecentSessionInfoEntity(recentSessionInfo);
}

void GroupListModule_Impl::_safePushBack(IN const std::string& Id, OUT std::list<std::string>& IdList)
{
	auto iterID = std::find_if(IdList.begin(), IdList.end(),
		[=](std::string id)
	{
		return id == Id;
	});
	if (iterID == IdList.end())
	{
		IdList.push_back(Id);
	}
}

BOOL GroupListModule_Impl::deleteGroupInfoById(IN const std::string& sGroupId)
{
	//删除内存数据
	CAutoLock lock(&m_lock);
	module::GroupInfoMap::iterator it = m_mapGroups.find(sGroupId);
	if (it != m_mapGroups.end())
	{
		m_mapGroups.erase(it);
		//删除数据库数据
		return module::getDatabaseModule()->sqlDeleteGroupInfoEntity(sGroupId);
	}
	return FALSE;
}



/******************************************************************************/