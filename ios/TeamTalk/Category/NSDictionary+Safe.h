//
//  DDNSDictionary+Safe.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-29.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary(Safe)

- (id)safeObjectForKey:(id)key;
- (int)intValueForKey:(id)key;
- (double)doubleValueForKey:(id)key;
- (NSString*)stringValueForKey:(id)key;

@end


@interface NSMutableDictionary(Safe)

- (void)safeSetObject:(id)anObject forKey:(id)aKey;
- (void)setIntValue:(int)value forKey:(id)aKey;
- (void)setDoubleValue:(double)value forKey:(id)aKey;
- (void)setStringValueForKey:(NSString*)string forKey:(id)aKey;

@end

@interface NSArray (Exception)

- (id)objectForKey:(id)key;

@end
