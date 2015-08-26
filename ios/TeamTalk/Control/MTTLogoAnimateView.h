//
//  MTTLogoAnimateView.h
//  CustomAnimate
//
//  Created by 丁丁 on 15/3/17.
//  Copyright (c) 2015年 丁丁. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MTTLogoAnimateView : UIImageView

@property (nonatomic,assign) float process;

//默认 whiteColor
@property (nonatomic,strong) UIColor *processTintColor;

//原图，默认为 image
@property (nonatomic,strong) UIImage *orginImage;

//底图，默认为 image处理后的图片
@property (nonatomic,strong) UIImage *emptyImage;

@property (nonatomic,strong) UIActivityIndicatorView *activityIndicatorView;

-(void)showAnimation;

-(void)hideAnimation;

@end
