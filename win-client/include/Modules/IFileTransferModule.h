/*******************************************************************************
 *  @file      IFileTransfer.h 2014\8\26 11:51:55 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef IFILETRANSFER_425BDB8D_221E_4952_93C5_4362AF5217CB_H__
#define IFILETRANSFER_425BDB8D_221E_4952_93C5_4362AF5217CB_H__

#include "GlobalDefine.h"
#include "Modules/IModuleInterface.h"
#include "Modules/ModuleDll.h"
#include "Modules/IMiscModule.h"
#include "Modules/ModuleBase.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodingAPI.h"
/******************************************************************************/
NAMESPACE_BEGIN(module)

const std::string FILETRANSFER_IP = "122.225.68.125";
const UInt16 FILETRANSFER_PORT = 29800;
const std::string MODULE_FILETRAS_PREFIX = "FileTransfer";

//KEYID
const std::string KEY_FILETRANSFER_SENDFILE		= MODULE_FILETRAS_PREFIX + "SendFile";	//发送文件
const std::string KEY_FILETRANSFER_REQUEST		= MODULE_FILETRAS_PREFIX + "Request";   //传输文件请求-接收方
const std::string KEY_FILETRANSFER_RESPONSE		= MODULE_FILETRAS_PREFIX + "Response";  //传输文件请求返回-发送方
const std::string KEY_FILESEVER_UPLOAD_OFFLINE_FINISH	= MODULE_FILETRAS_PREFIX + "UploadOfflineFinish";	//离线文件传输到文件服务器完成
const std::string KEY_FILESEVER_UPDATA_PROGRESSBAR		= MODULE_FILETRAS_PREFIX + "UploadProgressBar";		//更新文件传输进度条
const std::string KEY_FILESEVER_PROGRESSBAR_FINISHED	= MODULE_FILETRAS_PREFIX + "ProgressBarFinished";	//传输完成
const std::string KEY_FILESEVER_UPLOAD_FAILED	= MODULE_FILETRAS_PREFIX + "UploadFailed";		//传输失败
const std::string KEY_FILESEVER_UPLOAD_CANCEL	= MODULE_FILETRAS_PREFIX + "UploadCancel";		//传输取消
const std::string KEY_FILESEVER_UPLOAD_REJECT	= MODULE_FILETRAS_PREFIX + "UploadReject";		//拒绝传输

/**
 * The class <code>IFileTransfer</code> 
 *
 */
class IFileTransferModule : public module::ModuleBase
						   ,public module::IPduPacketParse
{
public:
	virtual BOOL sendFile(IN const CString& sFilePath, IN const std::string& sSendToSID, IN BOOL bOnlineMode) = 0;
	virtual BOOL acceptFileTransfer(IN const std::string& sTaskId, IN const BOOL bAccept = TRUE) = 0;
	virtual BOOL doCancel(IN const std::string& sFileID) = 0;
	virtual void showFileTransferDialog() = 0;
};

MODULE_API IFileTransferModule* getFileTransferModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IFILETRANSFER_425BDB8D_221E_4952_93C5_4362AF5217CB_H__
