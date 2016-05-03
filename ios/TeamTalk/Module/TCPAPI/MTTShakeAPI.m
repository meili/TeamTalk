//
//  MTTShakeAPI.m
//  TeamTalk
//
//  Created by scorpio on 15/7/10.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTShakeAPI.h"
#import "IMSwitchService.pb.h"

@implementation MTTShakeAPI
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
    return SID_SWITCH_SERVICE;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return SID_SWITCH_SERVICE;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return IM_P2P_CMD_MSG;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_P2P_CMD_MSG;
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
    Package package = (id)^(id object,uint16_t seqNo)
    {
        NSArray* array = (NSArray*)object;
        NSInteger toUid = [array[0] integerValue];
        IMP2PCmdMsgBuilder *shakePC = [IMP2PCmdMsg builder];
        NSString* theContent = [NSString stringWithFormat:@"{\"cmd_id\":%i,\"content\":\"%@\",\"service_id\":%i}",1<<16|1,@"shakewindow",1];
        [shakePC setFromUserId:0];
        [shakePC setToUserId:(UInt32)toUid];
        [shakePC setCmdMsgData:theContent];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_SWITCH_SERVICE
                                    cId:IM_P2P_CMD_MSG
                                  seqNo:seqNo];
        [dataout directWriteBytes:[shakePC build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}

@end
