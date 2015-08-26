#include "stdafx.h"
#include "../SessionManager.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IUserListModule.h"
#include "UIIMEdit.h"
#include "Modules/IScreenCaptureModule.h"
#include "Modules/ITcpClientModule.h"
#include "utility/utilStrCodingAPI.h"

const int kEmotionRefreshTimerId = 1001;
const int kEmotionRefreshInterval = 150;

#define  TIMER_CHECK_RECEIVE_WRITING_STATUS 2

DWORD WINAPI  WindowShake(LPVOID lpParam)//窗口抖动
{
	HWND hwnd = (HWND)lpParam;
	RECT rect;
	memset(&rect, 0, sizeof(rect));
	GetWindowRect(hwnd, &rect);
	for (int i = 0; i < 10; i++)
	{
		MoveWindow(hwnd, rect.left - 6, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top - 5, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left + 5, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top + 5, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	}
	return 0;
}


SessionDialog::SessionDialog(const std::string& sId)
: m_pSessionLayout(nullptr)
, m_pWritingStatus(nullptr)
, m_sId(sId)
, m_pBtnAvatar(nullptr)
, m_pTxtName(nullptr)
, m_pBtnMax(nullptr)
, m_pBtnRestore(nullptr)
, m_pEditSignature(nullptr)
{
}

SessionDialog::~SessionDialog()
{
	module::getSessionModule()->removeObserver(this);
	module::getSysConfigModule()->removeObserver(this);
	module::getLoginModule()->removeObserver(this);
	module::getUserListModule()->removeObserver(this);
	module::getTcpClientModule()->removeObserver(this);
    module::getScreenCaptureModule()->removeObserver(this);
}

LPCTSTR SessionDialog::GetWindowClassName() const
{
	return _T("SessionDialog");
}

CControlUI* SessionDialog::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("SessionLayout")) == 0)
	{
		m_pSessionLayout = new SessionLayout(m_sId,m_PaintManager);
		return m_pSessionLayout;
	}
	if (_tcsicmp(pstrClass, _T("UIIMEdit")) == 0)
	{
		return new UIIMEdit();
	}
	return NULL;
}

void SessionDialog::OnFinalMessage(HWND hWnd)
{
	module::SessionEntity sessEntity;
	if (module::getSessionModule()->getSessionEntityBySId(m_sId, sessEntity))//重置最近会话的消息id
	{	
		//窗口关闭了，top item Id自然就要清零了
		module::getMessageModule()->setSessionTopMsgId(m_sId, 0);
	}
	else
		LOGA__(ERR, "getSessionEntityBySId failed,sessionId:%s", util::stringToCString(m_sId));

	SessionDialogManager::getInstance()->removeSessionDialogPtr(m_sId);
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString SessionDialog::GetSkinFile()
{
	return _T("SessionDialog\\SessionDialog.xml");
}

CDuiString SessionDialog::GetSkinFolder()
{
	return _T("");
}

LRESULT SessionDialog::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		Close(IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

void SessionDialog::InitWindow()
{
	module::SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
		return;
	//群的话，从新计算大小
	if (module::SESSION_GROUPTYPE == pSessionInfo->sessionType)
	{
		ResizeClient(720,540);
	}
}

void SessionDialog::OnWindowInitialized(TNotifyUI& msg)
{
	module::SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
		return;

	m_pBtnAvatar = (CButtonUI*)m_PaintManager.FindControl(_T("UserAvatar"));
	PTR_VOID(m_pBtnAvatar);
	m_pBtnAvatar->SetBkImage(util::stringToCString(pSessionInfo->getAvatarPath()));

    //群的话，获取下人数
    if (module::SESSION_GROUPTYPE == pSessionInfo->sessionType)
    {
        if (m_pSessionLayout)
        {
            m_pBtnAvatar->SetToolTip(util::int32ToCString(m_pSessionLayout->GetGroupItemCnt()) + _T("人"));
        }
    }

	m_pBtnMax = (CButtonUI*)m_PaintManager.FindControl(_T("maxbtn"));
	PTR_VOID(m_pBtnMax);
	m_pBtnRestore = (CButtonUI*)m_PaintManager.FindControl(_T("restorebtn"));
	PTR_VOID(m_pBtnRestore);

	m_pTxtName = (CTextUI*)m_PaintManager.FindControl(_T("username"));
	PTR_VOID(m_pTxtName);
	m_pTxtName->SetText(pSessionInfo->getName());
	m_pTxtName->SetToolTip(pSessionInfo->getName());

	m_pWritingStatus = (CTextUI*)m_PaintManager.FindControl(_T("writingStatus"));
	PTR_VOID(m_pWritingStatus);

    m_pEditSignature = (CEditUI*)m_PaintManager.FindControl(_T("userSignature"));
    PTR_VOID(m_pEditSignature);

    _FreshSignature();//更新头像

	module::getSessionModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForSessionModuleCallback));
	module::getSysConfigModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForSysConfigModuleCallback));
	module::getLoginModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForLoginModuleCallback));
	module::getUserListModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForUserListModuleCallback));
	module::getTcpClientModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForTcpClientModuleCallback));
    module::getScreenCaptureModule()->addObserver(this, BIND_CALLBACK_2(SessionDialog::MKOForScreenCaptureModuleCallback));
}

