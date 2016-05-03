//
//  UIImage+UIImageAddition.h
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-30.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIImage (UIImageAddition)
+ (UIImage *)initWithColor:(UIColor*)color rect:(CGRect)rect;
+ (UIImage *)roundCorners:(UIImage*)img corner:(float)corner;
+ (UIImage*)maskImage:(UIImage *)image withMask:(UIImage *)maskImage;
@end
