//
//  MTTSundriesCenter.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-23.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void(^MTTTask)();

@interface MTTSundriesCenter : NSObject

@property (nonatomic,readonly)dispatch_queue_t serialQueue;
@property (nonatomic,readonly)dispatch_queue_t parallelQueue;

+ (instancetype)instance;
- (void)pushTaskToSerialQueue:(MTTTask)task;
- (void)pushTaskToParallelQueue:(MTTTask)task;
- (void)pushTaskToSynchronizationSerialQUeue:(MTTTask)task;
@end
