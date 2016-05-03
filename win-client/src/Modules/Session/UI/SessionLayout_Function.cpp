/******************************************************************************* 
 *  @file      SessionLayout_Function.cpp 2014\8\15 13:26:01 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/MessageEntity.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/IDatabaseModule.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../../Message/SendMsgManage.h"
#include "../Operation/SendImgHttpOperation.h"
#include "../SessionManager.h"
#include "json/reader.h"
#include "json/writer.h"
#include "UIIMEdit.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodingAPI.h"

/******************************************************************************/
void SessionLayout::_SendSessionMsg(IN MixedMsg mixMsg)
{
	if (mixMsg.IsPureTextMsg())
	{
		MessageEntity msg;
		std::string msgEncrypt;
		ENCRYPT_MSG(util::cStringToString(mixMsg.m_strTextData), msgEncrypt);
		msg.content = msgEncrypt;
		msg.sessionId = m_sId;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		module::SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
		PTR_VOID(pSessionInfo);
		msg.msgType = (pSessionInfo->sessionType == module::SESSION_USERTYPE) ? MSG_TYPE_TEXT_P2P : MSG_TYPE_TEXT_GROUP;
		msg.msgSessionType = pSessionInfo->sessionType;	
		msg.msgTime = module::getSessionModule()->getTime();
		SendMsgManage::getInstance()->pushSendingMsg(msg);

		//更新会话时间
		module::SessionEntity*  pSessionEntity = SessionEntityManager::getInstance()->getSessionEntityBySId(msg.sessionId);
		if (pSessionEntity)
		{
			pSessionEntity->updatedTime = msg.msgTime;
		}
		//主界面 消息内容，时间更新
		module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_NEWMSGSEND, msg.sessionId);
	}
	else
	{
		for (ST_picData& picData : mixMsg.m_picDataVec)
		{
			//图片需要上传
			SendImgParam param;
			param.csFilePath = picData.strLocalPicPath;
			m_pSendImgHttpOper = new SendImgHttpOperation(param
				, BIND_CALLBACK_1(SessionLayout::OnSendImageCallback));
			module::getHttpPoolModule()->pushHttpOperation(m_pSendImgHttpOper, TRUE);
		}
		m_SendingMixedMSGList.push_back(mixMsg);
	}
}
void SessionLayout::SendMsg()
{
	MessageEntity msg;
	module::UserInfoEntity myInfo;
	module::getUserListModule()->getMyInfo(myInfo);
	UInt8 netState = module::getTcpClientModule()->getTcpClientNetState();
	if (module::TCPCLIENT_STATE_OK == netState 
		&& IM::BaseDefine::USER_STATUS_OFFLINE != myInfo.onlineState)
	{
		MixedMsg mixMsg;
		if (!m_pInputRichEdit->GetContent(mixMsg))
		{
			return;
		}
		_DafoNetWorkPicMsg(mixMsg);//大佛彩蛋
		//将消息投递给对方
		_SendSessionMsg(mixMsg);
		//本地消息展现
		msg.msgType = MSG_TYPE_TEXT_P2P;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.sessionId = m_sId;
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.msgStatusType = MESSAGE_TYPE_RUNTIME;
		std::string content = util::cStringToString(mixMsg.MakeMixedLocalMSG());
		msg.content = content;
		msg.msgTime = module::getSessionModule()->getTime();
		_DisplayMsgToIE(msg);
	}
	else
	{
		//发送消息太快
		_DisplaySysMsg(_T("STRID_SESSIONMODULE_OFFLINE_SENDMSG_TIP"));
	}
	
}

void SessionLayout::_DisplaySysMsg(IN CString strID)
{
	MessageEntity msg;
	CString csTip = util::getMultilingual()->getStringById(strID);
	msg.content = util::cStringToString(csTip);
	msg.sessionId = m_sId;
	msg.talkerSid = module::getSysConfigModule()->userID();
	msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
	ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
	module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//收到屏幕抖动消息提示
}

