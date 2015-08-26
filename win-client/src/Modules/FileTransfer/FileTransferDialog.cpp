/*******************************************************************************
 *  @file      FileTransferDialog.cpp 2014\8\29 11:34:13 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief
 ******************************************************************************/

#include "stdafx.h"
#include "FileTransferDialog.h"
#include "GlobalDefine.h"
#include "TransferManager.h"
#include "ProtocolBuffer/IM.File.pb.h"
#include "utility/utilStrCodingAPI.h"
#include "utility/Multilingual.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IDatabaseModule.h"

namespace
{
	const TCHAR* const ktextFileName = _T("TextfileName");//文件名
	const TCHAR* const kTextTimeName = _T("TextTime");//
	const TCHAR* const kTextFileStatusName = _T("TextFileStatus");
	const TCHAR* const ktransProgressName = _T("transProgress");
	const TCHAR* const kFileTransferListName = _T("FileTransferList");
	const TCHAR* const kbtnOpenFileName = _T("btnOpenFile");
	const TCHAR* const kbtnOpenFolderName = _T("btnOpenFolder");
	const TCHAR* const kbtnAccpetName = _T("btnAccpet");
	const TCHAR* const kbtnRefuseName = _T("btnRefuse");
	const TCHAR* const kbtnCancelName = _T("btnCancel");
}
/******************************************************************************/

DUI_BEGIN_MESSAGE_MAP(FileTransferDialog, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

// -----------------------------------------------------------------------------
//  FileTransferDialog: Public, Constructor

FileTransferDialog::FileTransferDialog()
:m_plstFileTransfer(0)
, m_pprsTransProgress(0)
{
	module::getFileTransferModule()->addObserver(this, BIND_CALLBACK_2(FileTransferDialog::MKOForFileTransferModuleCallBack));
}

// -----------------------------------------------------------------------------
//  FileTransferDialog: Public, Destructor

FileTransferDialog::~FileTransferDialog()
{
	module::getFileTransferModule()->removeObserver(this);
}

LPCTSTR FileTransferDialog::GetWindowClassName() const
{
	return _T("FileTransferDialog");
}

DuiLib::CDuiString FileTransferDialog::GetSkinFile()
{
	return  _T("FileTransferDialog\\FileTransferDialog.xml");
}

DuiLib::CDuiString FileTransferDialog::GetSkinFolder()
{
	return _T("");
}

void FileTransferDialog::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void FileTransferDialog::OnWindowInitialized(TNotifyUI& msg)
{
	m_plstFileTransfer = (CListUI*)m_PaintManager.FindControl(_T("FileTransferList"));
	PTR_VOID(m_plstFileTransfer);
	std::vector<TransferFileEntity> historyfileList;
	module::getDatabaseModule()->sqlGetFileTransferHistory(historyfileList);
	for (std::vector<TransferFileEntity>::reverse_iterator reit = historyfileList.rbegin();
		reit != historyfileList.rend();++reit)//倒序输出，历史记录是按时间排的
	{
		TransferFileEntity& fileInfo = *reit;
		CListContainerElementUI*  pListElement = AddTransderItemToList(fileInfo);
		if (pListElement)
		{
			_UpdateItemButtons(pListElement, true, true, false, false, false);
			_UpdateFileTransferState(pListElement, module::KEY_FILESEVER_PROGRESSBAR_FINISHED);
			CProgressUI* pTransProgress = static_cast<CProgressUI*>(pListElement->FindSubControl(ktransProgressName));
			if (pTransProgress)
			{
				pTransProgress->SetVisible(false);
			}
		}
		TransferFileEntityManager::getInstance()->pushTransferFileEntityToVec(fileInfo);
	}

	std::list<std::string> fileTaskIdList;
	TransferFileEntityManager::getInstance()->getAllTransferFileEntityFileID(fileTaskIdList);

	for (string taskId : fileTaskIdList)
	{
		AddTransderItemToList(taskId);
	}
}

void FileTransferDialog::AddTransderItemToList(IN std::string& id)
{
	TransferFileEntity FileInfo;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(id, FileInfo))
	{
		return;
	}

	AddTransderItemToList(FileInfo);
}

