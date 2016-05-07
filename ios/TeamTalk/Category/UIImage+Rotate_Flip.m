//
//  UIImage+Rotate_Flip.m
//  SvImageEdit
//
//  Created by  maple on 5/14/13.
//  Copyright (c) 2013 smileEvday. All rights reserved.
//

#import "UIImage+Rotate_Flip.h"

@implementation UIImage (Rotate_Flip)

/*
 * @brief rotate image 90 with CounterClockWise
 */
- (UIImage*)rotate90CounterClockwise
{
    UIImage *image = nil;
    switch (self.imageOrientation) {
        case UIImageOrientationUp:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeft];
            break;
        }
        case UIImageOrientationDown:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRight];
            break;
        }
        case UIImageOrientationLeft:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDown];
            break;
        }
        case UIImageOrientationRight:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUp];
            break;
        }
        case UIImageOrientationUpMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRightMirrored];
            break;
        }
        case UIImageOrientationDownMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeftMirrored];
            break;
        }
        case UIImageOrientationLeftMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUpMirrored];
            break;
        }
        case UIImageOrientationRightMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDownMirrored];
            break;
        }
        default:
            break;
    }
    
    return image;
}

/*
 * @brief rotate image 90 with Clockwise
 */
- (UIImage*)rotate90Clockwise
{
    UIImage *image = nil;
    switch (self.imageOrientation) {
        case UIImageOrientationUp:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRight];
            break;
        }
        case UIImageOrientationDown:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeft];
            break;
        }
        case UIImageOrientationLeft:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUp];
            break;
        }
        case UIImageOrientationRight:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDown];
            break;
        }
        case UIImageOrientationUpMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeftMirrored];
            break;
        }
        case UIImageOrientationDownMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRightMirrored];
            break;
        }
        case UIImageOrientationLeftMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDownMirrored];
            break;
        }
        case UIImageOrientationRightMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUpMirrored];
            break;
        }
        default:
            break;
    }
    
    return image;
}

/*
 * @brief rotate image 180 degree
 */
- (UIImage*)rotate180
{
    UIImage *image = nil;
    switch (self.imageOrientation) {
        case UIImageOrientationUp:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDown];
            break;
        }
        case UIImageOrientationDown:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUp];
            break;
        }
        case UIImageOrientationLeft:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRight];
            break;
        }
        case UIImageOrientationRight:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeft];
            break;
        }
        case UIImageOrientationUpMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDownMirrored];
            break;
        }
        case UIImageOrientationDownMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUpMirrored];
            break;
        }
        case UIImageOrientationLeftMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRightMirrored];
            break;
        }
        case UIImageOrientationRightMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeftMirrored];
            break;
        }
        default:
            break;
    }
    
    return image;
}

/*
 * @brief rotate image to default orientation
 */
- (UIImage*)rotateImageToOrientationUp
{
    CGSize size = CGSizeMake(self.size.width * self.scale, self.size.height * self.scale);
    UIGraphicsBeginImageContext(size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextClearRect(context, CGRectMake(0, 0, size.width, size.height));
    
    [self drawInRect:CGRectMake(0, 0, size.width, size.height)];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

/*
 * @brief flip horizontal
 */
- (UIImage*)flipHorizontal
{
    UIImage *image = nil;
    switch (self.imageOrientation) {
        case UIImageOrientationUp:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUpMirrored];
            break;
        }
        case UIImageOrientationDown:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDownMirrored];
            break;
        }
        case UIImageOrientationLeft:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRightMirrored];
            break;
        }
        case UIImageOrientationRight:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeftMirrored];
            break;
        }
        case UIImageOrientationUpMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUp];
            break;
        }
        case UIImageOrientationDownMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDown];
            break;
        }
        case UIImageOrientationLeftMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRight];
            break;
        }
        case UIImageOrientationRightMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeft];
            break;
        }
        default:
            break;
    }
    
    return image;
}

/*
 * @brief flip vertical
 */
- (UIImage*)flipVertical
{
    UIImage *image = nil;
    switch (self.imageOrientation) {
        case UIImageOrientationUp:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDownMirrored];
            break;
        }
        case UIImageOrientationDown:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUpMirrored];
            break;
        }
        case UIImageOrientationLeft:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeftMirrored];
            break;
        }
        case UIImageOrientationRight:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRightMirrored];
            break;
        }
        case UIImageOrientationUpMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationDown];
            break;
        }
        case UIImageOrientationDownMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationUp];
            break;
        }
        case UIImageOrientationLeftMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationLeft];
            break;
        }
        case UIImageOrientationRightMirrored:
        {
            image = [UIImage imageWithCGImage:self.CGImage scale:1 orientation:UIImageOrientationRight];
            break;
        }
        default:
            break;
    }
    
    return image;
}

/*
 * @brief flip horizontal and vertical
 */
- (UIImage*)flipAll
{
    return [self rotate180];
}

@end