/******************************************************************************* 
 *  @file      ReceiveMsgManage.cpp 2014\8\7 14:57:37 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ReceiveMsgManage.h"
#include "utility/utilStrCodingAPI.h"
#include "utility/utilCommonAPI.h"
#include "utility/Multilingual.h"
#include "network/Lock.h"
#include "Modules/IUserListModule.h"
#include "Modules/IEmotionModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/UI/SessionDialog.h"
#include "../Session/SessionManager.h"
#include "../Session/UI/UIIMEdit.h"
#include "cxImage/cxImage/ximage.h"
#include "json/reader.h"
#include <regex>
/******************************************************************************/
namespace
{
	const CString HTML_IMG_EMOTION_NORMAL_TAG = _T("<img src=\"%s\" width=\"22\" height=\"22\" />");
	const CString HTML_IMG_EMOTION_YAYA_TAG = _T("<img src=\"%s\" width=\"64\" height=\"70\" />");
	const CString HTML_IMG_IMAGE_TAG
		= _T("<a href=\"%s\" target=\"_blank\"><img title=\"%s\" src=\"%s\" style=\"max-width:%dpx;\" /></a>");
	const CString HTML_A_TAG = _T("<a class=\"purple_link\" href=\"%s\" target=\"_blank\">%s</a>");
	const CString HTML_CHAT2_TAG = _T("<a class=\"blue_link\" href=\"moguim://moguim/:chat2?%s\" target=\"_blank\">@%s</a>");//替换成连接
	const UInt8 MAX_RECEIVEMSG_CNT = 10;	//消息去重，每个会话缓存10条信息
}
// -----------------------------------------------------------------------------
//  ReceiveMsgManage: Public, Constructor

ReceiveMsgManage::ReceiveMsgManage()
{

}

// -----------------------------------------------------------------------------
//  ReceiveMsgManage: Public, Destructor

ReceiveMsgManage::~ReceiveMsgManage()
{

}

ReceiveMsgManage* ReceiveMsgManage::getInstance()
{
	static ReceiveMsgManage manager;
	return &manager;
}

BOOL ReceiveMsgManage::pushMessageBySId(const std::string& sId, MessageEntity& msg)
{
	if (sId.empty())
	{
		LOG__(ERR, _T("sId is empty!"));
		return FALSE;
	}
	CAutoLock lock(&m_lock);
	try
	{
		SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
		if (listChatMsg)
		{
			listChatMsg->push_back(msg);
		}
		else
		{
			SessionMessage_List listChatMsg2;
			listChatMsg2.push_back(msg);
			m_mapSessionMsg[sId] = listChatMsg2;
		}
	}
	catch (...)
	{
		CString csSid = util::stringToCString(sId);
		CString csMsgCnt = util::stringToCString(msg.content, CP_UTF8);
		LOG__(ERR, _T("pushMessageBySId failed,SId:%s,msg conteng:%s"), csSid, csMsgCnt);
		return FALSE;
	}
	return TRUE;
}
BOOL ReceiveMsgManage::popMessageBySId(const std::string& sId, MessageEntity& msg)
{
	CAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		msg = listChatMsg->front();
		if (MSG_TYPE_AUDIO_P2P == msg.msgType && !msg.isReaded())//如果是语音未读消息,先存起来
		{
			AudioMessageMananger::getInstance()->pushAudioMessageBySId(sId, msg);
		}
		listChatMsg->pop_front();
		return TRUE;
	}

	return FALSE;
}
BOOL ReceiveMsgManage::frontMessageBySId(const std::string& sId, MessageEntity& msg)
{
	CAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		msg = listChatMsg->back();
		return TRUE;
	}

	return FALSE;
}

