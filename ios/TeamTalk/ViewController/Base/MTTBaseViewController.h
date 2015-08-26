//
//  MTTBaseViewController.h
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-28.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef NS_ENUM(int, MTTViewLifecycleState){

    MTTViewLifecycleInit =0,
    MTTViewLifecycleDidLoad,
    MTTViewLifecycleWillAppear,
    MTTViewLifecycleDidAppear,
    MTTViewLifecycleWillDisAppear,
    MTTViewLifecycleDidDisAppear,
};

@interface MTTBaseViewController : UIViewController

@property (nonatomic,assign) MTTViewLifecycleState viewLifecycleState;

-(void)setup;

- (void)pushViewController:(UIViewController *)viewController animated:(BOOL)animated;

- (UIViewController *)popViewControllerAnimated:(BOOL)animated;

- (NSArray *)popToViewController:(UIViewController *)viewController animated:(BOOL)animated;

@end
