/******************************************************************************* 
 *  @file      SessionLayout_Event.cpp 2014\8\15 13:03:04 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/IEmotionModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IP2PCmdModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "../SessionManager.h"
#include "../../Message/SendMsgManage.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../Operation/SendImgHttpOperation.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"
#include "UIIMEdit.h"
#include "../../3rdParty/src/cxImage/cxImage/ximage.h"

#define  TIMER_STOP_WRITING  1
#define  TIMER_WRITING_NOTIFY 2


/******************************************************************************/

void SessionLayout::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0 )
	{
		OnWindowInitialized(msg);
	}
	else if (msg.sType == DUI_MSGTYPE_TIMER)
	{
		if (INIT_FOCUS_TIMER_ID == msg.wParam)
		{
			if (m_pInputRichEdit)
			{
				m_pInputRichEdit->SetFocus();
			}
			m_pManager->KillTimer(this);
		}
	}
	else if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == m_pBtnSendMsg)
		{
			SendMsg();
		}
		else if (m_pBtnClose == msg.pSender)
		{
		}
		else if (msg.pSender == m_pBtnEmotion)
		{
			//表情先不展示
			POINT pt = { 0 };
			CDuiRect rcEmotionBtn = msg.pSender->GetPos();
			CDuiRect rcWindow;
			GetWindowRect(m_pManager->GetPaintWindow(), &rcWindow);

			pt.y = rcWindow.top + rcEmotionBtn.top;
			pt.x = rcWindow.left + rcEmotionBtn.left;

			module::getEmotionModule()->showEmotionDialog(m_sId,pt);
		}
		else if (msg.pSender == m_pBtnSendImage)
		{
			CFileDialog	fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
				, _T("图片 文件|*.png;*.jpeg;*.jpg;*.gif;*.bmp||"), AfxGetMainWnd());
			fileDlg.m_ofn.Flags |= OFN_NOCHANGEDIR;
			fileDlg.DoModal();

			CString sPathName;
			POSITION nPos = fileDlg.GetStartPosition();
			if (nPos != NULL)
			{
				sPathName = fileDlg.GetNextPathName(nPos);
			}
			if (sPathName.IsEmpty())
				return;
			_SendImage(sPathName);
		}
		else if (msg.pSender == m_pBtnbanGroupMsg)
		{
			module::getGroupListModule()->tcpShieldGroup(m_sId, FALSE);
		}
		else if (msg.pSender == m_pBtndisplayGroupMsg)
		{
			module::getGroupListModule()->tcpShieldGroup(m_sId, TRUE);
		}
		else if (msg.pSender == m_pBtnshock)
		{
			module::UserInfoEntity userInfo;
			if (module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
			{
				if (userInfo.onlineState != IM::BaseDefine::USER_STATUS_ONLINE)
				{
					_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_OFFLINE_TIP"));
					return;
				}
			}
			else
			{
				return;
			}
			DWORD CurTime = GetTickCount();
			if (CurTime - m_tShakeWindow < 5000)
			{
				_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_TOOMAMY_TIP"));
				return;
			}
			m_tShakeWindow = CurTime;
			_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_SEND_TIP"));
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG, m_sId);
			module::getP2PCmdModule()->tcpSendShakeWindowCMD(m_sId);
		}
        else if (msg.pSender == m_pBtnScreenShot)
        {
            //直接模拟截屏快捷键
            for (HWND hWnd = GetTopWindow(NULL); hWnd != NULL; hWnd = GetWindow(hWnd, GW_HWNDNEXT))
            {
                wchar_t szWndName[MAX_PATH] = { 0 };
                GetClassName(hWnd, szWndName, MAX_PATH);
                if (_wcsicmp(szWndName, L"TeamTalkMainDialog"))
                {
                    continue;
                }

                DWORD dwProcessId = 0;
                GetWindowThreadProcessId(hWnd, &dwProcessId);
                if (dwProcessId != GetCurrentProcessId())
                {
                    continue;
                }

                ::PostMessage(hWnd, WM_HOTKEY, 0, (LPARAM)MAKELPARAM(MOD_CONTROL | MOD_SHIFT, 0x51));
                break;
            }

        }
		else if (msg.pSender == m_pBtnadduser)
		{
			if (m_bGroupSession)
			{//讨论组修改成员
				module::getGroupListModule()->onChangeDiscussionGrpMemberDialog(m_sId);
			}
			else//创建群
				module::getGroupListModule()->onCreateDiscussionGrpDialog(m_sId);
		}   
        else if (msg.pSender == m_pBtnsendfile) //文件传输
        {
            module::UserInfoEntity userInfo;
            if (!module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
            {
                LOG__(ERR, _T("SendFile can't find the sid"));
                return;
            }

            CFileDialog	fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
                , _T("文件|*.*||"), AfxGetMainWnd());
            fileDlg.m_ofn.Flags |= OFN_NOCHANGEDIR;
            fileDlg.DoModal();

            CString sPathName;
            POSITION nPos = fileDlg.GetStartPosition();
            if (nPos != NULL)
            {
                sPathName = fileDlg.GetNextPathName(nPos);
            }
            if (sPathName.IsEmpty())
                return;

            module::getFileTransferModule()->sendFile(sPathName, m_sId, userInfo.isOnlne());
        }
	}
	else if (msg.sType == _T("return"))
	{
		if (msg.pSender == m_pInputRichEdit)
		{
			SendMsg();
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE))
	{
		if (msg.pSender->GetName() == _T("ListGroupMembersItem"))
		{
			CListContainerElementUI* pListElement = static_cast<CListContainerElementUI*>(msg.pSender);
			if (!pListElement->GetUserData().IsEmpty())
			{
				std::string sid = util::cStringToString(CString(pListElement->GetUserData()));
				module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, sid);//通知主窗口创建会话
			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, UIIMEdit_MSGTYPE_TEXTCHANGED))
	{
		if (msg.pSender->GetName() == _T("input_richedit"))
		{
			if (!m_bGroupSession && !m_bWritingTimerExist)
			{
				//LOG__(DEBG, _T("UIIMEdit_MSGTYPE_TEXTCHANGED"));
				m_pManager->SetTimer(this, TIMER_WRITING_NOTIFY, 3000);
				m_pManager->KillTimer(this, TIMER_STOP_WRITING);
				m_pManager->SetTimer(this, TIMER_STOP_WRITING, 5000);
			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_KILLFOCUS))
	{
		if (!m_bGroupSession&& m_bWritingTimerExist)
		{
			m_pManager->KillTimer(this, TIMER_WRITING_NOTIFY);
			m_pManager->KillTimer(this, TIMER_STOP_WRITING);
			m_bWritingTimerExist = FALSE;
			module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, FALSE);
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_TEXTCHANGED))
	{
		if (msg.pSender == m_pSearchEdit)
		{
			m_pSearchResultList->RemoveAll();
			CDuiString inputText = m_pSearchEdit->GetText();
			if (inputText.IsEmpty())
			{
				m_pSearchResultList->SetVisible(false);
				m_pGroupMemberList->SetVisible(true);
			}
			else
			{
				m_pGroupMemberList->SetVisible(false);
				m_pSearchResultList->SetVisible(true);

				module::UserInfoEntityVec userList;
				_GetGroupNameListByShortName(inputText.GetData(), userList);
				_UpdateSearchRsultList(userList);
			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_MENU))
	{
		_CreateMenu(msg);
	}
}
void SessionLayout::_CreateMenu(IN const TNotifyUI& msg)
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
		LOG__(DEBG, _T("group item don't have menu! sid = %s"), csId.GetData());
		return;
	}
	CMenuWnd* pMenu = new CMenuWnd(m_pManager->GetPaintWindow());
	pMenu->SetUserData(csId);
	DuiLib::CPoint point = msg.ptMouse;
	ClientToScreen(m_pManager->GetPaintWindow(), &point);
	STRINGorID xml(_T("menu\\userListMenu.xml"));
	pMenu->Init(NULL, xml, _T("xml"), point);
}


