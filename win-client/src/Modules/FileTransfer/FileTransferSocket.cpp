/******************************************************************************* 
 *  @file      FileTransferSocket.cpp 2014\8\30 13:32:37 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     文件传输socket
 ******************************************************************************/

#include "StdAfx.h"
#include "FileTransferSocket.h"
#include "TransferManager.h"
#include "network/imconn.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISysConfigModule.h"
#include "utility/utilCommonAPI.h"
#include "TransferFile.h"
#include "ProtocolBuffer/IM.File.pb.h"
#include "ProtocolBuffer/IM.Other.pb.h"
#include "network/TTPBHeader.h"
#include "network/ImCore.h"
/******************************************************************************/
FileTransferSocket::FileTransferSocket(std::string& taskId)
: m_pPingTimer(0)
, m_sTaskId(taskId)
{

}

FileTransferSocket::~FileTransferSocket(void)
{

}

BOOL FileTransferSocket::connect(const CString &linkaddr, UInt16 port)
{
    m_socketHandle = imcore::IMLibCoreConnect(util::cStringToString(linkaddr), port);
    imcore::IMLibCoreRegisterCallback(m_socketHandle, this);

	return TRUE;
}

BOOL FileTransferSocket::shutdown()
{
    stopHeartbeat();
    //_stopServerPingTimer();
    imcore::IMLibCoreShutdown(m_socketHandle);
	return TRUE;
}

void FileTransferSocket::sendPacket(IN UInt16 moduleId, IN UInt16 cmdId, IN google::protobuf::MessageLite* pbBody)
{
    m_TTPBHeader.clear();
    m_TTPBHeader.setModuleId(moduleId);
    m_TTPBHeader.setCommandId(cmdId);

    UInt32 length = imcore::HEADER_LENGTH + pbBody->ByteSize();//设置包长度
    m_TTPBHeader.setLength(length);

    std::unique_ptr<byte> data(new byte[length]);
    ZeroMemory(data.get(), length);
    memcpy(data.get(), m_TTPBHeader.getSerializeBuffer(), imcore::HEADER_LENGTH);//拷贝包头

    if (!pbBody->SerializeToArray(data.get() + imcore::HEADER_LENGTH, pbBody->ByteSize()))
    {
        LOG__(ERR, _T("pbBody SerializeToArray failed"));
        return;
    }
    imcore::IMLibCoreWrite(m_socketHandle, data.get(), length);
}

void FileTransferSocket::startHeartbeat() 
{
	if (!m_pPingTimer)
		m_pPingTimer = new PingFileSevTimer(this);
	module::getEventManager()->scheduleTimer(m_pPingTimer, 5, TRUE);
}

void FileTransferSocket::stopHeartbeat()
{
	if (m_pPingTimer)
		module::getEventManager()->killTimer(m_pPingTimer);
	m_pPingTimer = 0;
}

void FileTransferSocket::onReceiveData(const char* data, int32_t size)
{
    std::string pbBody;
    imcore::TTPBHeader pbHeader; 
	try
	{
        pbHeader.unSerialize((byte*)data, imcore::HEADER_LENGTH);
        pbBody.assign(data + imcore::HEADER_LENGTH, size - imcore::HEADER_LENGTH);

        if (IM::BaseDefine::OtherCmdID::CID_OTHER_HEARTBEAT == pbHeader.getCommandId()
            && IM::BaseDefine::ServiceID::SID_OTHER == pbHeader.getModuleId())
            return;
	}
	catch (CPduException e)
	{
		LOG__(ERR, _T("onPacket CPduException serviceId:%d,commandId:%d,errCode:%d")
			, e.GetModuleId(), e.GetCommandId(), e.GetErrorCode());
		return;
	}
	catch (...)
	{
		LOG__(ERR, _T("FileTransferSocket onPacket unknown exception"));
		return;
	}
    UInt16 ncmdid = pbHeader.getCommandId();
    switch (ncmdid)
	{	 
    case IM::BaseDefine::FileCmdID::CID_FILE_LOGIN_RES:
        _fileLoginResponse(pbBody);
		break;
    case IM::BaseDefine::FileCmdID::CID_FILE_PULL_DATA_REQ://发文件
        _filePullDataReqResponse(pbBody);
		break;
    case IM::BaseDefine::FileCmdID::CID_FILE_PULL_DATA_RSP://收文件
        _filePullDataRspResponse(pbBody);
		break;
    case IM::BaseDefine::FileCmdID::CID_FILE_STATE://
        _fileState(pbBody);
	default:
		break;
	}
}


