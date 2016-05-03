//
//  DDGroupModule.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-11.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTTGroupEntity.h"
typedef void(^GetGroupInfoCompletion)(MTTGroupEntity* group);
@interface DDGroupModule : NSObject
+ (instancetype)instance;
@property(assign)NSInteger recentGroupCount;
@property(strong) NSMutableDictionary* allGroups;         //所有群列表,key:group id value:MTTGroupEntity
@property(strong) NSMutableDictionary* allFixedGroup;     //所有固定群列表 
-(MTTGroupEntity*)getGroupByGId:(NSString*)gId;
-(void)addGroup:(MTTGroupEntity*)newGroup;
- (void)getGroupInfogroupID:(NSString*)groupID completion:(GetGroupInfoCompletion)completion;
-(NSArray*)getAllGroups;
@end
