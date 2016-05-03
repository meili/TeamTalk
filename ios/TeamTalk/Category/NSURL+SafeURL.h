//
//  NSURL+SafeURL.h
//  IOSDuoduo
//
//  Created by 独嘉 on 15/4/7.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSURL (SafeURL)
+ (NSURL*)safeURLWithString:(NSString*)string;
@end
