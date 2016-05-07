//
//  ContactsModel.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-21.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTTUserEntity.h"
@interface ContactsModule : NSObject
@property(strong)NSMutableArray *groups;
@property(assign)int contactsCount;
-(NSMutableDictionary *)sortByContactPy;
-(NSMutableDictionary *)sortByDepartment;
+(void)favContact:(MTTUserEntity *)user;
+(NSArray *)getFavContact;
-(BOOL)isInFavContactList:(MTTUserEntity *)user;
+(void)getDepartmentData:(void(^)(id response))block;
@end
