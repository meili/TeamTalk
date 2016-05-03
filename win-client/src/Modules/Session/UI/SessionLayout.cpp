/******************************************************************************* 
 *  @file      SessionLayout.cpp 2014\12\31 14:04:38 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include <comutil.h>
#include "json/writer.h"
#include "json/reader.h"
#include "utility/utilStrCodingAPI.h"
#include "utility/Multilingual.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/MessageEntity.h"
#include "Modules/IMiscModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IEmotionModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/IHttpPoolModule.h"
#include "ProtocolBuffer/IM.Message.pb.h"
#include "network/ImCore.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"

/******************************************************************************/

// -----------------------------------------------------------------------------
//  SessionLayout: Public, Constructor

namespace
{	
	int CALLBACK GroupMemberListItemCompareFunc(UINT_PTR pa, UINT_PTR pb, UINT_PTR pUser)
	{
		CListContainerElementUI* pListElement1 = (CListContainerElementUI*)pa;
		CListContainerElementUI* pListElement2 = (CListContainerElementUI*)pb;
		CDuiString node1 = pListElement1->GetUserData();
		CDuiString node2 = pListElement2->GetUserData();

		//获取会话的服务器时间

		CString s1 = node1;
		CString s2 = node2;
		if (s1.IsEmpty() || s2.IsEmpty())
		{
			return 0;
		}
		std::string sid1 = util::cStringToString(s1);
		std::string sid2 = util::cStringToString(s2);

		module::UserInfoEntity userInfo1;
		module::UserInfoEntity userInfo2;
		if (!(module::getUserListModule()->getUserInfoBySId(sid1, userInfo1)
			&& module::getUserListModule()->getUserInfoBySId(sid2, userInfo2)))
		{
			return 0;
		}

		if (userInfo1.onlineState == userInfo2.onlineState)
		{
			return 0;
		}

		int  nRes = userInfo1.onlineState < userInfo2.onlineState ? -1 : 1;
		return nRes;
	}
}
SessionLayout::SessionLayout(const std::string& sId, CPaintManagerUI& paint_manager)
:m_pWebBrowser(nullptr)//聊天显示框
, m_pInputRichEdit(nullptr)
, m_pBtnSendMsg(nullptr)
, m_pBtnClose(nullptr)
, m_pBtnEmotion(nullptr)
, m_pBtnSendImage(nullptr)
, m_pBtnshock(nullptr)
, m_pBtnsendfile(nullptr)
, m_pBtnadduser(nullptr)//添加讨论组成员
, m_sId(sId)
, m_paint_manager(paint_manager)
, m_bGroupSession(false)
, m_pSendDescription(nullptr)
, m_bottomLayout(nullptr)
, m_tShakeWindow(0)
, m_pSearchResultList(nullptr)
, m_pSearchEdit(nullptr)
, m_pBtnbanGroupMsg(nullptr)
, m_pBtndisplayGroupMsg(nullptr)
, m_bWritingTimerExist(FALSE)
{
}

// -----------------------------------------------------------------------------
//  SessionLayout: Public, Destructor

SessionLayout::~SessionLayout()
{
	module::getEmotionModule()->removeObserver(this);
	module::getGroupListModule()->removeObserver(this);
	m_pManager->RemoveNotifier(this);
	if (m_pInputRichEdit)
	{
		m_pInputRichEdit->ReleaseAllGif();
	}
	if (m_pSendImgHttpOper)
	{
		m_pSendImgHttpOper->cancel();
	}
}

void SessionLayout::DoInit()
{
	m_pManager->AddNotifier(this);
}

STDAPI_(void) OleUIMetafilePictIconFree(HGLOBAL hMetaPict)
{
	LPMETAFILEPICT pMF;

	if (NULL == hMetaPict)
		return;

	pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);

	if (NULL != pMF)
	{
		if (NULL != pMF->hMF)
			DeleteMetaFile(pMF->hMF);
	}

	GlobalUnlock(hMetaPict);
	GlobalFree(hMetaPict);
	return;
}

