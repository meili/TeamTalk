//
//  NSObject+DDObject.m
//  DDSlowScrollViewTEST
//
//  Created by dxl dxl on 12-12-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "NSObject+Block.h"

@implementation NSObject (Block)

-(void)perform:(void (^)(void))performBlock{
    
    performBlock();
    
}

-(void)perform:(void (^)(void))performBlock andDelay:(NSTimeInterval)delay{
    
    [self performSelector:@selector(perform:) withObject:(__bridge id)Block_copy((__bridge const void *)performBlock) afterDelay:delay];
    
    Block_release((__bridge const void *)performBlock);
}

@end
