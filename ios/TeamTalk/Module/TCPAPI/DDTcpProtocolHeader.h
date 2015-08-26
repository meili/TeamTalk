/************************************************************
 * @file         TcpProtocolHeader.h
 * @author       快刀<kuaidao@mogujie.com>
 * summery       tcp服务器协议头，包括每个service下的command Id定义
 *
     packet data unit header format:
     length     -- 4 byte
     version    -- 2 byte
     flag       -- 2 byte
     service_id -- 2 byte
     command_id -- 2 byte
     error      -- 2 byte
     reserved   -- 2 byte
 ************************************************************/

#import <Foundation/Foundation.h>
#import <stdint.h>

//SID
enum
{
    SID_LOGIN                                   = 0x0001,
    SID_BUDDY_LIST                              = 0x0002,
    SID_MSG                                     = 0x0003,
    SID_GROUP                                   = 0x0004,
    SID_SWITCH_SERVICE                          = 0x0006,
    SID_OTHER                                   = 0x0007,
};

//SID_LOGIN
enum{
    
    IM_LOGIN_REQ                                = 0x0103,
    IM_LOGIN_RES                                = 0x0104,
    IM_LOGOUT_REQ                               = 0x0105,
    IM_LOGOUT_RES                               = 0x0106,
    IM_KICK_USER                                = 0x0107,
    IM_DEVICE_TOKEN_REQ                         = 0x0108,
    IM_DEVICE_TOKEN_RES                         = 0x0109,
    IM_KICK_PC_CLIENT_REQ                       = 0x010a,
    IM_KICK_PC_CLIENT_RES                       = 0x010b,
    IM_PUSH_SHIELD_REQ                          = 0x010c,
    IM_PUSH_SHIELD_RES                          = 0x010d,
    IM_QUERY_PUSH_SHIELD_REQ                    = 0x010e,
    IM_QUERY_PUSH_SHIELD_RES                    = 0x010f,
};

//SID_BUDDY_LIST
enum{
    IM_RECENT_CCONTACT_SESSION_REQ              = 0x0201,
    IM_RECENT_CCONTACT_SESSION_RES              = 0x0202,
    IM_USERS_INFO_REQ                           = 0x0204,
    IM_USERS_INFO_RES                           = 0x0205,
    IM_REMOVE_SESSION_REQ                       = 0x0206,
    IM_REMOVE_SESSION_RES                       = 0x0207,
    IM_ALL_USER_REQ                             = 0x0208,
    IM_ALL_USER_RES                             = 0x0209,
    IM_USERS_STAT_REQ                           = 0x020a,
    IM_USERS_STAT_RSP                           = 0x020b,
    IM_PC_LOGIN_STATUS_NOTIFY                   = 0x020e,
    IM_CHANGE_SIGN_INFO_REQ                     = 0x0213,
    IM_CHANGE_SIGN_INFO_RES                     = 0x0214,
    IM_SIGN_INFO_CHANGED_NOTIFY                 = 0x0215,
};

//SID_MSG
enum
{
    IM_MSG_DATA                                 = 0x0301,
    IM_MSG_DATA_ACK                             = 0x0302,
    IM_MSG_DATA_READ_ACK                        = 0x0303,
    IM_MSG_DATA_READ_NOTIFY                     = 0x0304,
    IM_UNREAD_MSG_CNT_REQ                       = 0x0307,
    IM_UNREAD_MSG_CNT_RSP                       = 0x0308,
    IM_GET_MSG_LIST_REQ                         = 0x0309,
    IM_GET_MSG_LIST_RSP                         = 0x030a,
    IM_GET_LASTEST_MSGID_REQ                    = 0x030b,
    IM_GET_LASTEST_MSGID_RES                    = 0x030c,
    IM_GET_MSG_BY_ID_REQ                        = 0x030d,
    IM_GET_MSG_BY_ID_RES                        = 0x030e,
};

//SID_GROUP
enum
{
    IM_NORMAL_GROUP_LIST_REQ                    = 0x0401,
    IM_NORMAL_GROUP_LIST_RES                    = 0x0402,
    IM_GROUP_INFO_LIST_REQ                      = 0x0403,
    IM_GROUP_INFO_LIST_RES                      = 0x0404,
    IM_GROUP_CREATE_REQ                         = 0x0405,
    IM_GROUP_CREATE_RES                         = 0x0406,
    IM_GROUP_CHANGE_MEMBER_REQ                  = 0x0407,
    IM_GROUP_CHANGE_MEMBER_RES                  = 0x0408,
    IM_GROU_SHIELD_REQ                          = 0x0409,
    IM_GROU_SHIELD_RES                          = 0x040a,
    
};

// SID_SWITCH_SERVICE
enum
{
    IM_P2P_CMD_MSG                              = 0x0601,
};


// SID_OTHER
enum
{
    IM_HEART_BEAT                               = 0x0701
};

@interface DDTcpProtocolHeader : NSObject

@property (nonatomic,assign) UInt16 version;
@property (nonatomic,assign) UInt16 flag;
@property (nonatomic,assign) UInt16 serviceId;
@property (nonatomic,assign) UInt16 commandId;
@property (nonatomic,assign) UInt16 reserved;
@property (nonatomic,assign) UInt16 error;

@end
