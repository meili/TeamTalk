//
//  GetMessageQueueAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-12-08.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "GetMessageQueueAPI.h"
#import "MTTMessageEntity.h"
#import "Encapsulator.h"
#import "IMMessage.pb.h"
@implementation GetMessageQueueAPI
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
    return IM_GET_MSG_LIST_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_GET_MSG_LIST_RSP;
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
        IMGetMsgListRsp *rsp =[IMGetMsgListRsp parseFromData:data];
        SessionType sessionType = rsp.sessionType;
        NSString *sessionID = [MTTUtil changeOriginalToLocalID:rsp.sessionId SessionType:sessionType];
        NSUInteger begin = rsp.msgIdBegin;
         NSMutableArray *msgArray = [NSMutableArray new];
        for (MsgInfo *msgInfo in [rsp msgList]) {
            MTTMessageEntity *msg = [MTTMessageEntity makeMessageFromPB:msgInfo SessionType:sessionType];
            msg.sessionId=sessionID;
            msg.state=DDmessageSendSuccess;
            [msgArray addObject:msg];
        }
        
        
        return msgArray;
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
        IMGetMsgListReqBuilder *getMsgListReq = [IMGetMsgListReq builder];
        [getMsgListReq setMsgIdBegin:[array[0] integerValue]];
        [getMsgListReq setUserId:0];
        [getMsgListReq setMsgCnt:[array[1] integerValue]];
        [getMsgListReq setSessionType: [array[2] integerValue]];
        [getMsgListReq setSessionId:[MTTUtil changeIDToOriginal:array[3]]];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_MSG cId:IM_GET_MSG_LIST_REQ seqNo:seqNo];
        [dataout directWriteBytes:[getMsgListReq build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}

@end
