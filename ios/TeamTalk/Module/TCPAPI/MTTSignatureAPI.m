//
//  MTTSignatureAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTSignatureAPI.h"
#import "IMBuddy.pb.h"
#import "MTTUserEntity.h"
@implementation MTTSignatureAPI
/**
 *  请求超时时间
 *
 *  @return 超时时间
 */
- (int)requestTimeOutTimeInterval
{
    return 5;
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
    return IM_CHANGE_SIGN_INFO_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_CHANGE_SIGN_INFO_RES;
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
        IMChangeSignInfoRsp *changeSignRsp = [IMChangeSignInfoRsp parseFromData:data];
        return changeSignRsp.resultCode;
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
        NSString* signInfo= array[0];
        IMChangeSignInfoReqBuilder *changeSign = [IMChangeSignInfoReq builder];
        [changeSign setUserId:[MTTUserEntity localIDTopb:TheRuntime.user.objID]];
        [changeSign setSignInfo:signInfo];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_BUDDY_LIST
                                    cId:IM_CHANGE_SIGN_INFO_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[changeSign build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
