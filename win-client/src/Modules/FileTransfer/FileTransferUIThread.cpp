/******************************************************************************* 
 *  @file      FileTransferUIThread.cpp 2014\9\17 16:32:18 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     包含多个socket的文件传输UI thread
 ******************************************************************************/

#include "stdafx.h"
#include "FileTransferUIThread.h"
#include "FileTransferSocket.h"
#include "TransferManager.h"
#include "TransferFile.h"   //析构warning
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodingAPI.h"
#include "ProtocolBuffer/IM.File.pb.h"
#include "network/ImCore.h"
#include <algorithm>
/******************************************************************************/
namespace
{
	const UInt32 FILE_TRANSFER_BLOCK_SIZE = 1024 * 34;//每一块的大小
}

// -----------------------------------------------------------------------------
//  FileTransferUIThread: Public, Constructor

FileTransferUIThread::FileTransferUIThread()
:m_hWnd(0)
{
}

// -----------------------------------------------------------------------------
//  FileTransferUIThread: Public, Destructor

FileTransferUIThread::~FileTransferUIThread()
{

}

void FileTransferUIThread::Shutdown()
{
	::PostThreadMessage(getThreadId(), WM_QUIT, 0, 0);
	if (!wait(5000))
		destory();
}

void FileTransferUIThread::_releaseWnd()
{
	if (0 != m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

UInt32 FileTransferUIThread::process()
{
	m_hWnd = _createWnd();
	util::messagePump();

	_closeAllFileSockets();
	_releaseWnd();

	return 0;
}

HWND FileTransferUIThread::_createWnd()
{
	HWND hwnd = 0;

	//WNDCLASS wc;
	//wc.style = CS_HREDRAW | CS_VREDRAW;
	//wc.lpfnWndProc = _WndProc;
	//wc.cbClsExtra = 0;
	//wc.cbWndExtra = 0;
	//wc.hInstance = ::AfxGetInstanceHandle();
	//wc.hIcon = 0;
	//wc.hCursor = 0;
	//wc.hbrBackground = 0;
	//wc.lpszMenuName = 0;
	//wc.lpszClassName = _T("filetransfer");

	//if (!::RegisterClass(&wc))
	//	return 0;
    //hwnd = ::CreateWindowEx(0, _T("filetransferWindow"), _T("filetransfer"),
    //    0, 0, 0, 1, 1, HWND_MESSAGE, 0, 0, 0);

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = _WndProc;
    wc.hInstance = ::GetModuleHandle(0);
    wc.lpszClassName = _T("filetransfer");
    ATOM ret = ::RegisterClassExW(&wc);
    ASSERT(ret != NULL);

    hwnd = ::CreateWindowW(_T("filetransfer"), _T("filetransferWindow"),
        0, 0, 0, 0, 0, HWND_MESSAGE, 0, GetModuleHandle(0), 0);

	return hwnd;
}

LRESULT _stdcall FileTransferUIThread::_WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (WM_FILE_TRANSFER == message)
	{
		FileTransferSocket* pFileSocket = (FileTransferSocket*)lparam;
		pFileSocket->startFileTransLink();
	}

	return ::DefWindowProc(hWnd, message, wparam, lparam);
}

void FileTransferUIThread::openFileSocketByTaskId(std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (!pFileSocket)
	{
		pFileSocket = new FileTransferSocket(taskId);
		m_lstFileTransSockets.push_back(pFileSocket);
        assert(m_hWnd);
		::PostMessage(m_hWnd, WM_FILE_TRANSFER, 0, (LPARAM)pFileSocket);
	}
}

void FileTransferUIThread::closeFileSocketByTaskId(std::string& taskId)
{
	CAutoLock lock(&m_lock);
	auto fileSockIter = std::remove_if(m_lstFileTransSockets.begin()
		,m_lstFileTransSockets.end()
		,[=](FileTransferSocket* pFileSocket)
	{
		return(taskId == pFileSocket->m_sTaskId);
	});
	if (fileSockIter != m_lstFileTransSockets.end())
	{
		FileTransferSocket* pFileSocket = *fileSockIter;
		pFileSocket->stopfileTransLink();
		delete pFileSocket;
		pFileSocket = nullptr;
		m_lstFileTransSockets.erase(fileSockIter, m_lstFileTransSockets.end());
	}
}

BOOL FileTransferUIThread::acceptFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		imcore::IMLibCoreStartOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
				int mode = fileEntity.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_DOWNLOAD ? IM::BaseDefine::TransferFileType::FILE_TYPE_OFFLINE : IM::BaseDefine::TransferFileType::FILE_TYPE_ONLINE;
                LOG__(APP, _T("IMFilePullDataReq,taskId:%s"),util::stringToCString(taskId));
                IM::File::IMFilePullDataReq imFilePullDataReq;
				imFilePullDataReq.set_task_id(taskId);
				imFilePullDataReq.set_user_id(util::stringToInt32(fileEntity.sToID));
				imFilePullDataReq.set_trans_mode(static_cast<IM::BaseDefine::TransferFileType>(mode));
				imFilePullDataReq.set_offset(0);
                fileEntity.nFileSize > FILE_TRANSFER_BLOCK_SIZE ? imFilePullDataReq.set_data_size(FILE_TRANSFER_BLOCK_SIZE) : imFilePullDataReq.set_data_size(fileEntity.nFileSize);
				//发包
                pFileSocket->sendPacket(IM::BaseDefine::ServiceID::SID_FILE
                    , IM::BaseDefine::FileCmdID::CID_FILE_PULL_DATA_REQ, &imFilePullDataReq);
				//CImPduClientFilePullDataReq pduPullDataReq(taskId.c_str(), fileEntity.sToID.c_str()
				//	, mode, 0, FILE_TRANSFER_BLOCK_SIZE);
				//pFileSocket->sendPacket(&pduPullDataReq);
			}
		});
	}

	return FALSE;
}

