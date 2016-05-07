//
//  ContactsModel.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-21.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "ContactsModule.h"
#import "NSDictionary+Safe.h"
#import "DDDepartmentAPI.h"
#import "MTTDepartment.h"
#import "DDFixedGroupAPI.h"
#import "MTTDatabaseUtil.h"
#import "DDGroupModule.h"
#import "RuntimeStatus.h"
#import "DDUserModule.h"
#import "MTTGroupEntity.h"
#import "SpellLibrary.h"
@implementation ContactsModule
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.groups = [NSMutableArray new];
       
    }
    return self;
}


-(void)addContact:(MTTUserEntity *)user
{
    
}
/**
 *  按首字母展示
 *
 *  @return 返回界面需要的字典类型
 */
-(NSMutableDictionary *)sortByContactPy 
{
    NSMutableDictionary *dic = [NSMutableDictionary new];
    for (MTTUserEntity * user in [[DDUserModule shareInstance] getAllMaintanceUser]) {

        NSString *fl = [user.pyname substringWithRange:NSMakeRange(0, 1)];
        if ([dic safeObjectForKey:fl]) {
            NSMutableArray *arr = [dic safeObjectForKey:fl];
            [arr addObject:user];
        }else
        {
            NSMutableArray *arr = [[NSMutableArray alloc] initWithArray:@[user]];
            [dic setObject:arr forKey:fl];
        }
    }
    [dic enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSArray *preData = obj;
        NSMutableArray *tmp = [[NSMutableArray alloc]initWithArray:preData];
        NSArray *sortDescriptors = [NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"pyname" ascending:YES]];
        [tmp sortUsingDescriptors:sortDescriptors];
        [dic safeSetObject:tmp forKey:key];
    }];
     return dic;
}
/**
 *  按部门分类展示
 *
 *  @return 返回界面需要的字典类型
 */
-(NSMutableDictionary *)sortByDepartment
{
    NSMutableDictionary *dic = [NSMutableDictionary new];
    for (MTTUserEntity * user in [[DDUserModule shareInstance] getAllMaintanceUser]) {
        if ([dic safeObjectForKey:user.department]) {
            NSMutableArray *arr = [dic safeObjectForKey:user.department];
            [arr addObject:user];
        }else{
            NSMutableArray *arr = [[NSMutableArray alloc] initWithArray:@[user]];
            [dic safeSetObject:arr forKey:user.department];
        }
    }
    [dic enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSArray *preData = obj;
        NSMutableArray *tmp = [[NSMutableArray alloc]initWithArray:preData];
        NSArray *sortDescriptors = [NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"pyname" ascending:YES]];
        [tmp sortUsingDescriptors:sortDescriptors];
        [dic safeSetObject:tmp forKey:key];
    }];
    return dic;
    
}
/**
 *  获取本地收藏的联系人
 *
 *  @return 界面收藏联系人列表
 */
+(NSArray *)getFavContact
{
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSArray *arr = [userDefaults objectForKey:@"favuser"];
    NSMutableArray *contacts = [NSMutableArray new];
    [arr enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
       [contacts addObject:[MTTUserEntity dicToUserEntity:(NSDictionary *)obj]] ;
    }];
    return contacts;
}
/**
 *  收藏联系人接口
 *
 *  @param user 联系人对象
 */
+(void)favContact:(MTTUserEntity *)user
{
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if ([userDefaults objectForKey:@"favuser"] == nil) {
        [userDefaults setObject:@[[MTTUserEntity userToDic:user]] forKey:@"favuser"];
    }else
    {
        NSMutableArray *arr = [NSMutableArray arrayWithArray:[userDefaults objectForKey:@"favuser"]];
        if ([arr count] == 0) {
            [arr addObject:[MTTUserEntity userToDic:user]];
            [userDefaults setObject:arr forKey:@"favuser"];
            return;
        }
        for (int i = 0;i<[arr count];i++) {
            NSDictionary *dic = [arr objectAtIndex:i];
            if ([[dic objectForKey:@"userId"] isEqualToString: user.objID]) {
                [arr removeObject:dic];
                [userDefaults setObject:arr forKey:@"favuser"];
                return;
            }else
            {
                if ([[arr objectAtIndex:i] isEqualToDictionary:[arr lastObject]]) {
                    [arr addObject:[MTTUserEntity userToDic:user]];
                    [userDefaults setObject:arr forKey:@"favuser"];
                    return;
                }
                
            }
        }
        
      
    }
}
/**
 *  检查是否在收藏的联系人里
 *
 *  @param user 用户对象
 *
 *  @return 返回yes表示在收藏的联系人里
 */
-(BOOL)isInFavContactList:(MTTUserEntity *)user
{
      NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSMutableArray *arr = [NSMutableArray arrayWithArray:[userDefaults objectForKey:@"favuser"]];
    for (int i = 0;i<[arr count];i++) {
        NSDictionary *dic = [arr objectAtIndex:i];
        if ([[dic objectForKey:@"userId"] integerValue] == user.objID) {
            return YES;
        }
    }
    return NO;
}
+(void)getDepartmentData:(void(^)(id response))block
{
    DMTTDepartmentAPI* api = [[DMTTDepartmentAPI alloc] init];
    [api requestWithObject:nil Completion:^(id response, NSError *error) {
        if (!error)
        {
            if (response)
            {
                block(response);
                
            }
            else
            {
                block(nil);
            }
        }
        else
        {
            DDLog(@"error:%@",[error domain]);
            block(nil);
        }
    }];
}

@end
