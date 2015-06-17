//
//  apns_msg.cpp
//  my_push_server
//
//  Created by luoning on 14-11-17.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "apns_msg.h"
#include "byte_stream.h"
#include "jsonxx.h"
#include <string.h>
#include <arpa/inet.h>
#include "push_define.h"
CAPNSGateWayMsg::CAPNSGateWayMsg()
{
    memset(&m_stHead, 0, sizeof(ST_GATEWAY_HEAD));
    __SetHeadLength(sizeof(ST_GATEWAY_HEAD));
    m_stHead.command_id = (uchar_t)2;
    m_nExpirationDate = 1;
    m_cPriority = APNS_PRIORITY_IMMEDIATELY;
    m_bSound = TRUE;
    m_nBadge = 0;
    m_nNotificationID = 0;
}

CAPNSGateWayMsg::~CAPNSGateWayMsg()
{}

void CAPNSGateWayMsg::WriteHead()
{
    m_stHead.frame_length = htonl(GetBodyLength());
    char* buf = m_databuffer.GetBuffer();
    memcpy(buf, &m_stHead, sizeof(ST_GATEWAY_HEAD));
}

BOOL CAPNSGateWayMsg::SerializeToArray()
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() != 0)
    {
        PUSH_SERVER_WARN("push msg serialize failed, databuffer offset: %d.", m_databuffer.GetWriteOffset());
        return bRet;
    }
    if (m_strDeviceToken.length() != APNS_DEVICE_TOKEN_HEX_LENGTH )
    {
        PUSH_SERVER_WARN("push msg serialize failed, device token length: %d, token: %s.", m_strDeviceToken.length(), m_strDeviceToken.c_str());
        return bRet;
    }
    
    string strPayload = _BuildPayload();
    if (strPayload.length() > APNS_PAY_LOAD_MAX_LENGTH || strPayload.length() == 0)
    {
        PUSH_SERVER_WARN("push msg serialize failed, payload length: %d.", strPayload.length());
        return bRet;
    }
    
    m_databuffer.Write(NULL, GetHeadLength());
    
    //device token
    int8_t nItemID = APNS_ITEM_DEVICE_TOKEN;
    int16_t nItemDataLength = htons(APNS_DEVICE_TOKEN_BINARY_LENGTH);
    m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
    m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
    char szDeviceToken[APNS_DEVICE_TOKEN_HEX_LENGTH + 1] = {0};
    strcpy(szDeviceToken, m_strDeviceToken.c_str());
    int8_t device_token[APNS_DEVICE_TOKEN_BINARY_LENGTH] = {0};
    for (uint32_t i = 0, j = 0; i < APNS_DEVICE_TOKEN_BINARY_LENGTH; i++, j+=2)
    {
        int8_t binary = 0;
        char tmp[3] = {szDeviceToken[j], szDeviceToken[j + 1], '\0'};
        sscanf(tmp, "%x", &binary);
        device_token[i] = binary;
    }
    m_databuffer.Write((const char*)&device_token, sizeof(device_token));
    
    //pay load
    nItemID = APNS_ITEM_PAY_LOAD;
    nItemDataLength = htons(strPayload.length());
    m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
    m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
    m_databuffer.Write(strPayload.c_str(), (int32_t)strPayload.length());
    
    //notification ID
    nItemID = APNS_ITEM_NOTIFICATION_ID;
    nItemDataLength = htons(sizeof(m_nNotificationID));
    int32_t nNotificationID = htonl(m_nNotificationID);
    m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
    m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
    m_databuffer.Write((const char*)&nNotificationID, sizeof(nNotificationID));
    
    //expiration date
    nItemID = APNS_ITEM_EXPIRATION_DATE;
    nItemDataLength = htons(sizeof(m_nExpirationDate));
    int32_t nExpirationDate = htonl(m_nExpirationDate);
    m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
    m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
    m_databuffer.Write((const char*)&nExpirationDate, sizeof(nExpirationDate));
    
    //priority
    nItemID = APNS_ITEM_PRIORITY;
    nItemDataLength = htons(sizeof(m_cPriority));
    m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
    m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
    m_databuffer.Write((const char*)&m_cPriority, sizeof(m_cPriority));
    
    __SetTailLength(0);
    __SetBodyLength(m_databuffer.GetWriteOffset() - GetHeadLength() - GetTailLength());
    WriteHead();
    bRet = TRUE;
    PUSH_SERVER_DEBUG("push msg buffer length: %d, payload length: %d.", GetDataBufferLength(), strPayload.length());
    return bRet;
}

