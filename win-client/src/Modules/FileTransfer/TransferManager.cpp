/******************************************************************************* 
 *  @file      TransferManager.cpp 2014\9\3 11:20:03 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "TransferManager.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodingAPI.h"
#include "Modules/IMiscModule.h"
#include "FileTransferUIThread.h"
#include "ProtocolBuffer/IM.BaseDefine.pb.h"
#include "TransferFile.h"
/******************************************************************************/
TransferFileEntity::TransferFileEntity()
:nFileSize(0)
, nClientMode(0)
, nProgress(0)
, pFileObject(nullptr)
{
	sSavePath.Empty();
}
CString TransferFileEntity::getSaveFilePath()//完整的保存文件路径
{
	if (sSavePath.IsEmpty())
	{
		util::createAllDirectories(module::getMiscModule()->getFileRecvDir());
		CString strDown = module::getMiscModule()->getFileRecvDir();
		return strDown + getRealFileName();
	}
	else
		return sSavePath;
}
CString TransferFileEntity::getSaveFloderFilePath()//保存的文件夹位置
{
	CString strFloderPath;
	if (sSavePath.IsEmpty())
	{
		strFloderPath = module::getMiscModule()->getUsersDir();
	}
	else
	{
		TCHAR drive[_MAX_DRIVE] = { 0 };
		TCHAR dir[_MAX_DIR] = { 0 };
		TCHAR fname[_MAX_FNAME] = { 0 };
		TCHAR ext[_MAX_EXT] = { 0 };
		_wsplitpath(sSavePath, drive, dir, fname, ext);
		strFloderPath.Format(_T("%s%s"), drive, dir);
	}
	return strFloderPath;
}
void TransferFileEntity::setSaveFilePath(const CString& sPath)
{
	sSavePath = sPath;
}

CString TransferFileEntity::getRealFileName()
{
	CString strName;
    strName = util::stringToCString(sFileName);
	int npos = strName.ReverseFind(_T('/'));
	if (-1 != npos)
	{
		strName = strName.Mid(npos + 1, strName.GetLength() - npos);
	}
	return strName;
}

// -----------------------------------------------------------------------------
//  TransferManager: Public, Constructor

TransferFileEntityManager::TransferFileEntityManager()
:m_fileUIThread(0)
{

}

// -----------------------------------------------------------------------------
//  TransferManager: Public, Destructor

TransferFileEntityManager::~TransferFileEntityManager()
{

}

TransferFileEntityManager* TransferFileEntityManager::getInstance()
{
	static TransferFileEntityManager manager;
	return &manager;
}

BOOL TransferFileEntityManager::getFileInfoByTaskId(IN const std::string& sID, OUT TransferFileEntity& FileInfo)
{
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.find(sID);
	if (it != m_MapFile.end())
	{
		FileInfo = it->second;
		return TRUE;
	}
	return FALSE;
}
BOOL TransferFileEntityManager::DeleteFileInfoByTaskId(IN const std::string& sID)
{
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.find(sID);
	if (it != m_MapFile.end())
	{
		m_MapFile.erase(it);
		return TRUE;
	}
	return FALSE;
}
BOOL TransferFileEntityManager::kickMapFileItemToVecFile(IN std::string& sfId)
{
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.find(sfId);
	if (it != m_MapFile.end())
	{
		TransferFileEntity& fileInfo = it->second;
		m_VecFinishedFile.push_back(fileInfo);
		m_MapFile.erase(it);
		return TRUE;
	}
	return FALSE;
}


BOOL TransferFileEntityManager::updateFileInfoBysTaskID(IN const TransferFileEntity& info)
{
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.find(info.sTaskID);
	if (it != m_MapFile.end())
	{
		it->second = info;
		return TRUE;
	}
	return FALSE;
}