BOOL SessionLayout::_DisplayMsgToIE(IN MessageEntity msg)
{
	CString jsInterface = _T("sendMessage");
	module::UserInfoEntity userInfo;
	if (!module::getUserListModule()->getUserInfoBySId(msg.talkerSid, userInfo))
	{
		return FALSE;
	}

	Json::Value root;
	root["name"] = util::cStringToString(userInfo.getRealName());
	root["avatar"] = userInfo.getAvatarPathWithoutOnlineState();
	root["msgtype"] = msg.msgRenderType;
	root["uuid"] = msg.talkerSid;
	root["mtype"] = msg.isMySendMsg() ? "me" : "other";
	CTime timeData(msg.msgTime);
	root["time"] = util::cStringToString(timeData.Format(_T("%Y-%m-%d %H:%M:%S")));

	//语音内容特殊处理
	if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
	{
		root["voiceid"] = msg.content;
		CString sVoicetime;
		sVoicetime.Format(_T("%d秒"), msg.msgAudioTime);
		root["voicetime"] = util::cStringToString(sVoicetime);
		root["voiceisread"] = msg.msgAudioReaded ? std::string("true") : string("false");
	}
	else
	{
		CString csContent = util::stringToCString(msg.content);
		ReceiveMsgManage::getInstance()->parseContent(csContent, FALSE, GetWidth());
		std::string content = util::cStringToString(csContent);
		root["content"] = content;
	}
	Json::StyledWriter styleWrite;
	std::string record = styleWrite.write(root);
	Json::Reader jsonRead;
	Json::Value rootRead;
	CString jsData = _T("[]");
	if (!jsonRead.parse(record, rootRead) || rootRead.isNull())
	{
		CString csError = util::stringToCString(record, CP_UTF8);
		LOG__(APP, _T("json parse error:%s"), csError);
		jsData = _T("[]");
		return FALSE;
	}
	else
		jsData = util::stringToCString(record, CP_UTF8);
	//调用页面的JS代码
	if (m_pWebBrowser)
	{
		VARIANT VarResult;
		if (!m_pWebBrowser->CallJScript(jsInterface.GetBuffer(), jsData.GetBuffer(), &VarResult))
		{
			LOG__(ERR, _T("CallJScript failed:%s"),jsData);
		}
		jsData.ReleaseBuffer();
	}
	return TRUE;
}

void SessionLayout::DoDisplayHistoryMsgToIE(std::vector<MessageEntity>& msgList, BOOL scrollBottom)
{
	Json::Value root;
	for (auto itMsg = msgList.rbegin();itMsg != msgList.rend(); ++itMsg)
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(itMsg->talkerSid, userInfo))
			continue;

		//组装json data
		Json::Value msgItem;
		msgItem["name"] = util::cStringToString(userInfo.getRealName());
		msgItem["avatar"] = userInfo.getAvatarPathWithoutOnlineState();
		msgItem["mtype"] = itMsg->isMySendMsg() ? "me" : "other";
		CTime time(itMsg->msgTime);
		msgItem["time"] = util::cStringToString(time.Format(_T("%Y-%m-%d %H:%M:%S")));
		msgItem["uuid"] = itMsg->talkerSid;
		msgItem["msgtype"] = itMsg->msgRenderType;

		if (MESSAGE_RENDERTYPE_AUDIO == itMsg->msgRenderType)
		{
			msgItem["voiceid"] = itMsg->content;
			CString sVoicetime;
			sVoicetime.Format(_T("%d秒"), itMsg->msgAudioTime);
			msgItem["voicetime"] = util::cStringToString(sVoicetime, CP_UTF8);
			msgItem["voiceisread"] = itMsg->isReaded() ? std::string("true") : string("false");
		}
		else
		{
			std::string msgDecrptyCnt;
			DECRYPT_MSG(itMsg->content, msgDecrptyCnt);
			CString content = util::stringToCString(msgDecrptyCnt);
			ReceiveMsgManage::getInstance()->parseContent(content, FALSE, GetWidth());
			msgItem["content"] = util::cStringToString(content);
		}

		root.append(msgItem);
	}

	Json::StyledWriter styleWrite;
	std::string record = styleWrite.write(root);
	CString jsData = _T("[]");
	Json::Reader jsonRead;
	Json::Value rootRead;
	if (!jsonRead.parse(record, rootRead) || rootRead.isNull())
	{
		CString csError = util::stringToCString(record, CP_UTF8);
		LOG__(ERR, _T("history is null or json parse error:%s"), csError);
		jsData = _T("[]");
	}
	else
	{
		jsData = util::stringToCString(record);
	}

	//调用js
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("historyMessage"), jsData.GetBuffer(), &result);
	if (!bRet)
		LOG__(ERR, _T("CallJScript failed,%s"), jsData);
	if (scrollBottom)
	{
		module::getEventManager()->asynFireUIEventWithLambda(
			[=]()
		{
			CComVariant result;
			m_pWebBrowser->CallJScript(_T("scrollBottom"), _T(""), &result);
		});
	}
}

void SessionLayout::_DisplayHistoryMsgToIE(IN UInt32 nMsgCount,BOOL scrollBottom)
{
	std::vector<MessageEntity> msgList;
	if (module::getMessageModule()->getHistoryMessage(m_sId, nMsgCount,scrollBottom,msgList))
	{
		DoDisplayHistoryMsgToIE(msgList, scrollBottom);
	}
}

