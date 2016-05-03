/******************************************************************************* 
 *  @file      MainListLayout.cpp 2014\8\4 10:15:46 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/UI/MainListLayout.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IDatabaseModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IFileTransferModule.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "utility/Multilingual.h"
#include "ProtocolBuffer/IM.Buddy.pb.h"
#include "network/ImCore.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  TestButton: Public, Constructor
 MainListLayout::MainListLayout()
: m_Tab(nullptr)
,m_EAuserTreelist(nullptr)
,m_GroupList(nullptr)
,m_UIRecentConnectedList(nullptr)
,m_groupRootParent(nullptr)
,m_DiscussGroupRootParent(nullptr)
{
	SetContextMenuUsed(true);
	//MKO
	module::getUserListModule()->addObserver(this, BIND_CALLBACK_2(MainListLayout::MKOForUserlistModuleCallback));
	module::getGroupListModule()->addObserver(this, BIND_CALLBACK_2(MainListLayout::MKOForGrouplistModuleCallback));
	module::getSessionModule()->addObserver(this, BIND_CALLBACK_2(MainListLayout::MKOForSessionModuleCallback));
	module::getSysConfigModule()->addObserver(this, BIND_CALLBACK_2(MainListLayout::MKOForSysConfigModuleCallback));
    module::getFileTransferModule()->addObserver(this, BIND_CALLBACK_2(MainListLayout::MKOForFileTransferModuleCallback));
}

// -----------------------------------------------------------------------------
//  TestButton: Public, Destructor

MainListLayout::~MainListLayout()
{
	module::getUserListModule()->removeObserver(this);
	module::getGroupListModule()->removeObserver(this);
	module::getSessionModule()->removeObserver(this);
	module::getSysConfigModule()->removeObserver(this);
    module::getFileTransferModule()->removeObserver(this);
}

LPVOID MainListLayout::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("ListLayout")) == 0) return static_cast<MainListLayout*>(this);
	return __super::GetInterface(pstrName);
}

LPCTSTR MainListLayout::GetClass() const
{
	return _T("ListLayout");
}

void MainListLayout::DoInit()
{
	m_pManager->AddNotifier(this);

	m_Tab = static_cast<CTabLayoutUI*>(m_pManager->FindSubControlByName(this, _T("tabs")));
	m_EAuserTreelist = static_cast<CEAUserTreelistUI*>(m_pManager->FindSubControlByName(this, _T("friends")));
	m_GroupList = static_cast<CGroupsTreelistUI*>(m_pManager->FindSubControlByName(this, _T("groupsList")));
	m_UIRecentConnectedList = static_cast<CUIRecentSessionList*>(m_pManager->FindSubControlByName(this, _T("recentlyList")));

	//默认选中最近联系人列表
	m_Tab->SelectItem(2);

	//所有业务模块的本地化数据加载
	imcore::IMLibCoreStartOperationWithLambda(
		[]()
	{
		if (!module::getUserListModule()->startup())
		{
			LOG__(ERR, _T("userlist module local db datas start failed!"));
		}
		LOG__(APP, _T("userlist module local db datas start done"));
		if (!module::getGroupListModule()->startup())
		{
			LOG__(ERR, _T("grouplist module local db datas start failed!"));
		}
		LOG__(APP, _T("grouplist module local db datas start done"));
		if (!module::getSessionModule()->startup())
		{
			LOG__(ERR, _T("session module local db datas start failed!"));
		}
		LOG__(APP, _T("session module local db datas start done"));
		//获取最近联系会话
		UInt32 updateTime = module::getSessionModule()->getGlobalUpdateTime();
		LOG__(APP, _T("IMRecentContactSessionReq , local update time = %d"), updateTime);
		IM::Buddy::IMRecentContactSessionReq imRecentContactSessionReq;
		imRecentContactSessionReq.set_user_id(module::getSysConfigModule()->userId());
		imRecentContactSessionReq.set_latest_update_time(updateTime);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
			, IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST
			, &imRecentContactSessionReq);
	}
	);
}

void MainListLayout::_LoadAllDepartment()
{
	m_EAuserTreelist->RemoveAll();
	const module::DepartmentMap mapDeparments
		= module::getUserListModule()->getAllDepartments();
	std::vector<module::DepartmentEntity> vecDepart;//排序好的部门
	for (auto dep : mapDeparments)
	{
		vecDepart.push_back(dep.second);
	}
	std::sort(vecDepart.begin(), vecDepart.end(),
		[=](const module::DepartmentEntity& x, const module::DepartmentEntity& y){
		return x.priority < y.priority;
	});

	for (auto depart : vecDepart)
	{
		EAUserTreeListItemInfo item;
		//item.id = util::stringToCString(depart.dId);//部门信息不用存储id，方便菜单判断
		item.folder = true;
		item.empty = false;
		item.nickName = depart.name;
		Node* root_parent = m_EAuserTreelist->AddNode(item, NULL);
		for (std::string uId : depart.members)
		{
			module::UserInfoEntity user;			
			if (module::getUserListModule()->getUserInfoBySId(uId, user))
			{
				item.id = util::stringToCString(uId);
				item.folder = false;
				item.avatarPath = util::stringToCString(user.getAvatarPath());
				item.nickName = user.getRealName();
				item.description = _T("description...");
				m_EAuserTreelist->AddNode(item, root_parent);
			}
			else
			{
				LOG__(DEBG, _T("can't find the userInfo:%s,GroupID:%s")
					, util::stringToCString(uId), util::stringToCString(depart.dId));
			}
		}
	}
}

void MainListLayout::_AddGroupList()
{
	module::GroupInfoMap Groups;
	module::getGroupListModule()->getAllGroupListInfo(Groups);

	GroupsListItemInfo item;
	Node* rootParent = nullptr;
	item.folder = true;
	item.empty = false;

	UInt32 GroupCnt = 0;
	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (1 == GroupInfo.type)
			GroupCnt++;
	}
	item.id = MY_GROUP_ITEMID;
	item.nickName.Format(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_MAINLISTLAYOUT_GROUP_TIP")), GroupCnt);
	rootParent = m_GroupList->GetItemBySId(util::cStringToString(CString(item.id)));
	if (!rootParent)
		rootParent =m_GroupList->AddNode(item, NULL);

	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (1 == GroupInfo.type)
		{
			if (!m_GroupList->IsExistSId(GroupInfo.gId))
			{
				item.id = util::stringToCString(GroupInfo.gId);
				item.folder = false;
				item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
				item.nickName = GroupInfo.csName;
				item.description = GroupInfo.desc;
				m_GroupList->AddNode(item, rootParent);	
			}
		}
	}
	//TODO 展开群列表
	//if (m_GroupList->CanExpand(rootParent))
	//{
	//	m_GroupList->SetChildVisible(rootParent, true);
	//}
}

void MainListLayout::_AddDiscussGroupList()
{
	module::GroupInfoMap Groups;
	module::getGroupListModule()->getAllGroupListInfo(Groups);

	GroupsListItemInfo item;
	Node* rootParent = nullptr;
	item.folder = true;
	item.empty = false;
	UInt32 GroupCnt = 0;

	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (2 == GroupInfo.type)
			GroupCnt++;
	}
	item.id = MY_DISCUSSGROUP_ITEMID;
	item.nickName.Format(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_MAINLISTLAYOUT_DISCUSS_TIP")), GroupCnt);
	rootParent = m_GroupList->GetItemBySId(util::cStringToString(CString(item.id)));
	if (!rootParent)
		rootParent = m_GroupList->AddNode(item, NULL);
	
	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (2 == GroupInfo.type)
		{
			if (!m_GroupList->IsExistSId(GroupInfo.gId))
			{
				item.id = util::stringToCString(GroupInfo.gId);
				item.folder = false;
				item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
				item.nickName.Format(_T("%s(%d)"), GroupInfo.csName, GroupInfo.groupMemeberList.size());
				item.description = GroupInfo.desc;
				m_GroupList->AddNode(item, rootParent);
			}
		}
	}
	//TODO 展开讨论组列表
	//if (m_GroupList->CanExpand(rootParent))
	//{
	//	m_GroupList->SetChildVisible(rootParent, true);
	//}
}

void MainListLayout::_AddRecentSessionListToUI()
{
	std::vector<std::string> vecRecentSession;
	module::getSessionModule()->getRecentSessionList(vecRecentSession);
	module::UserInfoEntityVec UnKnowUserVec;
	module::GroupVec		  UnKnowGroupVec;
	for (std::string sessionId : vecRecentSession)
	{
		module::SessionEntity sessionEntity;
		if (!module::getSessionModule()->getSessionEntityBySId(sessionId, sessionEntity))
		{
			LOG__(ERR, _T("Can't get the SessionEntity:%s"), util::stringToCString(sessionId));
			continue;
		}
		SessionListItemInfo item;
		item.folder = false;
		item.empty = false;
		item.id = util::stringToCString(sessionEntity.sessionID);
		if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == static_cast<IM::BaseDefine::SessionType>(sessionEntity.sessionType))
		{
			module::GroupInfoEntity groupInfo;
			if (module::getGroupListModule()->getGroupInfoBySId(sessionEntity.sessionID, groupInfo))
			{
				item.avatarPath = util::stringToCString(groupInfo.getAvatarPath());
				item.nickName = groupInfo.csName;
				if (m_UIRecentConnectedList->IsExistSId(sessionEntity.sessionID))
				{
					LOG__(DEBG, _T("IsExistSI:%s"), util::stringToCString(sessionEntity.sessionID));
					m_UIRecentConnectedList->UpdateItemInfo(item);
				
				}
				else
				{
					m_UIRecentConnectedList->AddNode(item, NULL);
				}		
			}
			else//陌生群,走陌生群获取信息的流程
			{
				LOG__(APP, _T("Can't get the GroupInfoEntity,Need to get Unkonw GroupInfo:%s"), util::stringToCString(sessionEntity.sessionID));
				std::string sAvatarPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
				sAvatarPath += "DiscussionGroups.png";
				item.avatarPath = util::stringToCString(sAvatarPath);
				item.nickName = util::stringToCString(sessionId);
				m_UIRecentConnectedList->AddNode(item, NULL);
				UnKnowGroupVec.push_back(sessionId);
			}
		}
		else if (IM::BaseDefine::SessionType::SESSION_TYPE_SINGLE == static_cast<IM::BaseDefine::SessionType>(sessionEntity.sessionType))
		{
			item.description = util::stringToCString(sessionEntity.latestMsgContent);

			module::UserInfoEntity userInfo;
			if (module::getUserListModule()->getUserInfoBySId(sessionEntity.sessionID, userInfo))
			{				
				item.avatarPath = util::stringToCString(userInfo.getAvatarPath());
				item.nickName = userInfo.getRealName();
				if (m_UIRecentConnectedList->IsExistSId(sessionEntity.sessionID))
				{
					LOG__(DEBG, _T("IsExistSI:%s"), util::stringToCString(sessionEntity.sessionID));
					m_UIRecentConnectedList->UpdateItemInfo(item);
				}
				else
				{
					m_UIRecentConnectedList->AddNode(item, NULL);
				}				
			}
			else//陌生人,走陌生人获取信息的流程
			{
				LOG__(APP, _T("Can't get the UserInfoEntity:%s"), util::stringToCString(sessionEntity.sessionID));
				std::string sAvatarPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
				sAvatarPath += "gray_default.png";
				item.avatarPath = util::stringToCString(sAvatarPath);
				item.nickName = util::stringToCString(sessionId);
				m_UIRecentConnectedList->AddNode(item, NULL);
				UnKnowUserVec.push_back(sessionId);
			}			
		}
		m_UIRecentConnectedList->UpdateItemConentBySId(sessionEntity.sessionID);
	}
	//获取未知联系人、群
	module::getGroupListModule()->tcpGetGroupsInfo(UnKnowGroupVec);
	module::getUserListModule()->tcpGetUsersInfo(UnKnowUserVec);
}

void MainListLayout::_GroupMembersChanged(std::shared_ptr<void> pMembers)
{
	std::tuple<UInt32, std::list<std::string>>* pTuple = (std::tuple<UInt32, std::list<std::string>>*)pMembers.get();
	BOOL bChangeType = std::get<0>(*pTuple);
	std::list<std::string>& memberList = std::get<1>(*pTuple);
	//TODO 更新群成员列表
}
void MainListLayout::_NewMsgUpdate(std::string& sId)
{
	module::SessionEntity* psessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(sId);
	if (!psessionInfo)
	{
		LOG__(ERR, _T("can't find the SessionEntity,sessionId = %s"),util::stringToCString(sId));
		return;
	}	

	if (m_UIRecentConnectedList)//更新最近联系人列表
	{
		if (!m_UIRecentConnectedList->IsExistSId(sId))
			m_UIRecentConnectedList->AddNode(sId);
		else
			m_UIRecentConnectedList->UpdateItemConentBySId(sId);
	}

	//会话窗口已经存在，则即时显示消息(离线消息返回的时候，肯定没有窗口)
	SessionDialog* pDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(sId);
	if (pDialog)
	{
		pDialog->UpdateRunTimeMsg();
	}
	else
	{
		MessageEntity msg;
		ReceiveMsgManage::getInstance()->frontMessageBySId(sId, msg);
		if (msg.msgStatusType == MESSAGE_TYPE_RUNTIME)
		{
			//托盘图标闪烁
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_STARTEMOT);
			//飘窗提示
			module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
			if (pTTConfig && pTTConfig->sysBaseFlag & module::BASE_FLAG_NOTIPWHENNEWMSG)
			{
				//todo...
			}
		}
	}
    //更新总未读计数
    module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT);

	//播放声音
	module::getMiscModule()->playSysConfigSound();
}

void MainListLayout::_NewGroupAdded(std::string& gId)
{
	//相当于收到了该群的消息
	_NewMsgUpdate(gId);
}

void MainListLayout::_NewGroupsUpdate(std::shared_ptr<void> pGroupIDs)
{
	if (nullptr == m_UIRecentConnectedList
		|| nullptr == m_GroupList)
	{
		return;
	}
	std::list<std::string>* p = (std::list<std::string>*)pGroupIDs.get();
	for (std::string sGroupID : *p)
	{
		_UpdateGroupList(sGroupID);//更新群

		if (m_UIRecentConnectedList->IsExistSId(sGroupID))//更新最近联系人
		{
			_UpdateRecentSessionItem(sGroupID, module::SESSION_GROUPTYPE);
		}
	}
}

void MainListLayout::_CreatNewDiscussGroupRes(std::string& sId)
{
	module::GroupInfoEntity GroupInfo;
	if (module::getGroupListModule()->getGroupInfoBySId(sId, GroupInfo))
	{
		SessionEntityManager::getInstance()->createSessionEntity(GroupInfo.gId);//创建会话管理，显示的为系统时间

		SessionListItemInfo item;
		item.folder = false;
		item.empty = false;
		item.id = util::stringToCString(GroupInfo.gId);
		item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
		item.nickName = GroupInfo.csName;
		item.Time = GroupInfo.groupUpdated;
		m_UIRecentConnectedList->AddNode(item, NULL);
		m_UIRecentConnectedList->sort();

		GroupsListItemInfo groupItem;
		groupItem.folder = false;
		groupItem.empty = false;
		groupItem.id = util::stringToCString(GroupInfo.gId);
		groupItem.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
		groupItem.nickName.Format(_T("%s(%d)"),GroupInfo.csName, GroupInfo.groupMemeberList.size());

		Node* parent = m_GroupList->GetParentIdForAddNode(MY_DISCUSSGROUP_ITEMID);
		if (parent)
		{
			m_GroupList->AddNode(groupItem, parent);
		}
	}
}

void MainListLayout::_UpdateRecentSessionItem(IN const std::string& sessionId, IN const UInt32 sessionType)
{
	SessionListItemInfo item;
	item.folder = false;
	item.empty = false;
	if (IM::BaseDefine::SessionType::SESSION_TYPE_GROUP == static_cast<IM::BaseDefine::SessionType>(sessionType))
	{
		module::GroupInfoEntity groupInfo;
		if (!module::getGroupListModule()->getGroupInfoBySId(sessionId, groupInfo))
		{
			LOG__(ERR, _T("Can't get the groupInfo, groupId = %s"), util::stringToCString(sessionId));
			return;
		}
		item.id = util::stringToCString(groupInfo.gId);
		item.avatarPath = util::stringToCString(groupInfo.getAvatarPath());
		item.nickName = groupInfo.csName;
		m_UIRecentConnectedList->UpdateItemInfo(item);
	}
	else if (IM::BaseDefine::SessionType::SESSION_TYPE_SINGLE == static_cast<IM::BaseDefine::SessionType>(sessionType))
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(sessionId, userInfo))
		{
			LOG__(ERR, _T("Can't get the userInfo, userId = %s"), util::stringToCString(sessionId));
			return;
		}
		item.id = util::stringToCString(userInfo.sId);
		item.avatarPath = util::stringToCString(userInfo.getAvatarPath());
		item.nickName = userInfo.getRealName();
		m_UIRecentConnectedList->UpdateItemInfo(item);
	}
}


void MainListLayout::_UpdateGroupList(IN const std::string& groupID)
{
	module::GroupInfoEntity groupInfo;
	if (module::getGroupListModule()->getGroupInfoBySId(groupID, groupInfo))
	{
		//更新UI
		GroupsListItemInfo groupItem;
		groupItem.folder = false;
		groupItem.empty = false;
		groupItem.id = util::stringToCString(groupInfo.gId);
		groupItem.avatarPath = util::stringToCString(groupInfo.getAvatarPath());
		groupItem.nickName.Format(_T("%s(%d)"), groupInfo.csName, groupInfo.groupMemeberList.size());

		CDuiString strRootName = MY_GROUP_ITEMID;
		if (IM::BaseDefine::GroupType::GROUP_TYPE_NORMAL == groupInfo.type)
		{
			strRootName = MY_GROUP_ITEMID;
		}
		else if (IM::BaseDefine::GroupType::GROUP_TYPE_TMP == groupInfo.type)
		{
			strRootName = MY_DISCUSSGROUP_ITEMID;
		}
		else
		{
			LOG__(ERR, _T("worng group type"));
		}

		if (m_GroupList->IsExistSId(groupID))
		{	
			LOG__(DEBG, _T("UI Update group Node,name=%s,groupid=%s"), CString(groupItem.nickName), CString(groupItem.id));
			m_GroupList->UpdateNode(groupItem);
		}
		else
		{
			LOG__(DEBG, _T("UI Add Group Node,name=%s,groupid=%s"), CString(groupItem.nickName), CString(groupItem.id));
			Node* parent = m_GroupList->GetParentIdForAddNode(strRootName);
			if (parent)
			{
				m_GroupList->AddNode(groupItem, parent);
			}
		}

	}
}
















/******************************************************************************/