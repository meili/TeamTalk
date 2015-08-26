/******************************************************************************* 
 *  @file      FileTransfer_Impl.cpp 2014\8\26 11:53:09 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ISysConfigModule.h"
#include "FileTransferModule_Impl.h"
#include "FileTransferDialog.h"
#include "TransferFile.h"
#include "utility/Multilingual.h"
#include "FileTransfer/TransferManager.h"
#include "ProtocolBuffer/IM.File.pb.h"
#include "network/TTPBHeader.h"
#include "network/ImCore.h"
/******************************************************************************/
namespace module
{
	IFileTransferModule* getFileTransferModule()
	{
		static FileTransferModule_Impl module;
		return &module;
	}
}

// -----------------------------------------------------------------------------
//  FileTransfer_Impl: Public, Constructor

FileTransferModule_Impl::FileTransferModule_Impl()
:m_pFileTransferDialog(0)
{
    TransferFileEntityManager::getInstance()->startup();
    addObserver(this, BIND_CALLBACK_2(FileTransferModule_Impl::MKOForFileTransferModuleCallback));
}

// -----------------------------------------------------------------------------
//  FileTransfer_Impl: Public, Destructor

FileTransferModule_Impl::~FileTransferModule_Impl()
{
	std::list<std::string> fileIdList;
	TransferFileEntityManager::getInstance()->getAllTransferFileEntityFileID(fileIdList);
	for (std::string fileId : fileIdList)
	{
		doCancel(fileId);
	}
	TransferFileEntityManager::getInstance()->shutdown();

	module::getFileTransferModule()->removeObserver(this);
}

void FileTransferModule_Impl::onPacket(imcore::TTPBHeader& header, std::string& pbBody)
{
	switch (header.getCommandId())
	{
	case IM::BaseDefine::FileCmdID::CID_FILE_RESPONSE://发送“文件请求/离线文件”-返回
		_sendfileResponse(pbBody);
		break;
	case IM::BaseDefine::FileCmdID::CID_FILE_NOTIFY://收到“发送文件请求”
		_fileNotify(pbBody);
		break;
	case IM::BaseDefine::FileCmdID::CID_FILE_HAS_OFFLINE_RES:
		_hasOfflineRes(pbBody);
		break;
	default:
		return;
	}
}

void FileTransferModule_Impl::showFileTransferDialog()
{
	BOOL bRet = FALSE;
	if (!m_pFileTransferDialog)
	{
		m_pFileTransferDialog = new FileTransferDialog;
		CString csTitle = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_CAPTION"));
		m_pFileTransferDialog->Create(NULL, csTitle, UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
		m_pFileTransferDialog->CenterWindow();
	}

	m_pFileTransferDialog->BringToTop();
}
void FileTransferModule_Impl::MKOForFileTransferModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_FILETRANSFER_REQUEST == keyId
		|| module::KEY_FILETRANSFER_SENDFILE == keyId)
	{
		if (m_pFileTransferDialog)
		{
			std::string& sFileId = std::get<MKO_STRING>(mkoParam);
			m_pFileTransferDialog->AddTransderItemToList(sFileId);
			m_pFileTransferDialog->BringToTop();
		}
		else
		{
			showFileTransferDialog();
		}
	}
}

BOOL FileTransferModule_Impl::sendFile(IN const CString& sFilePath, IN const std::string& sSendToSID,IN BOOL bOnlineMode)
{
	if (TransferFileEntityManager::getInstance()->checkIfIsSending(sFilePath))
	{
		return FALSE;
	}
	TransferFileEntity fileEntity;
	
	//获取文件大小
	struct __stat64 buffer;
	_wstat64(sFilePath, &buffer);
	fileEntity.nFileSize = (UInt32)buffer.st_size;
	if (0 != fileEntity.nFileSize)
	{
		CString strFileName = sFilePath;
		strFileName.Replace(_T("\\"), _T("/"));//mac上对于路径字符“\”需要做特殊处理，windows上则可以识别
		fileEntity.sFileName = util::cStringToString(strFileName);
		fileEntity.sFromID = module::getSysConfigModule()->userID();
		fileEntity.sToID = sSendToSID;
		uint32_t transMode = 0;
		transMode = bOnlineMode ? IM::BaseDefine::TransferFileType::FILE_TYPE_ONLINE : IM::BaseDefine::TransferFileType::FILE_TYPE_OFFLINE;
		
		LOG__(DEBG,_T("FileTransferSevice_Impl::sendFile sTaskID = %s"), util::stringToCString(fileEntity.sTaskID));

		imcore::IMLibCoreStartOperationWithLambda(
			[=]()
		{
			IM::File::IMFileReq imFileReq;
            LOG__(APP, _T("imFileReq,name=%s,size=%d,toId=%s"),util::stringToCString(fileEntity.sFileName)
                ,fileEntity.nFileSize,util::stringToCString(fileEntity.sToID));
			imFileReq.set_from_user_id(util::stringToInt32(fileEntity.sFromID));
			imFileReq.set_to_user_id(util::stringToInt32(fileEntity.sToID));
			imFileReq.set_file_name(fileEntity.sFileName);
			imFileReq.set_file_size(fileEntity.nFileSize);
			imFileReq.set_trans_mode(static_cast<IM::BaseDefine::TransferFileType>(transMode));

			module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
				, IM::BaseDefine::FileCmdID::CID_FILE_REQUEST
				, &imFileReq);
		});
		
		return TRUE;
	}
    LOG__(ERR, _T("fileEntity FileSize error,size = %d"), fileEntity.nFileSize);
	return FALSE;
}