BOOL ReceiveMsgManage::popMessagesBySId(IN const std::string& sId, OUT SessionMessage_List& msgList, IN MSG_TYPE_STATUS status /*= MESSAGE_TYPE_NONE*/, IN const UINT32 msgId/* = 0*/)
{
	CAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		SessionMessage_List::iterator it = listChatMsg->begin();
		while (it != listChatMsg->end())
		{
			if (MESSAGE_RENDERTYPE_AUDIO == it->msgRenderType && !it->isReaded())
			{
				AudioMessageMananger::getInstance()->pushAudioMessageBySId(sId, *it);
			}
			if (MESSAGE_TYPE_NONE == status || it->msgStatusType == status)
			{
				if (0 == msgId)//获取该会话符合条件的所有消息
				{
					msgList.push_back(*it);
					it = listChatMsg->erase(it);
				}
				else if (it->msgId <= msgId)//获取这个msgId之前的消息
				{
					msgList.push_back(*it);
					it = listChatMsg->erase(it);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}
		return TRUE;
	}

	return FALSE;
}

UInt32 ReceiveMsgManage::getUnReadMsgCountBySId(const std::string& sId)
{
	CAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg)
	{
		SessionMessage_List& msgList = *listChatMsg;
		int nUnReadCount = 0;
		for (MessageEntity msg:msgList)
		{//若是自己发的消息，则是多端同步的，不需要统计到未读计数中
			if (msg.talkerSid != module::getSysConfigModule()->userID())
			{
				++nUnReadCount;
			}
		}
		return nUnReadCount;
	}
	return 0;
}

UInt32 ReceiveMsgManage::getTotalUnReadMsgCount()
{
	UInt32 nTotalCount = 0;
	CAutoLock lock(&m_lock);
	for (auto itmap:m_mapSessionMsg)
	{
		const SessionMessage_List& msgList = itmap.second;
        for (MessageEntity msg : msgList)
        {
            if (MESSAGE_RENDERTYPE_SYSTEMTIPS != msg.msgRenderType)
            {
                ++nTotalCount;
            }
        }
	}
	return nTotalCount;
}

void ReceiveMsgManage::removeAllMessage()
{
	CAutoLock lock(&m_lock);
	//记录下程序退出时所有的消息列表，并且记录下来，这个会成为离线消息
	SessionMessageMap::iterator iterMap = m_mapSessionMsg.begin();
	LOG__(ERR, _T("MessageMananger unread message trace begin："));
	for (; iterMap != m_mapSessionMsg.end(); ++iterMap)
	{
		SessionMessage_List msgList;
		if (popMessagesBySId(iterMap->first, msgList))
		{
			SessionMessage_List::iterator iter = msgList.begin();
			for (; iter != msgList.end(); ++iter)
			{
				MessageEntity& msg = *iter;
				CString csUId = util::stringToCString(msg.talkerSid);
				CString csCnt = util::stringToCString(msg.content, CP_UTF8);
				CTime time(msg.msgTime);
				CString csTime = time.Format(_T("%Y-%m-%d %H:%M:%S"));
				LOG__(DEBG, _T("time:%s id:%s,content:%s"), csTime, csUId, csCnt);
			}
		}
	}
	LOG__(DEBG, _T("MessageMananger unread message trace end"));
	m_mapSessionMsg.clear();
}

