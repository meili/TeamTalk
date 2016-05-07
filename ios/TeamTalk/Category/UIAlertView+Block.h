//
//  UIAlertView+Block.h
//  PNCMobileBank-bbw
//
//  Created by 丁丁 on 14-11-21.
//  Copyright (c) 2014年 张瑞. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIAlertView (Block)

+ (instancetype)alertWithTitle:(NSString *)title message:(NSString *)message  buttonIndex:(void (^)(NSInteger index))buttonIndex cancelButtonTitle:(NSString *)cancelButtonTitle otherButtonTitles:(NSString *)otherButtonTitles, ...;

+ (instancetype)alertWithTitle:(NSString *)title message:(NSString *)message  buttonIndex:(void (^)(NSInteger index))buttonIndex cancelButtonTitle:(NSString *)cancelButtonTitle otherButtonTitles:(NSArray *)otherButtonTitles count:(NSInteger)count;

@end
