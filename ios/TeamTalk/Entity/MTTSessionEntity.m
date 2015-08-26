//
//  DDMTTSessionEntity.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-5.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTSessionEntity.h"
#import "DDUserModule.h"
#import "MTTDatabaseUtil.h"
#import "MTTGroupEntity.h"
#import "DDGroupModule.h"
#import "DDMessageModule.h"
#import "MTTUserEntity.h"
#import "MTTGroupEntity.h"
@implementation MTTSessionEntity
@synthesize  name;
@synthesize timeInterval;
- (void)setSessionID:(NSString *)sessionID
{
    _sessionID = [sessionID copy];
    name = nil;
    timeInterval = 0;
}

- (void)setSessionType:(SessionType)sessionType
{
    _sessionType = sessionType;
    name = nil;
    timeInterval = 0;
}

- (NSString*)name
{
    if (!name)
    {
        switch (self.sessionType)
        {
            case SessionTypeSessionTypeSingle:
            {
                [[DDUserModule shareInstance] getUserForUserID:_sessionID Block:^(MTTUserEntity *user) {
                    if ([user.nick length] > 0)
                    {
                        name = user.nick;
                    }
                    else
                    {
                        name = user.name;
                    }

                }];
        }
                break;
            case SessionTypeSessionTypeGroup:
            {
                MTTGroupEntity* group = [[DDGroupModule instance] getGroupByGId:_sessionID];
                if (!group) {
                    [[DDGroupModule instance] getGroupInfogroupID:_sessionID completion:^(MTTGroupEntity *group) {
                             name=group.name;
                    }];
                }else{
                     name=group.name;
                }
                
            }
                break;

        }
    }
    return name;
}
-(void)setSessionName:(NSString *)theName
{
    name = theName;
}
- (NSUInteger)timeInterval
{
    if (timeInterval == 0)
    {
        switch (_sessionType)
        {
            case SessionTypeSessionTypeSingle:
            {
                 [[DDUserModule shareInstance] getUserForUserID:_sessionID Block:^(MTTUserEntity *user) {
                      timeInterval = user.lastUpdateTime;
                }];
              
            }
            break;
                
        }
    }
    return timeInterval;
}

#pragma mark -
#pragma mark Public API
- (id)initWithSessionID:(NSString*)sessionID SessionName:(NSString *)name type:(SessionType)type
{
    MTTSessionEntity *session = [self initWithSessionID:sessionID type:type];
    [session setSessionName:name];
    return session;
}
- (id)initWithSessionID:(NSString*)sessionID type:(SessionType)type
{
    self = [super init];
    if (self)
    {
        self.sessionID = sessionID;
        self.sessionType = type;
        self.unReadMsgCount=0;
        self.lastMsg=@"";
        self.lastMsgID=0;
        self.timeInterval= [[NSDate date] timeIntervalSince1970];
        
    }
    return self;
}

- (void)updateUpdateTime:(NSUInteger)date
{
     timeInterval = date;
    self.timeInterval = timeInterval;
    [[MTTDatabaseUtil instance] updateRecentSession:self completion:^(NSError *error) {
                    
    }];
        
  }
-(NSArray*)sessionUsers
{
    if(SessionTypeSessionTypeGroup == self.sessionType)
    {
        MTTGroupEntity* group = [[DDGroupModule instance] getGroupByGId:_sessionID];
        return group.groupUserIds;
    }
    
    return  nil;
}
-(NSString *)getSessionGroupID
{
    return _sessionID;
}
-(BOOL)isGroup
{
    if(SessionTypeSessionTypeGroup == self.sessionType)
    {
        return YES;
    }
    return NO;
}

- (id)initWithSessionIDByUser:(MTTUserEntity*)user
{
    MTTSessionEntity *session = [self initWithSessionID:user.objID type:SessionTypeSessionTypeSingle];
    [session setSessionName:user.name];
    return session;
}
- (id)initWithSessionIDByGroup:(MTTGroupEntity*)group
{
    SessionType sessionType =SessionTypeSessionTypeGroup;
    MTTSessionEntity *session = [self initWithSessionID:group.objID type:sessionType];
    [session setSessionName:group.name];
    
    return session;
}
+(id)initWithDicToGroup:(NSDictionary *)dic
{
    MTTSessionEntity *session =[MTTSessionEntity new];
    return session;
}
- (BOOL)isEqual:(id)other
{
   
    if (other == self) {
        return YES;
    }  else if([self class] != [other class])
    {
        return NO;
    }else {
         MTTSessionEntity *otherSession = (MTTSessionEntity *)other;
        if (![self.sessionID isEqualToString:otherSession.sessionID]) {
            return NO;
        }
        if (self.sessionType != otherSession.sessionType) {
            return NO;
        }
       
        
    }
    return YES;
}

- (NSUInteger)hash
{
    NSUInteger sessionIDhash = [self.sessionID hash];
    
    return sessionIDhash^self.sessionType;
}
@end