void FileTransferSocket::onConnectDone()
{
	LOG__(APP, _T("FileTransferSocket::onConnected()"));
	startHeartbeat();

	TransferFileEntity info;
    if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(m_sTaskId, info))
    {
        LOG__(APP, _T("Can't get the file info,task id:%s"),util::stringToCString(m_sTaskId));
        return;
    }
		
	//拉模式文件传输，传输taskid、token、client_mode
	IM::File::IMFileLoginReq imFileLoginReq;
	imFileLoginReq.set_user_id(module::getSysConfigModule()->userId());
	imFileLoginReq.set_task_id(info.sTaskID);
	imFileLoginReq.set_file_role(static_cast<IM::BaseDefine::ClientFileRole>(info.nClientMode));

    LOG__(APP, _T("IMFileLoginReq,sTaskID:%s,nClientMode:%d"), util::stringToCString(info.sTaskID), info.nClientMode);
	//send packet
    LOG__(APP, _T("IMFileLoginReq,taskId:%s"), util::stringToCString(info.sTaskID));
    sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_LOGIN_REQ, &imFileLoginReq);

	//CImPduClientFileLoginReq pduFileLoginReq(module::getSysConfigModule()->userID().c_str()
	//	, "", info.sTaskID.c_str(), );
	//sendPacket(&pduFileLoginReq);
}
void FileTransferSocket::onClose()
{
	stopHeartbeat();
}


BOOL FileTransferSocket::startFileTransLink()
{
	TransferFileEntity FileInfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(m_sTaskId, FileInfo))
	{
		//大佛：使用msg server 传过来的IP和端口
		LOG__(APP, _T("connect IP=%s,Port=%d"), util::stringToCString(FileInfo.sIP), FileInfo.nPort);
		connect(util::stringToCString(FileInfo.sIP), FileInfo.nPort);
		//connect(util::stringToCString(module::FILETRANSFER_IP), module::FILETRANSFER_PORT);
		return TRUE;
	}
	LOG__(ERR, _T("can't find the TaskId"));
	return FALSE;
}

void FileTransferSocket::stopfileTransLink()
{
	shutdown();
	stopHeartbeat();
}

void FileTransferSocket::_fileLoginResponse(IN std::string& body)
{
	IM::File::IMFileLoginRsp imFileLoginRsp;
    if (!imFileLoginRsp.ParseFromString(body))
    {
        LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
        return;
    }
	if (imFileLoginRsp.result_code() != 0)
	{
		LOG__(ERR, _T("file server login failed! "));
		return;
	}
	//打开文件
	std::string taskId = imFileLoginRsp.task_id();
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		LOG__(ERR, _T("file server login:can't find the fileInfo "));
		return;
	}

	LOG__(APP, _T("IMFileLoginRsp, file server login succeed"));
	//提示界面,界面上插入该项
	if (IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER == fileEntity.nClientMode
		|| IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == fileEntity.nClientMode)
	{
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILETRANSFER_SENDFILE, fileEntity.sTaskID);
	}
	else if (IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_RECVER == fileEntity.nClientMode
		|| IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD == fileEntity.nClientMode)
	{
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILETRANSFER_REQUEST, fileEntity.sTaskID);
	}
}

void FileTransferSocket::_filePullDataReqResponse(IN std::string& body)//发
{
	IM::File::IMFilePullDataReq imFilePullDataReq;
    if (!imFilePullDataReq.ParseFromString(body))
    {
        LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
        return;
    }
	UInt32 fileSize = imFilePullDataReq.data_size();
	UInt32 fileOffset = imFilePullDataReq.offset();
	std::string taskId = imFilePullDataReq.task_id();
	
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		LOG__(ERR, _T("PullDataReqResponse: can't find the fileInfo"));
		return;
	}
	LOG__(DEBG, _T("send:taskId=%s,filesize=%d,name=%s,BolckSize=%d")
		,util::stringToCString(fileEntity.sTaskID)
		,fileEntity.nFileSize
		,fileEntity.getRealFileName()
        ,fileSize);
	std::string buff;
	if (nullptr == fileEntity.pFileObject)
	{
		LOG__(ERR, _T("PullDataReqResponse: file boject Destoryed!"));
		return;
	}
	fileEntity.pFileObject->readBlock(fileOffset, fileSize, buff);//读取本地文件的数据块
	IM::File::IMFilePullDataRsp imFilePullDataRsp;//todo check
    imFilePullDataRsp.set_result_code(0);
	imFilePullDataRsp.set_task_id(taskId);
	imFilePullDataRsp.set_user_id(util::stringToInt32(fileEntity.sFromID));
	imFilePullDataRsp.set_offset(fileOffset);
    imFilePullDataRsp.set_file_data((void*)buff.data(), fileSize);

    //send packet
    sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_PULL_DATA_RSP
        , &imFilePullDataRsp);

	fileEntity.nProgress = fileOffset + fileSize;
	if (fileEntity.nProgress < fileEntity.nFileSize)
	{
		//更新进度条
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);//保存当前进度
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_PROGRESSBAR
            , fileEntity.sTaskID);
	}
	else//传输完成
	{
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED
            , fileEntity.sTaskID);
	}
	TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
}