void SessionLayout::OnWindowInitialized(TNotifyUI& msg)
{
	m_pManager->SetTimer(this, INIT_FOCUS_TIMER_ID, 1);

	module::getEmotionModule()->addObserver(this, BIND_CALLBACK_2(SessionLayout::MKOForEmotionModuleCallBack));
	module::getGroupListModule()->addObserver(this, BIND_CALLBACK_2(SessionLayout::MKOForGroupModuleCallBack));

	m_pWebBrowser = dynamic_cast<CWebBrowserUI*>(m_pManager->FindSubControlByName(this, _T("ie")));
	if (m_pWebBrowser) {
		m_pWebBrowser->SetWebBrowserEventHandler(this);
		CString csPath = module::getMiscModule()->getDataDir() + _T("module\\module-content.html");
		m_pWebBrowser->Navigate2(csPath);
	}

	m_pInputRichEdit = dynamic_cast <UIIMEdit*>(m_pManager->FindSubControlByName(this, _T("input_richedit")));
	m_pBtnSendMsg = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("sendbtn")));
	m_pBtnClose = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("closebtn")));

	m_pBtnEmotion = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("emotionbtn")));
	m_pBtnSendImage = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("imagebtn")));
    m_pBtnScreenShot = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("screenshot")));
	m_pBtnshock = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("shockbtn")));
	m_pBtnsendfile = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("sendfilebtn")));
	m_pBtnbanGroupMsg = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("banGroupMsg")));
	m_pBtndisplayGroupMsg = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("displayGroupMsg")));
	m_pBtnadduser = dynamic_cast <CButtonUI*>(m_pManager->FindSubControlByName(this, _T("adduserbtn")));
	m_pSendDescription = dynamic_cast <CTextUI*>(m_pManager->FindSubControlByName(this, _T("senddescription")));
	m_bottomLayout = dynamic_cast<CHorizontalLayoutUI*>(m_pManager->FindSubControlByName(this, _T("bottomLayout")));

	module::SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
	{
		return;
	}
	if (module::SESSION_GROUPTYPE == pSessionInfo->sessionType)
	{
		module::GroupInfoEntity groupInfo;
		if (module::getGroupListModule()->getGroupInfoBySId(m_sId, groupInfo))
		{
			groupInfo.shieldStatus ? m_pBtnbanGroupMsg->SetVisible(true) : m_pBtndisplayGroupMsg->SetVisible(true);
		}
		m_pBtnshock->SetVisible(false);
		m_pBtnsendfile->SetVisible(false);
		m_pBtnadduser->SetToolTip(util::getMultilingual()->getStringById(_T("STRID_GROUPLISTMODULE_DELETEORADDMEMBER")));
		if (groupInfo.type == 1)//固定群
		{
			m_pBtnadduser->SetVisible(false); 
		}		
		m_bGroupSession = TRUE;
		m_pSearchEdit = dynamic_cast<CEditUI*>(m_pManager->FindSubControlByName(this, _T("editSearch")));
		m_pSearchResultList = (CListUI*)m_pManager->FindSubControlByName(this, _T("SearchResult"));
		m_pGroupMemberList = (CListUI*)m_pManager->FindSubControlByName(this, _T("GroupMembers"));
		m_pRightContainer = (CContainerUI*)m_pManager->FindSubControlByName(this, _T("right_part"));
		_UpdateGroupMembersList();
		m_pRightContainer->SetVisible(true);
	}

	UpdateSendMsgShortcut ();
}

void SessionLayout::_AsynSendReadAck(IN MessageEntity& msg)
{
	if (MESSAGE_RENDERTYPE_SYSTEMTIPS == msg.msgRenderType
		|| MESSAGE_RENDERTYPE_FILETRANSFER == msg.msgRenderType)
	{
		LOG__(APP, _T("not send read ack msgRenderType:%d"),msg.msgRenderType);
		return;
	}
	if (MESSAGE_TYPE_HISTORY == msg.msgStatusType
		|| MESSAGE_TYPE_NONE == msg.msgStatusType)
	{
		LOG__(APP, _T("Not runtime or offline msgStatusType:%d,msgId:%d"), msg.msgStatusType,msg.msgId);
		return;
	}
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()mutable
	{
		std::string OriginSessionId = msg.getOriginSessionId();
		IM::Message::IMMsgDataReadAck imMsgDataReadAck;
		imMsgDataReadAck.set_user_id(module::getSysConfigModule()->userId());
		imMsgDataReadAck.set_session_id(util::stringToInt32(OriginSessionId));
		imMsgDataReadAck.set_msg_id(msg.msgId);
		imMsgDataReadAck.set_session_type(static_cast<IM::BaseDefine::SessionType>(msg.msgSessionType));
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::SID_MSG, IM::BaseDefine::CID_MSG_READ_ACK, &imMsgDataReadAck);
    },0, imcore::OPERATION_NAME_MSG_READ_ACK
	);
}

