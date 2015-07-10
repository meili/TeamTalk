/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�HttpClient.cpp
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��08��14��
 *   ��    ����
 *
 #include "HttpCurl.h"
 ================================================================*/

#include <string>
#include "HttpClient.h"
#include "json/json.h"
#include "util.h"
using namespace std;

size_t write_data_string(void *ptr, size_t size, size_t nmemb, void *userp)
{
    size_t len = size * nmemb;
    string* response = (string *)userp;
    
    response->append((char*)ptr, len);
    
    return len;
}

size_t write_data_binary(void *ptr, size_t size, size_t nmemb, AudioMsgInfo* pAudio)
{
    size_t nLen = size * nmemb;
    if(pAudio->data_len + nLen <= pAudio->fileSize + 4)
    {
        memcpy(pAudio->data + pAudio->data_len, ptr, nLen);
        pAudio->data_len += nLen;
    }
    return nLen;
}


CHttpClient::CHttpClient(void)
{
    
}

CHttpClient::~CHttpClient(void)
{
    
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    string* str = dynamic_cast<string*>((string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }
    
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}

CURLcode CHttpClient::Post(const string & strUrl, const string & strPost, string & strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

CURLcode CHttpClient::Get(const string & strUrl, string & strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    /**
     * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ�����
     * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳���
     */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

string CHttpClient::UploadByteFile(const string &strUrl, void* pData, int nSize)
{
    if(strUrl.empty())
        return "";
    
    CURL* curl = curl_easy_init();
    if (!curl)
        return "";
    struct curl_slist *headerlist = NULL;
    headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data; boundary=WebKitFormBoundary8riBH6S4ZsoT69so");
    // what URL that receives this POST
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
//    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);    //enable verbose for easier tracing
    string body = "--WebKitFormBoundary8riBH6S4ZsoT69so\r\nContent-Disposition: form-data; name=\"file\"; filename=\"1.audio\"\r\nContent-Type:image/jpg\r\n\r\n";
    body.append((char*)pData, nSize);    // image buffer
    string str = "\r\n--WebKitFormBoundary8riBH6S4ZsoT69so--\r\n\r\n";
    body.append(str.c_str(), str.size());
    // post binary data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    // set the size of the postfields data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    string strResp;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResp);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    
    // Perform the request, res will get the return code
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (CURLE_OK != res) {
        log("curl_easy_perform failed, res=%d", res);
        return "";
    }
    
    // upload ���ص�json��ʽ��һ����Ҫ���⴦��
    Json::Reader reader;
    Json::Value value;
    
    if (!reader.parse(strResp, value)) {
        log("json parse failed: %s", strResp.c_str());
        return "";
    }
    
    if (value["error_code"].isNull()) {
        log("no code in response %s", strResp.c_str());
        return "";
    }
    uint32_t nRet = value["error_code"].asUInt();
    if(nRet != 0)
    {
        log("upload faile:%u", nRet);
        return "";
    }
    return value["url"].asString();
}

bool CHttpClient::DownloadByteFile(const string &url, AudioMsgInfo* pAudioMsg)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return false;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,2);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_binary);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pAudioMsg);
    CURLcode res = curl_easy_perform(curl);
    
    int retcode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
    if(CURLE_OK != res || retcode != 200) {
        log("curl_easy_perform failed, res=%d, ret=%u", res, retcode);
    }
    double nLen = 0;
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD , &nLen);
    curl_easy_cleanup(curl);
    if (nLen != pAudioMsg->fileSize) {
        return false;
    }
    return true;
}