//
//  RuntimeStatus.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-31.
//  Copyright (c) 2014 dujia. All rights reserved.
//


#import <Foundation/Foundation.h>

#define TheRuntime [RuntimeStatus instance]

@class MTTUserEntity;

@interface RuntimeStatus : NSObject

@property(nonatomic,strong)MTTUserEntity *user;
@property(nonatomic,copy)NSString *token;
@property(nonatomic,copy)NSString *userID;
//@property(copy)NSString *dao;
@property(nonatomic,copy)NSString *pushToken;
@property(nonatomic,strong)NSDictionary *updateInfo;

+ (instancetype)instance;

-(void)updateData;

@end
