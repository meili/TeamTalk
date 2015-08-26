//
//  DDLoginManager.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-5.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>
@class DDHttpServer,DDMsgServer,DDTcpServer,MTTUserEntity;
@interface LoginModule : NSObject
{
    DDHttpServer* _httpServer;
    DDMsgServer* _msgServer;
    DDTcpServer* _tcpServer;
}

@property (nonatomic,readonly)NSString* token;
+ (instancetype)instance;

/**
 *  登录接口，整个登录流程
 *
 *  @param name     用户名
 *  @param password 密码
 */
- (void)loginWithUsername:(NSString*)name password:(NSString*)password success:(void(^)(MTTUserEntity* user))success failure:(void(^)(NSString* error))failure;
/**
 *  离线
 */
- (void)offlineCompletion:(void(^)())completion;
- (void)reloginSuccess:(void(^)())success failure:(void(^)(NSString* error))failure;
@end