BOOL FileTransferUIThread::rejectFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		imcore::IMLibCoreStartOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
				IM::File::IMFileState imFileState;
				imFileState.set_state(IM::BaseDefine::ClientFileState::CLIENT_FILE_REFUSE);
				imFileState.set_task_id(taskId);
				imFileState.set_user_id(util::stringToInt32(fileEntity.sToID));
				//发包
                pFileSocket->sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_STATE, &imFileState);
				//CImPduClientFileState pduRejectData(CLIENT_FILE_REFUSE, taskId.c_str(), fileEntity.sToID.c_str());
				//pFileSocket->sendPacket(&pduRejectData);
			}
		});
	}
    LOG__(ERR, _T("Can't find the taskId:%s"),util::stringToCString(taskId));
	return FALSE;
}

BOOL FileTransferUIThread::cancelFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		imcore::IMLibCoreStartOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
                //关闭文件
                if (nullptr != fileEntity.pFileObject)
                {
                    delete fileEntity.pFileObject;
                    fileEntity.pFileObject = nullptr;
                }

				std::string userid;
				if (fileEntity.nClientMode == IM::BaseDefine::ClientFileRole::CLIENT_REALTIME_SENDER
					|| IM::BaseDefine::ClientFileRole::CLIENT_OFFLINE_UPLOAD == fileEntity.nClientMode)
				{
					userid = fileEntity.sFromID;
				}
				else
				{
					userid = fileEntity.sToID;
				}
				IM::File::IMFileState imFileState;
				imFileState.set_state(IM::BaseDefine::ClientFileState::CLIENT_FILE_CANCEL);
				imFileState.set_task_id(taskId);
                imFileState.set_user_id(util::stringToInt32(userid));
				// 发包
                pFileSocket->sendPacket(IM::BaseDefine::ServiceID::SID_FILE, IM::BaseDefine::FileCmdID::CID_FILE_STATE, &imFileState);

                //本地删除记录
                TransferFileEntityManager::getInstance()->DeleteFileInfoByTaskId(taskId);
			}
		});
        return TRUE;
	}
    LOG__(ERR, _T("Can't find the taskId:%s"), util::stringToCString(taskId));
	return FALSE;
}

FileTransferSocket* FileTransferUIThread::_findFileSocketByTaskId(const std::string& taskId)
{
	CAutoLock lock(&m_lock);
	auto iter = std::find_if(m_lstFileTransSockets.begin(),m_lstFileTransSockets.end()
		, [=](FileTransferSocket* fileSock)
	{
		return (taskId == fileSock->m_sTaskId);
	});

	if (iter != m_lstFileTransSockets.end())
	{
		FileTransferSocket* fileSocket = *iter;
		return fileSocket;
	}

	return 0;
}

void FileTransferUIThread::_closeAllFileSockets()
{
	for (FileTransferSocket* pFileSock : m_lstFileTransSockets)
	{
		pFileSock->stopfileTransLink();
		delete pFileSock;
		pFileSock = 0;
	}
	m_lstFileTransSockets.clear();
}

/******************************************************************************/