BOOL TransferFileEntityManager::openFileByFileID(IN const std::string& sID)
{
	std::vector<TransferFileEntity>::iterator it = m_VecFinishedFile.begin();//已经传输完的文件,任何操作过的文件都会被扔到这里
	for (; it != m_VecFinishedFile.end(); ++it)
	{
		if (sID == it->sTaskID)
		{
			CString strPath = it->getSaveFilePath();
			if (PathFileExists(strPath))
			{				
				HINSTANCE hr = ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOWNORMAL);
				if ((long)hr < 32)
				{
					DWORD dwLastError = ::GetLastError();
					LOG__(ERR, _T("ShellExecute Failed GetLastError = %d")
						, dwLastError);
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOL TransferFileEntityManager::openFileFolderByTaskID(IN const std::string& sID)//打开文件夹，并同时选中指定文件
{
	std::vector<TransferFileEntity>::iterator it = m_VecFinishedFile.begin();//已经传输完的文件,任何操作过的文件都会被扔到这里
	for (; it != m_VecFinishedFile.end(); ++it)
	{
		if (sID == it->sTaskID)
		{
			CString strPath = it->getSaveFilePath();
			if (TRUE)//判断文件夹能不能找到
			{
				strPath.Replace(_T("/"), _T("\\"));//Explorer.exe打开所在目录，不识别“/”
				HINSTANCE hr = ShellExecute(NULL, _T("open"), _T("Explorer.exe"), _T("/select,") + strPath, NULL, SW_SHOWDEFAULT);
				if ((long)hr < 32)
				{
					DWORD dwLastError = ::GetLastError();
					LOG__(ERR, _T("ShellExecute Failed GetLastError = %d")
						, dwLastError);
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL TransferFileEntityManager::checkIfIsSending(IN CString sFilePath)
{
	sFilePath.Replace(_T("\\"), _T("/"));
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.begin();
	for (; it != m_MapFile.end(); ++it)
	{
		CString sfileName = util::stringToCString(it->second.sFileName);
		if ((IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == it->second.nClientMode || IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER == it->second.nClientMode)
			&& sfileName == sFilePath)
		{
			LOG__(DEBG, _T("FileTransferSevice_Impl::_checkIfIsSending isSending sFileID = %s"), sFilePath);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL TransferFileEntityManager::pushTransferFileEntity(IN TransferFileEntity& FileInfo)
{
	CAutoLock lock(&m_lock);
	TransferFileMap::iterator it = m_MapFile.find(FileInfo.sTaskID);
	if (it != m_MapFile.end())
	{
		return FALSE;
	}
	m_MapFile.insert(std::make_pair(FileInfo.sTaskID, FileInfo));
	return TRUE;
}

void TransferFileEntityManager::getAllTransferFileEntityFileID(std::list<std::string>& fileIdList)
{
	for (auto item : m_MapFile)
	{
		fileIdList.push_back(item.first);
	}
}

BOOL TransferFileEntityManager::startup()
{
	m_fileUIThread = new FileTransferUIThread();
	PTR_FALSE(m_fileUIThread);
	if (!m_fileUIThread->create())
		return FALSE;

	return TRUE;
}

void TransferFileEntityManager::shutdown()
{
	if (m_fileUIThread)
	{
		m_fileUIThread->Shutdown();
	}
	delete m_fileUIThread;
	m_fileUIThread = nullptr;

	for (TransferFileEntity& fileEntity : m_VecFinishedFile)
	{
		delete fileEntity.pFileObject;
		fileEntity.pFileObject = nullptr;
	}
	m_VecFinishedFile.clear();

	for (auto kv : m_MapFile)
	{
		TransferFileEntity& fileEntity = kv.second;
		delete fileEntity.pFileObject;
		fileEntity.pFileObject = nullptr;
	}
	m_MapFile.clear();
}

void TransferFileEntityManager::openFileSocketByTaskId(std::string& taskId)
{
	m_fileUIThread->openFileSocketByTaskId(taskId);
}

BOOL TransferFileEntityManager::acceptFileTransfer(const std::string& taskId)
{
	return m_fileUIThread->acceptFileTransfer(taskId);
}

BOOL TransferFileEntityManager::rejectFileTransfer(const std::string& taskId)
{
	return m_fileUIThread->rejectFileTransfer(taskId);
}

FileTransferUIThread* TransferFileEntityManager::getFileTransferThread()
{
	return m_fileUIThread;
}

BOOL TransferFileEntityManager::cancelFileTransfer(const std::string& taskId)
{
	return m_fileUIThread->cancelFileTransfer(taskId);
}

void TransferFileEntityManager::closeFileSocketByTaskId(std::string& taskId)
{
	m_fileUIThread->closeFileSocketByTaskId(taskId);
}

void TransferFileEntityManager::pushTransferFileEntityToVec(IN TransferFileEntity& FileInfo)
{
	CAutoLock lock(&m_lock);
	m_VecFinishedFile.push_back(FileInfo);
}



/******************************************************************************/