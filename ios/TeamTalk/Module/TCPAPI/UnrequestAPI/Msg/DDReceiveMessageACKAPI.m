//
//  DDReceiveMessageACKAPI.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-09.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "DDReceiveMessageACKAPI.h"
#import "IMMessage.pb.h"
@implementation DDReceiveMessageACKAPI

- (int)requestTimeOutTimeInterval
{
    return 0;
}

- (int)requestServiceID
{
    return SID_MSG;
}

- (int)responseServiceID
{
    return SID_MSG;
}

- (int)requestCommendID
{
    return IM_MSG_DATA_ACK;
}

- (int)responseCommendID
{
    return IM_MSG_DATA_ACK;
}

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
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        IMMsgDataAckBuilder *dataAck = [IMMsgDataAck builder];
        [dataAck setUserId:0];
        [dataAck setMsgId:[object[1] intValue]];
        [dataAck setSessionId:[MTTUtil changeIDToOriginal:object[2]]];
        [dataAck setSessionType:[object[3] integerValue]];

        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_MSG cId:IM_MSG_DATA_ACK seqNo:seqNo];
        [dataout directWriteBytes:[dataAck build].data];
        [dataout writeDataCount];
        return   [dataout toByteArray];
    };
    return package;
}
@end
