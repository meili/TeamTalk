//
//  MTTSignNotifyAPI.m
//  Mogujie4iPhone
//
//  Created by 独嘉 on 14-6-29.
//  Copyright (c) 2014年 juangua. All rights reserved.
//

#import "MTTSignNotifyAPI.h"
#import "IMBuddy.pb.h"
@implementation MTTSignNotifyAPI
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
    return IM_SIGN_INFO_CHANGED_NOTIFY;
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
        IMSignInfoChangedNotify *res = [IMSignInfoChangedNotify parseFromData:data];
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:[NSString stringWithFormat:@"%d",res.changedUserId] forKey:@"uid"];
        [dic setObject:res.signInfo forKey:@"sign"];
        return dic;
    };
    return analysis;
}

@end
