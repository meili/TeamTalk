//
//  RemoveSessionAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-12-10.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "RemoveSessionAPI.h"
#import "IMBuddy.pb.h"
@implementation RemoveSessionAPI
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
    return SID_BUDDY_LIST;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return SID_BUDDY_LIST;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return SID_BUDDY_LIST;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_REMOVE_SESSION_RES;
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
        return nil;
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
        NSArray* array = (NSArray*)object;
        NSString* sessionid= array[0];
        SessionType sessionType = [array[1] intValue];
        IMRemoveSessionReqBuilder *removeSession = [IMRemoveSessionReq builder];
        [removeSession setUserId:0];
        [removeSession setSessionId:[MTTUtil changeIDToOriginal:sessionid]];
        [removeSession setSessionType:sessionType];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_BUDDY_LIST
                                    cId:IM_REMOVE_SESSION_RES
                                  seqNo:seqNo];
        [dataout directWriteBytes:[removeSession build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
