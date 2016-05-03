//
//  DDChattingEditModel.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-17.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "ChattingEditModule.h"
#import "DDCreateGroupAPI.h"
@interface ChattingEditModule (Private)
@property(nonatomic,strong)NSMutableArray *group;
@end
@implementation ChattingEditModule
/**
 *  初始化Model
 *
 *  @param sessionID 当前会话的ID
 *
 *  @return 返回Model对象
 */
- (instancetype)initChattingEditModel:(NSString *)sessionID
{
    self = [super init];
    if (self) {
        
    }
    return self;
}
/**
 *  添加一个人到当前聊天
 *
 *  @param userID 要添加的用户的id
 */

/**
 *  从当前的Group中删除一个人
 *
 *  @param userID 要删除的用户的id
 */
-(void)removePersonFromGroup:(NSArray *)userIDs Block:(void(^)(BOOL success))block
{
    
}

@end