void SessionLayout::UpdateSendMsgShortcut()
{
	if (!m_pInputRichEdit)
	{
		return;
	}
	module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
	BOOL bWantCtrlEnter = (pTTConfig->sysBaseFlag & module::BASE_FLAG_SENDIMG_BY_CTRLENTER);
	if (bWantCtrlEnter)
	{
		m_pSendDescription->SetText(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_CTRLENTERSEND")));
	}
	else
	{
		m_pSendDescription->SetText(util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_ENTERSEND")));
	}
	m_pInputRichEdit->SetWantReturn(!bWantCtrlEnter);
	m_bottomLayout->NeedUpdate();
}

BOOL SessionLayout::_DisplayUnreadMsg()
{
	SessionMessage_List msgList;
	if (!ReceiveMsgManage::getInstance()->popMessagesBySId(m_sId, msgList) && msgList.empty())
	{
		//没有未读消息
		return FALSE;
	}

	for (auto MessageInfo : msgList)
	{
        if (MESSAGE_RENDERTYPE_SYSTEMTIPS != MessageInfo.msgRenderType)
        {
            DECRYPT_MSG(MessageInfo.content, MessageInfo.content);
        }
		_DisplayMsgToIE(MessageInfo);
	}

	//保存到历史消息中
	module::getDatabaseModule()->sqlBatchInsertMessage(msgList);
	//离线消息需要重置获取历史消息的msgid
	MessageEntity msgFront = msgList.front();
	module::getMessageModule()->setSessionTopMsgId(msgFront.sessionId, msgFront.msgId -1);
	
    //更新总未读计数
    module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT);

	//发送已读确认
	auto msgBack = msgList.back();
	_AsynSendReadAck(msgBack);
	return TRUE;
}
void SessionLayout::_LoadFirstOpenedMsg(void)
{
    LOG__(APP, _T("load historyMsg or UnreadMsg! sid:%s"), util::stringToCString(m_sId));
    module::getEventManager()->asynFireUIEventWithLambda(
        [=]()
    {
        if (!_DisplayUnreadMsg())
        {
            _DisplayHistoryMsgToIE(FETCH_MSG_COUNT_PERTIME, TRUE);
        }
        //滚动条滚动到最底
        CComVariant result;
        m_pWebBrowser->CallJScript(_T("scrollBottom"), _T(""), &result);
    }
    );
}
void SessionLayout::_DafoNetWorkPicMsg(OUT MixedMsg& mixMsg)
{
	if (!mixMsg.IsPureTextMsg())
	{
		return;
	}
	const CString strDafo = _T("dafo:");
	UInt32 nPos = mixMsg.m_strTextData.Find(strDafo);
	if (0 == nPos)
	{
		CString strContent = mixMsg.m_strTextData.Mid(strDafo.GetLength());
		mixMsg.m_strTextData = MixedMsg::AddPicTeg2Pic(strContent);
		return;
	}
	const CString strRandom = _T("@random");
	nPos = mixMsg.m_strTextData.Find(strRandom);
	if (0 == nPos)
	{
		std::string sid = module::getUserListModule()->randomGetUser();
		module::UserInfoEntity userInfo;
		module::getUserListModule()->getUserInfoBySId(sid, userInfo);
		mixMsg.m_strTextData = _T("@") + userInfo.getRealName();
		return;
	}
	const CString strUnReadCnt = _T("@unreadcnt");
	nPos = mixMsg.m_strTextData.Find(strUnReadCnt);
	if (0 == nPos)
	{
		UInt32 nCount = module::getMessageModule()->getTotalUnReadMsgCount();
		mixMsg.m_strTextData = _T("我的总未读计数为：") + util::int32ToCString(nCount);
		return;
	}
	const UInt32 nMySid = module::getSysConfigModule()->userId();
	if (374 == nMySid || 135 == nMySid)//只有大佛和快刀可用
	{
		const CString strDecode = _T("decode:");
		nPos = mixMsg.m_strTextData.Find(strDecode);
		if (0 == nPos)
		{
			CString strContent = mixMsg.m_strTextData.Mid(strDecode.GetLength());
			std::string content = util::cStringToString(strContent);
			std::string msgDecrptyCnt;
			DECRYPT_MSG(content, msgDecrptyCnt);
			mixMsg.m_strTextData = util::stringToCString(msgDecrptyCnt);
			return;
		}
		const CString strEncode = _T("encode:");
		nPos = mixMsg.m_strTextData.Find(strEncode);
		if (0 == nPos)
		{
			CString strContent = mixMsg.m_strTextData.Mid(strEncode.GetLength());
			std::string content = util::cStringToString(strContent);
			std::string msgDecrptyCnt;
			ENCRYPT_MSG(content, msgDecrptyCnt);
			mixMsg.m_strTextData = util::stringToCString(msgDecrptyCnt);
			return;
		}
	}
}
/******************************************************************************/