void SessionLayout::_GetGroupNameListByShortName(IN const CString& sShortName, OUT std::vector<string>& nameList)
{
	for (int n = 0; n < m_pGroupMemberList->GetCount();++n)
	{
		CListContainerElementUI* pListElement = static_cast<CListContainerElementUI*>(m_pGroupMemberList->GetItemAt(n));
		if (pListElement)
		{
			CLabelUI* pNameLable = static_cast<CLabelUI*>(pListElement->FindSubControl(_T("nickname")));
			if (!pNameLable)
			{
				continue;;
			}
			CString Name = pNameLable->GetText();
			std::string sid = util::cStringToString(CString(pListElement->GetUserData()));

			if (util::isIncludeChinese(util::cStringToString(sShortName, CP_ACP)))//检索中文
			{
				if (Name.Find(sShortName) != -1)
				{
					nameList.push_back(sid);
				}
			}
			else//检索字母
			{
				CString firstPY = util::HZ2FirstPY(util::cStringToString(Name, CP_ACP));
				if (firstPY.Find(sShortName) != -1)//先检索简拼
				{
					nameList.push_back(sid);
				}
				else
				{
					CString allPY = util::HZ2AllPY(Name);//再检索全拼
					if (allPY.Find(sShortName) != -1)
					{
						nameList.push_back(sid);
					}
				}
			}
		}
	}
}

