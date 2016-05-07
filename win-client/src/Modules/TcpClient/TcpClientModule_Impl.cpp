/******************************************************************************* 
 *  @file      TcpClientModule_Impl.cpp 2014\7\29 13:16:48 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "TcpClientModule_Impl.h"
#include "utility/utilStrCodingAPI.h"
#include "network/ImCore.h"
#include "ProtocolBuffer/im.Login.pb.h"
#include "ProtocolBuffer/im.Other.pb.h"
#include "Modules/IModuleInterface.h"
#include "Modules/ILoginModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IP2PCmdModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/UIEventManager.h"
/******************************************************************************/
namespace module
{
	ITcpClientModule* getTcpClientModule()
	{
		static TcpClientModule_Impl module;
		return &module;
	}
}
namespace
{
	UInt16 g_seqNum = 0;

	module::IPduPacketParse* __getModule(UInt16 moduleId)
	{
		module::IPduPacketParse* pRet = 0;
		switch (moduleId)
		{
		case IM::BaseDefine::SID_LOGIN:
			pRet = module::getLoginModule();
			break;
		case IM::BaseDefine::SID_BUDDY_LIST:
			pRet = module::getUserListModule();
			break;
		case IM::BaseDefine::SID_MSG:
			pRet = module::getSessionModule();
			break;
		case IM::BaseDefine::SID_SWITCH_SERVICE:
			pRet = module::getP2PCmdModule();
			break;
		case IM::BaseDefine::SID_GROUP:
			pRet = module::getGroupListModule();
			break;
        case IM::BaseDefine::SID_FILE: //文件传输
            pRet = module::getFileTransferModule();
            break;
		default:
			break;
		}

		return pRet;
	}
}

// -----------------------------------------------------------------------------
//  TcpClientModule_Impl: Public, Constructor

TcpClientModule_Impl::TcpClientModule_Impl()
:m_tcpClientState(TCPCLIENT_STATE_OK)
,m_pImLoginResp(0)
,m_pHearbeatTimer(0)
,m_pServerPingTimer(0)
,m_bDoReloginServerNow(FALSE)
{
	m_eventReceived = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_eventConnected = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_pImLoginResp = new IM::Login::IMLoginRes();
}

// -----------------------------------------------------------------------------
//  TcpClientModule_Impl: Public, Destructor

TcpClientModule_Impl::~TcpClientModule_Impl()
{
	CloseHandle(m_eventConnected);
	CloseHandle(m_eventReceived);
	delete m_pImLoginResp;
	m_pImLoginResp = 0;
}

void TcpClientModule_Impl::onClose()
{
	LOG__(ERR, _T("on socket onClose,handle:%d"),m_socketHandle);
	m_tcpClientState = TCPCLIENT_STATE_DISCONNECT;
	module::getTcpClientModule()->asynNotifyObserver(KEY_TCPCLIENT_STATE);
	_stopHearbeat();
	
	imcore::IMLibCoreClose(m_socketHandle);
	imcore::IMLibCoreUnRegisterCallback(m_socketHandle);
}

void TcpClientModule_Impl::onReceiveData(const char* data, int32_t size)
{
	if (m_pServerPingTimer)
		m_pServerPingTimer->m_bHasReceivedPing = TRUE;

	imcore::TTPBHeader header;
	header.unSerialize((byte*)data, imcore::HEADER_LENGTH);	
	if (IM::BaseDefine::CID_OTHER_HEARTBEAT == header.getCommandId() && IM::BaseDefine::SID_OTHER == header.getModuleId())
	{
		//模块器端过来的心跳包，不跳到业务层派发
		return;
	}

	LOG__(NET, _T("receiveData message moduleId:0x%x,commandId:0x%x")
		, header.getModuleId(), header.getCommandId());

	if (g_seqNum == header.getSeqNumber())
	{
		m_pImLoginResp->ParseFromArray(data + imcore::HEADER_LENGTH, size - imcore::HEADER_LENGTH);
		::SetEvent(m_eventReceived);
		return;
	}

	//将网络包包装成任务放到逻辑任务队列里面去
	_handlePacketOperation(data, size);
}

void TcpClientModule_Impl::onReceiveError()
{
}

void TcpClientModule_Impl::onConnectDone()
{
	m_tcpClientState = TCPCLIENT_STATE_OK;
	::SetEvent(m_eventConnected);

	m_bDoReloginServerNow = FALSE;
	if (!m_pServerPingTimer)
	{
		_startServerPingTimer();
	}
}

IM::Login::IMLoginRes* TcpClientModule_Impl::doLogin(CString &linkaddr, UInt16 port
	,CString& uName,std::string& pass)
{
	m_socketHandle = imcore::IMLibCoreConnect(util::cStringToString(linkaddr), port);
	imcore::IMLibCoreRegisterCallback(m_socketHandle, this);
	if(util::waitSingleObject(m_eventConnected, 5000))
	{
		IM::Login::IMLoginReq imLoginReq;
		string& name = util::cStringToString(uName);
		imLoginReq.set_user_name(name);
		imLoginReq.set_password(pass);
		imLoginReq.set_online_status(IM::BaseDefine::USER_STATUS_ONLINE);
		imLoginReq.set_client_type(IM::BaseDefine::CLIENT_TYPE_WINDOWS);
		imLoginReq.set_client_version("win_10086");

		if (TCPCLIENT_STATE_OK != m_tcpClientState)
			return 0;

		sendPacket(IM::BaseDefine::SID_LOGIN, IM::BaseDefine::CID_LOGIN_REQ_USERLOGIN, ++g_seqNum
			, &imLoginReq);
		m_pImLoginResp->Clear();
		util::waitSingleObject(m_eventReceived, 10000);
	}

	return m_pImLoginResp;
}