void ReceiveMsgManage::removeMessageBySId(const std::string& sId)
{
	CAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		listChatMsg->clear();
	}
}
void ReceiveMsgManage::parseContent(CString& content, BOOL bFloatForm, Int32 chatWidth)
{
	_urlScan(content);
	if (!bFloatForm)
	{
		content.Replace(_T("<"), _T("&lt;"));
		content.Replace(_T(">"), _T("&gt;"));
		content.Replace(_T("\""), _T("&quot;"));
		content.Replace(_T("\'"), _T("&#039;"));
		//content.Replace(_T("\n"),_T("<br>"));
		for (int i = 0; i <= content.GetLength() - 1; i++)
		if (content.GetAt(i) == 0x0D)//如果是回车
		{
			CString strfront = content.Mid(0, i);
			CString strBack = content.Mid(i + 1, content.GetLength() - i - 1);
			content = strfront + _T("<br>") + strBack;
			i = i + 3;//替换的字符比原来多了3个，可以略过检查
		}
	}
	int hitStartIndex = content.Find(CS_SPLIT_CODE_START);
	if (hitStartIndex < 0 && !bFloatForm)
	{
		content.Replace(_T(" "), _T("&nbsp"));
	}

	//解析表情
	int startIndex = content.Find(_T("["));
	while (startIndex != -1)
	{
		int endIndex = content.Find(_T("]"), startIndex);
		if (-1 == endIndex)
			break;
		CString csEmotion = content.Mid(startIndex, endIndex - startIndex + 1);

		CString csPath;
		if (module::getEmotionModule()->getEmotionNameByID(csEmotion, csPath))
		{
			CString csHtml;
			if (bFloatForm)
			{
				csHtml = util::getMultilingual()->getStringById(_T("STRID_FLOATFORM_EMOTION"));
			}
			else
			{
				if (-1 != csEmotion.Find(_T("牙牙")))
				{
					csHtml.Format(HTML_IMG_EMOTION_YAYA_TAG, csPath);
				}
				else
					csHtml.Format(HTML_IMG_EMOTION_NORMAL_TAG, csPath);
			}
			content.Replace(csEmotion, csHtml);
		}
		startIndex = content.Find(_T("["), endIndex);
	}

	//替换图片
	BOOL bCanRelaceSpace = TRUE;
	startIndex = content.Find(CS_SPLIT_CODE_START);
	while (startIndex != -1)
	{
		int endIndex = content.Find(CS_SPLIT_CODE_END, startIndex);
		if (-1 == endIndex)
			break;
		CString csImgUrlPath = content.Mid(startIndex + CS_SPLIT_CODE_START.GetLength(), endIndex
			- startIndex - CS_SPLIT_CODE_START.GetLength());
		CString csImgPathTag = content.Mid(startIndex, endIndex - startIndex + CS_SPLIT_CODE_END.GetLength());
		CString csHtml;
		if (bFloatForm)
		{
			csHtml = util::getMultilingual()->getStringById(_T("STRID_FLOATFORM_IMAGE"));
		}
		else
		{
			chatWidth -= 130;
			CString csTips = util::getMultilingual()->getStringById(_T("STRID_SESSIONMODULE_MESSAGE_SOURCEIMAGE"));
			module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
			csHtml.Format(HTML_IMG_IMAGE_TAG, csImgUrlPath, csTips, csImgUrlPath, chatWidth);
		}
		content.Replace(csImgPathTag, csHtml);
		startIndex = content.Find(CS_SPLIT_CODE_START, endIndex);
	}
	_urlReplace(content);
	_Quickchat2Fromat(content);
}

SessionMessage_List* ReceiveMsgManage::_getChatMsgListBySID(const std::string& sId)
{
	CAutoLock lock(&m_lock);
	SessionMessageMap::iterator iter = m_mapSessionMsg.find(sId);
	if (iter == m_mapSessionMsg.end())
		return 0;

	return &(iter->second);
}

//////////////////////////////////消息去重///////////////////////////////////////
BOOL ReceiveMsgManage::checkIsReduplicatedMsg(IN const MessageEntity& msg, IN const UInt32 seqNo)
{
	ReceiveMsgMap::iterator itMap = m_MsgMap.find(msg.sessionId);
	if (itMap != m_MsgMap.end())
	{
		ReceiveMsgList& MsgList = itMap->second;
		BOOL bFind = FALSE;
		for (ReceiveMsgList::reverse_iterator itList = MsgList.rbegin();
			itList != MsgList.rend(); ++itList)
		{
			if (itList->msg.content == msg.content
				&& itList->seqNo == seqNo)
			{
				bFind = TRUE;
				return TRUE;
			}
		}

		if (!bFind)
		{
			if (MAX_RECEIVEMSG_CNT == MsgList.size())
			{
				MsgList.pop_front();
			}
			ReceiveMsg rMsg;
			rMsg.msg = msg;
			rMsg.seqNo = seqNo;
			MsgList.push_back(rMsg);
		}
	}
	else
	{
		ReceiveMsgList rMsglist;
		ReceiveMsg rMsg;
		rMsg.msg = msg;
		rMsg.seqNo = seqNo;
		rMsglist.push_back(rMsg);
		m_MsgMap[msg.sessionId] = rMsglist;
	}
	return FALSE;
}

