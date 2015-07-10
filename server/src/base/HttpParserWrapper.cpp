//
//  HttpPdu.cpp
//  http_msg_server
//
//  Created by jianqing.du on 13-9-29.
//  Copyright (c) 2013Äê ziteng. All rights reserved.
//

#include "HttpParserWrapper.h"
#include "http_parser.h"

#define MAX_REFERER_LEN	32

CHttpParserWrapper::CHttpParserWrapper()
{

}


void CHttpParserWrapper::ParseHttpContent(const char* buf, uint32_t len)
{
	http_parser_init(&m_http_parser, HTTP_REQUEST);
	memset(&m_settings, 0, sizeof(m_settings));
	m_settings.on_url = OnUrl;
	m_settings.on_header_field = OnHeaderField;
	m_settings.on_header_value = OnHeaderValue;
	m_settings.on_headers_complete = OnHeadersComplete;
	m_settings.on_body = OnBody;
	m_settings.on_message_complete = OnMessageComplete;
    m_settings.object = this;

    m_read_all = false;
    m_read_referer = false;
    m_read_forward_ip = false;
    m_read_user_agent = false;
    m_read_content_type = false;
    m_read_content_len = false;
    m_read_host = false;
    m_total_length = 0;
    m_url.clear();
    m_body_content.clear();
    m_referer.clear();
    m_forward_ip.clear();
    m_user_agent.clear();
    m_content_type.clear();
    m_content_len = 0;
    m_host.clear();
    
    http_parser_execute(&m_http_parser, &m_settings, buf, len);
}

int CHttpParserWrapper::OnUrl(http_parser* parser, const char *at,
        size_t length, void* obj)
{
    ((CHttpParserWrapper*)obj)->SetUrl(at, length);
    return 0;
}

int CHttpParserWrapper::OnHeaderField(http_parser* parser, const char *at,
        size_t length, void* obj)
{
    if (!((CHttpParserWrapper*)obj)->HasReadReferer())
    {
        if (strncasecmp(at, "Referer", 7) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadReferer(true);
        }
    }

    if (!((CHttpParserWrapper*)obj)->HasReadForwardIP())
    {
        if (strncasecmp(at, "X-Forwarded-For", 15) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadForwardIP(true);
        }
    }

    if (!((CHttpParserWrapper*)obj)->HasReadUserAgent())
    {
        if (strncasecmp(at, "User-Agent", 10) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadUserAgent(true);
        }
    }

    if (!((CHttpParserWrapper*)obj)->HasReadContentType())
    {
        if (strncasecmp(at, "Content-Type", 12) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadContentType(true);
        }
    }

    if(!((CHttpParserWrapper*)obj)->HasReadContentLen())
    {
        if(strncasecmp(at, "Content-Length", 14) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadContentLen(true);
        }
    }
    if(!((CHttpParserWrapper*)obj)->HasReadHost())
    {
        if(strncasecmp(at, "Host", 4) == 0)
        {
            ((CHttpParserWrapper*)obj)->SetReadHost(true);
        }
    }
    return 0;
}

int CHttpParserWrapper::OnHeaderValue(http_parser* parser, const char *at, size_t length, void* obj)
{
    if (((CHttpParserWrapper*)obj)->IsReadReferer())
    {
        size_t referer_len =
                (length > MAX_REFERER_LEN) ? MAX_REFERER_LEN : length;
        ((CHttpParserWrapper*)obj)->SetReferer(at, referer_len);
        ((CHttpParserWrapper*)obj)->SetReadReferer(false);
    }

    if (((CHttpParserWrapper*)obj)->IsReadForwardIP())
    {
        ((CHttpParserWrapper*)obj)->SetForwardIP(at, length);
        ((CHttpParserWrapper*)obj)->SetReadForwardIP(false);
    }

    if (((CHttpParserWrapper*)obj)->IsReadUserAgent())
    {
        ((CHttpParserWrapper*)obj)->SetUserAgent(at, length);
        ((CHttpParserWrapper*)obj)->SetReadUserAgent(false);
    }

    if (((CHttpParserWrapper*)obj)->IsReadContentType())
    {
        ((CHttpParserWrapper*)obj)->SetContentType(at, length);
        ((CHttpParserWrapper*)obj)->SetReadContentType(false);
    }

    if(((CHttpParserWrapper*)obj)->IsReadContentLen())
    {
        string strContentLen(at, length);
        ((CHttpParserWrapper*)obj)->SetContentLen(atoi(strContentLen.c_str()));
        ((CHttpParserWrapper*)obj)->SetReadContentLen(false);
    }

    if(((CHttpParserWrapper*)obj)->IsReadHost())
    {
        ((CHttpParserWrapper*)obj)->SetHost(at, length);
        ((CHttpParserWrapper*)obj)->SetReadHost(false);
    }
    return 0;
}

int CHttpParserWrapper::OnHeadersComplete(http_parser* parser, void* obj)
{
    ((CHttpParserWrapper*)obj)->SetTotalLength(parser->nread + (uint32_t) parser->content_length);
    return 0;
}

int CHttpParserWrapper::OnBody(http_parser* parser, const char *at, size_t length, void* obj)
{
    ((CHttpParserWrapper*)obj)->SetBodyContent(at, length);
    return 0;
}

int CHttpParserWrapper::OnMessageComplete(http_parser* parser, void* obj)
{
    ((CHttpParserWrapper*)obj)->SetReadAll();
    return 0;
}
