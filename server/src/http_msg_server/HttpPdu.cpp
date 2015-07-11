/*
 * HttpPdu.cpp
 *
 *  Created on: 2013-10-1
 *      Author: ziteng@mogujie.com
 */

#include "util.h"
#include "HttpPdu.h"
#include "json/json.h"

#define HTTP_HEADER "HTTP/1.1 200 OK\r\n"\
		"Cache-Control:no-cache\r\n"\
		"Connection:close\r\n"\
		"Content-Length:%d\r\n"\
		"Content-Type:application/javascript\r\n\r\n%s(%s)"

#define HTTP_QUEYR_HEADER "HTTP/1.1 200 OK\r\n"\
		"Cache-Control:no-cache\r\n"\
		"Connection:close\r\n"\
		"Content-Length:%d\r\n"\
		"Content-Type:text/html;charset=utf-8\r\n\r\n%s"

#define MAX_BUF_SIZE 819200

#define OK_CODE		1001
#define OK_MSG		"OK"
#define ERROR_CODE_OFFSET	4000
const char* g_error_msg[] = {
	"general error",			// 4000
	"parse parameter failed", 	// 4001
	"parameter miss",			// 4002
	"uuid not match",			// 4003
	"token not valid",			// 4004
	"no business server",		// 4005
	"parse json failed",		// 4006
	"no http user",				// 4007
	"no such method",			// 4008
};

// for single thread
static char g_response_buf[MAX_BUF_SIZE];

//http://www.cnitblog.com/luckydmz/archive/2010/10/12/69959.html
inline char fromHex(const char &x)
{
    return isdigit(x) ? x-'0' : x-'A'+10;
}

bool CPostDataParser::Parse(const char* content)
{
	char* key_start = (char*)content;
	bool complete = false;

	while (!complete) {
		char* key_end = strchr(key_start, '=');
		if (!key_end) {
			return false;
		}

		std::string key(key_start, key_end - key_start);

		char* value_start = key_end + 1;
		char* value_end = strchr(value_start, '&');
		if (!value_end) {
			complete = true;
			value_end = value_start + strlen(value_start);
		}

		std::string value(value_start, value_end - value_start);
		//printf("post data: %s:%s\n", key.c_str(), value.c_str());
		m_post_map.insert(make_pair(key, value));

		key_start = value_end + 1;
	}
	return true;
}

char* CPostDataParser::GetValue(const char* key)
{
	std::map<std::string, std::string>::iterator it = m_post_map.find(key);

	if (it != m_post_map.end()) {
		return (char*)it->second.c_str();
	}

	return NULL;
}

char* GetErrorMsg(uint32_t error_code)
{
	uint32_t index = error_code - ERROR_CODE_OFFSET;
	if (index > sizeof(g_error_msg)) {
		index = 0;
	}

	return (char*)g_error_msg[index];
}

char* PackSendResult(uint32_t error_code, const char* error_msg)
{
    Json::Value json_obj;
    
    json_obj["error_code"] = error_code;
    json_obj["error_msg"] = error_msg;
    std::string json_str = json_obj.toStyledString();
    uint32_t content_len = json_str.size();
    
    snprintf(g_response_buf, MAX_BUF_SIZE, HTTP_QUEYR_HEADER, content_len, json_str.c_str());
    return g_response_buf;
}

char* PackSendCreateGroupResult(uint32_t error_code, const char* error_msg, uint32_t group_id)
{
    Json::Value json_obj;
    
    json_obj["error_code"] = error_code;
    json_obj["error_msg"] = error_msg;
    json_obj["group_id"] = group_id;
    std::string json_str = json_obj.toStyledString();
    uint32_t content_len = json_str.size();
    
    snprintf(g_response_buf, MAX_BUF_SIZE, HTTP_QUEYR_HEADER, content_len, json_str.c_str());
    return g_response_buf;
}

char* PackGetUserIdByNickNameResult(uint32_t result, std::list<IM::BaseDefine::UserInfo> user_list)
{
    Json::Value json_obj;
    Json::Value user_info_array;
    json_obj["error_code"] = result;
    json_obj["error_msg"] = "³É¹¦";
    if (user_list.size() > 0) {
        for (auto user_info : user_list)
        {
            Json::Value user_info_obj;
            user_info_obj.append(user_info.user_nick_name());
            user_info_obj.append(user_info.user_id());
            user_info_array.append(user_info_obj);
        }
        json_obj["user_info_list"] = user_info_array;
    }
    std::string json_str = json_obj.toStyledString();
    uint32_t content_len = json_str.size();
    
    snprintf(g_response_buf, MAX_BUF_SIZE, HTTP_QUEYR_HEADER, content_len, json_str.c_str());
    return g_response_buf;
}


