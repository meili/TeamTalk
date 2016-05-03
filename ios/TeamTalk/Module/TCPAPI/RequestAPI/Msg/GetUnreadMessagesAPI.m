//
//  DDGetUserUnreadMessagesAPI.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-12.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "GetUnreadMessagesAPI.h"
#import "MTTMessageEntity.h"
#import "Encapsulator.h"
#import "DDUserModule.h"
#import "DDMessageModule.h"
#import "RuntimeStatus.h"
#import "MTTSessionEntity.h"
#import "IMMessage.pb.h"
#import "IMBaseDefine.pb.h"
#import "MTTGroupEntity.h"
@implementation GetUnreadMessagesAPI
/**
 *  请求超时时间
 *
 *  @return 超时时间
 */
- (int)requestTimeOutTimeInterval
{
    return 20;
}

/**
 *  请求的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)requestServiceID
{
    return SID_MSG;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return SID_MSG;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return IM_UNREAD_MSG_CNT_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_UNREAD_MSG_CNT_RSP;
}

/**
 *  解析数据的block
 *
 *  @return 解析数据的block
 */
- (Analysis)analysisReturnData
{
    Analysis analysis = (id)^(NSData* data)
    {
        IMUnreadMsgCntRsp *unreadrsp = [IMUnreadMsgCntRsp parseFromData:data];
       
        NSMutableDictionary *dic = [NSMutableDictionary new];
        NSInteger m_total_cnt = unreadrsp.totalCnt;
        [dic setObject:@(m_total_cnt) forKey:@"m_total_cnt"];
         NSMutableArray *array = [NSMutableArray new];
        for (UnreadInfo *unreadInfo in [unreadrsp unreadinfoList]) {
            NSString *userID = @"";
            NSInteger sessionType = unreadInfo.sessionType;
            if (sessionType == SessionTypeSessionTypeSingle) {
                userID = [MTTUserEntity pbUserIdToLocalID:unreadInfo.sessionId];
            }else{
                userID = [MTTGroupEntity pbGroupIdToLocalID:unreadInfo.sessionId];
            }
            NSInteger unread_cnt = unreadInfo.unreadCnt;
            NSInteger latest_msg_id = unreadInfo.latestMsgId;
            NSString *latest_msg_content = unreadInfo.latestMsgData;
            MTTSessionEntity *session = [[MTTSessionEntity alloc] initWithSessionID:userID type:sessionType];
            session.unReadMsgCount=unread_cnt;
            session.lastMsg=latest_msg_content;
            session.lastMsgID=latest_msg_id;
            [array addObject:session];

        }
       
        [dic setObject:array forKey:@"sessions"];
        return dic;
    };
    return analysis;
}

/**
 *  打包数据的block
 *
 *  @return 打包数据的block
 */
- (Package)packageRequestObject
{
    Package package = (id)^(id object,uint32_t seqNo)
    {
        IMUnreadMsgCntReqBuilder *unreadreq = [IMUnreadMsgCntReq builder];
        [unreadreq setUserId:0];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_MSG
                                    cId:IM_UNREAD_MSG_CNT_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[unreadreq build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