void FileTransferModule_Impl::_sendfileResponse(IN std::string& body)
{
	IM::File::IMFileRsp imFileRsp;
	if (!imFileRsp.ParseFromString(body))
	{
		LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
		return;
	}

	UInt32 nResult = imFileRsp.result_code();
	if (nResult != 0)
	{
		LOG__(ERR, _T("_sendfileResponse result != 0"));
		module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPLOAD_FAILED);
	}

	TransferFileEntity fileEntity;
	fileEntity.sTaskID = imFileRsp.task_id();
    assert(!fileEntity.sTaskID.empty());
	fileEntity.sFromID = util::uint32ToString(imFileRsp.from_user_id());
	fileEntity.sToID = util::uint32ToString(imFileRsp.to_user_id());
	fileEntity.sFileName = imFileRsp.file_name();
	fileEntity.setSaveFilePath(util::stringToCString(fileEntity.sFileName));//发送方文件地址，就是保存地址
	fileEntity.time = static_cast<UInt32>(time(0));
	uint32_t transMode = imFileRsp.trans_mode();
	if (IM::BaseDefine::TransferFileType::FILE_TYPE_ONLINE == transMode)
	{
		fileEntity.nClientMode = IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER;
	}
	else if (IM::BaseDefine::TransferFileType::FILE_TYPE_OFFLINE == transMode)
	{
		fileEntity.nClientMode = IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD;
	}
	fileEntity.pFileObject = new TransferFile(util::stringToCString(fileEntity.sFileName),FALSE);
	if (fileEntity.pFileObject)
	{
		fileEntity.nFileSize = fileEntity.pFileObject->length();
	}
	
	UINT32 nIPCount = imFileRsp.ip_addr_list_size();
	if (nIPCount <= 0)
	{
		return;
	}
	const IM::BaseDefine::IpAddr& ipAdd = imFileRsp.ip_addr_list(0);
	fileEntity.sIP = ipAdd.ip();
	fileEntity.nPort = ipAdd.port();

	if (!TransferFileEntityManager::getInstance()->pushTransferFileEntity(fileEntity))
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);

	LOG__(DEBG, _T("FileTransferSevice_Impl::准备连接文件服务器 sTaskId = %s"), util::stringToCString(fileEntity.sTaskID));
	TransferFileEntityManager::getInstance()->openFileSocketByTaskId(fileEntity.sTaskID);
}

void FileTransferModule_Impl::_fileNotify(IN std::string& body)
{
	IM::File::IMFileNotify imFileNotify;
	if (!imFileNotify.ParseFromString(body))
	{
		LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
		return;
	}
	TransferFileEntity file;
	file.sFileName = imFileNotify.file_name();
	file.sFromID = util::uint32ToString(imFileNotify.from_user_id());
	file.sToID = util::uint32ToString(imFileNotify.to_user_id());
	file.sTaskID = imFileNotify.task_id();
	file.nFileSize = imFileNotify.file_size();

	UINT32 nIPCount = imFileNotify.ip_addr_list_size();
	if (nIPCount <= 0)
	{
		return;
	}
	const IM::BaseDefine::IpAddr& ipAdd = imFileNotify.ip_addr_list(0);
	file.sIP = ipAdd.ip();
	file.nPort = ipAdd.port();

	uint32_t transMode = imFileNotify.trans_mode();
	if (IM::BaseDefine::TransferFileType::FILE_TYPE_ONLINE == transMode)
	{
		file.nClientMode = IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_RECVER;
	}
	else if (IM::BaseDefine::TransferFileType::FILE_TYPE_OFFLINE == transMode)
	{
		file.nClientMode = IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD;
	}
	file.time = static_cast<UInt32>(time(0));
	TransferFileEntityManager::getInstance()->pushTransferFileEntity(file);
	LOG__(DEBG, _T("FileTransferSevice_Impl::给你发文件 sFileID = %s"), util::stringToCString(file.sTaskID));

	if (1 == imFileNotify.offline_ready())
	{
		//TODO离线文件传输结束
	}

	//连接服务器
	TransferFileEntityManager::getInstance()->openFileSocketByTaskId(file.sTaskID);
}