void SessionLayout::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_TIMER  )
	{
		if (event.pSender == this && !m_bGroupSession)
		{
			if (TIMER_STOP_WRITING == event.wParam)
			{
				//LOG__(DEBG, _T("timer:TIMER_STOP_WRITING"));
				m_pManager->KillTimer(this, TIMER_WRITING_NOTIFY);
				m_pManager->KillTimer(this, TIMER_STOP_WRITING);
				m_bWritingTimerExist = FALSE;
				module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, FALSE);
			}
			else if (TIMER_WRITING_NOTIFY == event.wParam)
			{
				//LOG__(DEBG, _T("timer:TIMER_WRITING_NOTIFY"));
				module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, TRUE);
			}
		}
	}
	else if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		//TODO 滚动条消息，发送给IE消息展示
		//PTR_VOID(m_pWebBrowser);
		//HWND hwnd = m_pWebBrowser->GetHostWindow();
		//PTR_VOID(hwnd);
		//PostMessage(hwnd,WM_MOUSEHWHEEL,event.wParam,event.lParam);
	}
	else if (event.Type == UIEVENT_CONTEXTMENU)
	{
		
	}
	return __super::DoEvent(event);
}

void SessionLayout::DocmentComplete(IDispatch *pDisp, VARIANT *&url)
{
    if (!m_bDocumentReady)
    {
        m_bDocumentReady = TRUE;
        _LoadFirstOpenedMsg();
    }
}

HRESULT STDMETHODCALLTYPE SessionLayout::TranslateUrl( /* [in] */ DWORD dwTranslate, /* [in] */ OLECHAR __RPC_FAR *pchURLIn, /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	m_csTobeTranslateUrl = pchURLIn;
	*ppchURLOut = 0;

	if (!IsWindow(m_pManager->GetPaintWindow()))
	{
		return S_OK;
	}
	//历史消息内容
	CString csUrl = pchURLIn;
	if (csUrl.Find(_T("moguim/:history")) > -1)//显示历史消息
	{
		_DisplayHistoryMsgToIE(FETCH_MSG_COUNT_PERTIME, FALSE);
	}
	else if (csUrl.Find(_T("moguim/:playvoice")) > -1)//播放语音文件
	{
		int npos = csUrl.Find(_T("?"));
		if (-1 != npos)
		{
			//先停掉前面一个的播放动画
			AudioMessageMananger::getInstance()->popPlayingAudioMsg();

			//播放当前选择的声音
			string sAudioID = util::cStringToString(csUrl.Mid(npos + 1, csUrl.GetLength() - npos));
			AudioMessageMananger::getInstance()->playAudioMsgByAudioSid(m_sId, sAudioID);

			//由于播放同一个文件两次，在第二次的时候，gif动画就不出来了，这个时候需要手动再调一下
			StartPlayingAnimate(sAudioID);
		}
	}
	else if (csUrl.Find(_T("moguim/:chat2")) > -1)//点击了联系某个人
	{
		int npos = csUrl.Find(_T("?"));
		if (-1 != npos)
		{
			string sUesrID = util::cStringToString(csUrl.Mid(npos + 1, csUrl.GetLength() - npos));
			if (!sUesrID.empty()
				&& sUesrID != module::getSysConfigModule()->userID())//不能和自己联系
			{
				//创建会话窗口
				module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, sUesrID);//通知主窗口创建会话
			}
		}
	}
    else if (csUrl.Find(_T("moguim/:documentcompleted")) > -1)
    {
        if (!m_bDocumentReady)
        {
            m_bDocumentReady = TRUE;
            _LoadFirstOpenedMsg();
        }
    }

	return S_OK;
}

