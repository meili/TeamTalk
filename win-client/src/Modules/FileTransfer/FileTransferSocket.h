/*******************************************************************************
 *  @file      FileTransferSocket.h 2014\8\30 13:31:33 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     文件传输socket
 ******************************************************************************/

#ifndef FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__
#define FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__

#include "GlobalDefine.h"
#include "Modules/IEvent.h"
#include "Modules/ITcpClientModule.h"
#include "network/TTPBHeader.h"
#include "network/imconn.h"
#include "google/protobuf/message_lite.h"
/******************************************************************************/
class CImPdu;
class FileTransferSocket;
class FileTransTaskBase;

namespace module
{
    class CImConn;
}

class PingFileSevTimer : public module::ITimerEvent
{
public:
	PingFileSevTimer(FileTransferSocket* pTransSocket);
	virtual void process();
	virtual void release();

private:
	FileTransferSocket* m_pFileTransSocket;
};

/**
* The class <code>文件传输socket</code>
*
*/
class FileTransferSocket :public ITcpSocketCallback
{
public:
	FileTransferSocket(std::string& taskId);
	~FileTransferSocket(void);

public:
	BOOL startFileTransLink();
	void stopfileTransLink();
    void sendPacket(IN UInt16 moduleId, IN UInt16 cmdId,IN google::protobuf::MessageLite* pbBody);

private:
	//创建连接
	virtual BOOL connect(const CString &linkaddr, UInt16 port);
	virtual BOOL shutdown();

	//心跳包
	virtual void startHeartbeat();
	virtual void stopHeartbeat();

    virtual void onClose();
    virtual void onReceiveData(const char* data, int32_t size);
    virtual void onConnectDone();
    virtual void onReceiveError();


private:
	/**@name 服务器端拆包*/
	//@{
    void _fileLoginResponse(IN std::string& body);
    void _filePullDataReqResponse(IN std::string& body);
    void _filePullDataRspResponse(IN std::string& body);
    void _fileState(IN std::string& body);
	//@}

public:
	std::string							m_sTaskId;

private:

    imcore::TTPBHeader				    m_TTPBHeader;
	PingFileSevTimer*                   m_pPingTimer;
	UInt32                              m_progressRefreshMark;

    int								m_socketHandle;

};
/******************************************************************************/
#endif// #define FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__