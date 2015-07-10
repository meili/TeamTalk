/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：HttpClient.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年08月14日
 *   描    述：
 *
 #pragma once
 ================================================================*/

#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

#include <string>
#include <curl/curl.h>
#include "public_define.h"

class CHttpClient
{
public:
    CHttpClient(void);
    ~CHttpClient(void);
    
public:
    CURLcode Post(const string & strUrl, const string & strPost, string & strResponse);
    CURLcode Get(const string & strUrl, string & strResponse);
    string UploadByteFile(const string &url, void* data, int data_len);
    bool DownloadByteFile(const string &url, AudioMsgInfo* pAudioMsg);
};

#endif