BOOL FileTransferModule_Impl::acceptFileTransfer(IN const std::string& sTaskId, IN const BOOL bAccept /*= TRUE*/)
{
	TransferFileEntity FileInfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sTaskId, FileInfo))
	{
		if (bAccept)//接收文件
		{
			FileInfo.pFileObject = new TransferFile(FileInfo.getSaveFilePath(), TRUE);
			FileInfo.time = static_cast<UInt32>(time(0));
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(FileInfo);
			LOG__(DEBG, _T("FileTransferSevice_Impl::acceptFileTransfer 接收文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
			TransferFileEntityManager::getInstance()->acceptFileTransfer(std::string(sTaskId));
		}
		else//拒绝文件
		{
			if (IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_RECVER == FileInfo.nClientMode)//在线文件
			{
				LOG__(DEBG, _T("FileTransferSevice_Impl::acceptFileTransfer 拒绝文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
				TransferFileEntityManager::getInstance()->rejectFileTransfer(sTaskId);
			}
			else if (IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD == FileInfo.nClientMode)//离线文件
			{
				LOG__(DEBG, _T("FileTransferSevice_Impl::acceptFileTransfer 拒绝离线文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
				imcore::IMLibCoreStartOperationWithLambda(
					[=]()
				{
                    LOG__(APP, _T("拒绝离线文件 imFileDelOfflineReq sFileID = %s"), util::stringToCString(FileInfo.sFileName));
					IM::File::IMFileDelOfflineReq imFileDelOfflineReq;
					imFileDelOfflineReq.set_from_user_id(util::stringToInt32(FileInfo.sFromID));
					imFileDelOfflineReq.set_to_user_id(util::stringToInt32(FileInfo.sToID));
					imFileDelOfflineReq.set_task_id(FileInfo.sTaskID);
					module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
						, IM::BaseDefine::FileCmdID::CID_FILE_DEL_OFFLINE_REQ
						, &imFileDelOfflineReq);
				});
			}
			module::getFileTransferModule()->asynNotifyObserver(module::KEY_FILESEVER_UPLOAD_REJECT, FileInfo.sTaskID);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL FileTransferModule_Impl::doCancel(IN const std::string& sTaskId)
{
	TransferFileEntity fileinfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sTaskId, fileinfo))
	{
        if (TransferFileEntityManager::getInstance()->cancelFileTransfer(sTaskId))
        {
            TransferFileEntityManager::getInstance()->DeleteFileInfoByTaskId(sTaskId);
            return TRUE;
        }
	}
	return FALSE;
}

void FileTransferModule_Impl::_hasOfflineRes(IN std::string& body)
{
	IM::File::IMFileHasOfflineRsp imFileHasOfflineRsp;
	if (!imFileHasOfflineRsp.ParseFromString(body))
	{
		LOG__(ERR, _T("parse failed,body:%s"), util::stringToCString(body));
		return;
	}
	UINT32 nIPCount = imFileHasOfflineRsp.ip_addr_list_size();
	if (nIPCount <= 0)
	{
        LOG__(ERR, _T("imFileHasOfflineRsp,ip_addr_list_size == 0"));
		return;
	}
	UINT32 nFileNum = imFileHasOfflineRsp.offline_file_list_size();
	for (UINT32 i = nFileNum; i > 0; --i)
	{
		const IM::BaseDefine::OfflineFileInfo& offLineInfo = imFileHasOfflineRsp.offline_file_list(i - 1);
		TransferFileEntity fileInfo;
		fileInfo.sFromID = util::uint32ToString(offLineInfo.from_user_id());
		fileInfo.sToID = module::getSysConfigModule()->userID();
		fileInfo.nFileSize = offLineInfo.file_size();
		fileInfo.sTaskID = offLineInfo.task_id();
		fileInfo.sFileName = offLineInfo.file_name();
		fileInfo.nClientMode = IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD;
		const IM::BaseDefine::IpAddr& ipAdd = imFileHasOfflineRsp.ip_addr_list(0);
		fileInfo.sIP = ipAdd.ip();
		fileInfo.nPort = ipAdd.port();
		fileInfo.time = static_cast<UInt32>(time(0));
		if (TransferFileEntityManager::getInstance()->pushTransferFileEntity(fileInfo))
		{
			LOG__(APP, _T("离线文件 sFileID = %s"), util::stringToCString(fileInfo.sTaskID));
			TransferFileEntityManager::getInstance()->openFileSocketByTaskId(fileInfo.sTaskID);
		}
	}
}

/******************************************************************************/