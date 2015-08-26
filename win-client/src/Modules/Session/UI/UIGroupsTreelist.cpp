/******************************************************************************* 
 *  @file      UIGroupsTreelist.cpp 2014\8\7 15:45:20 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/IMiscModule.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"
/******************************************************************************/

const int kGroupListFolderItemHeight = 32;
const int kGroupListNormalItemHeight = 60;

const TCHAR* const kLogoButtonControlName = _T("logo");
const TCHAR* const kLogoContainerControlName = _T("logo_container");
const TCHAR* const kNickNameControlName = _T("nickname");
const TCHAR* const kDescriptionControlName = _T("description");
const TCHAR* const kcreatGroupControlName = _T("creatGroup");
const TCHAR* const kOperatorPannelControlName = _T("operation");
const TCHAR* const lastContentTimeControlName = _T("lastContentTime");
const TCHAR* const kUnreadcntControlName = _T("unreadcnt");
const TCHAR* const kXMLFilePath = _T("MainDialog\\groupListItem.xml");


CGroupsTreelistUI::CGroupsTreelistUI(CPaintManagerUI& paint_manager)
:UIIMList(paint_manager)
{
}

static bool OnButtonEvent(void* event) {
	if (((TEventUI*)event)->Type == UIEVENT_BUTTONDOWN) {
		CControlUI* pButton = ((TEventUI*)event)->pSender;
		if (pButton != NULL) {
			CListContainerElementUI* pListElement = (CListContainerElementUI*)(pButton->GetTag());
			if (pButton->GetName() == kcreatGroupControlName)
			{
				//创建讨论组
				std::string sid;
				module::getGroupListModule()->onCreateDiscussionGrpDialog(sid);
			}
			else if (pButton->GetName() == kLogoButtonControlName)
			{
				std::string sID = util::cStringToString(CString(pListElement->GetUserData()));
				if (!sID.empty())
				{
					//show the detail window.
					module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, sID);
				}
			}
			else
			{
				if (pListElement != NULL) pListElement->DoEvent(*(TEventUI*)event);
			}
		
		}
	}
	return true;
}


Node* CGroupsTreelistUI::AddNode(const GroupsListItemInfo& item, Node* parent)
{
	if (!parent)
		parent = root_node_;

	TCHAR szBuf[MAX_PATH] = {0};

    CListContainerElementUI* pListElement = NULL;
	if (!UIIMList::m_dlgBuilder.GetMarkup()->IsValid()) {
		pListElement = static_cast<CListContainerElementUI*>(UIIMList::m_dlgBuilder.Create(kXMLFilePath, (UINT)0, NULL, &paint_manager_));
    }
    else {
		pListElement = static_cast<CListContainerElementUI*>(UIIMList::m_dlgBuilder.Create((UINT)0, &paint_manager_));
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
	node->data().sId = item.id;
	node->data().text_ = item.nickName;
	node->data().list_elment_ = pListElement;

	if (!parent->data().child_visible_)
		pListElement->SetVisible(false);

	if (parent != UIIMList::root_node_ && !parent->data().list_elment_->IsVisible())
		pListElement->SetVisible(false);

	CDuiRect rcPadding = UIIMList::text_padding_;
	for (int i = 0; i < node->data().level_; ++i)
	{
		rcPadding.left += UIIMList::level_text_start_pos_;
	}
	pListElement->SetPadding(rcPadding);

	CControlUI* log_button = static_cast<CControlUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
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
        //log_button->OnEvent += MakeDelegate(&OnButtonEvent);
	}

	CDuiString html_text;
	if (node->data().has_child_)
	{
		if (node->data().child_visible_)
			html_text += UIIMList::level_expand_image_;
		else
			html_text += UIIMList::level_collapse_image_;

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

	CLabelUI* nick_name = static_cast<CLabelUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		if (item.folder)
			nick_name->SetFixedWidth(0);

		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text);
	}

	if (item.folder && item.id == MY_DISCUSSGROUP_ITEMID)
	{
		CButtonUI* btnCreatGroup = static_cast<CButtonUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kcreatGroupControlName));
		if (btnCreatGroup != NULL)
		{
			btnCreatGroup->SetVisible(true);
			btnCreatGroup->SetTag((UINT_PTR)pListElement);
			btnCreatGroup->OnEvent += MakeDelegate(&OnButtonEvent);
		}
	}

	if (!item.folder && !item.description.IsEmpty())
	{
		CLabelUI* description = static_cast<CLabelUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
		if (description != NULL)
		{
			_stprintf_s(szBuf, MAX_PATH - 1, _T("<x 10><c #808080>%s</c>"), item.description);

			description->SetShowHtml(true);
			description->SetText(szBuf);
		}
	}

	if (item.folder)
	{
		pListElement->SetFixedHeight(kGroupListFolderItemHeight);
	}
	else
	{
		pListElement->SetFixedHeight(kGroupListNormalItemHeight);
		pListElement->SetUserData(item.id);
	}
	pListElement->SetTag((UINT_PTR)node);
	int index = 0;
	if (parent->has_children())
	{
		Node* prev = parent->get_last_child();
		index = prev->data().list_elment_->GetIndex() + 1;
	}
	else 
	{
		if (parent == UIIMList::root_node_)
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

Node* CGroupsTreelistUI::UpdateNode(const GroupsListItemInfo& item)
{
	Node* pNode = GetItemBySId(util::cStringToString(CString(item.id)));
	if (nullptr == pNode)
	{
		return nullptr;
	}
	pNode->data().text_ = item.nickName;

	CListContainerElementUI* pListElement = pNode->data().list_elment_;
	if (nullptr == pListElement)
	{
		return nullptr;
	}
	CControlUI* log_button = static_cast<CControlUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
	if (log_button)
	{
		log_button->SetBkImage(item.avatarPath);
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(UIIMList::paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		nick_name->SetText(pNode->data().text_);
	}

	return pNode;
}
BOOL CGroupsTreelistUI::UpdateItemBySId(const std::string& sId)
{
	Node* pNode = GetItemBySId(sId);
	if (nullptr == pNode)
	{
		return FALSE;
	}
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

void CGroupsTreelistUI::ClearItemMsgCount(IN const std::string& sId)
{
	Node* pNode = GetItemBySId(sId);
	if (nullptr == pNode)return;
	CControlUI* pListElement = pNode->data().list_elment_;
	PTR_VOID(pListElement);

	//清除未读计数
	CLabelUI* Unreadcnt_button = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kUnreadcntControlName));
	if (!Unreadcnt_button)
	{
		LOG__(DEBG, _T("can't find the Unreadcnt_button"));
		return;
	}
	Unreadcnt_button->SetText(_T(""));
	Unreadcnt_button->SetVisible(false);
}

BOOL CGroupsTreelistUI::GetFirstChildItemSId(OUT std::string& sId)
{
	if (nullptr == root_node_)
	{
		return FALSE;
	}
	for (int i = 0; i < root_node_->num_children(); ++i)
	{
		Node* child = root_node_->child(i);
		if (child && child->has_children())
		{
			Node*	subRoot = child;
			if (subRoot->has_children())
			{
				child = subRoot->child(0);
				CString strID = child->data().sId;
				sId = util::cStringToString(strID);
				return TRUE;
			}
		}
	}

	return FALSE;
}



/******************************************************************************/