void SessionLayout::UpdateRunTimeMsg()
{
	_DisplayUnreadMsg();
}

void SessionLayout::_UpdateGroupMembersList()
{
	if (!m_pRightContainer
		|| !m_pGroupMemberList)
	{
		return;
	}
	module::GroupInfoEntity groupInfo;
	if (!module::getGroupListModule()->getGroupInfoBySId(m_sId, groupInfo))
	{
		LOG__(ERR, _T("Can't find the group id:%s"),util::stringToCString(m_sId));
		return;
	}
	m_pGroupMemberList->RemoveAll();
	for (std::string sid : groupInfo.groupMemeberList)
	{
		_AddGroupMemberToList(sid,groupInfo.creatorId == sid);
	}
	m_pGroupMemberList->SortItems(GroupMemberListItemCompareFunc, 0);
}

void SessionLayout::_AddGroupMemberToList(IN const std::string& sID, IN const BOOL bCreator)
{
	module::UserInfoEntity userInfo;
	if (!module::getUserListModule()->getUserInfoBySId(sID, userInfo))
	{
		LOG__(ERR, _T("can't find the userInfo:%s,GroupId:%s"),util::stringToCString(sID),util::stringToCString(m_sId));
		return;
	}
	CDialogBuilder dlgBuilder;
	CListContainerElementUI* pListElement = (CListContainerElementUI*)dlgBuilder.Create(_T("SessionDialog\\groupMembersListItem.xml"), (UINT)0, NULL, &m_paint_manager);
	if (!pListElement)
	{
		LOG__(ERR, _T("群item创建失败"));
		return;
	}
	CButtonUI* pLogo = static_cast<CButtonUI*>(pListElement->FindSubControl(_T("AvatarInfo")));
	PTR_VOID(pLogo);
	pLogo->SetBkImage(util::stringToCString(userInfo.getAvatarPath()));

	if (bCreator)
	{
		CButtonUI* pCreator = static_cast<CButtonUI*>(pListElement->FindSubControl(_T("Creator")));
		PTR_VOID(pCreator);
		pCreator->SetVisible(true);
		pCreator->SetUserData(_T("Creator"));
	}

	CLabelUI* pNameLable = static_cast<CLabelUI*>(pListElement->FindSubControl(_T("nickname")));
	PTR_VOID(pNameLable);
	pNameLable->SetText(userInfo.getRealName());
	pListElement->SetUserData(util::stringToCString(userInfo.sId));

	m_pGroupMemberList->Add(pListElement);
}

void SessionLayout::FreshGroupMemberAvatar(IN const std::string& sID)
{
	module::UserInfoEntity userInfo;
	if (!module::getUserListModule()->getUserInfoBySId(sID, userInfo))
	{
		LOG__(ERR, _T("can't find the userInfo:%s,GroupId:%s"),util::stringToCString(sID),util::stringToCString(m_sId));
		return;
	}

	if (!m_pGroupMemberList)
	{
		return;
	}
	BOOL bExist = FALSE;
	for (int n = 0; n < m_pGroupMemberList->GetCount(); n++)
	{
		CListContainerElementUI* pItem = (CListContainerElementUI*)m_pGroupMemberList->GetItemAt(n);
		if (pItem)
		{
			CDuiString userData = pItem->GetUserData();
			if (!userData.IsEmpty())
			{
				std::string sid = util::cStringToString(CString(userData));
				if (sid == sID)
				{
					CButtonUI* btnAvatarInfo = (CButtonUI*)pItem->FindSubControl(_T("AvatarInfo"));
					if (btnAvatarInfo)
					{
						btnAvatarInfo->SetBkImage(util::stringToCString(userInfo.getAvatarPath()));
					}
					bExist = TRUE;
				}
			}
		}
	}
	if (bExist)
	{
		m_pGroupMemberList->SortItems(GroupMemberListItemCompareFunc, 0);
	}
}

