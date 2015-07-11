/*================================================================
 *     Copyright (c) 2014�� lanhu. All rights reserved.
 *
 *   �ļ����ƣ�HandlerMap.cpp
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��02��
 *   ��    ����
 *
 ================================================================*/

#include "HandlerMap.h"

#include "business/Login.h"
#include "business/MessageContent.h"
#include "business/RecentSession.h"
#include "business/UserAction.h"
#include "business/MessageCounter.h"
#include "business/GroupAction.h"
#include "business/DepartAction.h"
#include "IM.BaseDefine.pb.h"

using namespace IM::BaseDefine;


CHandlerMap* CHandlerMap::s_handler_instance = NULL;

/**
 *  ���캯��
 */
CHandlerMap::CHandlerMap()
{

}

/**
 *  ��������
 */
CHandlerMap::~CHandlerMap()
{

}

/**
 *  ����
 *
 *  @return ����ָ��CHandlerMap�ĵ���ָ��
 */
CHandlerMap* CHandlerMap::getInstance()
{
	if (!s_handler_instance) {
		s_handler_instance = new CHandlerMap();
		s_handler_instance->Init();
	}

	return s_handler_instance;
}

/**
 *  ��ʼ������,�����˸���commandId ��Ӧ�Ĵ�����
 */
void CHandlerMap::Init()
{
	// Login validate
	m_handler_map.insert(make_pair(uint32_t(CID_OTHER_VALIDATE_REQ), DB_PROXY::doLogin));
    
    
    // recent session
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST), DB_PROXY::getRecentSession));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_REMOVE_SESSION_REQ), DB_PROXY::deleteRecentSession));
    
    // users
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_USER_INFO_REQUEST), DB_PROXY::getUserInfo));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_ALL_USER_REQUEST), DB_PROXY::getChangedUser));
    m_handler_map.insert(make_pair(uint32_t(CID_BUDDY_LIST_DEPARTMENT_REQUEST), DB_PROXY::getChgedDepart));
    
    // message content
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_DATA), DB_PROXY::sendMessage));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_LIST_REQUEST), DB_PROXY::getMessage));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_UNREAD_CNT_REQUEST), DB_PROXY::getUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_READ_ACK), DB_PROXY::clearUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_GET_BY_MSG_ID_REQ), DB_PROXY::getMessageById));
    m_handler_map.insert(make_pair(uint32_t(CID_MSG_GET_LATEST_MSG_ID_REQ), DB_PROXY::getLatestMsgId));
    
    // device token
    m_handler_map.insert(make_pair(uint32_t(CID_LOGIN_REQ_DEVICETOKEN), DB_PROXY::setDevicesToken));
    m_handler_map.insert(make_pair(uint32_t(CID_OTHER_GET_DEVICE_TOKEN_REQ), DB_PROXY::getDevicesToken));
    
    //push ��������
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_SHIELD_GROUP_REQUEST), DB_PROXY::setGroupPush));
    m_handler_map.insert(make_pair(uint32_t(CID_OTHER_GET_SHIELD_REQ), DB_PROXY::getGroupPush));
    
    
    // group
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_NORMAL_LIST_REQUEST), DB_PROXY::getNormalGroupList));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_INFO_REQUEST), DB_PROXY::getGroupInfo));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_CREATE_REQUEST), DB_PROXY::createGroup));
    m_handler_map.insert(make_pair(uint32_t(CID_GROUP_CHANGE_MEMBER_REQUEST), DB_PROXY::modifyMember));
    
}

/**
 *  ͨ��commandId��ȡ������
 *
 *  @param pdu_type commandId
 *
 *  @return �������ĺ���ָ��
 */
pdu_handler_t CHandlerMap::GetHandler(uint32_t pdu_type)
{
	HandlerMap_t::iterator it = m_handler_map.find(pdu_type);
	if (it != m_handler_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}