string CAPNSGateWayMsg::_BuildPayload()
{
    jsonxx::Object payload_obj, aps_obj, alert_obj;
    if (GetAlterBody().length() != 0)
    {
        alert_obj << "body" << GetAlterBody();
    }
    if (GetActionLocKey().length() != 0)
    {
        alert_obj << "action-loc-key" << GetActionLocKey();
    }
    //alert_obj << "action-loc-key" << "View" ;

    if (GetLocKey().length() != 0)
    {
        alert_obj << "loc-key" << GetLocKey();
    }
    
    if (GetLocArgsList().size() != 0)
    {
        jsonxx::Array loc_args_array;
        const list<string>& loc_args_list = GetLocArgsList();
        for (list<string>::const_iterator it = loc_args_list.begin(); it != loc_args_list.end();
             it++)
        {
            loc_args_array << *it;
        }
        alert_obj << "loc-args" << loc_args_list;
    }
    
    if (GetLaunchImage().length() != 0)
    {
        alert_obj << "launch-image" << GetLaunchImage();
    }

    if (GetSound() == FALSE)
    {
        //TODO:静音推送
        //aps_obj << "alert" << alert_obj;
        aps_obj << "badge" << GetBadge();
        //aps_obj << "sound" << string(g_silent_music);
    }
    else
    {
        aps_obj << "alert" << alert_obj;
        aps_obj << "badge" << GetBadge();
        aps_obj << "sound" << "bingbong.aiff";
    }
    
    //custom_data
    //custom_obj << "custom" << GetCustomData();
    
    payload_obj << "aps" << aps_obj;
    payload_obj << "custom" << GetCustomData();
    PUSH_SERVER_DEBUG("%s", payload_obj.json().c_str());
    return payload_obj.json();
}


CAPNSGateWayResMsg::CAPNSGateWayResMsg()
{
    __SetHeadLength(1);
    __SetBodyLength(5);
    __SetTailLength(0);
    m_CommandID = 0;
    m_Status = 0;
    m_NotificationID = 0;
}


CAPNSGateWayResMsg::~CAPNSGateWayResMsg()
{}

BOOL CAPNSGateWayResMsg::CheckMsgAvailable()
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() >= GetDataLength())
    {
        bRet = TRUE;
    }
    return bRet;
}

BOOL CAPNSGateWayResMsg::ParseFromArray(const char* buf, uint32_t len)
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() != 0)
    {
        return bRet;
    }
    Append(buf, len);
    if (CheckMsgAvailable())
    {
        memcpy(&m_CommandID, (void*)buf, sizeof(m_CommandID));
        memcpy(&m_Status, (void*)(buf + sizeof(m_CommandID)), sizeof(m_Status));
        memcpy(&m_NotificationID, (void*)(buf + sizeof(m_CommandID) + sizeof(m_Status)),
               sizeof(m_NotificationID));
        m_NotificationID = ntohl(m_NotificationID);
        bRet = TRUE;
    }
    return bRet;
}

CAPNSFeedBackResMsg::CAPNSFeedBackResMsg()
{
    __SetHeadLength(APNS_FEEDBACK_MSG_TIME_LENGTH + APNS_FEEDBACK_MSG_TOKEN_LENGTH);
    __SetBodyLength(APNS_FEEDBACK_MSG_TOKEN);
    __SetTailLength(0);
    m_Time = 0;
    m_TokenLength = 0;
}

CAPNSFeedBackResMsg::~CAPNSFeedBackResMsg()
{}

BOOL CAPNSFeedBackResMsg::CheckMsgAvailable()
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() >= GetDataLength())
    {
        bRet = TRUE;
    }
    else
    {
        PUSH_SERVER_INFO("CheckMsgAvailable error.");
    }
    return bRet;
}

BOOL CAPNSFeedBackResMsg::ParseFromArray(const char *buf, uint32_t len)
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() != 0)
    {
        PUSH_SERVER_INFO("ParseFromArray error, GetWriteOffset.");
        return bRet;
    }
    Append(buf, len);
    if (CheckMsgAvailable())
    {
        memcpy(&m_Time, (void*)buf, sizeof(m_Time));
        m_Time = ntohl(m_Time);
        memcpy(&m_TokenLength, (void*)(buf + sizeof(m_Time)), sizeof(m_TokenLength));
        m_TokenLength = ntohs(m_TokenLength);
        uchar_t binary_token[32] = {0};
        char device_token[APNS_DEVICE_TOKEN_HEX_LENGTH + 1] = {0};
        char* p = device_token;
        memcpy(binary_token, buf + sizeof(m_Time) + sizeof(m_TokenLength), m_TokenLength);
        //需要换算成16进制的字符串表示
        for (uint32_t i = 0; i < APNS_DEVICE_TOKEN_BINARY_LENGTH; i++) {
            snprintf(p, 3, "%2.2hhX", binary_token[i]);
            p += 2;
        }
        m_Token = device_token;
        bRet = TRUE;
    }
    else
    {
        PUSH_SERVER_INFO("CheckMsgAvailable error 2.0");
    }
    return bRet;
}