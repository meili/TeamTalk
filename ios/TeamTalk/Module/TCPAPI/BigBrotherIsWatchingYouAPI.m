//
//  WeDistrustYouAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2015-04-13.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import "BigBrotherIsWatchingYouAPI.h"
#import "IMOther.pb.h"
#import "MTTUserEntity.h"
@implementation BigBrotherIsWatchingYouAPI
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
    return 0x0007;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return 0x0007;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return 0x0735;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return 0x0735;
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
        IMCallReportBuilder *call = [IMCallReport builder];
        [call setUserId:[MTTUtil changeIDToOriginal:TheRuntime.user.objID]];
        [call setPeerId:[object intValue]];
        [call setClientType:ClientTypeClientTypeIos];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:0x0007
                                    cId:0x0735
                                  seqNo:seqNo];
        [dataout directWriteBytes:[call build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
