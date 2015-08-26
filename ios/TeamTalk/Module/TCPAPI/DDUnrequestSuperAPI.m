//
//  DDUnrequestSuperAPI.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-7.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDUnrequestSuperAPI.h"
#import "DDAPISchedule.h"
@implementation DDUnrequestSuperAPI
- (BOOL)registerAPIInAPIScheduleReceiveData:(ReceiveData)received
{
    BOOL registerSuccess = [[DDAPISchedule instance] registerUnrequestAPI:(id<DDAPIUnrequestScheduleProtocol>)self];
    if (registerSuccess)
    {
        self.receivedData = received;
        return YES;
    }
    else
    {
        return NO;
    }
}
@end
