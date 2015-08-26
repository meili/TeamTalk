//
//  DDAvatarImageView.m
//  IMBaseSDK
//
//  Created by 独嘉 on 14/11/17.
//  Copyright (c) 2014年 mogujie. All rights reserved.
//

#import "MTTAvatarImageView.h"
#import "UIImage+UIImageAddition.h"
#import "UIImageView+WebCache.h"
#import "MTTAvatarManager.h"
#import "NSURL+SafeURL.h"
#import "UIView+Addition.h"
@interface MTTAvatarImageView(PrivateAPI)

- (void)p_setGroupAvatar:(NSString*)avatar avatars:(NSArray*)avatarImages;

//-------------

- (NSArray*)p_getAvatarLayout:(NSUInteger)avatarCount forScale:(NSUInteger)scale;

//-------------

- (CGSize)p_avatarSizeForCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale;

//-------------

- (NSUInteger)p_getRowForCount:(NSUInteger)avatarCount;

//-------------

- (NSUInteger)p_getCountInRow:(NSUInteger)row avatarCount:(NSUInteger)avatarCount;

//-------------

- (float)p_getUpAndDownGapForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale;

//-------------
- (float)p_getLeftAndRightForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale;

//-------------
- (float)p_getOriginStartYForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale;

- (float)p_getOriginStartXForAvatarCount:(NSUInteger)avatarCount row:(NSUInteger)row forScale:(NSUInteger)scale;
@end
@implementation MTTAvatarImageView
- (void)setAvatar:(NSString*)avatar group:(BOOL)group;
{
    [self removeAllSubviews];
    [self setImage:nil];
    if (group)
    {
        UIImage* image = [[SDImageCache sharedImageCache] imageFromDiskCacheForKey:avatar];
        if (image) {
            [self setImage:image];
        }
        else
        {
            NSString* avatarKey = [avatar copy];
            if ([avatar hasSuffix:@";"])
            {
                avatar = [avatar substringToIndex:[avatar length] - 1];
            }
            NSArray* avatarArray = [avatar componentsSeparatedByString:@";"];
            [self p_setGroupAvatar:avatarKey avatars:avatarArray];
        }
    }
    else
    {
        //只有一个头像
        NSURL* avatarURL = [NSURL safeURLWithString:avatar];
        UIImage* placeholdImage = [UIImage imageNamed:@"user_placeholder"];
        [self sd_setImageWithURL:avatarURL placeholderImage:placeholdImage];
    }
}

#pragma mark -
#pragma mark PrivateAPI
- (void)p_setGroupAvatar:(NSString*)avatar avatars:(NSArray*)avatarImages
{
    UIImage* placeholdImage = [UIImage imageNamed:@"user_placeholder"];
    NSArray* imageViewFrames = [self p_getAvatarLayout:[avatarImages count] forScale:1];
    NSArray* frames = [self p_getAvatarLayout:[avatarImages count] forScale:2];
    if (avatar&&[frames count] > 0)
    {
        [[MTTAvatarManager shareInstance] addKey:avatar Avatar:avatarImages forLayout:frames];

    }
    for (NSInteger index = 0; index < [imageViewFrames count]; index ++)
    {
        CGRect frame = [imageViewFrames[index] CGRectValue];
        UIImageView* imageView = [[UIImageView alloc] initWithFrame:frame];
        [imageView.layer setCornerRadius:2];
        [imageView setClipsToBounds:YES];
        [imageView setUserInteractionEnabled:NO];
        NSString* avatar = avatarImages[index];
        NSURL* url = [NSURL safeURLWithString:avatar];
        [imageView sd_setImageWithURL:url placeholderImage:placeholdImage];
        [self addSubview:imageView];
    }
}

- (NSArray*)p_getAvatarLayout:(NSUInteger)avatarCount forScale:(NSUInteger)scale
{
    CGSize size = [self p_avatarSizeForCount:avatarCount forScale:scale];
    NSUInteger row = [self p_getRowForCount:avatarCount];
    NSMutableArray* frames = [[NSMutableArray alloc] init];
    for (NSUInteger rowIndex = 0; rowIndex < row; rowIndex ++)
    {
        NSUInteger countInRow = [self p_getCountInRow:rowIndex avatarCount:avatarCount];
        for (NSUInteger index = 0; index < countInRow; index ++)
        {
            float leftRightGap = [self p_getLeftAndRightForAvatarCount:avatarCount forScale:scale];
            float upDownGap = [self p_getUpAndDownGapForAvatarCount:avatarCount forScale:scale];
            float startX = [self p_getOriginStartXForAvatarCount:avatarCount row:rowIndex forScale:scale];
            float startY = [self p_getOriginStartYForAvatarCount:avatarCount forScale:scale];
            float x = startX + leftRightGap * index + index * size.width;
            float y = startY + upDownGap * rowIndex + rowIndex * size.height;
            CGRect frame = CGRectMake(x, y, size.width, size.height);
            [frames addObject:[NSValue valueWithCGRect:frame]];
        }
    }
    return frames;
}

- (CGSize)p_avatarSizeForCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale
{
    switch (avatarCount)
    {
        case 1:
            return CGSizeMake(22 * scale, 22  * scale);
        case 2:
        case 3:
        case 4:
            return CGSizeMake(22 * scale, 22 * scale);
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            return CGSizeMake(14 * scale, 14 * scale);
        default:
            break;
    }
    return CGSizeZero;
}

- (float)p_getUpAndDownGapForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale
{
    switch (avatarCount)
    {
        case 1:
        case 2:
            return 0;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            return 2 * scale;
    }
    return 0;
}

- (float)p_getLeftAndRightForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale
{
    return 2 * scale;
}

- (float)p_getOriginStartYForAvatarCount:(NSUInteger)avatarCount forScale:(NSUInteger)scale
{
    switch (avatarCount)
    {
        case 1:
        case 2:
            return 14 * scale;
            break;
        case 3:
        case 4:
            return 2 * scale;
        case 5:
        case 6:
            return 10 * scale;
        case 7:
        case 8:
        case 9:
            return 2 * scale;
    }
    return 0;
}

- (float)p_getOriginStartXForAvatarCount:(NSUInteger)avatarCount row:(NSUInteger)row forScale:(NSUInteger)scale
{
    if (avatarCount == 1 || avatarCount == 3)
    {
        if (row == 0)
        {
            return 14 * scale;
        }
    }
    if (avatarCount == 5 || avatarCount == 8)
    {
        if (row == 0)
        {
            return 10 * scale;
        }
    }
    if (avatarCount == 7)
    {
        if (row == 0)
        {
            return 18 * scale;
        }
    }
    return 2 * scale;
}

- (NSUInteger)p_getRowForCount:(NSUInteger)avatarCount
{
    switch (avatarCount)
    {
        case 1:
        case 2:
            return 1;
        case 3:
        case 4:
        case 5:
        case 6:
            return 2;
        case 7:
        case 8:
        case 9:
            return 3;
    }
    return 0;
}

- (NSUInteger)p_getCountInRow:(NSUInteger)row avatarCount:(NSUInteger)avatarCount
{
    if (avatarCount <= 4)
    {
        if (avatarCount <= 2)
        {
            return avatarCount;
        }
        else
        {
            if (row == 0)
            {
                return avatarCount % 2 == 0 ? 2 : avatarCount % 2;
            }
            else
            {
                return 2;
            }
        }
    }
    else
    {
        if (row == 0)
        {
            return avatarCount % 3 == 0 ? 3 : avatarCount % 3;
        }
        else
        {
            return 3;
        }
    }
}

@end
