//
//  DDMessageModule.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-27.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTTMessageEntity.h"
#import "MTTSessionEntity.h"

@interface DDMessageModule : NSObject
@property(assign)NSInteger unreadMsgCount;
+ (instancetype)shareInstance;

+ (NSUInteger )getMessageID;

- (void)removeFromUnreadMessageButNotSendRead:(NSString*)sessionID;
- (void)removeAllUnreadMessages;
- (NSUInteger)getUnreadMessgeCount;

-(void)sendMsgRead:(MTTMessageEntity *)message;
-(void)getMessageFromServer:(NSInteger)fromMsgID currentSession:(MTTSessionEntity *)session count:(NSInteger)count Block:(void(^)(NSMutableArray *array, NSError *error))block;
@end
