//
//  DDFixedGroupAPI.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-6.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDFixedGroupAPI.h"
#import "MTTGroupEntity.h"
#import "IMGroup.pb.h"
@implementation DDFixedGroupAPI
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
    return IM_NORMAL_GROUP_LIST_REQ;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return IM_NORMAL_GROUP_LIST_RES;
}

/**
 *  解析数据的block
 *
 *  @return 解析数据的block
 */
- (Analysis)analysisReturnData
{
    Analysis analysis = (id)^(NSData* object)
    {
     
        IMNormalGroupListRsp *imNormalRsp = [IMNormalGroupListRsp parseFromData:object];
        NSMutableArray *array = [NSMutableArray new];
        for (GroupVersionInfo *info in [imNormalRsp groupVersionList]) {
            NSDictionary *dic = @{@"groupid":@(info.groupId),@"version":@(info.version)};
            [array addObject:dic];
        }
        return  array;
//        NSMutableArray* recentlyGroup = [[NSMutableArray alloc] init];
//        uint32_t groupCnt = [bodyData readInt];
//        for (uint32_t i = 0; i < groupCnt; i++)
//        {
//            NSString* groupId = [bodyData readUTF];
//            NSString* groupName = [bodyData readUTF];
//            NSString* groupAvatar = [bodyData readUTF];
//            NSString* groupCreator = [bodyData readUTF];
//            
//            int groupType = [bodyData readInt];
//            MTTGroupEntity* group = [[MTTGroupEntity alloc] init];
//            group.objID = groupId;
//            group.name = groupName;
//            group.avatar = groupAvatar;
//            group.groupCreatorId = groupCreator;
//            group.groupType = groupType;
//            group.isShield=[bodyData readInt];
//            uint32_t groupMemberCnt = [bodyData readInt];
//            if(groupMemberCnt > 0)
//                group.groupUserIds = [[NSMutableArray alloc] init];
//            for (uint32_t i = 0; i < groupMemberCnt; i++)
//            {
//                NSString *userId = [bodyData readUTF];
//                [group.groupUserIds addObject:userId];
//                [group addFixOrderGroupUserIDS:userId];
//            }
//            
//            [recentlyGroup addObject:group];
//        }
//        //log4CInfo(@"get recently group count:%i",[recentlyGroup count]);
        
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
        IMNormalGroupListReqBuilder *imnormal = [IMNormalGroupListReq builder];
        [imnormal setUserId:0];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_GROUP
                                    cId:IM_NORMAL_GROUP_LIST_REQ
                                  seqNo:seqNo];
        [dataout directWriteBytes:imnormal.build.data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