void ReceiveMsgManage::_Quickchat2Fromat(OUT CString& content)
{
	int nStart = 0;
	while (TRUE)
	{
		int nLen = content.GetLength();
		int nPos = content.Find('@', nStart);
		if (-1 != nPos && nPos + 2 < nLen)
		{
			CString sName = content.Mid(nPos + 1, 2);
			BOOL bFind = FALSE;
			CString csHTML;
			module::UserInfoEntityMap MapUsers;
			module::getUserListModule()->getAllUsersInfo(MapUsers);
			for (auto itMapItem:MapUsers)
			{
				module::UserInfoEntity&  userInfo = itMapItem.second;
				if (userInfo.csNickName == sName)
				{
					//do替换
					csHTML.Format(HTML_CHAT2_TAG, util::stringToCString(userInfo.sId), sName);
					CString strA = content.Mid(0, nPos);
					CString strB = content.Mid(nPos + 3);
					content = strA + csHTML + strB;
					bFind = TRUE;
					break;
				}
			}
			if (bFind)
			{
				nStart = nPos + csHTML.GetLength();
			}
			else
			{
				nStart = nPos + 1;
			}
		}
		else
		{
			break;
		}
	}
}
void ReceiveMsgManage::_urlReplace(CString& content)
{
	std::vector<CString>::iterator iter = m_scanUrls.begin();
	int nStart = 0;
	for (; iter != m_scanUrls.end(); ++iter)
	{
		CString csHtml, csUrl(*iter);
		//对www开头的链接前面加http://
		if (csUrl.Find(_T("www.")) > -1 && csUrl.Find(_T("http://")) < 0)
		{
			csUrl = _T("http://") + csUrl;
		}
		csHtml.Format(HTML_A_TAG, csUrl, *iter);
		int nPos = content.Find(*iter, nStart);
		if (-1 != nPos)
		{
			CString strA = content.Mid(0, nPos);
			CString strB = content.Mid(nPos + iter->GetLength());
			content = strA + csHtml + strB;
			nStart = nPos + csHtml.GetLength();
		}
	}
}

void ReceiveMsgManage::_urlScan(CString& content)
{
	const std::wregex pattern(_T("((http[s]{0,1}|ftp)://[a-zA-Z0-9\\.\\-]+\\.([a-zA-Z]{2,4})(:\\d+)?(/[a-zA-Z0-9\\.\\-~!@#$%^&*+?:_/=<>]*)?)|(www.[a-zA-Z0-9\\.\\-]+\\.([a-zA-Z]{2,4})(:\\d+)?(/[a-zA-Z0-9\\.\\-~!@#$%^&*+?:_/=<>]*)?)"));
	std::wsmatch what;
	std::wstring result = content;
	m_scanUrls.clear();
	while (std::regex_search(result, what, pattern))
	{
		std::wstring s = what[0];
		m_scanUrls.push_back(CString(s.c_str()));
		result = what.suffix();
	}
}

void ReceiveMsgManage::_replaceUrlParts(CString& content)
{
	
}

/******************************************************************************/

//////////////////////////////语音消息接受处理////////////////////////////////////////////
AudioMessageMananger::~AudioMessageMananger()
{
}

AudioMessageMananger* AudioMessageMananger::getInstance()
{
	static AudioMessageMananger manager;
	return &manager;
}

