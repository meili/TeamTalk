//
//  DDClientState.m
//  Duoduo
//
//  Created by 独嘉 on 14-4-14.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDClientState.h"
#import "DDReachability.h"
@interface DDClientState(PrivateAPI)

- (void)n_receiveReachabilityChangedNotification:(NSNotification*)notification;

@end

@implementation DDClientState
+ (instancetype)shareInstance
{
    static DDClientState* g_clientState;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_clientState = [[DDClientState alloc] init];
    });
    return g_clientState;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        _userState = DDUserOffLine;
        _socketState = DDSocketDisconnect;
        _reachability = [DDReachability reachabilityForInternetConnection];
        [_reachability startNotifier];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(n_receiveReachabilityChangedNotification:)
                                                     name:kDDReachabilityChangedNotification
                                                   object:nil];
    }
    return self;
}


- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)setUseStateWithoutObserver:(DDUserState)userState
{
    _userState = userState;
}

#pragma mark - privateAPI
- (void)n_receiveReachabilityChangedNotification:(NSNotification*)notification
{
    DDReachability * reach = [notification object];
    NetworkStatus netWorkStatus = [reach currentReachabilityStatus];
    switch (netWorkStatus)
    {
        case NotReachable:
            [self setValue:@(DDNetWorkDisconnect) forKeyPath:DD_NETWORK_STATE_KEYPATH];
            break;
        case ReachableViaWiFi:
            [self setValue:@(DDNetWorkWifi) forKeyPath:DD_NETWORK_STATE_KEYPATH];
            break;
        case ReachableViaWWAN:
            [self setValue:@(DDNetWork3G) forKeyPath:DD_NETWORK_STATE_KEYPATH];
            break;
    }
}
@end
