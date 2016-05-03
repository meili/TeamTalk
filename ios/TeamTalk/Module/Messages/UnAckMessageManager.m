//
//  UnAckMessageManage.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-16.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "UnAckMessageManager.h"
#import "MTTMessageEntity.h"
#import "MTTDatabaseUtil.h"
#define MESSAGE_TIMEOUT_SEC 5
@interface UnAckMessageManager()
@property(strong)NSMutableDictionary *msgDic;
@property(strong)NSTimer *ack_Timer;
@end
@implementation UnAckMessageManager
+ (instancetype)instance
{
    static UnAckMessageManager* unackManage;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        unackManage = [[UnAckMessageManager alloc] init];
        
    });
    return unackManage;
}
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.msgDic = [NSMutableDictionary new];
        self.ack_Timer = [NSTimer scheduledTimerWithTimeInterval:5.0 target:self selector:@selector(checkMessageTimeout) userInfo:nil repeats:YES];
        
    }
    return self;
}
-(BOOL)isInUnAckQueue:(MTTMessageEntity *)message
{
    if ([[self.msgDic allKeys] containsObject:@(message.msgID)]) {
        return YES;
    }
    return NO;

}
-(void)removeMessageFromUnAckQueue:(MTTMessageEntity *)message
{
    if ([[self.msgDic allKeys] containsObject:@(message.msgID)]) {
        [self.msgDic removeObjectForKey:@(message.msgID)];
    }
}
-(void)addMessageToUnAckQueue:(MTTMessageEntity *)message
{
    NSLog(@"add message to %d,%@",message.msgID,[message description]);
    MessageAndTime *msgAndTime = [MessageAndTime new];
    msgAndTime.msg=message;
    msgAndTime.nowDate =[[NSDate date] timeIntervalSince1970];
    NSLog(@"%d",msgAndTime.nowDate);
    if (self.msgDic) {
        [self.msgDic setObject:msgAndTime forKey:@(message.msgID)];
    }
}
-(void)checkMessageTimeout
{
    [[self.msgDic allValues] enumerateObjectsUsingBlock:^(MessageAndTime *obj, NSUInteger idx, BOOL *stop) {
        NSUInteger timeNow = [[NSDate date] timeIntervalSince1970];
        NSUInteger msgTimeOut = obj.nowDate+MESSAGE_TIMEOUT_SEC;
        if (timeNow >= msgTimeOut) {
            NSLog(@"timeout time is %d,msg id is %d",msgTimeOut,obj.msg.msgID);
            obj.msg.state=DDMessageSendFailure;
            [[MTTDatabaseUtil instance] updateMessageForMessage:obj.msg completion:^(BOOL result) {
                
            }];
            [self removeMessageFromUnAckQueue:obj.msg];
        }
    }];
}
@end

@implementation MessageAndTime
@end;