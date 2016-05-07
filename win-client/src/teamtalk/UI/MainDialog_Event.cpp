/******************************************************************************* 
 *  @file      MainDialog_Event.cpp 2014\8\20 15:43:48 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "MainDialog.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "ProtocolBuffer/IM.BaseDefine.pb.h"
#include "utility/Multilingual.h"

/******************************************************************************/
void MainDialog::OnClick(TNotifyUI& msg)
{
	PTR_VOID(msg.pSender);
	if (msg.pSender == m_pbtnSysConfig)
	{
		//系统设置
		module::getSysConfigModule()->showSysConfigDialog(m_hWnd);
	}
	else if (msg.pSender == m_pbtnOnlineStatus)
	{
		CMenuWnd* pMenu = new CMenuWnd(m_hWnd);
		DuiLib::CPoint point = msg.ptMouse;
		ClientToScreen(m_hWnd, &point);
		STRINGorID xml(_T("menu\\lineStatus.xml"));
		pMenu->Init(NULL, xml, _T("xml"), point);
	}
	else if (msg.pSender == m_pbtnMyFace)
	{
		//show the detail of myself.
		module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, module::getSysConfigModule()->userID());
	}
	else if (msg.pSender == m_pbtnClose
		|| msg.pSender == m_pbtnMinMize)
	{
		ShowWindow(false);
		return;
	}
	__super::OnClick(msg);
}

void MainDialog::MKOForTcpClientModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_TCPCLIENT_STATE == keyId)
	{
		//TCP长连断开处理
		if (module::TCPCLIENT_STATE_DISCONNECT == module::getTcpClientModule()->getTcpClientNetState())
		{
			LOG__(ERR, _T("TCPCLIENT_STATE_DISCONNECT:TCP Client link broken!!!"));

			//托盘区图标灰度掉
			CString csTip = util::getMultilingual()->getStringById(_T("STRID_MAINDIALOG_TIP_DISCONNECT"));
			SetTrayTooltipText(csTip);
			UpdateLineStatus(IM::BaseDefine::UserStatType::USER_STATUS_OFFLINE);

			//开启重连业务
			module::getTcpClientModule()->shutdown();
			if (!module::getLoginModule()->isOfflineByMyself())
				module::getLoginModule()->relogin(FALSE);
		}
	}
}
void MainDialog::OnCopyData(IN COPYDATASTRUCT* pCopyData)
{
	if (nullptr == pCopyData)
	{
		return;
	}
	module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_COPYDATA, pCopyData->dwData);
}
void MainDialog::MKOForLoginModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_LOGIN_RELOGINOK == keyId)
	{
		CString sText = util::getMultilingual()->getStringById(_T("STRID_GLOBAL_CAPTION_NAME"));
#ifdef _DEBUG
		sText += _T("(Debug)");
#endif
		SetTrayTooltipText(sText);
		UpdateLineStatus(IM::BaseDefine::UserStatType::USER_STATUS_ONLINE);
	}
	else if (module::KEY_LOGIN_KICKOUT == keyId)//账户被踢出了，设置自己离线
	{
		Int32 nRes = std::get<MKO_INT>(mkoParam);
		LOG__(APP, _T("Offline by kickout,%d"), nRes);
		module::getTcpClientModule()->shutdown();
		module::getLoginModule()->setOfflineByMyself(TRUE);
		CString strText = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_DUPLICATE_USER_KICKOUT"));
		if (IM::BaseDefine::KickReasonType::KICK_REASON_MOBILE_KICK == nRes)
		{
			strText = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_MOBILE_KICK"));
		}		
		CString strCaption = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_CAPTION"));
		::MessageBoxEx(m_hWnd, strText, strCaption, MB_OK, LANG_CHINESE);
	}
}

