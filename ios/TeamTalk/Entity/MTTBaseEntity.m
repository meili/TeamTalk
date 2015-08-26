//
//  DDBaseEntity.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-16.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTBaseEntity.h"

@implementation MTTBaseEntity
-(NSUInteger)getOriginalID
{
    NSArray *array = [self.objID componentsSeparatedByString:@"_"];
    if (array[1]) {
        return [array[1] intValue];
    }
    return 0;
}

@end
