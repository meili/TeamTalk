//
//  DDMsgServer.m
//  Duoduo
//
//  Created by 独嘉 on 14-4-5.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DDMsgServer.h"
#import "DDTcpClientManager.h"
#import "LoginAPI.h"

static int const timeOutTimeInterval = 10;

typedef void(^CheckSuccess)(id object);
typedef void(^CheckFailure)(NSError* error);

@interface DDMsgServer(PrivateAPI)

- (void)n_receiveLoginMsgServerNotification:(NSNotification*)notification;
- (void)n_receiveLoginLoginServerNotification:(NSNotification*)notification;

@end

@implementation DDMsgServer
{
    CheckSuccess _success;
    CheckFailure _failure;
    
    BOOL _connecting;
    NSUInteger _connectTimes;
}
- (id)init
{
    self = [super init];
    if (self)
    {
        _connecting = NO;
        _connectTimes = 0;
//        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(n_receiveLoginMsgServerNotification:) name:notificationLoginMsgServerSuccess object:nil];
    }
    return self;
}

-(void)checkUserID:(NSString*)userID Pwd:(NSString *)password token:(NSString*)token success:(void(^)(id object))success failure:(void(^)(id object))failure
{
    
    if (!_connecting)
    {
        
        NSNumber* clientType = @(17);
        NSString *clientVersion = [NSString stringWithFormat:@"MAC/%@-%@",[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"],[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]];
        NSArray* parameter = @[userID,password,clientVersion,clientType];
        
        LoginAPI* api = [[LoginAPI alloc] init];
        [api requestWithObject:parameter Completion:^(id response, NSError *error) {
            if (!error)
            {
                if (response)
                {
                    NSInteger code =[response[@"code"] integerValue];
                    if (code !=0) {
                        NSString *errString= @"";
                        switch (code) {
                            case 0:
                                errString=@"登陆异常";
                                break;
                            case 1:
                                errString=@"连接服务器失败";
                                break;
                            case 2:
                                errString=@"连接服务器失败";
                                break;
                            case 3:
                                errString=@"连接服务器失败";
                                break;
                            case 4:
                                errString=@"连接服务器失败";
                                break;
                            case 5:
                                errString=@"连接服务器失败";
                                break;
                            case 6:
                                errString=@"用户名或密码错误";
                                break;
                            case 7:
                                errString=@"版本过低";
                                break;
                                
                            default:
                                break;
                        }
                        NSError *error1 = [NSError errorWithDomain:errString code:code userInfo:nil];
                        failure(error1);
                        }else{
                            NSString *resultString =response[@"resultString"];
                            if (resultString == nil) {
                                success(response);
                            }
                        }
                }
                   
                
                
            }
            else
            {
                DDLog(@"error:%@",[error domain]);
                
                failure(error);
            }
        }];
    }
}

@end
