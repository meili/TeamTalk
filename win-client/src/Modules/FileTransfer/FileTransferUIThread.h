/*******************************************************************************
 *  @file      FileTransferUIThread.h 2014\9\17 16:32:15 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     包含多个socket的文件传输UI thread
 ******************************************************************************/

#ifndef FILETRANSFERUITHREAD_16C2B941_3E94_4B6F_B488_0B82EC2B3F26_H__
#define FILETRANSFERUITHREAD_16C2B941_3E94_4B6F_B488_0B82EC2B3F26_H__

#include "utility/TTThread.h"
#include "network/Lock.h"
#include <list>
/******************************************************************************/
using namespace util;
class FileTransferSocket;

#define WM_FILE_TRANSFER		WM_USER + 888

/**
 * The class <code>包含多个socket的文件传输UI thread</code> 
 *
 */
class FileTransferUIThread : public TTThread
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    FileTransferUIThread();
    /**
     * Destructor
     */
    virtual ~FileTransferUIThread();
    //@}

public:
	void Shutdown();
	virtual UInt32 process();

public:
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
	BOOL acceptFileTransfer(const std::string& taskId);
	BOOL rejectFileTransfer(const std::string& taskId);
	BOOL cancelFileTransfer(const std::string& taskId);

private:
	HWND _createWnd();
	void _releaseWnd();
	void _closeAllFileSockets();
	static LRESULT _stdcall _WndProc(HWND hWnd
		, UINT message
		, WPARAM wparam
		, LPARAM lparam);
	FileTransferSocket* _findFileSocketByTaskId(const std::string& taskId);

public:
	HWND							m_hWnd;
	std::list<FileTransferSocket*>	m_lstFileTransSockets;
	CLock				m_lock;
};
/******************************************************************************/
#endif// FILETRANSFERUITHREAD_16C2B941_3E94_4B6F_B488_0B82EC2B3F26_H__