void SessionLayout::FreshAllGroupMemberAvatar()
{
	if (!m_pGroupMemberList)
	{
		return;
	}
	for (int n = 0; n < m_pGroupMemberList->GetCount(); n++)
	{
		CListContainerElementUI* pItem = (CListContainerElementUI*)m_pGroupMemberList->GetItemAt(n);
		if (pItem)
		{
			CDuiString userData = pItem->GetUserData();
			if (!userData.IsEmpty())
			{
				std::string sid = util::cStringToString(CString(userData));
				module::UserInfoEntity userInfo;
				if (!module::getUserListModule()->getUserInfoBySId(sid, userInfo))
				{
					LOG__(ERR, _T("can't find the userInfo:%s,GroupId:%s"),util::stringToCString(sid),util::stringToCString(m_sId));
					continue;;
				}
				CButtonUI* btnAvatarInfo = (CButtonUI*)pItem->FindSubControl(_T("AvatarInfo"));
				if (btnAvatarInfo)
				{
					btnAvatarInfo->SetBkImage(util::stringToCString(userInfo.getAvatarPath()));
				}
			}
		}
	}
	m_pGroupMemberList->SortItems(GroupMemberListItemCompareFunc, 0);
}

void SessionLayout::UpdateBottomLayout()
{
	if (m_bottomLayout)
	{
		NeedUpdate();
	}
}

void SessionLayout::_UpdateSearchRsultList(IN const std::vector<std::string>& nameList)
{
	if (nameList.empty())
	{
		return;
	}
	for (std::string sid : nameList)
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(sid, userInfo))
		{
			LOG__(ERR, _T("can't find the userInfo:%s,GroupId: %s"),util::stringToCString(sid),util::stringToCString(m_sId));
			continue;
		}
		CDialogBuilder dlgBuilder;
		CListContainerElementUI* pListElement = (CListContainerElementUI*)dlgBuilder.Create(_T("SessionDialog\\groupMembersListItem.xml"), (UINT)0, NULL, &m_paint_manager);
		if (!pListElement)
		{
			LOG__(ERR, _T("creat group item failed"));
			return;
		}
		CButtonUI* pLogo = static_cast<CButtonUI*>(pListElement->FindSubControl(_T("AvatarInfo")));
		if (!pLogo)
		{
			return;
		}
		pLogo->SetBkImage(util::stringToCString(userInfo.getAvatarPath()));

		CLabelUI* pNameLable = static_cast<CLabelUI*>(pListElement->FindSubControl(_T("nickname")));
		if (!pNameLable)
		{
			return;
		}
		pNameLable->SetText(userInfo.getRealName());
		pListElement->SetUserData(util::stringToCString(userInfo.sId));

		m_pSearchResultList->Add(pListElement);
	}
}

void SessionLayout::_SendImage(CString& csFilePath)
{
	//构造纯图片消息
	ST_picData picData;
	picData.nPos = 0;
	picData.strLocalPicPath = csFilePath;
	MixedMsg mixMsg;
	mixMsg.m_picDataVec.push_back(picData);
	//发送图片
	_SendSessionMsg(mixMsg);

	//本地消息展现
	MessageEntity msg;
	msg.msgType = MSG_TYPE_TEXT_P2P;
	msg.talkerSid = module::getSysConfigModule()->userID();
	msg.sessionId = m_sId;
	msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
	msg.msgStatusType = MESSAGE_TYPE_RUNTIME;
	msg.content = util::cStringToString(mixMsg.MakeMixedLocalMSG());
	msg.msgTime = module::getSessionModule()->getTime();
	_DisplayMsgToIE(msg);
}

UInt32 SessionLayout::GetGroupItemCnt(void)
{
    if (m_pGroupMemberList)
    {
        return m_pGroupMemberList->GetCount();
    }
    return 0;
}

/******************************************************************************/