BOOL AudioMessageMananger::playAudioMsgByAudioSid(IN const std::string& sSessionID, IN const std::string& sAID)
{
	if (sAID.empty() || sSessionID.empty())
	{
		LOG__(ERR, _T("date error,sSessionID:%s ,sAID:%s"), util::stringToCString(sSessionID), util::stringToCString(sAID));
		return FALSE;
	}
	//若是未读语音消息，则置成已读
	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(sSessionID);
	if (itAudio != m_mapUnReadAudioMsg.end())
	{
		SessionMessage_List& listChatMsg = itAudio->second;
		for (SessionMessage_List::iterator it = listChatMsg.begin();
			it != listChatMsg.end(); ++it)
		{
			if (it->content == sAID)
			{
				it->msgAudioReaded = 1;
			}
		}
	}

	//播放声音
	CString csFilePath = module::getMiscModule()->GetAudioFileSavedPath();
	csFilePath += util::stringToCString(sAID, CP_UTF8);
	if (!PathFileExists(csFilePath))
	{
		LOG__(ERR, _T("File not exist:%s "), csFilePath);
		return FALSE;
	}
	csFilePath = _T("\"") + csFilePath + _T("\"");

	//先关闭原来的播放窗口 
	HWND hTTWnd = FindWindow(NULL, _T("teamtalkaudio-speexdecWindow"));
	if (hTTWnd)
	{
		COPYDATASTRUCT cpyData = { 0 };
		//cpyData.lpData = (PVOID)m_sPlayingAID.c_str();
		//cpyData.cbData = m_sPlayingAID.length();
		cpyData.dwData = 1; // 1 表示退出语音播放进程
		::SendMessage(hTTWnd, WM_COPYDATA, 0, (LPARAM)&cpyData);
		LOG__(DEBG, _T("breakPlayingAnimate"));
	}

	//解析播放语音文件
	CString csPath = util::getAppPath() + _T("speexdec.exe");
	LOG__(DEBG, _T("playAudioMsgByAudioSid FilePath:%s"), csFilePath);
	::ShellExecute(NULL, _T("open"), csPath, csFilePath, _T(""), SW_HIDE);

	m_sPlayingSessionID = sSessionID;
	m_sPlayingAID = sAID;
	return TRUE;
}