CListContainerElementUI* FileTransferDialog::AddTransderItemToList(IN TransferFileEntity fileInfo)
{
	CString strTaskId = util::stringToCString(fileInfo.sTaskID);
	if (strTaskId.IsEmpty())
	{
		return NULL;
	}
	CDialogBuilder dlgBuilder;
	CListContainerElementUI* pListElement = (CListContainerElementUI*)dlgBuilder.Create(_T("FileTransferDialog\\FileTransferListItem.xml")
		, (UINT)0, NULL, &m_PaintManager);
	if (!pListElement)
	{
		return NULL;
	}
	pListElement->SetUserData(strTaskId);

	CControlUI* ptextFileName = pListElement->FindSubControl(ktextFileName);
	if (ptextFileName)
	{
		CString strSendName;
		if (fileInfo.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER
			|| IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == fileInfo.nClientMode)
		{
			strSendName = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_SENDERNAME"));
		}
		else
		{
			module::UserInfoEntity userInfo;
			module::getUserListModule()->getUserInfoBySId(fileInfo.sFromID, userInfo);
			strSendName = userInfo.getRealName();
		}
		CString strText;
		strText.Format(_T("%s:%s(%s)"), strSendName, fileInfo.getRealFileName()
			, util::stringToCString(util::getHumanReadableSizeString((double)fileInfo.nFileSize)));
		ptextFileName->SetText(strText);
	}

	CControlUI* pbtnOpenFile = pListElement->FindSubControl(kbtnOpenFileName);
	if (pbtnOpenFile)
	{
		pbtnOpenFile->SetUserData(strTaskId);
	}
	CControlUI* pbtnOpenFolder = pListElement->FindSubControl(kbtnOpenFolderName);
	if (pbtnOpenFolder)
	{
		pbtnOpenFolder->SetUserData(strTaskId);
	}

	CControlUI* pTextTime = static_cast<CControlUI*>(pListElement->FindSubControl(kTextTimeName));
	if (pTextTime)
	{
		CString strTime = module::getMiscModule()->makeShortTimeDescription(static_cast<UInt32>(fileInfo.time));
		pTextTime->SetText(strTime);
	}

	CControlUI* pbtnAccpet = pListElement->FindSubControl(kbtnAccpetName);
	if (pbtnAccpet)
	{
		pbtnAccpet->SetTag((UINT_PTR(pListElement)));
	}
	CControlUI* pbtnRefuse = pListElement->FindSubControl(kbtnRefuseName);
	if (pbtnRefuse)
	{
		pbtnRefuse->SetTag((UINT_PTR(pListElement)));
	}
	CControlUI* pBtnCancel = pListElement->FindSubControl(kbtnCancelName);
	if (pBtnCancel)
	{
		pBtnCancel->SetTag((UINT_PTR(pListElement)));
	}

	//发送方
	if (IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER == fileInfo.nClientMode
		|| IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == fileInfo.nClientMode)
	{
		if (pbtnAccpet)
		{
			pbtnAccpet->SetVisible(false);
		}
		if (pbtnRefuse)
		{
			pbtnRefuse->SetVisible(false);
		}
		if (pBtnCancel)
		{
			pBtnCancel->SetVisible(true);
		}
	}

	if (!m_plstFileTransfer)
	{
		LOG__(ERR,_T("FileTransferDialog::AddTransderItemToList--m_plstFileTransfer is null!"));
		return nullptr;
	}
	m_plstFileTransfer->AddAt(pListElement,0);

	return pListElement;
}

CListContainerElementUI* FileTransferDialog::GetTransferItemById(std::string& id)
{
	for (int i = 0; i < m_plstFileTransfer->GetCount(); ++i)
	{
		CListContainerElementUI* pItem = (CListContainerElementUI*)m_plstFileTransfer->GetItemAt(i);
		if (pItem)
		{
			CDuiString userData = pItem->GetUserData();
			if (!userData.IsEmpty())
			{
				std::string sid = util::cStringToString(CString(userData));
				if (sid == id)
				{
					return pItem;
				}
			}
		}
	}

	return 0;
}

void FileTransferDialog::_UpdateFileTransferState(IN CListContainerElementUI* pListElement, IN const std::string& keyId)
{
	if (!pListElement)
	{
		return;
	}

	CControlUI* pTextTime = static_cast<CControlUI*>(pListElement->FindSubControl(kTextFileStatusName));
	if (!pTextTime)
	{
		return;
	}
	pTextTime->SetVisible(true);

	if (module::KEY_FILESEVER_UPLOAD_FAILED == keyId)
	{
		CString csText = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_FAILED"));
		pTextTime->SetText(csText);
	}
	else if (module::KEY_FILESEVER_UPLOAD_REJECT == keyId)
	{
		CString csText = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_REJECT"));
		pTextTime->SetText(csText);
	}
	else if (module::KEY_FILESEVER_UPLOAD_CANCEL == keyId)
	{
		CString csText = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_CANCEL"));
		pTextTime->SetText(csText);
	}
	else if (module::KEY_FILESEVER_PROGRESSBAR_FINISHED == keyId)
	{
		CString csText = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_SUCCEED"));
		pTextTime->SetText(csText);
	}
    else if (module::KEY_FILESEVER_UPDATA_PROGRESSBAR == keyId)
    {
        CString csText = util::getMultilingual()->getStringById(_T("STRID_FILETRANSFERDIALOG_NOW"));
        pTextTime->SetText(csText);
    }
	else
	{
		assert(false);
	}
}

