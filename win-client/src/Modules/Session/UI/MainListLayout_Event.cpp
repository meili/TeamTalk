/******************************************************************************* 
 *  @file      MainListLayout_Event.cpp 2014\8\11 14:16:22 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/UI/MainListLayout.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/MessageEntity.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/UI/UserDetailInfoDialog.h"
#include "Modules/IDatabaseModule.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../../Message/SendMsgManage.h"
#include "../../FileTransfer/TransferManager.h"
#include "ProtocolBuffer/IM.Message.pb.h"
#include "ProtocolBuffer/IM.File.pb.h"
#include "network/ImCore.h"
#include "network/TTPBHeader.h"
/******************************************************************************/


void MainListLayout::MKOForUserlistModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_USERLIST_UPDATE_DEPARTMENTLIST == keyId)
	{
//#ifndef DEBUG	//TODO 组织架构复杂，会本地调试卡
		_LoadAllDepartment();
		//LOG__(DEBG, _T("_LoadAllDepartment"));
//#endif
	}
	else if (module::KEY_USERLIST_UPDATE_RECENTLIST == keyId)
	{
		//给所有的用户创建会话信息
		module::UserInfoEntityMap mapUserInfos;
		module::getUserListModule()->getAllUsersInfo(mapUserInfos);
		for (auto kv: mapUserInfos)
		{
			module::UserInfoEntity& user = kv.second;
			SessionEntityManager::getInstance()->createSessionEntity(user.sId);
		}

		_AddRecentSessionListToUI();

		//获取会话离线消息
		imcore::IMLibCoreStartOperationWithLambda(
			[]()
		{
			LOG__(APP, _T("IMUnreadMsgCntReq"));
			IM::Message::IMUnreadMsgCntReq imUnreadMsgCntReq;
			imUnreadMsgCntReq.set_user_id(module::getSysConfigModule()->userId());
			module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_MSG
				,IM::BaseDefine::MessageCmdID::CID_MSG_UNREAD_CNT_REQUEST
				,&imUnreadMsgCntReq);
		});

        //获取离线文件 todo
        imcore::IMLibCoreStartOperationWithLambda(
            []()
        {
            LOG__(APP, _T("IMFileHasOfflineReq"));
            IM::File::IMFileHasOfflineReq imFileHasOfflineReq;
            imFileHasOfflineReq.set_user_id(module::getSysConfigModule()->userId());
            module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
                , IM::BaseDefine::FileCmdID::CID_FILE_HAS_OFFLINE_REQ
                , &imFileHasOfflineReq);//获取个人会话离线文件
        });
	}
	else if (module::KEY_USERLIST_UPDATE_NEWUSESADDED == keyId)
	{
		//TODO:新的用户，或者一堆用户更新
	}
}

