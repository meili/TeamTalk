//
//  DDReceiveGroupAddMemberAPI.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-8.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDReceiveGroupAddMemberAPI.h"
#import "DDGroupModule.h"
#import "RuntimeStatus.h"
#import "MTTGroupEntity.h"
@implementation DDReceiveGroupAddMemberAPI
/**
 *  数据包中的serviceID
 *
 *  @return serviceID
 */
- (int)responseServiceID
{
    return SID_GROUP;
}

/**
 *  数据包中的commandID
 *
 *  @return commandID
 */
- (int)responseCommandID
{
    return IM_GROUP_CHANGE_MEMBER_REQ;
}

/**
 *  解析数据包
 *
 *  @return 解析数据包的block
 */
- (UnrequestAPIAnalysis)unrequestAnalysis
{
    UnrequestAPIAnalysis analysis = (id)^(NSData* data)
    {
        DDDataInputStream* bodyData = [DDDataInputStream dataInputStreamWithData:data];
        uint32_t result = [bodyData readInt];
        MTTGroupEntity* MTTGroupEntity = nil;
        if (result != 0)
        {
           // log4CInfo(@"change group member failure");
            return MTTGroupEntity;
        }
        NSString *groupId = [bodyData readUTF];
        uint32_t userCnt = [bodyData readInt];
        MTTGroupEntity =  [[DDGroupModule instance] getGroupByGId:groupId];
//        if (!MTTGroupEntity)
//        {
//            [groupModule tcpGetUnkownGroupInfo:groupId];
//        }
        if (MTTGroupEntity) {
            for (uint32_t i = 0; i < userCnt; i++) {
                NSString* userId = [bodyData readUTF];
                if (![MTTGroupEntity.groupUserIds containsObject:userId]) {
                    [MTTGroupEntity.groupUserIds addObject:userId];
                    [MTTGroupEntity addFixOrderGroupUserIDS:userId];
                }
                //log4CInfo(@"group add member success,member userID:%@",userId);
            }
        }
        
        NSLog(@"result: %d, goupId: %@", result, groupId);
        return MTTGroupEntity;
    };
    return analysis;
}
@end
