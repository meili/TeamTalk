//
//  DDSearch.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-22.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void(^SearchCompletion)(NSArray* result,NSError* error);

@interface DDSearch : NSObject
+ (instancetype)instance;
- (void)searchContent:(NSString*)content completion:(SearchCompletion)completion;
- (void)searchContent:(NSString *)content inRange:(NSArray*)ranges completion:(SearchCompletion)completion;
- (void)searchDepartment:(NSString*)content completion:(SearchCompletion)completion;
@end