void MainListLayout::MKOForGrouplistModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_GROUPLIST_UPDATE_GROUPLIST == keyId)
	{
		_AddGroupList();
		_AddDiscussGroupList();
		//给所有的群/讨论组创建会话元信息
		module::GroupInfoMap mapGroupInfo;
		module::getGroupListModule()->getAllGroupListInfo(mapGroupInfo);
		for (auto kv : mapGroupInfo)
		{
			module::GroupInfoEntity& groupInfo = kv.second;
			SessionEntityManager::getInstance()->createSessionEntity(groupInfo.gId);
		}
	}
	else if (module::KEY_GROUPLIST_UPDATE_MYSELF_KICKED == keyId)
	{//自己被踢出了讨论组
		std::string sGroupId = std::get<MKO_STRING>(mkoParam);
		_MySelfKickedFromDiscusGrp(sGroupId);
	}
	else if (module::KEY_GROUPLIST_UPDATE_GROUPINFO_UPDATE == keyId)
	{
		_NewGroupsUpdate(std::get<MKO_SHARED_VOIDPTR>(mkoParam));
	}
	else if (module::KEY_GROUPLIST_UPDATE_CREATNEWGROUP == keyId)//新讨论组创建
	{
		_CreatNewDiscussGroupRes(std::get<MKO_STRING>(mkoParam));
	}
	else if (module::KEY_GROUPLIST_UPDATE_MEMBER_CHANGED == keyId)
	{
		std::string sGroupId = std::get<MKO_STRING>(mkoParam);
		_UpdateGroupList(sGroupId);
	}
}
void MainListLayout::MKOForSessionModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SESSION_NEWMESSAGE == keyId)
	{
		_NewMsgUpdate(std::get<MKO_STRING>(mkoParam));
	}
	else if (module::KEY_SESSION_UPDATE_RECENTLY_SESSIONLIST == keyId)
	{
		std::string sId = std::get<MKO_STRING>(mkoParam);
		if (m_UIRecentConnectedList)
		{
			if (!m_UIRecentConnectedList->IsExistSId(sId))
				m_UIRecentConnectedList->AddNode(sId);
			else
				m_UIRecentConnectedList->UpdateItemConentBySId(sId);
			m_GroupList->UpdateItemBySId(sId);
			m_EAuserTreelist->UpdateItemBySId(sId);
		}
	}
	else if (module::KEY_SESSION_MESSGEREADED_NOTIFY == keyId)
	{
		std::string sessionId = std::get<MKO_STRING>(mkoParam);
		LOG__(APP, _T("The messages are read,sessionId:%s"),util::stringToCString(sessionId));
		if (m_UIRecentConnectedList)
		{
			m_UIRecentConnectedList->UpdateItemConentBySId(sessionId);
			m_GroupList->UpdateItemBySId(sessionId);
			m_EAuserTreelist->UpdateItemBySId(sessionId);
		}
	}
	else if (module::KEY_SESSION_OPENNEWSESSION == keyId)
	{
		std::string sId = std::get<MKO_STRING>(mkoParam);
		if (module::getSysConfigModule()->userID() != sId)
		{
			SessionEntityManager::getInstance()->createSessionEntity(sId);
			SessionDialogManager::getInstance()->openSessionDialog(sId);
			m_UIRecentConnectedList->ClearItemMsgCount(sId);//清除显示的未读计数
			m_GroupList->ClearItemMsgCount(sId);
			m_EAuserTreelist->ClearItemMsgCount(sId);
		}
		else
		{
			LOG__(DEBG, _T("click myself!"));
		}

	}
	else if (module::KEY_SESSION_SENDMSG_TOOFAST == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(sId);
		if (pSessionDialog)
		{
			MessageEntity msg;
			msg.content = util::cStringToString(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_SENDMSG_TOOFAST")));
			msg.sessionId = sId;
			msg.talkerSid = module::getSysConfigModule()->userID(); 
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			ReceiveMsgManage::getInstance()->pushMessageBySId(sId, msg);
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//发送消息太快
		}
	}
	else if (module::KEY_SESSION_SENDMSG_FAILED == keyId)//消息发送失败的提示
	{
		SendingMsgList FailedMsgList;
		SendMsgManage::getInstance()->getSendFailedMsgs(FailedMsgList);
		CString csErrorTipFormat = util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_SENDMSG_FAIL"));
		for (SendingMsg failedmsg:FailedMsgList)
		{
			std::string msgDecrptyCnt;
			DECRYPT_MSG(failedmsg.msg.content, msgDecrptyCnt);//发送消息队列里的全是加密过的
			CString csContent = util::stringToCString(msgDecrptyCnt);
			CString csErrorTip;
			MessageEntity msg;
			csErrorTip.Format(csErrorTipFormat, csContent);
			msg.content = util::cStringToString(csErrorTip);
			msg.sessionId = failedmsg.msg.sessionId;
			msg.talkerSid = module::getSysConfigModule()->userID();
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//发送消息失败
		}
	}
	else if (module::KEY_SESSION_TRAY_NEWMSGSEND == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (!m_UIRecentConnectedList->IsExistSId(sId))
			m_UIRecentConnectedList->AddNode(sId);
		m_UIRecentConnectedList->UpdateItemConentBySId(sId);
		m_GroupList->UpdateItemBySId(sId);
		m_EAuserTreelist->UpdateItemBySId(sId);
		m_UIRecentConnectedList->sort();
	}
	else if (module::KEY_SESSION_TRAY_COPYDATA == keyId)
	{
		int	nParam = std::get<MKO_INT>(mkoParam);
		LOG__(DEBG, _T("Receive KEY_SESSION_TRAY_COPYDATA,nParam:%d"),nParam);
		if (0 == nParam)    //表示结束语音播放
		{
			AudioMessageMananger::getInstance()->autoplayNextUnReadAudioMsg();
		}
		else if (2 == nParam)
		{
			AudioMessageMananger::getInstance()->stopPlayingAnimate();
		}
	}
	else if (module::KEY_SESSION_HISTORY_MESSAGE == keyId)
	{
		std::tuple<std::string, UInt16>* pTuple = (std::tuple<std::string, UInt16>*)std::get<MKO_SHARED_VOIDPTR>(mkoParam).get();		
		std::string& sId = std::get<0>(*pTuple);
		UInt16 reserved = std::get<1>(*pTuple);
		SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(sId);
		if (pSessionDialog)
		{
			SessionMessage_List msgList;
			ReceiveMsgManage::getInstance()->popMessagesBySId(sId, msgList, MESSAGE_TYPE_HISTORY);
			std::vector<MessageEntity> msgVec;
			std::copy(msgList.begin(), msgList.end(), std::back_inserter(msgVec));
			BOOL scrollBottom = (imcore::RESERVED_TYPE_HISTORY_MESSAGE == reserved) ? FALSE : TRUE;
			pSessionDialog->DoDisplayHistoryMsgToIE(msgVec, scrollBottom);

			if (!msgList.empty())
			{
				const MessageEntity& msg = msgList.back();
				module::getMessageModule()->setSessionTopMsgId(msg.sessionId, msg.msgId - 1);
				//serialize to DB
				module::getDatabaseModule()->sqlBatchInsertMessage(msgList);
			}
		}
	}
}
void MainListLayout::MKOForSysConfigModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SYSCONFIG_UPDATED == keyId)
	{
		module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
		if (pTTConfig)
		{
			return;
		}
		if (pTTConfig->sysBaseFlag & module::BASE_FLAG_TOPMOST)
			::SetWindowPos(m_pManager->GetPaintWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		else
			::SetWindowPos(m_pManager->GetPaintWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else if (module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(sId, userInfo))
		{
			LOG__(APP, _T("Only Show user information!"));//TODO 如果是群的话，暂时先打开群会话
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, sId);//通知主窗口创建会话
			return;
		}
		UserDetailInfoDialog* pFloatWnd = new UserDetailInfoDialog(sId);
		if (pFloatWnd == NULL) return;
		CString csTip = util::getMultilingual()->getStringById(_T("STRID_GLOBAL_DETAILINFO"));
		pFloatWnd->Create(m_pManager->GetPaintWindow(), csTip
			,UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW
			//, UI_WNDSTYLE_FRAME | WS_THICKFRAME, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_STATICEDGE
			, 0, 0, 0, 0);
		pFloatWnd->CenterWindow();
		pFloatWnd->ShowWindow(true);
	}
}