void MainDialog::OnMenuClicked(IN const CString& itemName, IN const CString& strLparam)
{
	if (_T("exitMenuItem") == itemName)
	{
		module::getMiscModule()->quitTheApplication();
	}
	else if (_T("LogDir") == itemName)//打开日志目录
	{
		CString strLogDir = util::getParentAppPath() + _T("bin\\log\\ttlog.log");
		if (PathFileExists(strLogDir))
		{
			HINSTANCE hr = ShellExecute(NULL, _T("open"), _T("Explorer.exe"), _T("/select,") + strLogDir, NULL, SW_SHOWDEFAULT);
			if ((long)hr < 32)
			{
				DWORD dwLastError = ::GetLastError();
				LOG__(ERR, _T("ShellExecute Failed GetLastError = %d")
					, dwLastError);
			}
		}
	}
	else if (_T("OnlineMenuItem") == itemName)
	{
		if (IM::BaseDefine::USER_STATUS_ONLINE == module::getUserListModule()->getMyLineStatus())
		{
			LOG__(APP, _T("already online"));
			return;
		}
		module::getLoginModule()->setOfflineByMyself(FALSE);
		module::getLoginModule()->relogin(TRUE);
	}
	else if (_T("OfflineMenuItem") == itemName)
	{
		if (IM::BaseDefine::USER_STATUS_OFFLINE == module::getUserListModule()->getMyLineStatus())
		{
			LOG__(APP, _T("already offline"));
			return;
		}
		LOG__(APP,_T("Offline by myself"));
		module::getTcpClientModule()->shutdown();
		module::getLoginModule()->setOfflineByMyself(TRUE);
	}
	else if (_T("SysSettingMenuItem") == itemName)
	{
		//系统设置
		module::getSysConfigModule()->showSysConfigDialog(m_hWnd);
	}
	else if (_T("serverAddressSetting") == itemName)
	{
		module::getSysConfigModule()->showServerConfigDialog(m_hWnd);
	}
    else if (_T("showFileTransferDialog") == itemName)
    {
        //打开文件传输面板
        module::getFileTransferModule()->showFileTransferDialog();
    }
	else if (_T("UserDetailInfo") == itemName)
	{
		if (strLparam.IsEmpty())
		{
			return;
		}
		std::string sid = util::cStringToString(strLparam);
		module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, sid);
	}
}

void MainDialog::MKOForUserListModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_USERLIST_DOWNAVATAR_SUCC == keyId)
	{
		std::shared_ptr<void> p = std::get<MKO_SHARED_VOIDPTR>(mkoParam);
		std::tuple<std::string, std::string>* pTuple = (std::tuple<std::string, std::string>*)p.get();
		std::string& sId = std::get<1>(*pTuple);
		//刷新头像
		module::UserInfoEntity myInfo;
		module::getUserListModule()->getMyInfo(myInfo);
		if (sId == myInfo.sId)
		{
			if (m_pbtnMyFace)
				m_pbtnMyFace->SetBkImage(util::stringToCString(myInfo.getAvatarPath()));
		}
	}
    else if (module::KEY_USERLIST_USERSIGNINFO_CHANGED == keyId)
    {
        //更新自己的签名，若失败则改回去。
        std::string& sId = std::get<MKO_STRING>(mkoParam);
        if (sId != module::getSysConfigModule()->userID())
        {
            return;
        }
        _FreshMySignature();
    }
}
void MainDialog::MKOForSessionModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SESSION_TRAY_STARTEMOT == keyId)
	{
		StartNewMsgTrayEmot();
	}
	else if (module::KEY_SESSION_TRAY_STOPEMOT == keyId)
	{
		StopNewMsgTrayEmot();
	}
    else if (module::KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT == keyId)
    {
        _UpdateTotalUnReadMsgCount();//更新总未读计数
    }
}

void MainDialog::StartNewMsgTrayEmot()
{
	SetTimer(m_hWnd, TIMER_TRAYEMOT, 500, NULL);
}

void MainDialog::StopNewMsgTrayEmot()
{
	KillTimer(m_hWnd, TIMER_TRAYEMOT);
	UpdateLineStatus(module::getUserListModule()->getMyLineStatus());
}
/******************************************************************************/