//
//  MTTNightModeAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTNightModeAPI.h"
#import "IMLogin.pb.h"
@implementation MTTNightModeAPI
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
    return IM_QUERY_PUSH_SHIELD_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_QUERY_PUSH_SHIELD_RES;
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
        IMQueryPushShieldRsp *queryPush = [IMQueryPushShieldRsp parseFromData:data];
        NSMutableArray *array = [NSMutableArray new];
//        [array addObject:@(queryPush.resultCode)];
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
        IMQueryPushShieldReqBuilder *queryPush = [IMQueryPushShieldReq builder];
        [queryPush setUserId:0];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_LOGIN
                                    cId:IM_QUERY_PUSH_SHIELD_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[queryPush build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
