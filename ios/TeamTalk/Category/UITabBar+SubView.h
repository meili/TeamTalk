//
//  UITabBar+SubView.h
//  TeamTalk
//
//  Created by 宪法 on 15/6/24.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UITabBar (SubView)

-(UIView *)barButtonWithTitle:(NSString *)title;

-(UIImageView *)tabBarButtonImageViewWithTitle:(NSString *)title;

@end
