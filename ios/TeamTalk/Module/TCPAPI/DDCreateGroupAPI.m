//
//  DDCreateGroupAPI.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-8.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDCreateGroupAPI.h"
#import "DDTcpProtocolHeader.h"
#import "MTTGroupEntity.h"
#import "MTTUserEntity.h"
#import "IMGroup.pb.h"

@implementation DDCreateGroupAPI
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
    return SID_GROUP;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return SID_GROUP;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return IM_GROUP_CREATE_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_GROUP_CREATE_RES;
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
        IMGroupCreateRsp *rsp = [IMGroupCreateRsp parseFromData:data];
        uint32_t result = rsp.resultCode;
        MTTGroupEntity* group = nil;
        if (result != 0)
        {
            return group;
        }
        else
        {
            NSString *groupId = [MTTGroupEntity pbGroupIdToLocalID:rsp.groupId];
            NSString *groupName = rsp.groupName;
            uint32_t userCnt =[[rsp userIdList] count];
            group = [[MTTGroupEntity alloc] init];
            group.objID = groupId;
            group.name = groupName;
            group.groupUserIds = [[NSMutableArray alloc] init];
            
            for (uint32_t i = 0; i < userCnt; i++) {
                NSString* userId = [MTTUserEntity pbUserIdToLocalID:[[rsp userIdList][i] integerValue]];
                [group.groupUserIds addObject:userId];
                [group addFixOrderGroupUserIDS:userId];
            }
           
            return group;
        }
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
        NSString* groupName = array[0];
        NSString* groupAvatar = array[1];
        NSArray* groupUserList = array[2];
        
        IMGroupCreateReqBuilder *req = [IMGroupCreateReq builder];
        [req setUserId:0];
        [req setGroupName:groupName];
        [req setGroupAvatar:groupAvatar];
        [req setGroupType:GroupTypeGroupTypeTmp];
        NSMutableArray *originalID = [NSMutableArray new];
        for (NSString *localID in groupUserList) {
            [originalID addObject:@([MTTUtil changeIDToOriginal:localID])];
        }
        [req setMemberIdListArray:originalID];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_GROUP
                                    cId:IM_GROUP_CREATE_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:[req build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
