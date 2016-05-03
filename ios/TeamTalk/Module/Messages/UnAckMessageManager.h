//
//  UnAckMessageManage.h
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-16.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTTMessageEntity.h"
@interface UnAckMessageManager : NSObject
+ (instancetype)instance;
-(void)removeMessageFromUnAckQueue:(MTTMessageEntity *)message;
-(void)addMessageToUnAckQueue:(MTTMessageEntity *)message;
-(BOOL)isInUnAckQueue:(MTTMessageEntity *)message;
@end


@interface MessageAndTime : NSObject
@property(strong)MTTMessageEntity *msg;
@property(assign)NSUInteger nowDate;
@end
