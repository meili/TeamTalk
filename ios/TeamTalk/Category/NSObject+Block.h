//
//  NSObject+DDObject.h
//  DDSlowScrollViewTEST
//
//  Created by dxl dxl on 12-12-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSObject (Block)

-(void)perform:(void (^)(void))performBlock;

-(void)perform:(void (^)(void))performBlock andDelay:(NSTimeInterval)delay;

@end
