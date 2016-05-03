//
//  DDUserDetailInfoAPI.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-22.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDUserDetailInfoAPI.h"
#import "IMBuddy.pb.h"
#import "RuntimeStatus.h"
#import "MTTUserEntity.h"
@implementation DDUserDetailInfoAPI
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
    return IM_USERS_INFO_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_USERS_INFO_RES;
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
        IMUsersInfoRsp *rsp = [IMUsersInfoRsp parseFromData:data];
        NSMutableArray* userList = [[NSMutableArray alloc] init];
        for (UserInfo *userInfo in [rsp userInfoList]) {
            MTTUserEntity *user = [[MTTUserEntity alloc] initWithPB:userInfo];
            [userList addObject:user];
        }
        return userList;
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
        NSArray* users = (NSArray*)object;
        IMUsersInfoReqBuilder *userInfo = [IMUsersInfoReq builder];
        [userInfo setUserId:0];
        [userInfo setUserIdListArray:users];
        
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_BUDDY_LIST
                                    cId:IM_USERS_INFO_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[userInfo build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
        
        
    };
    return package;
}
@end