void TcpClientModule_Impl::shutdown()
{
	_stopHearbeat();
	_stopServerPingTimer();

	imcore::IMLibCoreShutdown(m_socketHandle);
}

void TcpClientModule_Impl::sendPacket(UInt16 moduleId, google::protobuf::MessageLite* pbBody)
{
	m_TTPBHeader.clear();
	m_TTPBHeader.setModuleId(moduleId);

	_sendPacket(pbBody);
}

void TcpClientModule_Impl::sendPacket(UInt16 moduleId, UInt16 cmdId, google::protobuf::MessageLite* pbBody)
{
	m_TTPBHeader.clear();
	m_TTPBHeader.setModuleId(moduleId);
	m_TTPBHeader.setCommandId(cmdId);

	_sendPacket(pbBody);
}

void TcpClientModule_Impl::sendPacket(UInt16 moduleId, UInt16 cmdId, UInt16 seq, google::protobuf::MessageLite* pbBody)
{
	m_TTPBHeader.clear();
	m_TTPBHeader.setModuleId(moduleId);
	m_TTPBHeader.setCommandId(cmdId);
	m_TTPBHeader.setSeqNumber(seq);

	_sendPacket(pbBody);
}

void TcpClientModule_Impl::_sendPacket(google::protobuf::MessageLite* pbBody)
{
	UInt32 length = imcore::HEADER_LENGTH + pbBody->ByteSize();
	m_TTPBHeader.setLength(length);
	std::unique_ptr<byte> data(new byte[length]);
	memset(data.get(), 0, length);
	memcpy(data.get(), m_TTPBHeader.getSerializeBuffer(), imcore::HEADER_LENGTH);
	if (!pbBody->SerializeToArray(data.get() + imcore::HEADER_LENGTH, pbBody->ByteSize()))
	{
		LOG__(ERR, _T("pbBody SerializeToArray failed"));
		return;
	}
	imcore::IMLibCoreWrite(m_socketHandle, data.get(), length);
}

void TcpClientModule_Impl::startHeartbeat()
{
	module::getEventManager()->scheduleTimerWithLambda(5, TRUE,
		[=]()
	{
		imcore::IMLibCoreStartOperationWithLambda(
			[=]()
		{
			IM::Other::IMHeartBeat imHearBeat;
			sendPacket(IM::BaseDefine::SID_OTHER, IM::BaseDefine::CID_OTHER_HEARTBEAT, &imHearBeat);
		}
		);
	}
	, &m_pHearbeatTimer);
}

void TcpClientModule_Impl::_stopHearbeat()
{
	if (m_pHearbeatTimer)
		module::getEventManager()->killTimer(m_pHearbeatTimer);
	m_pHearbeatTimer = 0;
}

void TcpClientModule_Impl::_handlePacketOperation(const char* data, UInt32 size)
{
	std::string copyInBuffer(data, size);
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
		imcore::TTPBHeader header;
		header.unSerialize((byte*)copyInBuffer.data(),imcore::HEADER_LENGTH);

		module::IPduPacketParse* pModule
			= (module::IPduPacketParse*)__getModule(header.getModuleId());
		if (!pModule)
		{
			assert(FALSE);
			LOG__(ERR, _T("module is null, moduleId:%d,commandId:%d")
				, header.getModuleId(), header.getCommandId());
			return;
		}
		std::string pbBody(copyInBuffer.data() + imcore::HEADER_LENGTH, size - imcore::HEADER_LENGTH);
		pModule->onPacket(header, pbBody);
	});
}

void TcpClientModule_Impl::_startServerPingTimer()
{
	if (!m_pServerPingTimer)
		m_pServerPingTimer = new ServerPingTimer(this);
	module::getEventManager()->scheduleTimer(m_pServerPingTimer, 60, TRUE);
}

void TcpClientModule_Impl::_stopServerPingTimer()
{
	if (m_pServerPingTimer)
		module::getEventManager()->killTimer(m_pServerPingTimer);
	m_pServerPingTimer = 0;
}

void TcpClientModule_Impl::_doReloginServer()
{
	LOG__(APP, _T("doReloginServer now!!!"));
	m_bDoReloginServerNow = TRUE;
	onClose();
}

UInt8 TcpClientModule_Impl::getTcpClientNetState() const
{
	return m_tcpClientState;
}

ServerPingTimer::ServerPingTimer(TcpClientModule_Impl* pTcpClient)
:m_pTcpClient(pTcpClient)
,m_bHasReceivedPing(FALSE)
{

}

void ServerPingTimer::process()
{
	if (!m_bHasReceivedPing && !m_pTcpClient->m_bDoReloginServerNow)
	{
		m_pTcpClient->_doReloginServer();
	}
	m_bHasReceivedPing = FALSE;
}

void ServerPingTimer::release()
{
	delete this;
}
/******************************************************************************/