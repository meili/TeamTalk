/******************************************************************************* 
 *  @file      UIEAUserTreelist.cpp 2014\8\7 15:47:34 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISysConfigModule.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"
/******************************************************************************/
const int kFriendListNormalItemHeight = 60;
const int kFriendListFolderItemHeight = 32;
const TCHAR* const kLogoButtonControlName = _T("logo");
const TCHAR* const kLogoContainerControlName = _T("logo_container");
const TCHAR* const kNickNameControlName = _T("nickname");
const TCHAR* const kDescriptionControlName = _T("description");
const TCHAR* const kOperatorPannelControlName = _T("operation");
const TCHAR* const kUnreadcntControlName = _T("unreadcnt");

CEAUserTreelistUI::CEAUserTreelistUI(CPaintManagerUI& paint_manager)
	:UIIMList(paint_manager)
{
}

static bool OnLogoButtonEvent(void* event) {
    if( ((TEventUI*)event)->Type == UIEVENT_BUTTONDOWN ) {
        CControlUI* pButton = ((TEventUI*)event)->pSender;
        if( pButton != NULL ) {
            CListContainerElementUI* pListElement = (CListContainerElementUI*)(pButton->GetTag());
			if (pListElement != NULL)
			{
				pListElement->DoEvent(*(TEventUI*)event);
				std::string sid = util::cStringToString(CString(pListElement->GetUserData()));
				if (!sid.empty())
				{
					//show the detail window.
					module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, sid);
				}
			}
        }
    }
    return true;
}

Node* CEAUserTreelistUI::AddNode(const EAUserTreeListItemInfo& item, Node* parent)
{
	if (!parent)
		parent = root_node_;

	TCHAR szBuf[MAX_PATH] = {0};

    CListContainerElementUI* pListElement = NULL;
    if( !m_dlgBuilder.GetMarkup()->IsValid() ) {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("MainDialog\\eaUserListItem.xml"), (UINT)0, NULL, &paint_manager_));
    }
    else {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &paint_manager_));
    }
    if (pListElement == NULL)
        return NULL;

	Node* node = new Node;

	node->data().level_ = parent->data().level_ + 1;
	if (item.folder)
		node->data().has_child_ = !item.empty;
	else
		node->data().has_child_ = false;

	node->data().folder_ = item.folder;

	node->data().child_visible_ = (node->data().level_ == 0);
	node->data().child_visible_ = false;

	node->data().text_ = item.nickName;
	node->data().sId = item.id;
	node->data().list_elment_ = pListElement;

	if (!parent->data().child_visible_)
		pListElement->SetVisible(false);

	if (parent != root_node_ && !parent->data().list_elment_->IsVisible())
		pListElement->SetVisible(false);

	CDuiRect rcPadding = text_padding_;
	for (int i = 0; i < node->data().level_; ++i)
	{
		rcPadding.left += level_text_start_pos_;		
	}
	pListElement->SetPadding(rcPadding);

	CControlUI* log_button = static_cast<CControlUI*>(paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
	if (log_button != NULL)
	{
		if (!item.folder && !item.avatarPath.IsEmpty())
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.avatarPath);
			log_button->SetBkImage(szBuf);
		}
		else
		{
			CContainerUI* logo_container = static_cast<CContainerUI*>(paint_manager_.FindSubControlByName(pListElement, kLogoContainerControlName));
			if (logo_container != NULL)
				logo_container->SetVisible(false);
		}
        log_button->SetTag((UINT_PTR)pListElement);
        //log_button->OnEvent += MakeDelegate(&OnLogoButtonEvent);
	}

	CDuiString html_text;
	if (node->data().has_child_)
	{
		if (node->data().child_visible_)
			html_text += level_expand_image_;
		else
			html_text += level_collapse_image_;

		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos_);
		html_text += szBuf;
	}

	if (item.folder)
	{
		html_text += node->data().text_;
	}
	else
	{
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.nickName);
		html_text += szBuf;
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		if (item.folder)
		{
			nick_name->SetPadding(CRect(2,4,0,0));
			nick_name->SetFixedWidth(0);
		}
		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}

	if (!item.folder && !item.description.IsEmpty())
	{
		CLabelUI* description = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
		if (description != NULL)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("<x 1><c #808080>%s</c>"), item.description);
			description->SetShowHtml(true);
			description->SetText(szBuf);
		}
	}
	if (item.folder)
	{
		pListElement->SetFixedHeight(kFriendListFolderItemHeight);
	}
	else
		pListElement->SetFixedHeight(kFriendListNormalItemHeight);
	pListElement->SetTag((UINT_PTR)node);
	pListElement->SetUserData(item.id);
	int index = 0;
	if (parent->has_children())
	{
		Node* prev = parent->get_last_child();
		index = prev->data().list_elment_->GetIndex() + 1;
	}
	else 
	{
		if (parent == root_node_)
			index = 0;
		else
			index = parent->data().list_elment_->GetIndex() + 1;
	}
	if (!CListUI::AddAt(pListElement, index))
	{
		delete pListElement;
		delete node;
		node = NULL;
	}

	parent->add_child(node);
	return node;
}

