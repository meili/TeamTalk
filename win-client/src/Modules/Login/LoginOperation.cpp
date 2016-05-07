/******************************************************************************* 
 *  @file      LoginOperation.cpp 2014\7\30 15:32:28 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "LoginOperation.h"
#include "utility/utilStrCodingAPI.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/ITcpClientModule.h"
#include "GlobalConfig.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  LoginOperation: Public, Constructor

LoginOperation::LoginOperation(module::IOperationDelegate callback, LoginParam& param)
:ICallbackOpertaion(callback)
,m_loginParam(param)
{

}

// -----------------------------------------------------------------------------
//  LoginOperation: Public, Destructor

LoginOperation::~LoginOperation()
{

}

void LoginOperation::processOpertion()
{
	LOG__(APP,_T("login start,uname:%s,status:%d"), m_loginParam.csUserName
		, m_loginParam.mySelectedStatus);

	LoginParam* pParam = new LoginParam;
	pParam->csUserName = m_loginParam.csUserName;
	pParam->mySelectedStatus = m_loginParam.mySelectedStatus;

	//连接消息服务器
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	CString server = util::stringToCString(pCfg->msgSevPriorIP);
	LOG__(APP, _T("MsgServeIp:%s,Port:%d"), server, pCfg->msgServPort);
	IM::Login::IMLoginRes* pImLoginResp = (IM::Login::IMLoginRes*)module::getTcpClientModule()
		->doLogin(server, pCfg->msgServPort,m_loginParam.csUserName,m_loginParam.password);
	if (0 == pImLoginResp || pImLoginResp->result_code() != IM::BaseDefine::REFUSE_REASON_NONE 
		|| !pImLoginResp->has_user_info())
	{
		//TODO,若失败，尝试备用IP
		LOG__(ERR,_T("add:%s:%d,uname:%s,login for msg server failed"),server,pCfg->msgServPort, m_loginParam.csUserName);
		if (pImLoginResp)
		{
			CString errInfo = util::stringToCString(pImLoginResp->result_string());
			pParam->errInfo = errInfo;
			pParam->result = LOGIN_FAIL;
			pParam->server_result = pImLoginResp->result_code();
			LOG__(ERR, _T("error code :%d,error info:%s"), pImLoginResp->result_code(), errInfo);
		}
		else
		{
			pParam->result = IM::BaseDefine::REFUSE_REASON_NO_MSG_SERVER;
			LOG__(ERR, _T("login msg server faild！"));
		}
		asyncCallback(std::shared_ptr<void>(pParam));
		return;
	}
	pParam->result = LOGIN_OK;
	pParam->serverTime = pImLoginResp->server_time();
	pParam->mySelectedStatus = pImLoginResp->online_status();

	//存储服务器端返回的userId
	IM::BaseDefine::UserInfo userInfo = pImLoginResp->user_info();
	pCfg->userId = util::uint32ToString(userInfo.user_id());
	pCfg->csUserId = util::stringToCString(pCfg->userId);

	//登陆成功，创建自己的信息
	module::UserInfoEntity myInfo;
	myInfo.sId = pCfg->userId;
	myInfo.csName = m_loginParam.csUserName;
	myInfo.onlineState = IM::BaseDefine::USER_STATUS_ONLINE;
	myInfo.csNickName = util::stringToCString(userInfo.user_nick_name());
	myInfo.avatarUrl = userInfo.avatar_url();
	myInfo.dId = util::uint32ToString(userInfo.department_id());
	myInfo.department = myInfo.dId;
	myInfo.email = userInfo.email();
	myInfo.gender = userInfo.user_gender();
	myInfo.user_domain = userInfo.user_domain();
	myInfo.telephone = userInfo.user_tel();
	myInfo.status = userInfo.status();
    myInfo.signature = userInfo.sign_info();

	module::getUserListModule()->createUserInfo(myInfo);

	asyncCallback(std::shared_ptr<void>(pParam));

	LOG__(APP, _T("login succeed! Name = %s Nickname = %s sId = %s status = %d")
		, m_loginParam.csUserName
		, util::stringToCString(userInfo.user_nick_name())
		, module::getSysConfigModule()->UserID()
		, m_loginParam.mySelectedStatus);

	//开始发送心跳包
	module::getTcpClientModule()->startHeartbeat();
}

void LoginOperation::release()
{
	delete this;
}

/******************************************************************************/