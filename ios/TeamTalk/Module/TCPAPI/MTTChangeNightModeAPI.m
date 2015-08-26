//
//  MTTChangeNightModeAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTChangeNightModeAPI.h"
#import "IMLogin.pb.h"
@implementation MTTChangeNightModeAPI
/**
 *  请求超时时间
 *
 *  @return 超时时间
 */
- (int)requestTimeOutTimeInterval
{
    return TimeOutTimeInterval;
}

/**
 *  请求的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)requestServiceID
{
    return SID_LOGIN;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return SID_LOGIN;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return IM_PUSH_SHIELD_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_PUSH_SHIELD_RES;
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
        IMPushShieldRsp *queryPush = [IMPushShieldRsp parseFromData:data];
        NSMutableArray *array = [NSMutableArray new];
        [array addObject:@(queryPush.shieldStatus)];
        return array;
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
    Package package = (id)^(id object,uint16_t seqNo)
    {
        IMPushShieldReqBuilder *queryPush = [IMPushShieldReq builder];
        NSArray* array = (NSArray*)object;
        uint32_t isShield = [array[0] intValue];
        [queryPush setUserId:0];
        [queryPush setShieldStatus:isShield];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_LOGIN
                                    cId:IM_PUSH_SHIELD_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[queryPush build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
