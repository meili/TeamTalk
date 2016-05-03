/******************************************************************************* 
 *  @file      UserDetailInfoDialog.cpp 2014\10\22 11:15:09 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/UserDetailInfoDialog.h"
#include "Modules/IUserListModule.h"
#include "Modules/IDatabaseModule.h"
#include "utility/utilStrCodingAPI.h"
#include "../Operation/DownloadAvatarHttpOperation.h"
/******************************************************************************/
DUI_BEGIN_MESSAGE_MAP(UserDetailInfoDialog, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
// -----------------------------------------------------------------------------
//  UserDetailInfoDialog: Public, Constructor

UserDetailInfoDialog::UserDetailInfoDialog(IN std::string sid)
:m_sid(sid)
, m_AvatarUI(nullptr)
{

}

LPCTSTR UserDetailInfoDialog::GetWindowClassName() const
{
	return _T("UserDetailInfoDialog");
}

DuiLib::CDuiString UserDetailInfoDialog::GetSkinFile()
{
	return  _T("MainDialog\\UserDetailInfoDialog.xml");
}

DuiLib::CDuiString UserDetailInfoDialog::GetSkinFolder()
{
	return _T("");
}

void UserDetailInfoDialog::OnFinalMessage(HWND hWnd)
{
	m_AvatarUI = nullptr;
	if (m_pDownHttpOper)
	{
		m_pDownHttpOper->cancel();
	}
	__super::OnFinalMessage(hWnd);
	delete this;
}

void UserDetailInfoDialog::OnWindowInitialized(TNotifyUI& msg)
{
	module::UserInfoEntity userInfo;
	if (!module::getUserListModule()->getUserInfoBySId(m_sid, userInfo))
	{
		LOG__(ERR, _T("getUserInfoBySId failed"));
		return;
	}
	CControlUI* name = m_PaintManager.FindControl(_T("name"));
	if (name)
	{
		name->SetText(userInfo.csNickName);
	}
	CControlUI* realName = m_PaintManager.FindControl(_T("realname"));
	if (realName)
	{
		realName->SetText(userInfo.csName);
	}
    CControlUI* signInfo = m_PaintManager.FindControl(_T("signInfo"));
    if (signInfo)
    {
        signInfo->SetText(util::stringToCString(userInfo.signature));
        signInfo->SetToolTip(util::stringToCString(userInfo.signature));
    }
	CControlUI* sex = m_PaintManager.FindControl(_T("sex"));
	if (sex)
	{
		if (1 == userInfo.gender)
		{
			sex->SetText(_T("小侠"));
		}
		else if (2 == userInfo.gender)
		{
			sex->SetText(_T("小仙"));
		}
		else
			sex->SetText(_T("人妖"));
	}
	CControlUI* department = m_PaintManager.FindControl(_T("department"));
	if (department)
	{
		department->SetText(util::stringToCString(userInfo.department));
	}
	CControlUI* telphone = m_PaintManager.FindControl(_T("telphone"));
	if (telphone)
	{
		telphone->SetText(util::stringToCString(userInfo.telephone));
	}
	CControlUI* mail = m_PaintManager.FindControl(_T("mail"));
	if (mail)
	{
		mail->SetText(util::stringToCString(userInfo.email));
	}
	m_AvatarUI = m_PaintManager.FindControl(_T("Avatar"));
	if (m_AvatarUI)
	{
		m_AvatarUI->SetBkImage(util::stringToCString(userInfo.getAvatarPath()));
		m_AvatarUI->SetToolTip(util::stringToCString(m_sid));
	}
	_refreshAvatar(userInfo.avatarUrl);
}

void UserDetailInfoDialog::_refreshAvatar(IN const std::string& avatarPath)
{
	if (avatarPath.empty())
	{
		return;
	}
	UInt32 hashcode = util::hash_BKDR((avatarPath + AVATAR_FORMAT_170X170).c_str());
	module::ImImageEntity imageEntity;
	module::getDatabaseModule()->sqlGetImImageEntityByHashcode(hashcode, imageEntity);
	CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(imageEntity.filename);
	if (!imageEntity.filename.empty() && PathFileExists(csLocalPath))
	{
		//本地磁盘存在
		if (m_AvatarUI)
		{
			m_AvatarUI->SetBkImage(csLocalPath);
		}
	}
	else//不存在则去服务器下载
	{
		std::string path = avatarPath;
		m_pDownHttpOper = new DownloadAvatarHttpOperation(m_sid, path, FALSE
			, AVATAR_FORMAT_170X170, BIND_CALLBACK_1(UserDetailInfoDialog::onHttpGetAvatarCallback));

		module::getHttpPoolModule()->pushHttpOperation(m_pDownHttpOper);
	}
}
void UserDetailInfoDialog::onHttpGetAvatarCallback(std::shared_ptr<void> param)
{
	m_pDownHttpOper = 0;
	DownloadImgParam* pDownParam = (DownloadImgParam*)param.get();
	if (DownloadImgParam::DOWNLOADIMG_OK == pDownParam->m_result)
	{
		CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(pDownParam->m_imgEntity.filename);
		if (m_AvatarUI)
		{
			m_AvatarUI->SetBkImage(csLocalPath);
		}
	}
}
void UserDetailInfoDialog::OnClick(TNotifyUI& msg)
{
	PTR_VOID(msg.pSender);
	if (msg.pSender == m_AvatarUI)
	{
		module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, m_sid);//通知主窗口创建会话
	}
	__super::OnClick(msg);
}

/******************************************************************************/