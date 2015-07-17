/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：HttpConn.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年07月29日
 *   描    述：
 *
 #pragma once
 ================================================================*/
#ifndef __HTTP_CONN_H__
#define __HTTP_CONN_H__

#include "util.h"
#if (MSFS_LINUX)
#include <sys/sendfile.h>
#elif (MSFS_BSD)
#include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/uio.h>
#endif
#include <pthread.h>
#include "netlib.h"
#include "FileManager.h"
#include "ConfigFileReader.h"
#include "ThreadPool.h"
#include "HttpParserWrapper.h"

#define HTTP_CONN_TIMEOUT            30000
#define HTTP_UPLOAD_MAX                 0xA00000     //10M
#define BOUNDARY_MARK                    "boundary="
#define HTTP_END_MARK                        "\r\n\r\n"
#define CONTENT_TYPE                            "Content-Type:"
#define CONTENT_DISPOSITION         "Content-Disposition:"
#define READ_BUF_SIZE    0x100000 //1M
#define HTTP_RESPONSE_HEADER    "HTTP/1.1 200 OK\r\n"\
"Connection:close\r\n"\
"Content-Length:%d\r\n"\
"Content-Type:multipart/form-data\r\n\r\n"
#define HTTP_RESPONSE_EXTEND        "HTTP/1.1 200 OK\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:%d\r\n"\
                                    "Content-Type:multipart/form-data\r\n\r\n"
#define HTTP_RESPONSE_IMAGE         "HTTP/1.1 200 OK\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:%d\r\n"\
                                    "Content-Type:image/%s\r\n\r\n"
#define HTTP_RESPONSE_HTML          "HTTP/1.1 200 OK\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:%d\r\n"\
                                    "Content-Type:text/html;charset=utf-8\r\n\r\n%s"
#define HTTP_RESPONSE_HTML_MAX      1024
#define HTTP_RESPONSE_403           "HTTP/1.1 403 Access Forbidden\r\n"\
                                    "Content-Length: 0\r\n"\
                                    "Connection: close\r\n"\
                                    "Content-Type: text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_403_LEN       strlen(HTTP_RESPONSE_403)
#define HTTP_RESPONSE_404           "HTTP/1.1 404 Not Found\r\n"\
                                    "Content-Length: 0\r\n"\
                                    "Connection: close\r\n"\
                                    "Content-Type: text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_404_LEN       strlen(HTTP_RESPONSE_404)
#define HTTP_RESPONSE_500           "HTTP/1.1 500 Internal Server Error\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:0\r\n"\
                                    "Content-Type:text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_500_LEN       strlen(HTTP_RESPONSE_500)

using namespace msfs;
enum
{
    CONN_STATE_IDLE, CONN_STATE_CONNECTED, CONN_STATE_OPEN, CONN_STATE_CLOSED,
};

extern FileManager * g_fileManager;
extern CConfigFileReader config_file;
extern CThreadPool g_PostThreadPool;
extern CThreadPool g_GetThreadPool;

typedef struct {
    uint32_t conn_handle;
    int method;
    int nContentLen;
    string strAccessHost;
    char* pContent;
    string strUrl;
    string strContentType;
}Request_t;

typedef struct {
    uint32_t    conn_handle;
    char*     pContent;
    uint32_t content_len;
} Response_t;

class CHttpConn;
class CHttpTask: public CTask
{
public:
    CHttpTask(Request_t request);
    virtual ~CHttpTask();
    void run();
    void OnUpload();
    void OnDownload();
private:
    uint32_t m_ConnHandle;
    int m_nMethod;
    string m_strUrl;
    string m_strContentType;
    char* m_pContent;
    int m_nContentLen;
    string m_strAccessHost;
};

class CHttpConn: public CRefObject
{
public:
    CHttpConn();
    virtual ~CHttpConn();

    uint32_t GetConnHandle()
    {
        return m_conn_handle;
    }
    char* GetPeerIP()
    {
        return (char*) m_peer_ip.c_str();
    }

    int Send(void* data, int len);

    void Close();
    void OnConnect(net_handle_t handle);
    void OnRead();
    void OnWrite();
    void OnClose();
    void OnTimer(uint64_t curr_tick);
    void OnSendComplete();

    static void AddResponsePdu(uint32_t conn_handle, char* pContent, int nLen);   // 工作线程调用
    static void SendResponsePduList();  // 主线程调用
protected:
    net_handle_t m_sock_handle;
    uint32_t m_conn_handle;
    bool m_busy;

    uint32_t m_state;
    string m_peer_ip;
    uint16_t m_peer_port;
    string m_access_host;
    CSimpleBuffer m_in_buf;
    CSimpleBuffer m_out_buf;
    uint64_t m_last_send_tick;
    uint64_t m_last_recv_tick;

    CHttpParserWrapper m_HttpParser;

    static CLock          s_list_lock;
    static list<Response_t*> s_response_pdu_list;    // 主线程发送回复消息
};

typedef hash_map<uint32_t, CHttpConn*> HttpConnMap_t;

CHttpConn* FindHttpConnByHandle(uint32_t handle);
void init_http_conn();

#endif
