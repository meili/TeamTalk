//
//  HttpPdu.h
//  http_msg_server
//
//  Created by jianqing.du on 13-9-29.
//  Copyright (c) 2013å¹? ziteng. All rights reserved.
//

#ifndef http_msg_server_HttpParserWrapper_h
#define http_msg_server_HttpParserWrapper_h

#include "util.h"
#include "http_parser.h"

// extract url and content body from an ajax request
class CHttpParserWrapper {
public:
    CHttpParserWrapper();
    
    virtual ~CHttpParserWrapper() { }
    
    void ParseHttpContent(const char* buf, uint32_t len);
    
    bool IsReadAll() { return m_read_all; }
    bool IsReadReferer() { return m_read_referer; }
    bool HasReadReferer() { return m_referer.size() > 0; }
    bool IsReadForwardIP() { return m_read_forward_ip; }
    bool HasReadForwardIP() { return m_forward_ip.size() > 0; }
    bool IsReadUserAgent() { return m_read_user_agent; }
    bool HasReadUserAgent() { return m_user_agent.size() > 0; }
    bool IsReadContentType() { return m_read_content_type;  }
    bool HasReadContentType() { return m_content_type.size() > 0; }
    bool IsReadContentLen()  { return m_read_content_len; }
    bool HasReadContentLen() { return m_content_len != 0; }
    bool IsReadHost()  { return m_read_host;  }
    bool HasReadHost() { return m_host.size()>0; }

    uint32_t GetTotalLength() { return m_total_length; }
    char* GetUrl() { return (char*)m_url.c_str(); }
    char* GetBodyContent() { return (char*)m_body_content.c_str(); }
    uint32_t GetBodyContentLen() { return (uint32_t)m_body_content.length(); }
    char* GetReferer() { return (char*)m_referer.c_str(); }
    char* GetForwardIP() { return (char*)m_forward_ip.c_str(); }
    char* GetUserAgent() { return (char*)m_user_agent.c_str(); }
    char GetMethod() { return (char) m_http_parser.method; }
    char* GetContentType() { return (char*) m_content_type.c_str(); }
    uint32_t  GetContentLen() { return m_content_len; }
    char* GetHost() { return (char*) m_host.c_str(); }
    
    void SetUrl(const char* url, size_t length) { m_url.append(url, length); }
    void SetReferer(const char* referer, size_t length) { m_referer.append(referer, length); }
    void SetForwardIP(const char* forward_ip, size_t length) { m_forward_ip.append(forward_ip, length); }
    void SetUserAgent(const char* user_agent, size_t length) { m_user_agent.append(user_agent, length); }
    void SetBodyContent(const char* content, size_t length) { m_body_content.append(content, length); }
    void SetContentType(const char* content_type, size_t length) {  m_content_type.append(content_type, length); }
    void SetContentLen(uint32_t content_len) { m_content_len = content_len; }
    void SetTotalLength(uint32_t total_len) { m_total_length = total_len; }
    void SetHost(const char* host, size_t length) { m_host.append(host, length); }
    void SetReadAll() { m_read_all = true; }
    void SetReadReferer(bool read_referer) { m_read_referer = read_referer; }
    void SetReadForwardIP(bool read_forward_ip) { m_read_forward_ip = read_forward_ip; }
    void SetReadUserAgent(bool read_user_agent) { m_read_user_agent = read_user_agent; }
    void SetReadContentType(bool read_content_type) { m_read_content_type =  read_content_type; }
    void SetReadContentLen(bool read_content_len) { m_read_content_len = read_content_len; }
    void SetReadHost(bool read_host) { m_read_host = read_host; }

    static int OnUrl(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeaderField(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeaderValue(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeadersComplete (http_parser* parser, void* obj);
    static int OnBody (http_parser* parser, const char *at, size_t length, void* obj);
    static int OnMessageComplete (http_parser* parser, void* obj);

private:

private:

    http_parser m_http_parser;
    http_parser_settings m_settings;

    bool m_read_all;
    bool m_read_referer;
    bool m_read_forward_ip;
    bool m_read_user_agent;
    bool m_read_content_type;
    bool m_read_content_len;
    bool m_read_host;
    uint32_t    m_total_length;
    string	m_url;
    string	m_body_content;
    string	m_referer;
    string m_forward_ip;
    string m_user_agent;
    string m_content_type;
    uint32_t  m_content_len;
    string m_host;
};

#endif
