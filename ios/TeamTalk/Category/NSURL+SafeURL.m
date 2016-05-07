//
//  NSURL+SafeURL.m
//  IOSDuoduo
//
//  Created by 独嘉 on 15/4/7.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "NSURL+SafeURL.h"

#define isValidURLString(string) ((string) != nil && ![string isKindOfClass:[NSNull class]])

@implementation NSURL (SafeURL)
+ (NSURL*)safeURLWithString:(NSString*)string
{
    if (isValidURLString(string))
    {
        return [NSURL URLWithString:string];
    }
    else
    {
        return nil;
    }
}
@end
