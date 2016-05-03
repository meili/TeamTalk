//
//  MTTKickPCClientAPI.m
//  TeamTalk
//
//  Created by scorpio on 15/7/21.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTKickPCClientAPI.h"
#import "IMLogin.pb.h"

@implementation MTTKickPCClientAPI
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
    return IM_KICK_PC_CLIENT_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_KICK_PC_CLIENT_RES;
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
        IMKickPCClientRsp *kickRsp = [IMKickPCClientRsp parseFromData:data];
        NSMutableArray *array = [NSMutableArray new];
        [array addObject:@(kickRsp.resultCode)];
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
        IMKickPCClientReqBuilder *queryPush = [IMKickPCClientReq builder];
        [queryPush setUserId:0];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_LOGIN
                                    cId:IM_KICK_PC_CLIENT_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[queryPush build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}

@end
