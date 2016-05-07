//
//  MTTUserEntity.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTUserEntity.h"
#import "NSDictionary+Safe.h"
#import "PublicProfileViewControll.h"
#define USER_PRE @"user_"
#import "MTTDatabaseUtil.h"
#import "IMBuddy.pb.h"
@implementation MTTUserEntity
- (id)initWithUserID:(NSString*)userID name:(NSString*)name nick:(NSString*)nick avatar:(NSString*)avatar userRole:(NSInteger)userRole userUpdated:(NSUInteger)updated
{
    self = [super init];
    if (self)
    {
        self.objID = [userID copy];
        _name = [name copy];
        _nick = [nick copy];
        _avatar = [avatar copy];

        self.lastUpdateTime = updated;
    }
    return self;
}
//- (NSString*)avatar
//{
//    if (![_avatar hasSuffix:@"_100x100"])
//    {
//        return [NSString stringWithFormat:@"%@_100x100",_avatar];
//    }
//    return _avatar;
//}

+(NSMutableDictionary *)userToDic:(MTTUserEntity *)user
{
    NSMutableDictionary *dic = [NSMutableDictionary new];
    [dic safeSetObject:user.objID forKey:@"userId"];
    [dic safeSetObject:user.name forKey:@"name"];
    [dic safeSetObject:user.nick forKey:@"nick"];
    [dic safeSetObject:user.avatar forKey:@"avatar"];
    [dic safeSetObject:user.departId forKey:@"departId"];
    [dic safeSetObject:user.email forKey:@"email"];
    [dic safeSetObject:user.department forKey:@"department"];
    [dic safeSetObject:user.position forKey:@"position"];
    [dic safeSetObject:user.telphone forKey:@"telphone"];
    [dic safeSetObject:user.department forKey:@"departName"];
    [dic safeSetObject:[NSNumber numberWithInt:user.sex ]forKey:@"sex"];
    [dic safeSetObject:[NSNumber numberWithInt:user.lastUpdateTime] forKey:@"lastUpdateTime"];
    return dic;
}
- (void) encodeWithCoder:(NSCoder *)encoder {

    [encoder encodeObject:self.objID forKey:@"userId"];
    [encoder encodeObject:self.name forKey:@"name"];
    [encoder encodeObject:self.nick forKey:@"nick"];
    [encoder encodeObject:self.avatar forKey:@"avatar"];
    [encoder encodeObject:self.departId forKey:@"departId"];
    [encoder encodeObject:self.email forKey:@"email"];
    [encoder encodeObject:self.department forKey:@"department"];
    [encoder encodeObject:self.position forKey:@"position"];
    [encoder encodeObject:self.telphone forKey:@"telphone"];
    [encoder encodeObject:[NSNumber numberWithInt:self.sex ]forKey:@"sex"];
    [encoder encodeObject:[NSNumber numberWithInt:self.lastUpdateTime] forKey:@"lastUpdateTime"];
}
- (id)initWithCoder:(NSCoder *)aDecoder
{
    if((self = [super init])) {
        self.objID = [aDecoder decodeObjectForKey:@"userId"];
        self.name = [aDecoder decodeObjectForKey:@"name"];
        self.nick = [aDecoder decodeObjectForKey:@"nickName"];

        self.avatar = [aDecoder decodeObjectForKey:@"avatar"];
        self.department = [aDecoder decodeObjectForKey:@"department"];
        self.departId = [aDecoder decodeObjectForKey:@"departId"];
        self.email = [aDecoder decodeObjectForKey:@"email"];
        self.position = [aDecoder decodeObjectForKey:@"position"];
        self.telphone = [aDecoder decodeObjectForKey:@"telphone"];

    }
    return self;
    
}