void MainListLayout::MKOForFileTransferModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    std::string& sFileId = std::get<MKO_STRING>(mkoParam);
    TransferFileEntity FileInfo;
    if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sFileId, FileInfo))
    {
        LOG__(ERR, _T("can't get the fileInfo:%s"), util::stringToCString(sFileId));
        return;
    }

    if (module::KEY_FILETRANSFER_REQUEST == keyId)
    {
        module::UserInfoEntity userInfo;
        if (!module::getUserListModule()->getUserInfoBySId(FileInfo.sFromID, userInfo))
        {
            LOG__(ERR, _T("can't get the userInfo:%s"), util::stringToCString(FileInfo.sFromID));
            return;
        }
        MessageEntity msg;
        CString csTipFormat = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_SEND_TIP"));
        CString csContent;
        csContent.Format(csTipFormat, userInfo.getRealName(), FileInfo.getRealFileName());
        msg.content = util::cStringToString(csContent);
        msg.sessionId = FileInfo.sFromID;
        msg.talkerSid = module::getSysConfigModule()->userID();
        msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
        ReceiveMsgManage::getInstance()->pushMessageBySId(FileInfo.sFromID, msg);
        module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//给你发送了文件
    }
}
void MainListLayout::_CreatSessionDialog(IN UIIMList* pList, IN CControlUI* pMsgSender)
{
	if (nullptr == pList || pMsgSender == nullptr)
	{
		return;
	}
	if (-1 != pList->GetItemIndex(pMsgSender)
		&& 0 == _tcsicmp(pMsgSender->GetClass(), _T("ListContainerElementUI")))
	{
		Node* node = (Node*)pMsgSender->GetTag();
		if (!pList->CanExpand(node))
		{
			CString csId = node->data().sId;
			if (csId.IsEmpty())
				return;
			std::string sId = util::cStringToString(csId);
			SessionEntityManager::getInstance()->createSessionEntity(sId);
			SessionDialogManager::getInstance()->openSessionDialog(sId);

			m_UIRecentConnectedList->ClearItemMsgCount(sId);//清除显示的未读计数
			m_GroupList->ClearItemMsgCount(sId);
			m_EAuserTreelist->ClearItemMsgCount(sId);

            //更新总未读计数
            module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT);
			//停止托盘闪烁
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_STOPEMOT);
		}
	}
}
void MainListLayout::_CreateMenu( IN const TNotifyUI& msg)
{
	CListUI* pList = static_cast<CListUI*>(msg.pSender);
	PTR_VOID(pList);
	int nPos = pList->GetCurSel();
	CControlUI* pControl = pList->GetItemAt(nPos);
	PTR_VOID(pControl);
	CDuiString csId = pControl->GetUserData();
	if (csId.IsEmpty())
	{
		return;
	}
	module::GroupInfoEntity groupInfo;
	CString strId = csId;
	if (module::getGroupListModule()->getGroupInfoBySId(util::cStringToString(strId), groupInfo))
	{
		LOG__(DEBG, _T("group item don't have menu! sid = %s"),csId.GetData());
		return;
	}
	CMenuWnd* pMenu = new CMenuWnd(m_pManager->GetPaintWindow());
	pMenu->SetUserData(csId);
	DuiLib::CPoint point = msg.ptMouse;
	ClientToScreen(m_pManager->GetPaintWindow(), &point);
	STRINGorID xml(_T("menu\\userListMenu.xml"));
	pMenu->Init(NULL, xml, _T("xml"), point);
}