void SessionDialog::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		OnWindowInitialized(msg);
	}
	else if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == m_pBtnAvatar)//点击了会话的头像
		{
			module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG,m_sId);
		}
	}
	__super::Notify(msg);
}

void SessionDialog::UpdateRunTimeMsg(void)
{
	if (m_pSessionLayout)
	{
		m_pSessionLayout->UpdateRunTimeMsg();
	}
}

void SessionDialog::MKOForSessionModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SESSION_NEWMESSAGE == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);

		//任务栏消息提示动画
		if (m_sId == sId && ::GetForegroundWindow() != m_hWnd)
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize = sizeof(FLASHWINFO);
			flashInfo.dwFlags = FLASHW_TRAY;
			flashInfo.dwTimeout = 0;
			flashInfo.uCount = 2;
			flashInfo.hwnd = m_hWnd;
			::FlashWindowEx(&flashInfo);
		}
	}
	else if (module::KEY_SESSION_WRITING_MSG == keyId)	//对方正在输入
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (m_sId == sId)
		{
			m_pWritingStatus->SetVisible(true);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
			SetTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS, 6000, NULL);
		}
	}
	else if (module::KEY_SESSION_STOPWRITING_MSG == keyId)//对方停止输入
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (m_sId == sId)
		{
			m_pWritingStatus->SetVisible(false);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
		}
	}
	else if (module::KEY_SESSION_SHAKEWINDOW_MSG == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (sId == m_sId)
		{
			_ShakeWindow();
		}
	}
}
void SessionDialog::MKOForSysConfigModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SYSCONFIG_UPDATED == keyId)
	{
		if (m_pSessionLayout)
		{
			m_pSessionLayout->UpdateSendMsgShortcut ();
		}
	}
}
void SessionDialog::_ShakeWindow()
{
	BringToTop();
	if (IsWindow(m_hWnd))
	{
		HANDLE hThread = CreateThread(NULL, 0, WindowShake, m_hWnd, 0, NULL);//窗口抖动
		CloseHandle(hThread);
	}
}

LRESULT SessionDialog::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (WM_TIMER == uMsg)
	{
		if (wParam == TIMER_CHECK_RECEIVE_WRITING_STATUS)
		{
			m_pWritingStatus->SetVisible(false);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
		}
	}
	else if (WM_MENU_NOTIFY == uMsg)
	{
		LPCTSTR pName = (LPCTSTR)wParam;
		LPCTSTR pSid = (LPCTSTR)lParam;
		if (pName)
		{
			//LOG__(APP, _T("WM_MENU_NOTIFY:%s"),pName);
			OnMenuClicked(CString(pName), CString(pSid));
		}
		return 0;
	}
	return __super::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}