//@"serverTime":@(serverTime),
//@"result":@(loginResult),
//@"state":@(state),
//@"nickName":nickName,
//@"userId":userId,
//@"title":title,
//@"position":position,
//@"isDeleted":@(isDeleted),
//@"sex":@(sex),
//@"departId":departId,
//@"jobNum":@(jobNum),
//@"telphone":telphone,
//@"email":email,
//@"creatTime":@(creatTime),
//@"updateTime":@(updateTime),
//@"token":token,
//@"userType":@(userType)
+(id)dicToUserEntity:(NSDictionary *)dic
{
    MTTUserEntity *user = [MTTUserEntity new];
    user.objID = [dic safeObjectForKey:@"userId"];
    user.name = [dic safeObjectForKey:@"name"];
    user.nick = [dic safeObjectForKey:@"nickName"]?[dic safeObjectForKey:@"nickName"]:user.name;

    user.avatar = [dic safeObjectForKey:@"avatar"];
    user.department = [dic safeObjectForKey:@"department"];
    user.departId =[dic safeObjectForKey:@"departId"];
    user.email = [dic safeObjectForKey:@"email"];
    user.position = [dic safeObjectForKey:@"position"];
    user.telphone = [dic safeObjectForKey:@"telphone"];
    user.sex = [[dic safeObjectForKey:@"sex"] integerValue];
    user.lastUpdateTime = [[dic safeObjectForKey:@"lastUpdateTime"] integerValue];
    user.pyname = [dic safeObjectForKey:@"pyname"];
    user.signature = [dic safeObjectForKey:@"signature"];
    return user;

}
-(void)sendEmail
{
    NSString *stringURL =[NSString stringWithFormat:@"mailto:%@",self.email];
    NSURL *url = [NSURL URLWithString:stringURL];
    [[UIApplication sharedApplication] openURL:url];
}
-(void)callPhoneNum
{
    NSString *string = [NSString stringWithFormat:@"tel:%@",self.telphone];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:string]];
}
- (BOOL)isEqual:(id)other
{
    if (other == self) {
        return YES;
    }else if([self class] != [other class])
    {
        return NO;
    }else {
        MTTUserEntity *otherUser = (MTTUserEntity *)other;
        if (![otherUser.objID isEqualToString:self.objID]) {
            return NO;
        }
        if (![otherUser.name isEqualToString:self.name]) {
            return NO;
        }
        if (![otherUser.nick isEqualToString:self.nick]) {
            return NO;
        }
        if (![otherUser.pyname isEqualToString:self.pyname]) {
            return NO;
        }
    }
    return YES;
}

- (NSUInteger)hash
{
    NSUInteger objIDHash = [self.objID hash];
    NSUInteger nameHash = [self.name hash];
    NSUInteger nickHash = [self.nick hash];
    NSUInteger pynameHash = [self.pyname hash];
    
    return objIDHash^nameHash^nickHash^pynameHash;
}
+(NSString *)pbUserIdToLocalID:(NSUInteger)userID
{
    return [NSString stringWithFormat:@"%@%ld",USER_PRE,userID];
}
+(UInt32)localIDTopb:(NSString *)userid
{
    if (![userid hasPrefix:USER_PRE]) {
        return 0;
    }
    return [[userid substringFromIndex:[USER_PRE length]] integerValue];
}

-(id)initWithPB:(UserInfo *)pbUser
{
    self = [super init];
    if (self) {
        self.objID = [[self class] pbUserIdToLocalID:pbUser.userId];
        self.name  = pbUser.userRealName;
        self.nick  = pbUser.userNickName;
        self.avatar= pbUser.avatarUrl;
        self.department = @(pbUser.departmentId);
        self.departId = @"";
        self.telphone = pbUser.userTel;
        self.sex =   pbUser.userGender;
        self.email = pbUser.email;
        self.pyname = pbUser.userDomain;
        self.userStatus = pbUser.status;
        self.signature = pbUser.signInfo;
    }
    return self;
}

-(NSString *)getAvatarUrl
{
    return [NSString stringWithFormat:@"%@_100x100.jpg",self.avatar];
}
-(NSString *)get300AvatarUrl
{
    return [NSString stringWithFormat:@"%@_310x310.jpg",self.avatar];
}
-(NSString *)getAvatarPreImageUrl
{
    return [NSString stringWithFormat:@"%@_640×999.jpg",self.avatar];
}
@end
