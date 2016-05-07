//
//  UIView+PointBadge.h
//  TeamTalk
//
//  Created by 宪法 on 15/6/24.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>

#define PointBadgeRadius 4.0f

#define PointBadgeDiameter 2*PointBadgeRadius

@interface UIView (PointBadge)

@property (nonatomic)UIView *pointBadgeView;

-(void)showPointBadge:(BOOL)animated;

-(void)removePointBadge:(BOOL)animated;

@end