void MainListLayout::Notify(TNotifyUI& msg)
{
	if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMCLICK))
	{
		if (_tcsicmp(msg.pSender->GetClass(), _T("ListContainerElementUI")) == 0)
		{
			if (m_Tab)
			{
				if (0 == m_Tab->GetCurSel()
					&& m_EAuserTreelist
					&& - 1 != m_EAuserTreelist->GetItemIndex(msg.pSender))
				{
					Node* node = (Node*)msg.pSender->GetTag();

					if (m_EAuserTreelist->CanExpand(node))
					{
						m_EAuserTreelist->SetChildVisible(node, !node->data().child_visible_);
					}
				}
				else if (1 == m_Tab->GetCurSel()
					&& m_GroupList
					&& -1 != m_GroupList->GetItemIndex(msg.pSender))
				{
					Node* node = (Node*)msg.pSender->GetTag();

					if (m_GroupList->CanExpand(node))
					{
						m_GroupList->SetChildVisible(node, !node->data().child_visible_);
					}
				}

			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED))
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("friendbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 0)
			{
				m_Tab->SelectItem(0);		
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("groupbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 1)
			{
				m_Tab->SelectItem(1);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("RecentlyListbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 2)
			{
				m_Tab->SelectItem(2);
			}
		}
		if (m_UIRecentConnectedList)
		{
			m_UIRecentConnectedList->sort();
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE))
	{
		if (m_Tab->GetCurSel() == 0)
		{
			_CreatSessionDialog(m_EAuserTreelist, msg.pSender);
		}
		else if (1 == m_Tab->GetCurSel())
		{
			_CreatSessionDialog(m_GroupList, msg.pSender);
		}
		else if (m_Tab->GetCurSel() == 2)
		{
			_CreatSessionDialog(m_UIRecentConnectedList, msg.pSender);
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_CLICK))
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("searchbtn")) == 0)
		{

		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_MENU))
	{
		_CreateMenu(msg);
	}
}

void MainListLayout::_MySelfKickedFromDiscusGrp(IN const std::string& groupID)
{
	module::GroupInfoEntity groupInfo;
	if (module::getGroupListModule()->getGroupInfoBySId(groupID, groupInfo))
	{
		Node* pRecentNode = m_UIRecentConnectedList->GetItemBySId(groupID);
		if (pRecentNode)
		{//删除最近联系人中的讨论组
			m_UIRecentConnectedList->Remove(pRecentNode->data().list_elment_);
		}		
		Node* pGroupNode = m_GroupList->GetItemBySId(groupID);
		if (pGroupNode)
		{//删除群列表中的讨论组
			m_GroupList->Remove(pGroupNode->data().list_elment_);
		}
		//关闭已经打开的对话框
		SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(groupID);
		if (pSessionDialog)
		{
			pSessionDialog->Close();
		}

		CString text = util::getMultilingual()->getStringById(_T("STRID_MAINLISTLAYOUT_KICKEDOUT_MYSELF"))
			+ groupInfo.csName;
		MessageBox(m_pManager->GetPaintWindow(), text, module::getMiscModule()->getAppTitle(), MB_OK);

		module::getGroupListModule()->deleteGroupInfoById(groupID);
	}
}

/******************************************************************************/