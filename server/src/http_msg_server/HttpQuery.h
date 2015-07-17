/*
 * HttpQuery.h
 *
 *  Created on: 2013-10-22
 *      Author: ziteng@mogujie.com
 */

#ifndef HTTPQUERY_H_
#define HTTPQUERY_H_
#include "json/json.h"
#include "HttpConn.h"
#include "public_define.h"

typedef enum {
    HTTP_ERROR_SUCCESS = 0,
    HTTP_ERROR_PARMENT,
    HTTP_ERROR_APPKEY,
    HTTP_ERROR_MATCH,
    HTTP_ERROR_PERMISSION,
    HTTP_ERROR_INTERFACE,
    HTTP_ERROR_IP,
    HTTP_ERROR_SEND_TYPE,
    HTTP_ERROR_UNKNOWN,
    HTTP_ERROR_MAX,
    HTTP_ERROR_SERVER_EXCEPTION,
    HTTP_ERROR_CREATE_GROUP,
    HTTP_ERROR_CHANGE_MEMBER,
    HTTP_ERROR_ENCRYPT,
} HTTP_ERROR_CODE;

typedef enum {
    HTTP_SEND_MSG_TYPE_SIGNEL = 1,
    HTTP_SEND_MSG_TYPE_GROUP = 2,
} HTTP_SEND_MSG_TYPE;

static string HTTP_ERROR_MSG[] =
{
    "�ɹ�",
    "��������",
    "appKey������",
    "appKey���û���ƥ��",
    "���в������͵�Id",
    "δ��Ȩ�Ľӿ�",
    "δ��Ȩ��IP",
    "�Ƿ��ķ�������",
    "δ֪����",
    "�������쳣",
    "����Ⱥʧ��",
    "����Ⱥ��Աʧ��",
    "��Ϣ����ʧ��",
};


class CHttpQuery
{
public:
	virtual ~CHttpQuery() {}

	static CHttpQuery* GetInstance();

	static void DispatchQuery(std::string& url, std::string& post_data, CHttpConn* pHttpConn);
    
private:
	CHttpQuery() {}
    static void _QueryCreateGroup(const string& strAppKey,Json::Value& post_json_obj, CHttpConn* pHttpConn);
    static void _QueryChangeMember(const string& strAppKey,Json::Value& post_json_obj, CHttpConn* pHttpConn);
    static HTTP_ERROR_CODE _CheckAuth(const string& strAppKey, const uint32_t userId, const string& strInterface, const string& strIp);
    static HTTP_ERROR_CODE _CheckPermission(const string& strAppKey, uint8_t nType, const list<uint32_t>& lsToId , string strMsg);
    
private:
	static CHttpQuery*	m_query_instance;
};


#endif /* HTTPQUERY_H_ */
