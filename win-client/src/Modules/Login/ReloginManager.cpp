/******************************************************************************* 
 *  @file      ReloginManager.cpp 2013\9\4 16:44:21 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "ReloginManager.h"
#include "LoginOperation.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/IMessageModule.h"
#include "ProtocolBuffer/IM.Buddy.pb.h"
#include "utility/Multilingual.h"
#include "network/ImCore.h"
/******************************************************************************/
// -----------------------------------------------------------------------------
//  ReloginManager: Public, Constructor

ReloginManager::ReloginManager()
:m_secondCount(3)
,m_bDoReloginNow(FALSE)
{

}

// -----------------------------------------------------------------------------
//  ReloginManager: Public, Destructor

ReloginManager::~ReloginManager()
{
}
// -----------------------------------------------------------------------------
// public   
void ReloginManager::startReloginTimer(UInt32 second)
{
    if(second > 15)
        second = 15;
	module::ITimerEvent* pTimer = 0;
	module::getEventManager()->scheduleTimerWithLambda(second, FALSE,
		[=]()
	{
		doRelogin();
	}
	, &pTimer);
    m_secondCount = second;
}
/******************************************************************************/
// -----------------------------------------------------------------------------
// public   
void ReloginManager::forceRelogin()
{
    doRelogin();
}

// -----------------------------------------------------------------------------
// private   
void ReloginManager::doRelogin()
{
    try
    {
        if(m_bDoReloginNow)
        {
            LOG__(APP,_T("is doing Relogin now..."));
            return;
        }

        //清理掉队列里面 未发送的 消息已读确认 Operation《不处理可能会引发丢消息》
        imcore::IMLibCoreClearOperationByName(imcore::OPERATION_NAME_MSG_READ_ACK);

        LoginParam param;
		module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
		param.mySelectedStatus = pCfg->myselectStatus;
		param.csUserName = pCfg->userName;
		param.password = pCfg->password;
		param.csUserName.Trim();

		LoginOperation* pOperation = new LoginOperation(BIND_CALLBACK_1(ReloginManager::OnOperationCallback), param);
		imcore::IMLibCoreStartOperation(pOperation);
        m_bDoReloginNow = TRUE;
    }
    catch (...)
    {
		module::getTcpClientModule()->shutdown();
        startReloginTimer(++m_secondCount);
        LOG__(ERR, _T("relogin unknown exception"));
        m_bDoReloginNow = FALSE;
    }
}

void ReloginManager::OnOperationCallback(std::shared_ptr<void> param)
{
	m_bDoReloginNow = FALSE;
	LoginParam* pLoginParam = (LoginParam*)param.get();
	if (LOGIN_OK == pLoginParam->result)
	{
		LOG__(ERR, _T("ReloginManager regloin success!!!"));

		module::getSessionModule()->setTime(pLoginParam->serverTime);

		//通知服务器客户端初始化完毕,获取组织架构信息和群列表
		module::getLoginModule()->notifyLoginDone();

		//清空存在客户端的未读消息，因为服务端又会将这个消息送过来，避免重复
		module::getMessageModule()->removeAllMessage();

		//获取最近联系会话
		UInt32 updateTime = module::getSessionModule()->getGlobalUpdateTime();
		LOG__(APP, _T("IMRecentContactSessionReq after relogin done, local update time = %d"), updateTime);
		IM::Buddy::IMRecentContactSessionReq imRecentContactSessionReq;
		imRecentContactSessionReq.set_user_id(module::getSysConfigModule()->userId());
		imRecentContactSessionReq.set_latest_update_time(updateTime);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_BUDDY_LIST
			, IM::BaseDefine::BuddyListCmdID::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST
			, &imRecentContactSessionReq);

		//通知网络已经恢复正常，可以进行各种操作了
		module::getLoginModule()->asynNotifyObserver(module::KEY_LOGIN_RELOGINOK, pLoginParam->mySelectedStatus);
	}
	else
	{
		LOG__(ERR, _T("ReloginManager regloin failed!!!"));
		module::getTcpClientModule()->shutdown();

		//TCP\IP验证token失效,开启重新获取token的task
		//if (LOGIN_TOKEN_FAILED == pLoginParam->result)
		{
			//开启定时获取token的定时器
		}
		if (IM::BaseDefine::REFUSE_REASON_VERSION_TOO_OLD == pLoginParam->result)
		{
			CString csTip = util::getMultilingual()->getStringById(_T("STRID_WEBLOGINFORM_TIP_VERSION_TOOOLD"));
			CString csTitle = module::getMiscModule()->getAppTitle();
			::MessageBox(0, csTip, csTitle, MB_OK | MB_ICONINFORMATION);
			module::getMiscModule()->quitTheApplication();
		}
		else
		{
			startReloginTimer(++m_secondCount);
		}
	}
}