BOOL CEAUserTreelistUI::UpdateItemBySId(const std::string& sId)
{
	Node* pNode = GetItemBySId(sId);
	if (nullptr == pNode)return FALSE;

	CControlUI* pListElement = pNode->data().list_elment_;
	PTR_FALSE(pListElement);
	if (!SessionDialogManager::getInstance()->findSessionDialogBySId(sId))//窗口不存在的时候更新计数
	{
		//更新未读计数
		CLabelUI* Unreadcnt_button = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kUnreadcntControlName));
		if (!Unreadcnt_button)
		{
			return FALSE;
		}
		UInt32 nCnt = ReceiveMsgManage::getInstance()->getUnReadMsgCountBySId(sId);
        SetTextUICount(Unreadcnt_button, nCnt);
	}

	return TRUE;
}

void CEAUserTreelistUI::ClearItemMsgCount(IN const std::string& sId)
{
	Node* pNode = GetItemBySId(sId);
	if (nullptr == pNode)return;
	CControlUI* pListElement = pNode->data().list_elment_;
	PTR_VOID(pListElement);
	//清除未读计数
	CLabelUI* Unreadcnt_button = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kUnreadcntControlName));
	if (!Unreadcnt_button)
	{
		LOG__(ERR, _T("can't find the Unreadcnt_button"));
		return;
	}
	Unreadcnt_button->SetText(_T(""));
	Unreadcnt_button->SetVisible(false);
}
int CALLBACK IMEAListItemCompareFunc(UINT_PTR pa, UINT_PTR pb, UINT_PTR pUser)
{
	CListContainerElementUI* pListElement1 = (CListContainerElementUI*)pa;
	CListContainerElementUI* pListElement2 = (CListContainerElementUI*)pb;
	Node* node1 = (Node*)pListElement1->GetTag();
	Node* node2 = (Node*)pListElement2->GetTag();

	//获取会话的服务器时间

	CString s1 = node1->data().sId;
	CString s2 = node2->data().sId;
	if (s1.IsEmpty() || s2.IsEmpty())
	{
		return 0;
	}
	std::string sid1 = util::cStringToString(s1);
	std::string sid2 = util::cStringToString(s2);

	module::UserInfoEntity userInfo1;
	module::UserInfoEntity userInfo2;
	if (!module::getUserListModule()->getUserInfoBySId(sid1, userInfo1)
		|| !module::getUserListModule()->getUserInfoBySId(sid2, userInfo2))
	{
		return 0;
	}

	if (userInfo1.onlineState == userInfo2.onlineState)
	{
		return 0;
	}

	int  nRes = userInfo1.onlineState > userInfo2.onlineState ? -1 : 1;
	return nRes;
}
void CEAUserTreelistUI::sortByOnlineState()
{
	SortItems(IMEAListItemCompareFunc, 0);
}
/******************************************************************************/