void FileTransferDialog::MKOForFileTransferModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_FILESEVER_UPLOAD_FAILED == keyId
		|| module::KEY_FILESEVER_UPLOAD_REJECT == keyId
		|| module::KEY_FILESEVER_UPLOAD_CANCEL == keyId)
	{
		std::string& sFileId = std::get<MKO_STRING>(mkoParam);
		CListContainerElementUI* pItem = _GetItemBySid(sFileId);
		if (pItem)
		{
			_UpdateItemButtons(pItem, false, false, false, false, false);
			_UpdateFileTransferState(pItem, keyId);
		}
		//todo...
		//TransferFileEntityManager::getInstance()->closeFileSocketByTaskId(sFileId);
		TransferFileEntityManager::getInstance()->DeleteFileInfoByTaskId(sFileId);
	}
	else if (module::KEY_FILESEVER_PROGRESSBAR_FINISHED == keyId)
	{
		std::string& sFileId = std::get<MKO_STRING>(mkoParam);

		CListContainerElementUI* pItem = _GetItemBySid(sFileId);
		if (pItem)
		{
			_UpdateItemButtons(pItem, true, true, false, false, false);
			_UpdateFileTransferState(pItem, keyId);
			CProgressUI* pTransProgress = static_cast<CProgressUI*>(pItem->FindSubControl(ktransProgressName));
			if (pTransProgress)
			{
				pTransProgress->SetVisible(false);
			}
		}
		TransferFileEntity FileInfo;
		if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sFileId, FileInfo))
		{
			return;
		}
		if (IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD == FileInfo.nClientMode)
		{
            LOG__(APP, _T("IMFileDelOfflineReq,name=%s,taskid=%s")
                ,util::stringToCString(FileInfo.sFileName)
                ,util::stringToCString(FileInfo.sTaskID));
			IM::File::IMFileDelOfflineReq imFileDelOfflineReq;
			imFileDelOfflineReq.set_from_user_id(util::stringToInt32(FileInfo.sFromID));
			imFileDelOfflineReq.set_to_user_id(util::stringToInt32(FileInfo.sToID));
			imFileDelOfflineReq.set_task_id(FileInfo.sTaskID);
			module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
				, IM::BaseDefine::FileCmdID::CID_FILE_DEL_OFFLINE_REQ
				, &imFileDelOfflineReq);
		}
		else if (IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == FileInfo.nClientMode)
		{
			IM::File::IMFileAddOfflineReq imFileAddOfflineReq;
			imFileAddOfflineReq.set_from_user_id(util::stringToInt32(FileInfo.sFromID));
			imFileAddOfflineReq.set_to_user_id(util::stringToInt32(FileInfo.sToID));
			imFileAddOfflineReq.set_task_id(FileInfo.sTaskID);
			imFileAddOfflineReq.set_file_name(util::cStringToString(FileInfo.getRealFileName()));
			imFileAddOfflineReq.set_file_size(FileInfo.nFileSize);
			module::getTcpClientModule()->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
				, IM::BaseDefine::FileCmdID::CID_FILE_ADD_OFFLINE_REQ
				, &imFileAddOfflineReq);
		}
		//todo...
		//TransferFileEntityManager::getInstance()->closeFileSocketByTaskId(sFileId);
		module::getDatabaseModule()->sqlInsertFileTransferHistory(FileInfo);
		TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(sFileId);
	}
	else if (module::KEY_FILESEVER_UPDATA_PROGRESSBAR == keyId)//更新进度条
	{
		std::string& sFileId = std::get<MKO_STRING>(mkoParam);
		TransferFileEntity FileInfo;
		if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sFileId, FileInfo))
		{
			return;
		}
		CListContainerElementUI* pListElement = _GetItemBySid(sFileId);
		if (pListElement)
		{
			CProgressUI* pTransProgress = static_cast<CProgressUI*>(pListElement->FindSubControl(ktransProgressName));
			if (pTransProgress)
			{
				int	nValue = FileInfo.nProgress * 100 / FileInfo.nFileSize;
				LOG__(DEBG, _T("Transfer Percent = %d"), nValue);
				pTransProgress->SetValue(nValue);
			}
		}
	}
}