void FileTransferSocket::_filePullDataRspResponse(IN std::string& body)//收
{
	IM::File::IMFilePullDataRsp imFilePullDataRsp;
    if (!imFilePullDataRsp.ParseFromString(body))
    {
        LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
        return;
    }
	UInt32 nRes = imFilePullDataRsp.result_code();
	if (0 != nRes)
	{
		LOG__(ERR, _T("PullDataRspResponse: error result:%d"),nRes);
		return;
	}
	std::string taskId = imFilePullDataRsp.task_id();
	const std::string& strData = imFilePullDataRsp.file_data();//todo ？？？？？要长度
	void* pData = (void*)(strData.data());
	UInt32 nBlockSize = strData.size();
	UInt32 fileOffset = imFilePullDataRsp.offset();
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		LOG__(ERR, _T("can't find the fileInfo"));
		return;
	}
	LOG__(DEBG, _T("receive:taskId=%s,filesize=%d,name=%s,BolckSize=%d")
		, util::stringToCString(fileEntity.sTaskID)
		, fileEntity.nFileSize
		, fileEntity.getRealFileName()
        , nBlockSize);

	//存文件...
	if (!fileEntity.pFileObject->writeBlock(fileOffset, nBlockSize, pData))
	{
		LOG__(DEBG, _T("writeBlock failed "));
		return;
	}

	fileEntity.nProgress = fileOffset + nBlockSize;
	if (fileEntity.nProgress < fileEntity.nFileSize)
	{
		//更新进度条
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);//保存当前进度
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_PROGRESSBAR
            , fileEntity.sTaskID);

		//继续发file block req...
		int mode = fileEntity.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD ? IM::BaseDefine::TransferFileType::FILE_TYPE_OFFLINE : IM::BaseDefine::TransferFileType::FILE_TYPE_ONLINE;
		IM::File::IMFilePullDataReq imFilePullDataReq;
		imFilePullDataReq.set_task_id(taskId);
		imFilePullDataReq.set_user_id(util::stringToInt32(fileEntity.sToID));
		imFilePullDataReq.set_trans_mode(static_cast<IM::BaseDefine::TransferFileType>(mode));
		imFilePullDataReq.set_offset(fileEntity.nProgress);

        UInt32 pullSize = fileEntity.nFileSize - fileEntity.nProgress;
        pullSize > nBlockSize ? imFilePullDataReq.set_data_size(nBlockSize) : imFilePullDataReq.set_data_size(pullSize);
		
		// 发包
        sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_PULL_DATA_REQ, &imFilePullDataReq);
	}
	else//传输完成
	{
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}

        //告知对方文件传输完成了。
        IM::File::IMFileState imFileState;
        imFileState.set_state(IM::BaseDefine::ClientFileState::CLIENT_FILE_DONE);
        imFileState.set_task_id(taskId);
        imFileState.set_user_id(util::stringToInt32(fileEntity.sToID));
        sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_STATE, &imFileState);

		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, fileEntity.sTaskID);
	}
}

void FileTransferSocket::_fileState(IN std::string& body)
{
	IM::File::IMFileState imFileState;
    if (!imFileState.ParseFromString(body))
    {
        LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
        return;
    }
	UINT32 nfileState = imFileState.state();

	std::string taskId = imFileState.task_id();
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		LOG__(ERR, _T("fileState:can't find the fileInfo "));
		return;
	}

	switch (nfileState)
	{
	case IM::BaseDefine::ClientFileState::CLIENT_FILE_PEER_READY:
		LOG__(APP, _T("fileState--CLIENT_FILE_PEER_READY "));
		break;
	case IM::BaseDefine::ClientFileState::CLIENT_FILE_CANCEL ://取消的了文件传输
		LOG__(APP, _T("fileState--CLIENT_FILE_CANCEL "));
		{
            if (fileEntity.pFileObject)
            {
                delete fileEntity.pFileObject;
                fileEntity.pFileObject = nullptr;
            }
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
			module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPLOAD_CANCEL, fileEntity.sTaskID);
		}
		break;
	case IM::BaseDefine::ClientFileState::CLIENT_FILE_REFUSE://拒绝了文件
		LOG__(APP, _T("fileState--CLIENT_FILE_REFUSE "));
		{
            if (fileEntity.pFileObject)
            {
                delete fileEntity.pFileObject;
                fileEntity.pFileObject = nullptr;
            }
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
			module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPLOAD_REJECT, fileEntity.sTaskID);
		}
		break;
	case IM::BaseDefine::ClientFileState::CLIENT_FILE_DONE:
		LOG__(APP, _T("fileState--CLIENT_FILE_DONE "));
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, fileEntity.sTaskID);
		break;
	default:
		break;
	}
}

void FileTransferSocket::onReceiveError()
{

}

//////////////////////////////////////////////////////////////////////////
void PingFileSevTimer::release()
{
	delete this;
}

void PingFileSevTimer::process()
{
	imcore::IMLibCoreStartOperationWithLambda(
		[=]()
	{
        IM::Other::IMHeartBeat imHearBeat;
        m_pFileTransSocket->sendPacket(IM::BaseDefine::ServiceID::SID_OTHER
            , IM::BaseDefine::OtherCmdID::CID_OTHER_HEARTBEAT, &imHearBeat);
	}
		);
}

PingFileSevTimer::PingFileSevTimer(FileTransferSocket* pTransSocket)
:m_pFileTransSocket(pTransSocket)
{

}

/******************************************************************************/
