//
//  MTTSundriesCenter.m
//  Duoduo
//
//  Created by 独嘉 on 14-4-23.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "MTTSundriesCenter.h"

@implementation MTTSundriesCenter
+ (instancetype)instance
{
    static MTTSundriesCenter* g_MTTSundriesCenter;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_MTTSundriesCenter = [[MTTSundriesCenter alloc] init];
    });
    return g_MTTSundriesCenter;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        _serialQueue = dispatch_queue_create("com.mogujie.SundriesSerial", NULL);
        _parallelQueue = dispatch_queue_create("com.mogujie.SundriesParallel", NULL);
    }
    return self;
}

- (void)pushTaskToSerialQueue:(MTTTask)task
{
    dispatch_async(self.serialQueue, ^{
        task();
    });
}

- (void)pushTaskToParallelQueue:(MTTTask)task
{
    dispatch_async(self.parallelQueue, ^{
        task();
    });
}

- (void)pushTaskToSynchronizationSerialQUeue:(MTTTask)task
{
    dispatch_sync(self.serialQueue, ^{
        task();
    });
}

@end
