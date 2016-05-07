//
//  MTTPCLoginStatusNotifyAPI.m
//  TeamTalk
//
//  Created by scorpio on 15/7/21.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTPCLoginStatusNotifyAPI.h"
#import "IMBuddy.pb.h"

@implementation MTTPCLoginStatusNotifyAPI
- (int)responseServiceID
{
    return SID_BUDDY_LIST;
}

/**
 *  数据包中的commandID
 *
 *  @return commandID
 */
- (int)responseCommandID
{
    return IM_PC_LOGIN_STATUS_NOTIFY;
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
        IMPCLoginStatusNotify *res = [IMPCLoginStatusNotify parseFromData:data];
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:@(res.userId) forKey:@"uid"];
        [dic setObject:@(res.loginStat) forKey:@"loginStat"];
        return dic;
    };
    return analysis;
}

@end
