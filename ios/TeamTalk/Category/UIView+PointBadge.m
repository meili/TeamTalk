//
//  UIView+PointBadge.m
//  TeamTalk
//
//  Created by 宪法 on 15/6/24.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "UIView+PointBadge.h"

#import <objc/runtime.h>

static char *UIViewPointBadge ="UIViewPointBadge";

@implementation UIView (PointBadge)

@dynamic pointBadgeView;

- (void)setPointBadgeView:(UIView *)pointBadgeView {
    objc_setAssociatedObject(self, &UIViewPointBadge, pointBadgeView,OBJC_ASSOCIATION_RETAIN);
}

- (UIView *)pointBadgeView {
    UIView *view =objc_getAssociatedObject(self, &UIViewPointBadge);
    return view;
}


-(void)showPointBadge:(BOOL)animated{

    UIView *pointBadgeView =[self pointBadgeView];
    
    if (pointBadgeView ==nil) {
        
        CGFloat selfWidth =CGRectGetWidth(self.frame);
        
        pointBadgeView =[[UIView alloc] initWithFrame:CGRectMake(selfWidth -PointBadgeRadius -1, -PointBadgeRadius +1, PointBadgeDiameter, PointBadgeDiameter)];
        pointBadgeView.layer.cornerRadius =PointBadgeRadius;
        pointBadgeView.hidden =YES;
        pointBadgeView.backgroundColor =[UIColor redColor];
        pointBadgeView.userInteractionEnabled =NO;
        [self addSubview:pointBadgeView];
        
        [self setPointBadgeView:pointBadgeView];
    }
    
    if (pointBadgeView.hidden) {
        
        if (animated) {
            pointBadgeView.alpha =0;
            pointBadgeView.hidden =NO;
            [UIView animateWithDuration:0.2 animations:^{
                pointBadgeView.alpha =1;
            }];
        }
        else{
            pointBadgeView.hidden =NO;
        }
    }
}

-(void)removePointBadge:(BOOL)animated{

    UIView *pointBadgeView =[self pointBadgeView];
    
    if (pointBadgeView) {
        
        if (animated) {
            [UIView animateWithDuration:0.2 animations:^{
                pointBadgeView.alpha =0;
            } completion:^(BOOL finish){
                pointBadgeView.hidden =YES;
                pointBadgeView.alpha =1;
            }];
        }
        else{
            pointBadgeView.hidden =YES;
        }
    }
}

@end
