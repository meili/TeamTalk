 /*******************************************************************************
 *  @file      TransferManager.h 2014\9\3 11:19:29 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     文件传输管理
 ******************************************************************************/

#ifndef TRANSFERMANAGER_3046D4C9_3E8D_4C36_93B4_092651F5B66F_H__
#define TRANSFERMANAGER_3046D4C9_3E8D_4C36_93B4_092651F5B66F_H__

#include "GlobalDefine.h"
#include "network/Lock.h"
#include <list>
#include <map>

/******************************************************************************/
class TransferFile;
class FileTransferUIThread;

class TransferFileEntity
{
public:
	TransferFileEntity();
public:
	UInt32				nPort;
	UInt32				nFileSize;
	UInt32				nClientMode;			//CLIENT_REALTIME_SENDER = 1,CLIENT_REALTIME_RECVER,CLIENT_OFFLINE_UPLOAD,CLIENT_OFFLINE_DOWNLOAD
	UInt32				nProgress;				//传输进度
	TransferFile*		pFileObject;			//传输文件
	std::string			sFromID;
	std::string         sTaskID;				//文件唯一标示符
	std::string         sToID;
	std::string         sFileName;
	std::string			sIP;
	std::string         sPathOfflineFileOnSev;	//离线文件传输时，保存在文件服务器上的位置

	UInt32		        time;            //传文件发起时间
private:
	CString				sSavePath;				//接收时，另存为的地址

public:
	CString getSaveFilePath();					//完整的保存文件路径
	CString getSaveFloderFilePath();			//保存的文件夹位置
	void setSaveFilePath(const CString& sPath);

	CString getRealFileName();
};

typedef std::map<std::string, TransferFileEntity>     TransferFileMap;//群列表

class TransferFileEntityManager
{
public:
	~TransferFileEntityManager();
	static TransferFileEntityManager* getInstance();
	BOOL pushTransferFileEntity(IN  TransferFileEntity& FileInfo);
	BOOL getFileInfoByTaskId(IN const std::string& sID, OUT TransferFileEntity& FileInfo);
	BOOL DeleteFileInfoByTaskId(IN const std::string& sID);
	void getAllTransferFileEntityFileID(std::list<std::string>& fileIdList);
	BOOL updateFileInfoBysTaskID(IN const TransferFileEntity& info);
	BOOL openFileByFileID(IN const std::string& sID);
	BOOL openFileFolderByTaskID(IN const std::string& sID);
	BOOL kickMapFileItemToVecFile(IN std::string& sfId);
	void pushTransferFileEntityToVec(IN  TransferFileEntity& FileInfo);
	BOOL checkIfIsSending(IN  CString sFilePath);//正在传输的文件不能再次传输

public:
	/**@name 包含多个file socket资源的线程相关*/
	//@{
	BOOL startup();
	void shutdown();
	/**
	* 在UI thread中异步创建文件传输socket
	*
	* @param   std::string & taskId 文件传输任务id
	* @return  void
	* @exception there is no any exception to throw.
	*/
	void openFileSocketByTaskId(std::string& taskId);
	/**
	* 在UI thread中关闭文件传输socket
	*
	* @param   std::string & taskId 文件传输任务id
	* @return  void
	* @exception there is no any exception to throw.
	*/
	void closeFileSocketByTaskId(std::string& taskId);
	/**
	* 向文件传输服务器发送接收文件信令
	*
	* @param   std::string & taskId
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	BOOL acceptFileTransfer(const std::string& taskId);//接受文件传输
	BOOL rejectFileTransfer(const std::string& taskId);//拒绝文件传输
	BOOL cancelFileTransfer(const std::string& taskId);//取消文件传输
	FileTransferUIThread* getFileTransferThread();
	//@}

private:
	TransferFileEntityManager();

private:
	CLock					m_lock;
	TransferFileMap						m_MapFile;
	std::vector<TransferFileEntity>		m_VecFinishedFile;			//已经传输完的文件,任何操作过的文件都会被扔到这里
	FileTransferUIThread*				m_fileUIThread;
};
/******************************************************************************/
#endif// TRANSFERMANAGER_3046D4C9_3E8D_4C36_93B4_092651F5B66F_H__
