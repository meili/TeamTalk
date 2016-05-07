/*******************************************************************************
 *  @file      ISessionModule.h 2014\7\27 10:06:08 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__
#define ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__

#include "GlobalDefine.h"
#include "Modules/ModuleDll.h"
#include "Modules/ModuleBase.h"
#include "Modules/IModuleInterface.h"
/******************************************************************************/
namespace DuiLib
{
	class CControlUI;
	class CPaintManagerUI;
}
class MessageEntity;
NAMESPACE_BEGIN(module)
const std::string MODULE_SESSION_PREFIX = "session";

//KEYID
const std::string KEY_SESSION_NEWMESSAGE		= MODULE_SESSION_PREFIX + "NewMessage";			//接收到消息，包括运行时消息、离线消息
const std::string KEY_SESSION_OPENNEWSESSION	= MODULE_SESSION_PREFIX + "OpenNewSession";     //通知打开一个新的会话
const std::string KEY_SESSION_SENDMSG_TOOFAST	= MODULE_SESSION_PREFIX + "SendMsgTooFast";     //发送消息太快
const std::string KEY_SESSION_SENDMSG_FAILED	= MODULE_SESSION_PREFIX + "SendMsgFialed";      //发送消息失败
const std::string KEY_SESSION_SHAKEWINDOW_MSG	= MODULE_SESSION_PREFIX + "ShakeWindowMsg";		//抖动主窗口
const std::string KEY_SESSION_WRITING_MSG		= MODULE_SESSION_PREFIX + "WritingMsg";			//正在输入
const std::string KEY_SESSION_STOPWRITING_MSG	= MODULE_SESSION_PREFIX + "StopWrtingMsg";		//停止了正在输入
const std::string KEY_SESSION_SENDMSGTIP_KEY	= MODULE_SESSION_PREFIX + "SendMsgTipKey";		//发送消息键值改变
const std::string KEY_SESSION_TRAY_STARTEMOT	= MODULE_SESSION_PREFIX + "TrayStartEmot";		//开启托盘图标闪烁
const std::string KEY_SESSION_TRAY_STOPEMOT		= MODULE_SESSION_PREFIX + "TrayStopEmot";		//关闭托盘图标闪烁
const std::string KEY_SESSION_TRAY_NEWMSGSEND	= MODULE_SESSION_PREFIX + "TrayNewMsgSend"; 	//发送了一个消息，最近联系人更新
const std::string KEY_SESSION_TRAY_COPYDATA		= MODULE_SESSION_PREFIX + "TrayCopyData";		//语音播放模块发过来数据通知
const std::string KEY_SESSION_HISTORY_MESSAGE	= MODULE_SESSION_PREFIX + "HistoryMessage";		//接收到历史消息
const std::string KEY_SESSION_MESSGEREADED_NOTIFY = MODULE_SESSION_PREFIX + "MessageReadEdNotify";//其他端读取了部分的消息
const std::string KEY_SESSION_UPDATE_RECENTLY_SESSIONLIST = MODULE_SESSION_PREFIX + "UpdateRecentlySessionList";		//更新最近会话列表内容
const std::string KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT = MODULE_SESSION_PREFIX + "UpdateTotalUnReadMsgCount";		//更新总未读计数

/////////////////////////////////SessionEntity/////////////////////////////////////////
enum
{
	SESSION_ERRORTYPE = 0,
	SESSION_USERTYPE,		//个人会话
	SESSION_GROUPTYPE,		//群会话
};
class SessionEntity
{
public:
	SessionEntity();
	std::string getAvatarPath();
	UInt8 getOnlineState();
	CString getName();
	void setUpdatedTimeByServerTime();
	std::string getOriginSessionId();
	UInt32 getOriginIntegerSessionId();
public:
	UInt8								sessionType;		//SESSION_USERTYPE / SESSION_GROUPTYPE
	UInt32                              updatedTime;      //消息最后更新时间
	std::string                         sessionID;				//会话ID
	UInt32								latestmsgId = 0;
	std::string							latestMsgContent;			//最后一条消息内容
	std::string							latestMsgFromId;			//发送最后一条消息的人员ID
	UInt8								sessionStatus;				//是否被删除
};
typedef std::map<std::string, SessionEntity>    SessionEntityMap;
/////////////////////////////////ISessionModule/////////////////////////////////////////
/**
 * The class <code>ISessionModule</code> 
 *
 */
class MODULE_API ISessionModule : public module::ModuleBase
								 ,public module::IPduPacketParse
{
public:
	virtual DuiLib::CControlUI* createMainDialogControl(
		LPCTSTR pstrClass,DuiLib::CPaintManagerUI& paintManager) = 0;

	/**
	* 用户模块初始化处理（加载本地用户数据）
	*
	* @return 
	* @exception there is no any exception to throw.
	*/
	virtual BOOL startup() = 0;


	/**@name 同步服务器时间*/
	//@{
	virtual UInt32 getTime()const = 0;
	virtual void setTime(IN UInt32 time) = 0;
	virtual void startSyncTimeTimer() = 0;
	//@}

	/**@name SessionEntityManager委派包装*/
	//@{
	virtual void setSessionEntity(IN const module::SessionEntity& sessionInfo) = 0;//更新会话，若不存在，则会创建
	virtual void getRecentSessionList(OUT std::vector<std::string>& vecRecentSession) = 0;//程序启动后，用户展示最近会话列表
	virtual BOOL getSessionEntityBySId(IN const std::string& sId, OUT module::SessionEntity& sessionEntity) = 0;
	virtual void updateSessionEntityByMsg(IN const MessageEntity& msg) = 0;
	virtual void deleteSessionEntity(IN const std::string& sessionId) = 0;
	virtual UInt32 getGlobalUpdateTime()const = 0;//从服务器获取最近更新的会话列表，需要提交最近一次更新会话的时间
	virtual void setGlobalUpdateTime(IN const UInt32 updateTime)const = 0;
	//@}

};

MODULE_API ISessionModule* getSessionModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__