void SessionDialog::OnMenuClicked(IN const CString& itemName, IN const CString& strLparam)
{
	if (_T("UserDetailInfo") == itemName)
	{
		if (strLparam.IsEmpty())
		{
			return;
		}
		std::string sid = util::cStringToString(strLparam);
		module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, sid);
	}
}

void SessionDialog::MKOForTcpClientModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_TCPCLIENT_STATE == keyId)
	{
		_FreshAvatar();
		m_pSessionLayout->FreshAllGroupMemberAvatar();
	}
}

void SessionDialog::MKOForLoginModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_LOGIN_RELOGINOK == keyId)
	{
		_FreshAvatar();
		m_pSessionLayout->FreshAllGroupMemberAvatar();
	}
}

void SessionDialog::_FreshAvatar()
{
	//刷新个人在线状态，群不刷新
	module::SessionEntity* pSession = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (pSession && module::SESSION_USERTYPE == pSession->sessionType)
	{
		m_pBtnAvatar->SetBkImage(util::stringToCString(pSession->getAvatarPath()));
	}
}

void SessionDialog::MKOForUserListModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_USERLIST_USERLINESTATE == keyId)
	{
		//刷新个人在线状态，群不刷新
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (sId == m_sId)
		{
			_FreshAvatar();
		}
		else
		{
			m_pSessionLayout->FreshGroupMemberAvatar(sId);
		}
	}
    else if (module::KEY_USERLIST_USERSIGNINFO_CHANGED == keyId)
    {
        //更新个性签名
        std::string& sId = std::get<MKO_STRING>(mkoParam);
        if (sId != m_sId)
        {
            return;
        }
        _FreshSignature();
    }
}

void SessionDialog::MKOForScreenCaptureModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    std::string strImgPath = std::get<MKO_STRING>(mkoParam);

    //检查当前窗口是不是最上层的窗口，如果是最上层的，则插入图片
    for (HWND hWnd = GetTopWindow(NULL); NULL != hWnd; hWnd = GetWindow(hWnd, GW_HWNDNEXT))
    {
        wchar_t szClsName[MAX_PATH] = { 0 };
        GetClassName(hWnd, szClsName, MAX_PATH);
        if (!_wcsicmp(szClsName, L"SessionDialog"))
        {
            if (m_hWnd == hWnd)
                m_pSessionLayout->OnFinishScreenCapture(util::s2ws(strImgPath).c_str());

            break;
        }
    }
}

BOOL SessionDialog::StopPlayingAnimate(std::string& sAudioPlayingID)
{
	if (m_pSessionLayout)
	{
		 return m_pSessionLayout->StopPlayingAnimate(sAudioPlayingID);
	}
	return FALSE;
}

BOOL SessionDialog::StartPlayingAnimate(std::string sAudioPlayingID)
{
	if (m_pSessionLayout)
	{
		return m_pSessionLayout->StartPlayingAnimate(sAudioPlayingID);
	}
	return FALSE;
}

LRESULT SessionDialog::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pSessionLayout && m_pSessionLayout->m_pInputRichEdit)
	{
		m_pSessionLayout->m_pInputRichEdit->SetFocus();//bug，切换的时候，第一次不能获得焦点
	}
	return __super::OnSetFocus(uMsg, wParam, lParam, bHandled);
}

void SessionDialog::DoDisplayHistoryMsgToIE(IN std::vector<MessageEntity>& msgList,IN const BOOL scrollBottom)
{
	if (m_pSessionLayout)
	{
		m_pSessionLayout->DoDisplayHistoryMsgToIE(msgList, scrollBottom);
	}
}

LRESULT SessionDialog::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, lParam);
	return __super::OnLButtonDown(uMsg,wParam,lParam,bHandled);
}
void SessionDialog::_FreshSignature(void)
{
    PTR_VOID(m_pEditSignature);
    module::UserInfoEntity userInfo;
    if (module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
    {
        m_pEditSignature->SetText(util::stringToCString(userInfo.signature));
        m_pEditSignature->SetToolTip(util::stringToCString(userInfo.signature));
    }
}



