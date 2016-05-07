#import <Foundation/Foundation.h>
#import "MTTBaseEntity.h"
static NSString* const GROUP_PRE = @"group_";          //group id 前缀

enum
{
    GROUP_TYPE_FIXED = 1,       //固定群
    GROUP_TYPE_TEMPORARY,       //临时群
};
@class GroupInfo;

@interface MTTGroupEntity : MTTBaseEntity

@property(copy) NSString *groupCreatorId;        //群创建者ID
@property(nonatomic,assign) int groupType;                //群类型
@property(nonatomic,strong) NSString* name;                  //群名称
@property(nonatomic,strong) NSString* avatar;                //群头像
@property(nonatomic,strong) NSMutableArray* groupUserIds;    //群用户列表ids
@property(nonatomic,readonly)NSMutableArray* fixGroupUserIds;//固定的群用户列表IDS，用户生成群头像
@property(strong)NSString *lastMsg;
@property(assign)BOOL isShield;
-(void)copyContent:(MTTGroupEntity*)entity;
+(UInt32)localGroupIDTopb:(NSString *)groupID;
+(NSString *)pbGroupIdToLocalID:(UInt32)groupID;
- (void)addFixOrderGroupUserIDS:(NSString*)ID;
+(MTTGroupEntity *)dicToMTTGroupEntity:(NSDictionary *)dic;
+(NSString *)getSessionId:(NSString *)groupId;
+(MTTGroupEntity *)initMTTGroupEntityFromPBData:(GroupInfo *)groupInfo;
@end