void SessionLayout::NewWindow2(VARIANT_BOOL *&Cancel, BSTR bstrUrl)
{
	*Cancel = VARIANT_TRUE;
	if (m_csTobeTranslateUrl.Find(_T("moguim/:history")) > -1
		|| m_csTobeTranslateUrl.Find(_T("moguim/:chat2")) > -1
		|| m_csTobeTranslateUrl.Find(_T("moguim/:playvoice"))>-1
        || m_csTobeTranslateUrl.Find(_T("moguim/:documentcompleted")) > -1)
		return;
	module::getMiscModule()->asynOpenWebBrowser(m_csTobeTranslateUrl);
}
void SessionLayout::MKOForGroupModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (keyId == module::KEY_GROUPLIST_UPDATE_SHIELD_SUCCEED)//更新群屏蔽按钮
	{
		std::string sGroupId = std::get<MKO_STRING>(mkoParam);
		if (sGroupId != m_sId)
		{
			return;
		}
		module::GroupInfoEntity groupInfo;
		if (module::getGroupListModule()->getGroupInfoBySId(sGroupId, groupInfo)
			&& m_pBtnbanGroupMsg&&m_pBtndisplayGroupMsg)
		{
			bool bShow = false;
			groupInfo.shieldStatus ? bShow = true : bShow = false;
			m_pBtnbanGroupMsg->SetVisible(bShow);
			m_pBtndisplayGroupMsg->SetVisible(!bShow);
		}
	}
	else if (keyId == module::KEY_GROUPLIST_UPDATE_MEMBER_CHANGED)//群成员变动
	{
		std::string sGroupId = std::get<MKO_STRING>(mkoParam);
		if (m_sId != sGroupId)
		{
			LOG__(DEBG, _T("not current session!"));
			return;
		}
		_UpdateGroupMembersList();
	}
}

void SessionLayout::MKOForEmotionModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (keyId == module::KEY_EMOTION_SELECTED)
	{
		std::shared_ptr<void> p = std::get<MKO_SHARED_VOIDPTR>(mkoParam);
		module::EmotionParam* pEmotionParam = (module::EmotionParam*) p.get();
		if (pEmotionParam && pEmotionParam->sid == m_sId)
		{
			CString strPath = pEmotionParam->strPath;
			if (m_pInputRichEdit && !strPath.IsEmpty())
			{
				SIZE size = { 0, 0 };			
				m_pInputRichEdit->InsertImage(strPath.GetBuffer(),size,TRUE);
			}
		}
	}
}

BOOL SessionLayout::StopPlayingAnimate(std::string& sAudioPlayingID)
{
	if (sAudioPlayingID.empty())
	{
		return FALSE;
	}

	CString param;
	param = util::stringToCString(sAudioPlayingID);
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("stopVoice"), param.GetBuffer(), &result);
	if (!bRet)
		LOG__(ERR, _T("stopVoice CallJScript failed,%s"), util::stringToCString(sAudioPlayingID));

	return TRUE;
}
BOOL SessionLayout::StartPlayingAnimate(std::string sAudioPlayingID)
{
	if (sAudioPlayingID.empty())
	{
		return FALSE;
	}

	CString param;
	param = util::stringToCString(sAudioPlayingID);
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("playVoice"), param.GetBuffer(), &result);
	if (!bRet)
		LOG__(ERR, _T("playVoice CallJScript failed,%s"), util::stringToCString(sAudioPlayingID));

	return TRUE;
}

