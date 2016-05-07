//
//  DDClientState.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-14.
//  Copyright (c) 2014年 mogujie. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 *  用户状态
 */
typedef NS_ENUM(NSUInteger, DDUserState)
{
    /**
     *  用户在线
     */
    DDUserOnline,
    /**
     *  用户被挤下线
     */
    DDUserKickout,
    /**
     *  用户离线
     */
    DDUserOffLine,
    /**
     *  用户主动下线
     */
    DDUserOffLineInitiative,
    /**
     *  用户正在连接
     */
    DDUserLogining
};

/**
 *  客户端网络状态
 */
typedef NS_ENUM(NSUInteger, DDNetWorkState)
{
    /**
     *  wifi
     */
    DDNetWorkWifi,
    /**
     *  3G
     */
    DDNetWork3G,
    /**
     *  2G
     */
    DDNetWork2G,
    /**
     *  无网
     */
    DDNetWorkDisconnect
};

/**
 *  Socket 连接状态
 */
typedef NS_ENUM(NSUInteger, DDSocketState)
{
    /**
     *  Socket连接登录服务器
     */
    DDSocketLinkLoginServer,
    /**
     *  Socket连接消息服务器
     */
    DDSocketLinkMessageServer,
    /**
     *  Socket没有连接
     */
    DDSocketDisconnect
};

#define DD_USER_STATE_KEYPATH               @"userState"
#define DD_NETWORK_STATE_KEYPATH            @"networkState"
#define DD_SOCKET_STATE_KEYPATH             @"socketState"
#define DD_USER_ID_KEYPATH                  @"userID"
@class DDReachability;
@interface DDClientState : NSObject
{
    DDReachability* _reachability;
}
/**
 *  当前登录用户的状态
 */
@property(nonatomic,assign)DDUserState userState;

/**
 *  网络连接状态
 */
@property(nonatomic,assign)DDNetWorkState networkState;

/**
 *  Socket连接状态
 */
@property(nonatomic,assign)DDSocketState socketState;

/**
 *  当前登录用户的ID
 */
@property (nonatomic,retain)NSString* userID;

/**
 *  单例
 *
 *  @return 客户端状态机
 */
+ (instancetype)shareInstance;

- (void)setUseStateWithoutObserver:(DDUserState)userState;

@end
