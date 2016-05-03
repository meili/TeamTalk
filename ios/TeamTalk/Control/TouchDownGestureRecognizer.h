//
//  DDTouchDownGestureRecognizer.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-11.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef void(^DDTouchDown)();
typedef void(^DDTouchMoveOutside)();
typedef void(^DDTouchMoveInside)();
typedef void(^DDTouchEnd)(BOOL inside);
@class DDCustomButton;
@interface TouchDownGestureRecognizer : UIGestureRecognizer
@property (nonatomic,copy) DDTouchDown touchDown;
@property (nonatomic,copy) DDTouchMoveOutside moveOutside;
@property (nonatomic,copy) DDTouchMoveInside moveInside;
@property (nonatomic,copy) DDTouchEnd touchEnd;

@end
