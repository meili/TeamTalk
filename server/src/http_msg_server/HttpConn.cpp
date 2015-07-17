/*
 * HttpConn.cpp
 *
 *  Created on: 2013-9-29
 *      Author: ziteng@mogujie.com
 */

#include "HttpConn.h"
#include "HttpParserWrapper.h"
#include "HttpQuery.h"

static HttpConnMap_t g_http_conn_map;

// conn_handle 从0开始递增，可以防止因socket handle重用引起的一些冲突
static uint32_t g_conn_handle_generator = 0;

CHttpConn* FindHttpConnByHandle(uint32_t conn_handle)
{
    CHttpConn* pConn = NULL;
    HttpConnMap_t::iterator it = g_http_conn_map.find(conn_handle);
    if (it != g_http_conn_map.end()) {
        pConn = it->second;
    }

    return pConn;
}

void httpconn_callback(void* callback_data, uint8_t msg, uint32_t handle, uint32_t uParam, void* pParam)
{
	NOTUSED_ARG(uParam);
	NOTUSED_ARG(pParam);

	// convert void* to uint32_t, oops
	uint32_t conn_handle = *((uint32_t*)(&callback_data));
    CHttpConn* pConn = FindHttpConnByHandle(conn_handle);
    if (!pConn) {
        return;
    }

	switch (msg)
	{
	case NETLIB_MSG_READ:
		pConn->OnRead();
		break;
	case NETLIB_MSG_WRITE:
		pConn->OnWrite();
		break;
	case NETLIB_MSG_CLOSE:
		pConn->OnClose();
		break;
	default:
		log("!!!httpconn_callback error msg: %d ", msg);
		break;
	}
}

void http_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	CHttpConn* pConn = NULL;
	HttpConnMap_t::iterator it, it_old;
	uint64_t cur_time = get_tick_count();

	for (it = g_http_conn_map.begin(); it != g_http_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = it_old->second;
		pConn->OnTimer(cur_time);
	}
}

void init_http_conn()
{
	netlib_register_timer(http_conn_timer_callback, NULL, 1000);
}

//////////////////////////
CHttpConn::CHttpConn()
{
	m_busy = false;
	m_sock_handle = NETLIB_INVALID_HANDLE;
    m_state = CONN_STATE_IDLE;
    
	m_last_send_tick = m_last_recv_tick = get_tick_count();
	m_conn_handle = ++g_conn_handle_generator;
	if (m_conn_handle == 0) {
		m_conn_handle = ++g_conn_handle_generator;
	}

	//log("CHttpConn, handle=%u ", m_conn_handle);
}

CHttpConn::~CHttpConn()
{
	//log("~CHttpConn, handle=%u ", m_conn_handle);
}

int CHttpConn::Send(void* data, int len)
{
	m_last_send_tick = get_tick_count();

	if (m_busy)
	{
		m_out_buf.Write(data, len);
		return len;
	}

	int ret = netlib_send(m_sock_handle, data, len);
	if (ret < 0)
		ret = 0;

	if (ret < len)
	{
		m_out_buf.Write((char*)data + ret, len - ret);
		m_busy = true;
		//log("not send all, remain=%d ", m_out_buf.GetWriteOffset());
	}
    else
    {
        OnWriteCompelete();
    }

	return len;
}

void CHttpConn::Close()
{
    if (m_state != CONN_STATE_CLOSED) {
        m_state = CONN_STATE_CLOSED;
        
        g_http_conn_map.erase(m_conn_handle);
        netlib_close(m_sock_handle);
        
        ReleaseRef();
    }
}

void CHttpConn::OnConnect(net_handle_t handle)
{
    printf("OnConnect, handle=%d\n", handle);
    m_sock_handle = handle;
    m_state = CONN_STATE_CONNECTED;
    g_http_conn_map.insert(make_pair(m_conn_handle, this));
    
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)httpconn_callback);
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, reinterpret_cast<void *>(m_conn_handle) );
    netlib_option(handle, NETLIB_OPT_GET_REMOTE_IP, (void*)&m_peer_ip);
}

void CHttpConn::OnRead()
{
	for (;;)
	{
		uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE + 1)
			m_in_buf.Extend(READ_BUF_SIZE + 1);

		int ret = netlib_recv(m_sock_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
		if (ret <= 0)
			break;

		m_in_buf.IncWriteOffset(ret);

		m_last_recv_tick = get_tick_count();
	}

	// 每次请求对应一个HTTP连接，所以读完数据后，不用在同一个连接里面准备读取下个请求
	char* in_buf = (char*)m_in_buf.GetBuffer();
	uint32_t buf_len = m_in_buf.GetWriteOffset();
	in_buf[buf_len] = '\0';

	//log("OnRead, buf_len=%u, conn_handle=%u\n", buf_len, m_conn_handle); // for debug

	m_HttpParser.ParseHttpContent(in_buf, buf_len);

	if (m_HttpParser.IsReadAll()) {
		string url =  m_HttpParser.GetUrl();
		if (strncmp(url.c_str(), "/query/", 7) == 0) {
			string content = m_HttpParser.GetBodyContent();
			CHttpQuery* pQueryInstance = CHttpQuery::GetInstance();
			pQueryInstance->DispatchQuery(url, content, this);
		} else {
			log("url unknown, url=%s ", url.c_str());
			Close();
		}
	}
}

void CHttpConn::OnWrite()
{
	if (!m_busy)
		return;

	int ret = netlib_send(m_sock_handle, m_out_buf.GetBuffer(), m_out_buf.GetWriteOffset());
	if (ret < 0)
		ret = 0;

	int out_buf_size = (int)m_out_buf.GetWriteOffset();

	m_out_buf.Read(NULL, ret);

	if (ret < out_buf_size)
	{
		m_busy = true;
		log("not send all, remain=%d ", m_out_buf.GetWriteOffset());
	}
	else
	{
		m_busy = false;
        OnWriteCompelete();
	}
}

void CHttpConn::OnClose()
{
    Close();
}

void CHttpConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_recv_tick + HTTP_CONN_TIMEOUT) {
		log("HttpConn timeout, handle=%d ", m_conn_handle);
		Close();
	}
}

void CHttpConn::OnWriteCompelete()
{
    Close();
}

