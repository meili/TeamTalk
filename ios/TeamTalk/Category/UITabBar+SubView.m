//
//  UITabBar+SubView.m
//  TeamTalk
//
//  Created by 宪法 on 15/6/24.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "UITabBar+SubView.h"

@implementation UITabBar (SubView)

-(UIView *)barButtonWithTitle:(NSString *)title{

    UIView *tabBarButton =nil;
    
    for (UIView *tabBarButtonTmp in self.subviews) {
        
        if ([tabBarButtonTmp isKindOfClass:NSClassFromString(@"UITabBarButton")]) {
            
            BOOL isFind =NO;
            for (UILabel *subLabel in tabBarButtonTmp.subviews) {
                if ([subLabel isKindOfClass:[UILabel class]]) {
                    if ([subLabel.text isEqualToString:title]) {
                        isFind =YES;
                        break;
                    }
                }
            }
            if (isFind) {
                tabBarButton =tabBarButtonTmp;
                break;
            }
        }
    }
    return tabBarButton;
}

-(UIImageView *)tabBarButtonImageViewWithTitle:(NSString *)title{

    UIView *tabBarButton =[self barButtonWithTitle:title];
    
    UIImageView *myTabBarButtonImage =nil;
    for (UIImageView *tabImage in tabBarButton.subviews) {
        if ([tabImage isKindOfClass:[UIImageView class]]) {
            myTabBarButtonImage =tabImage;
            break;
        }
    }
    
    return myTabBarButtonImage;
}

@end
