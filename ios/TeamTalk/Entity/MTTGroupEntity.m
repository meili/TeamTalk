#import "MTTGroupEntity.h"
#import "MTTUserEntity.h"
#import "NSDictionary+Safe.h"
#import "MTTDatabaseUtil.h"
#import "IMBaseDefine.pb.h"

@implementation MTTGroupEntity

- (void)setGroupUserIds:(NSMutableArray *)groupUserIds
{
    if (_groupUserIds)
    {
        _groupUserIds = nil;
        _fixGroupUserIds = nil;
    }
//    _groupUserIds = groupUserIds;
    NSArray *tmp = [[NSArray alloc]init];
    tmp = [groupUserIds sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSString *uid1 = [obj1 stringByReplacingOccurrencesOfString:@"user_" withString:@""];
        NSString *uid2 = [obj2 stringByReplacingOccurrencesOfString:@"user_" withString:@""];
        if([uid1 integerValue]>[uid2 integerValue]){
            return NSOrderedAscending;
        }else{
            return NSOrderedDescending;
        }
    }];
    _groupUserIds = [[NSMutableArray alloc]initWithArray:tmp];
    [_groupUserIds enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [self addFixOrderGroupUserIDS:obj];
    }];
}

-(void)copyContent:(MTTGroupEntity*)entity
{
    self.groupType = entity.groupType;
    self.lastUpdateTime = entity.lastUpdateTime;
    self.name = entity.name;
    self.avatar = entity.avatar;
    self.groupUserIds = entity.groupUserIds;
}

+(NSString *)getSessionId:(NSString *)groupId
{
     return groupId;
}
+(NSString *)pbGroupIdToLocalID:(UInt32)groupID
{
    return [NSString stringWithFormat:@"%@%ld",GROUP_PRE,groupID];
}
+(UInt32)localGroupIDTopb:(NSString *)groupID
{
    if (![groupID hasPrefix:GROUP_PRE]) {
        return 0;
    }
    return [[groupID substringFromIndex:[GROUP_PRE length]] integerValue];
}
+(MTTGroupEntity *)initMTTGroupEntityFromPBData:(GroupInfo *)groupInfo
{
    MTTGroupEntity *group = [MTTGroupEntity new];
    group.objID=[self pbGroupIdToLocalID:groupInfo.groupId];
    group.objectVersion=groupInfo.version;
    group.name=groupInfo.groupName;
    group.avatar = groupInfo.groupAvatar;
    group.groupCreatorId = [MTTUtil changeOriginalToLocalID:groupInfo.groupCreatorId SessionType:SessionTypeSessionTypeSingle];
    group.groupType = groupInfo.groupType;
    group.isShield=groupInfo.shieldStatus;
    NSMutableArray *ids  = [NSMutableArray new];
    for (int i = 0; i<[[groupInfo groupMemberList] count]; i++) {
        [ids addObject:[MTTUtil changeOriginalToLocalID:[[groupInfo groupMemberList][i] integerValue] SessionType:SessionTypeSessionTypeSingle]];
    }
    group.groupUserIds = ids;
    group.lastMsg=@"";
    return group;
}
- (void)addFixOrderGroupUserIDS:(NSString*)ID
{
    if (!_fixGroupUserIds)
    {
        _fixGroupUserIds = [[NSMutableArray alloc] init];
    }
    [_fixGroupUserIds addObject:ID];
}

+(MTTGroupEntity *)dicToMTTGroupEntity:(NSDictionary *)dic
{
    MTTGroupEntity *group = [MTTGroupEntity new];
    group.groupCreatorId=[dic safeObjectForKey:@"creatID"];
    group.objID = [dic safeObjectForKey:@"groupId"];
    group.avatar = [dic safeObjectForKey:@"avatar"];
    group.GroupType = [[dic safeObjectForKey:@"groupType"] integerValue];
    group.name = [dic safeObjectForKey:@"name"];
    group.avatar = [dic safeObjectForKey:@"avatar"];
    group.isShield = [[dic safeObjectForKey:@"isshield"] boolValue];
    NSString *string =[dic safeObjectForKey:@"Users"];
    NSMutableArray *array =[NSMutableArray arrayWithArray:[string componentsSeparatedByString:@"-"]] ;
    if ([array count] >0) {
        group.groupUserIds=[array copy];
    }
    group.lastMsg =[dic safeObjectForKey:@"lastMessage"];
    group.objectVersion = [[dic safeObjectForKey:@"version"] integerValue];
    group.lastUpdateTime=[[dic safeObjectForKey:@"lastUpdateTime"] longValue];
    return group;
}
-(BOOL)theVersionIsChanged
{
    return YES;
}
-(void)updateGroupInfo
{
    
}
@end
