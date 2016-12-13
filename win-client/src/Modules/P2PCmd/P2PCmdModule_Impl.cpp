/******************************************************************************* 
 *  @file      P2PCmdModule_Impl.cpp 2015\1\5 15:57:26 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ProtocolBuffer/IM.SwitchService.pb.h"
#include "ProtocolBuffer/IM.BaseDefine.pb.h"
#include "P2PCmdModule_Impl.h"
#include "json/reader.h"
#include "json/writer.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ITcpClientModule.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"
#include "../Message/ReceiveMsgManage.h"
#include "network/ImCore.h"
#include "network/TTPBHeader.h"
/******************************************************************************/
namespace module
{
	module::IP2PCmdModule* getP2PCmdModule()
	{
		static P2PCmdModule_Impl module;
		return &module;
	}
}

void P2PCmdModule_Impl::onPacket(imcore::TTPBHeader& header, std::string& pbBody)
{
	switch (header.getCommandId())
	{
	case IM::BaseDefine::SwitchServiceCmdID::CID_SWITCH_P2P_CMD:
		_p2pCmdNotifyResponse(pbBody);
		break;
	default:
		LOG__(ERR, _T("Unknow commandID:%d"), header.getCommandId());
		return;
	}
}

void P2PCmdModule_Impl::_p2pCmdNotifyResponse(std::string& pbBody)
{
	IM::SwitchService::IMP2PCmdMsg imP2PCmdMsg;
	if (!imP2PCmdMsg.ParseFromString(pbBody))
	{
		LOG__(ERR, _T("ParseFromString failed:%s"), util::stringToCString(pbBody));
		return;
	}
	std::string jsonMsgData = imP2PCmdMsg.cmd_msg_data();
	std::string sFromId = util::uint32ToString(imP2PCmdMsg.from_user_id());
	LOG__(APP, _T("IMP2PCmdMsg cmd_msg_data = %s,sFromId = %s"), util::stringToCString(jsonMsgData), util::stringToCString(sFromId));
	UINT32 nServiceID = 0;
	UINT32 nCmdID = 0;
	_parseMsgJsonData(jsonMsgData, nServiceID, nCmdID);

	if (module::KEY_P2PCMD_SHAKEWINDOW == nServiceID && module::KEY_P2PCMD_SHAKEWINDOW_NOTIFY == nCmdID)
	{
		//窗口抖动生成一条系统消息，提示用户
		module::UserInfoEntity userInfo;
		if (module::getUserListModule()->getUserInfoBySId(sFromId, userInfo))
		{
			CString csTipFormat = util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_SHAKEWINDOW_TIP"));
			CString csTip;
			MessageEntity msg;
			csTip.Format(csTipFormat, userInfo.getRealName());
			msg.content = util::cStringToString(csTip);
            ENCRYPT_MSG(msg.content, msg.content);
			msg.sessionId = sFromId;
			msg.talkerSid = module::getSysConfigModule()->userID();
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			msg.msgSessionType = MESSAGETYPE_FROM_FRIEND;
			msg.msgTime = (UInt32)time(0);
			ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, sFromId);//收到屏幕抖动消息提示	
		}
		module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG, sFromId);//抖动窗口
	}
	else if (module::KEY_P2PCMD_WRITING == nServiceID)
	{
		if (module::KEY_P2PCMD_WRITING_NOTIFY == nCmdID)
		{
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_WRITING_MSG, sFromId);
		}
		else if (module::KEY_P2PCMD_STOP_WRITING_NOTIFY == nCmdID)
		{
			module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_STOPWRITING_MSG, sFromId);
		}
	}
	else
	{
		LOG__(APP, _T("Unkown p2p message,nServiceID:%d,nCmdID:%d"), nServiceID, nCmdID);
	}
}
void P2PCmdModule_Impl::_parseMsgJsonData(IN std::string sJsonData, OUT UInt32& nServiceID, OUT UInt32& nCmdID)
{
	Json::Reader reader;
	Json::Value root;
	try
	{
		if (!reader.parse(sJsonData, root) || root.isNull())
		{
			LOG__(ERR, _T("parse json data failed! json data:%s"), util::stringToCString(sJsonData));
			return;
		}
		nServiceID = (root.get("service_id", "")).asUInt();
		nCmdID = (root.get("cmd_id", "")).asUInt();
	}
	catch (...)
	{
		LOG__(ERR, _T("parse json data failed! json data:%s"),util::stringToCString(sJsonData));
	}
}

BOOL P2PCmdModule_Impl::tcpSendShakeWindowCMD(IN std::string sToID)
{
	std::string fromId = module::getSysConfigModule()->userID();
	module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG, fromId);
	std::string cmdMsgData;
	_makeJsonData(module::KEY_P2PCMD_SHAKEWINDOW, module::KEY_P2PCMD_SHAKEWINDOW_NOTIFY, "shakewindow", cmdMsgData);

	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		IM::SwitchService::IMP2PCmdMsg imP2PCmdMsg;
		imP2PCmdMsg.set_from_user_id(module::getSysConfigModule()->userId());
		UInt32 nToId = util::stringToInt32(sToID);
		imP2PCmdMsg.set_to_user_id(nToId);
		imP2PCmdMsg.set_cmd_msg_data(cmdMsgData);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_SWITCH_SERVICE
			,IM::BaseDefine::SwitchServiceCmdID::CID_SWITCH_P2P_CMD
			,&imP2PCmdMsg);
	}
		);

	return TRUE;
}

void P2PCmdModule_Impl::tcpSendWritingCMD(IN std::string sToID, IN const BOOL bWriting)
{
	std::string fromId = module::getSysConfigModule()->userID();
	std::string cmdMsgData;
	if (bWriting)
	{
		_makeJsonData(module::KEY_P2PCMD_WRITING, module::KEY_P2PCMD_WRITING_NOTIFY, "writing", cmdMsgData);
	}
	else
	{
		_makeJsonData(module::KEY_P2PCMD_WRITING, module::KEY_P2PCMD_STOP_WRITING_NOTIFY, "stop writing", cmdMsgData);
	}

	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		//LOG__(DEBG, _T("IMP2PCmdMsg,bWriting = %d"),bWriting);
		IM::SwitchService::IMP2PCmdMsg imP2PCmdMsg;
		imP2PCmdMsg.set_from_user_id(module::getSysConfigModule()->userId());
		UInt32 nToId = util::stringToInt32(sToID);
		imP2PCmdMsg.set_to_user_id(nToId);
		imP2PCmdMsg.set_cmd_msg_data(cmdMsgData);
		module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_SWITCH_SERVICE
			, IM::BaseDefine::SwitchServiceCmdID::CID_SWITCH_P2P_CMD
			, &imP2PCmdMsg);
	}
	);
}

void P2PCmdModule_Impl::_makeJsonData(IN UINT32 nServiceID, IN UINT32 nCmdID, IN std::string sContent, OUT std::string& sJsonData)
{
	Json::Value root;
	root["service_id"] = nServiceID;
	root["cmd_id"] = nCmdID;
	root["content"] = sContent;

	Json::FastWriter fstWrite;
	sJsonData = fstWrite.write(root);
}

/******************************************************************************/