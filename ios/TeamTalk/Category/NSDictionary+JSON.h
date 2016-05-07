//
//  NSDictionary+JSON.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-15.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary (JSON)

- (NSString*)jsonString;
+ (NSDictionary*)initWithJsonString:(NSString*)json;

@end
