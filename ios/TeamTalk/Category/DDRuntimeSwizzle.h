//
//  DDRuntimeSwizzle.h
//  TeamTalk
//
//  Created by zuoye on 15/7/16.
//  Copyright (c) 2015年 mogujie. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary (DDNilInsertableDictionary)

@end

@interface NSArray (DDNilInsertableArray)

@end

void ClassMethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel);
void MethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel);