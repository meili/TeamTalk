//
//  DDAvatarManager.h
//  IMBaseSDK
//
//  Created by 独嘉 on 14/11/25.
//  Copyright (c) 2014年 mogujie. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MTTAvatarManager : NSObject

+ (instancetype)shareInstance;

- (void)addKey:(NSString*)key Avatar:(NSArray *)avatar forLayout:(NSArray *)layout;

@end
