//
//  UIAlertView+Block.m
//  PNCMobileBank-bbw
//
//  Created by 丁丁 on 14-11-21.
//  Copyright (c) 2014年 张瑞. All rights reserved.
//

#import "UIAlertView+Block.h"

#import <objc/runtime.h>

#ifndef SelectBlock
#define SelectBlock @"__SelectBlock"
#endif

@interface UIAlertView ()<UIAlertViewDelegate>
@end

@implementation UIAlertView (Block)

-(void)setSelectBlock:(void (^)(NSInteger index))select{
    
    objc_setAssociatedObject(self, SelectBlock, select, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

-(void (^)(NSInteger index))selectBlock{
    
    return objc_getAssociatedObject(self, SelectBlock);
}

+ (instancetype)alertWithTitle:(NSString *)title message:(NSString *)message  buttonIndex:(void (^)(NSInteger index))buttonIndex cancelButtonTitle:(NSString *)cancelButtonTitle otherButtonTitles:(NSString *)otherButtonTitles, ...{

    UIAlertView *alert =[[UIAlertView alloc] initWithTitle:title message:message delegate:nil cancelButtonTitle:cancelButtonTitle otherButtonTitles:otherButtonTitles, nil];
    
    alert.selectBlock =buttonIndex;
    
    alert.delegate =alert;
    
    return alert;
}

+ (instancetype)alertWithTitle:(NSString *)title message:(NSString *)message buttonIndex:(void (^)(NSInteger))buttonIndex cancelButtonTitle:(NSString *)cancelButtonTitle otherButtonTitles:(NSArray *)otherButtonTitles count:(NSInteger)count{
    
    UIAlertView *alert =[[UIAlertView alloc] initWithTitle:title message:message delegate:nil cancelButtonTitle:cancelButtonTitle otherButtonTitles:otherButtonTitles[0],otherButtonTitles[1], nil];
    
    alert.selectBlock =buttonIndex;
    
    alert.delegate =alert;
    
    return alert;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{

    if ([self selectBlock] !=nil) {
        [self selectBlock](buttonIndex);
    }
}

@end
