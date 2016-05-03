//
//  DDRuntimeSwizzle.m
//  TeamTalk
//
//  Created by zuoye on 15/7/16.
//  Copyright (c) 2015年 mogujie. All rights reserved.
//

#import "DDRuntimeSwizzle.h"
#import <objc/runtime.h>


@implementation NSDictionary (FMNilInsertableDictionary)

+ (void)load {
    ClassMethodSwizzle(NSDictionary.class, @selector(dictionaryWithObjects:forKeys:count:), @selector(_DDNilInsertableDictionary_dictionaryWithObjects:forKeys:count:));
}
+ (instancetype)_DDNilInsertableDictionary_dictionaryWithObjects:(const id [])objects forKeys:(const id<NSCopying> [])keys count:(NSUInteger)cnt {
    NSMutableDictionary* dictionary = NSMutableDictionary.new;
    
    for (int i = 0; i < cnt; i++) {
        if (!objects[i]) continue;	 // nil safe
        [dictionary setObject:objects[i] forKey:keys[i]];
    }
    
    NSDictionary* dict = dictionary.copy;
    return dict;
}

@end

@implementation NSArray (FMNilInsertableArray)

+ (void)load {
    ClassMethodSwizzle(NSArray.class, @selector(arrayWithObjects:count:), @selector(_DDNilInsertableArray_arrayWithObjects:count:));
}
+ (instancetype)_DDNilInsertableArray_arrayWithObjects:(const id [])objects count:(NSUInteger)cnt {
    NSMutableArray* mutableArray = NSMutableArray.new;
    
    for (int i = 0; i < cnt; i++) {
        if (!objects[i]) continue;
        [mutableArray addObject:objects[i]];
    }
    
    NSArray* array = mutableArray.copy;
    return array;
}



@end

void ClassMethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel) {
    
    Method orig_method = nil, alt_method = nil;
    
    orig_method = class_getClassMethod(aClass, orig_sel);
    alt_method = class_getClassMethod(aClass, alt_sel);
    
    if ((orig_method != nil) && (alt_method != nil)) {

        IMP temp2;
        
        temp2 = method_getImplementation(orig_method);
        method_setImplementation(orig_method, method_getImplementation(alt_method));
        method_setImplementation(alt_method, temp2);
    }
}

void MethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel) {
    
    Method orig_method = nil, alt_method = nil;
    
    orig_method = class_getInstanceMethod(aClass, orig_sel);
    alt_method = class_getInstanceMethod(aClass, alt_sel);
    

    if ((orig_method != nil) && (alt_method != nil)) {

        IMP temp2;
        
        
        temp2 = method_getImplementation(orig_method);
        method_setImplementation(orig_method, method_getImplementation(alt_method));
        method_setImplementation(alt_method, temp2);
    }
}