BOOL AudioMessageMananger::makeAppAudioSid(IN const UInt32 msgId, IN const std::string sSessionId, OUT std::string& sAID)
{
	//CString strTime;
	//CTime timeData(msgTime);
	//strTime = timeData.Format(_T("-%Y%m%d-%H-%M-%S-"));

	//CString strGuid;
	//GUID guid;
	//if (CoCreateGuid(&guid))
	//{
	//	LOG__(ERR, _T("create guid error"));
	//	//return FALSE;
	//}
	//strGuid.Format(_T("%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X"),
	//	guid.Data1, guid.Data2, guid.Data3,
	//	guid.Data4[0], guid.Data4[1], guid.Data4[2],
	//	guid.Data4[3], guid.Data4[4], guid.Data4[5],
	//	guid.Data4[6], guid.Data4[7]);

	sAID = util::uint32ToString(msgId) + string("_") + sSessionId;
	return TRUE;
}
BOOL AudioMessageMananger::saveAudioDataToFile(IN UCHAR* data, IN UINT32 lenth, IN std::string sFileName)
{
	if (lenth <= 4)
	{
		LOG__(ERR, _T("lenth<=4"));
		return FALSE;
	}
	CString sFilePath = module::getMiscModule()->GetAudioFileSavedPath();

	if (!::PathIsDirectory(sFilePath))
	{
		util::createAllDirectories(sFilePath);
		sFilePath += _T("\\");
	}

	sFilePath += util::stringToCString(sFileName, CP_UTF8);

	if (util::isFileExist(sFilePath))
	{
		return TRUE;
	}

	HANDLE hFile = CreateFile(sFilePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD res = GetLastError();
		LOG__(ERR, _T("CreateFile error:GetLastError = %d"), res);
		return FALSE;
	}
	::SetFilePointer(hFile, 0, 0, FILE_END);

	DWORD lret = 0;
	BOOL  bRes = WriteFile(hFile, (LPCVOID)data, lenth, &lret, NULL);
	if (!bRes)
	{
		DWORD res = GetLastError();
		LOG__(ERR, _T("WriteFile error:GetLastError = %d"), res);
		return FALSE;
	}

	if (NULL != hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

	return TRUE;
}

BOOL AudioMessageMananger::getAudioMsgLenth(IN UCHAR* data, IN UINT32 lenth, OUT UInt8& AudioMsgLen)
{
	AudioMsgLen = 0;
	if (lenth < 4 || NULL == data)
	{
		//前4个字节为语音长度
		return FALSE;
	}
	AudioMsgLen = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
	if (0 == AudioMsgLen)
	{
		LOG__(ERR, _T("语音时长为零"));
	}
	return TRUE;
}

BOOL AudioMessageMananger::pushAudioMessageBySId(const std::string& sId, MessageEntity& msg)
{
	SessionMessageMap::iterator it = m_mapUnReadAudioMsg.find(sId);
	if (it == m_mapUnReadAudioMsg.end())
	{
		SessionMessage_List listChatMsg;
		listChatMsg.push_back(msg);
		m_mapUnReadAudioMsg[sId] = listChatMsg;
	}
	else
	{
		SessionMessage_List& listChatMsg = it->second;
		listChatMsg.push_back(msg);
	}
	return TRUE;
}

BOOL AudioMessageMananger::autoplayNextUnReadAudioMsg()
{
	//停掉之前播放的动画
	stopPlayingAnimate();
	LOG__(ERR, _T("stopPlayingAnimate"));

	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(m_sPlayingSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		LOG__(DEBG, _T("Not exist in the list!"));
		return FALSE;
	}

	SessionMessage_List& listChatMsg = itAudio->second;

	SessionMessage_List::iterator itNext = listChatMsg.end();
	SessionMessage_List::iterator it = listChatMsg.begin();
	for (; it != listChatMsg.end();)
	{
		if (it->isReaded() && it->content == m_sPlayingAID)
		{
			itNext = listChatMsg.erase(it);//删除已经播放过的
			break;
		}
		else
			++it;
	}

	if (itNext != listChatMsg.end())//说明是有效的ID
	{
		LOG__(DEBG, _T("AutoplayNextUnReadAudioMsg"));
		startPlayingAnimate(itNext->content);//大补鸡支持
		playAudioMsgByAudioSid(m_sPlayingSessionID, itNext->content);
		return TRUE;
	}
	return FALSE;
}

BOOL AudioMessageMananger::popPlayingAudioMsg()
{
	stopPlayingAnimate();

	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(m_sPlayingSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		LOG__(DEBG, _T("Not exist in the list!"));
		return FALSE;
	}
	SessionMessage_List& listChatMsg = itAudio->second;
	SessionMessage_List::iterator it = listChatMsg.begin();
	for (; it != listChatMsg.end();)
	{
		if (it->isReaded() && it->content == m_sPlayingAID)
		{
			listChatMsg.erase(it);//删除已经播放过的
			m_sPlayingSessionID.clear();
			m_sPlayingAID.clear();
			return TRUE;
		}
		else
			++it;
	}
	return FALSE;
}

BOOL AudioMessageMananger::clearAudioMsgBySessionID(IN const std::string sSessionID)
{
	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(sSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		LOG__(DEBG, _T("Not exist in the list!"));
		return FALSE;
	}
	SessionMessage_List& listChatMsg = itAudio->second;
	listChatMsg.clear();
	return TRUE;
}

BOOL AudioMessageMananger::stopPlayingAnimate()
{
	SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(m_sPlayingSessionID);
	if (pSessionDialog)
	{
		pSessionDialog->StopPlayingAnimate(m_sPlayingAID);
		return TRUE;
	}
	return FALSE;
}

BOOL AudioMessageMananger::startPlayingAnimate(IN const std::string& sToPlayAID)
{
	SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(m_sPlayingSessionID);
	if (pSessionDialog)
	{
		pSessionDialog->StartPlayingAnimate(sToPlayAID);
		return TRUE;
	}
	return FALSE;
}