void FileTransferDialog::OnClick(TNotifyUI& msg)
{
	if (0 == _tcsicmp(msg.pSender->GetName(), kbtnOpenFileName))
	{
		CString strID = msg.pSender->GetUserData();
		TransferFileEntityManager::getInstance()->openFileByFileID(util::cStringToString(strID));
	}
	else if (0 == _tcsicmp(msg.pSender->GetName(), kbtnOpenFolderName))
	{
		CString strID = msg.pSender->GetUserData();
		TransferFileEntityManager::getInstance()->openFileFolderByTaskID(util::cStringToString(strID));
	}
	else if (0 == _tcsicmp(msg.pSender->GetName(), kbtnAccpetName))
	{
		CListContainerElementUI* pListElement = reinterpret_cast<CListContainerElementUI*>(msg.pSender->GetTag());
		if (pListElement)
		{
			_UpdateItemButtons(pListElement, false, false, false, false, TRUE);
            _UpdateFileTransferState(pListElement, module::KEY_FILESEVER_UPDATA_PROGRESSBAR);
			CString strID = pListElement->GetUserData();
			module::getFileTransferModule()->acceptFileTransfer(util::cStringToString(strID));
            
		}
	}
	else if (0 == _tcsicmp(msg.pSender->GetName(), kbtnRefuseName))
	{
		CListContainerElementUI* pListElement = reinterpret_cast<CListContainerElementUI*>(msg.pSender->GetTag());
		if (pListElement)
		{
			_UpdateItemButtons(pListElement, false, false, false, false, false);
			_UpdateFileTransferState(pListElement, module::KEY_FILESEVER_UPLOAD_REJECT);
			CString strID = pListElement->GetUserData();
			module::getFileTransferModule()->acceptFileTransfer(util::cStringToString(strID), FALSE);
		}
	}
	else if (0 == _tcsicmp(msg.pSender->GetName(), kbtnCancelName))
	{
		CListContainerElementUI* pListElement = reinterpret_cast<CListContainerElementUI*>(msg.pSender->GetTag());
		if (pListElement)
		{
			_UpdateItemButtons(pListElement, false, false, false, false, false);
			_UpdateFileTransferState(pListElement, module::KEY_FILESEVER_UPLOAD_CANCEL);
			CString strID = pListElement->GetUserData();
			module::getFileTransferModule()->doCancel(util::cStringToString(strID));
		}
	}
	else if (msg.pSender->GetName() == _T("closebtn"))
	{
		ShowWindow(false);
		return;
	}
	__super::OnClick(msg);
}

void FileTransferDialog::_UpdateItemButtons(IN CListContainerElementUI* pListElement
	, IN bool bShowbtnOpenFile
	, IN bool bShowbtnOpenFolder
	, IN bool bShowbtnAccpet
	, IN bool bShowbtnRefuse
	, IN bool bShowbtnCancel)
{
	if (!pListElement)
	{
		return;
	}
	CControlUI* pbtnOpenFile = pListElement->FindSubControl(kbtnOpenFileName);
	if (pbtnOpenFile)
	{
		pbtnOpenFile->SetVisible(bShowbtnOpenFile);
	}
	CControlUI* pbtnOpenFolder = pListElement->FindSubControl(kbtnOpenFolderName);
	if (pbtnOpenFolder)
	{
		pbtnOpenFolder->SetVisible(bShowbtnOpenFolder);
	}
	CControlUI* pbtnAccpet = pListElement->FindSubControl(kbtnAccpetName);
	if (pbtnAccpet)
	{
		pbtnAccpet->SetVisible(bShowbtnAccpet);
	}
	CControlUI* pbtnRefuse = pListElement->FindSubControl(kbtnRefuseName);
	if (pbtnRefuse)
	{
		pbtnRefuse->SetVisible(bShowbtnRefuse);
	}
	CControlUI* pBtnCancel = pListElement->FindSubControl(kbtnCancelName);
	if (pBtnCancel)
	{
		pBtnCancel->SetVisible(bShowbtnCancel);
	}
}

CListContainerElementUI* FileTransferDialog::_GetItemBySid(IN std::string sid)
{
	if (!m_plstFileTransfer)
	{
		return NULL;
	}
	CString strSid = util::stringToCString(sid);
	for (int n = 0; n < m_plstFileTransfer->GetCount(); ++n)
	{
		CControlUI* pItem = m_plstFileTransfer->GetItemAt(n);
		if (pItem && 0 == _tcsicmp(pItem->GetUserData(), strSid))
		{
			return dynamic_cast<CListContainerElementUI*>(pItem);
		}
	}
	return NULL;
}


/******************************************************************************/