//
//  SendPushTokenAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-09-17.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "SendPushTokenAPI.h"
#import "RuntimeStatus.h"
#import "IMLogin.pb.h"
#import "MTTUserEntity.h"

@implementation SendPushTokenAPI
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
    return IM_DEVICE_TOKEN_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_DEVICE_TOKEN_RES;
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
        NSString *token = (NSString *)object;
        IMDeviceTokenReqBuilder *deviceToken = [IMDeviceTokenReq builder];
        [deviceToken setUserId:[MTTUserEntity localIDTopb:TheRuntime.user.objID]];
        [deviceToken setDeviceToken:token];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_LOGIN
                                    cId:IM_DEVICE_TOKEN_REQ
                                  seqNo:seqNo];
        [dataout writeDataCount];
        [dataout directWriteBytes:[deviceToken build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}

@end
