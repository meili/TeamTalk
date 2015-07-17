//
//  apns_msg.h
//  my_push_server
//
//  Created by luoning on 14-11-17.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__apns_msg__
#define __my_push_server__apns_msg__

#include <stdio.h>
#include "socket/base_msg.h"
#include <string>
#include <list>
using namespace std;

static const char* g_silent_music = "silent.caf";

#define APNS_DEVICE_TOKEN_HEX_LENGTH    64
#define APNS_DEVICE_TOKEN_BINARY_LENGTH 32
#define APNS_PAY_LOAD_MAX_LENGTH        256




#define APNS_GATEWAY_RESP_LENGTH        6

#define APNS_FEEDBACK_MSG_TIME_LENGTH   4
#define APNS_FEEDBACK_MSG_TOKEN_LENGTH  2
#define APNS_FEEDBACK_MSG_TOKEN         32


/* The device token in binary form, as was registered by the device. */
#define APNS_ITEM_DEVICE_TOKEN          1
/* The JSON-formatted payload.
   The payload must not be null-terminated. */
#define APNS_ITEM_PAY_LOAD              2
/* An arbitrary, opaque value that identifies this notification. This identifier is used for
   reporting errors to your server. */
#define APNS_ITEM_NOTIFICATION_ID       3
/* A UNIX epoch date expressed in seconds (UTC) that identifies when the notification is no longer
   valid and can be discarded.
   If this value is non-zero, APNs stores the notification tries to deliver the notification at
   least once. Specify zero to indicate that the notification expires immediately and that APNs
   should not store the notification at all. */
#define APNS_ITEM_EXPIRATION_DATE       4
/* The notification¡¯s priority. */
#define APNS_ITEM_PRIORITY              5


/* 10 The push message is sent immediately.
   The remote notification must trigger an alert, sound, or badge on the device. It is an error to
   use this priority for a push that contains only the content-available key. */
#define APNS_PRIORITY_IMMEDIATELY       10
/* 5 The push message is sent at a time that conserves power on the device receiving it. */
#define APNS_PRIORITY_CONSERVE          5


/* APNS Gateway Response */
/*
0                   No errors encountered
1                   Processing error
2                   Missing device token
3                   Missing topic
4                   Missing payload
5                   Invalid token size
6                   Invalid topic size
7                   Invalid payload size
8                   Invalid token
10                  Shutdown
255                 None (unknown)
*/
#define PACKED __attribute__( ( packed, aligned(1) ) )

typedef struct tag_gateway_head_t
{
    uchar_t command_id;         //default = 2
    uint32_t frame_length;
}PACKED ST_GATEWAY_HEAD, *LPST_GATEWAY_HEAD;


class CAPNSGateWayMsg : public CBaseMsg
{
public:
    CAPNSGateWayMsg();
    virtual ~CAPNSGateWayMsg();
    
    void WriteHead();
    
    BOOL SerializeToArray();
    
    void SetDeviceToken(string strDeviceToken) { m_strDeviceToken = strDeviceToken; }
    const string& GetDeviceToken() { return m_strDeviceToken; }
    void SetNotificationID(uint32_t nNotificationID) { m_nNotificationID = nNotificationID; }
    uint32_t GetNotificationID() { return m_nNotificationID; }
    void SetExpirationDate(uint32_t nExpirationDate) { m_nExpirationDate = nExpirationDate; }
    uint32_t GetExpirationDate() { return m_nExpirationDate; }
    void SetPriority(char cPriority) { m_cPriority = cPriority; }
    char GetPriority() { return m_cPriority; }
    void SetSound(BOOL bSound) { m_bSound = bSound; }
    BOOL GetSound() { return m_bSound; }
    void SetCustomData(string strCustomData) { m_strCustomData = strCustomData; }
    const string& GetCustomData() { return m_strCustomData; }
    void SetBadge(uint32_t nBadge) { m_nBadge = nBadge; }
    uint32_t GetBadge() { return m_nBadge; }
    void SetAlterBody(string strAlterBody) { m_strAlterBody = strAlterBody; }
    const string& GetAlterBody() { return m_strAlterBody; }
    void SetActionLocKey(string strActionLocKey) { m_strActionLocKey = strActionLocKey; }
    const string& GetActionLocKey() { return m_strActionLocKey; }
    void SetLocKey(string strLocKey) { m_strLocKey = strLocKey; }
    const string& GetLocKey() { return m_strLocKey; }
    void SetLocArgsList(list<string>& LocArgsList) { m_LocArgsList = LocArgsList; }
    const list<string>& GetLocArgsList() { return m_LocArgsList; }
    void SetLaunchImage(string strLaunchImage) { m_strLaunchImage = strLaunchImage; }
    const string& GetLaunchImage() { return m_strLaunchImage; }
private:
    string _BuildPayload();
private:
    ST_GATEWAY_HEAD m_stHead;

    
    string          m_strDeviceToken;
    uint32_t        m_nNotificationID;
    uint32_t        m_nExpirationDate;
    char            m_cPriority;
    
    /* payload */
    BOOL            m_bSound;
    uint32_t        m_nBadge;
    string          m_strCustomData;
    /* payload alter */
    string          m_strAlterBody;
    string          m_strActionLocKey;
    string          m_strLocKey;
    list<string>    m_LocArgsList;
    string          m_strLaunchImage;
};

class CAPNSGateWayResMsg : public CBaseMsg
{
public:
    CAPNSGateWayResMsg();
    ~CAPNSGateWayResMsg();
    
    BOOL CheckMsgAvailable();
    BOOL ParseFromArray(const char* buf, uint32_t len);

    uint8_t GetCommandID() { return m_CommandID; }
    uint8_t GetStatus() { return m_Status; }
    uint32_t GetNotificationID() { return m_NotificationID; }
    uint32_t GetResMsgLength() { return sizeof(m_CommandID) + sizeof(m_Status) + sizeof(m_NotificationID); }
private:
    uint8_t  m_CommandID;
    uint8_t  m_Status;
    uint32_t m_NotificationID;
    
};

class CAPNSFeedBackResMsg: public CBaseMsg
{
public:
    CAPNSFeedBackResMsg();
    virtual ~CAPNSFeedBackResMsg();
    
    BOOL CheckMsgAvailable();
    BOOL ParseFromArray(const char* buf, uint32_t len);
    
    uint32_t GetTime() { return m_Time; }
    uint16_t GetTokenLength() { return m_TokenLength; }
    string   GetToken() { return m_Token; }
private:
    uint32_t m_Time;
    uint16_t m_TokenLength;
    string   m_Token;
};


#endif /* defined(__my_push_server__apns_msg__) */