void SessionLayout::OnSendImageCallback(std::shared_ptr<void> param)
{
	m_pSendImgHttpOper = 0;
	SendImgParam* pParam = (SendImgParam*)param.get();
	PTR_VOID(pParam);

	if (m_SendingMixedMSGList.empty())
		return;

	for (auto mixedMsgIt = m_SendingMixedMSGList.begin(); mixedMsgIt != m_SendingMixedMSGList.end(); mixedMsgIt++)
	{
		if (mixedMsgIt->m_picDataVec.empty())
		{
			continue;
		}
		for (auto picdata : mixedMsgIt->m_picDataVec)
		{
			if (picdata.strLocalPicPath != pParam->csFilePath)
			{
				continue;
			}
			if (SendImgParam::SENDIMG_OK == pParam->m_result)
			{
				mixedMsgIt->SetNetWorkPicPath(pParam->csFilePath, util::stringToCString(pParam->m_pathUrl));
				if (mixedMsgIt->SucceedToGetAllNetWorkPic())
				{
					MessageEntity msg;
					std::string msgEncrypt;
					ENCRYPT_MSG(util::cStringToString(mixedMsgIt->MakeMixedNetWorkMSG()), msgEncrypt);
					msg.content = msgEncrypt;
					msg.sessionId = m_sId;
					msg.talkerSid = module::getSysConfigModule()->userID();
					msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;

					module::SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
					if (!pSessionInfo)
					{
						return;
					}
					if (pSessionInfo->sessionType == module::SESSION_USERTYPE)
					{
						msg.msgType = MSG_TYPE_TEXT_P2P;
					}
					else
					{
						msg.msgType = MSG_TYPE_TEXT_GROUP;
					}
					msg.msgSessionType = pSessionInfo->sessionType;	//sessionType和FromType定义一致
					msg.msgTime = module::getSessionModule()->getTime();
					SendMsgManage::getInstance()->pushSendingMsg(msg);
					m_SendingMixedMSGList.erase(mixedMsgIt);
					//更新会话时间
					module::SessionEntity*  pSessionEntity = SessionEntityManager::getInstance()->getSessionEntityBySId(msg.sessionId);
					if (pSessionEntity)
					{
						pSessionEntity->updatedTime = msg.msgTime;
					}
					//主界面 消息内容，时间更新
					module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_NEWMSGSEND, msg.sessionId);
				}
			}
			else
			{
				LOG__(ERR, _T("STRID_SESSIONMODULE_MESSAGE_IMAGEFAILED:%d"), pParam->m_result);
				m_SendingMixedMSGList.erase(mixedMsgIt);
				MessageEntity msg;
				msg.content = util::cStringToString(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_MESSAGE_IMAGEFAILED")));
				msg.sessionId = m_sId;
				msg.talkerSid = module::getSysConfigModule()->userID();
				msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
				ReceiveMsgManage::getInstance()->pushMessageBySId(m_sId, msg);
				module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//传图失败
			}
			return;
		}
	}
}

void SessionLayout::OnFinishScreenCapture(__in LPCTSTR lpFilePath)
{
    LOG__(APP, _T("ScreenShot Finish-->%s"), lpFilePath);
    CxImage img;
    img.Load(lpFilePath, CXIMAGE_SUPPORT_BMP);
    HBITMAP hBitmap = img.MakeBitmap();
    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bitmap);
    SIZE bitmapSize = { bitmap.bmWidth, bitmap.bmHeight };

    CString szImgPath(lpFilePath);
    m_pInputRichEdit->InsertImage(szImgPath.GetBuffer(), bitmapSize, FALSE);
}
/******************************************************************************/