//
//  DDAvatarManager.m
//  IMBaseSDK
//
//  Created by 独嘉 on 14/11/25.
//  Copyright (c) 2014年 mogujie. All rights reserved.
//

#import "MTTAvatarManager.h"
#import "UIImage+UIImageAddition.h"
#import "SDWebImageManager.h"
#import <SDWebImage/SDImageCache.h>
#import "NSURL+SafeURL.h"
#define DD_AVATAR_ARRAY         @"avatarUrlArray"
#define DD_AVATAR_LAYOUT        @"avatarLayout"

@interface MTTAvatarManager(PrivateAPI)

- (void)p_endCurrentGroupAvatarTaskAndStartNextTask;
- (void)p_startOneGroupAvatarCombination;
- (void)p_combinationCompletion;

@end

@implementation MTTAvatarManager
{
    NSMutableDictionary* _avatars;
    NSMutableDictionary* _downloadImages;
    NSUInteger _currentLeftImages;
    NSString* _currentAvatarKey;
    BOOL _currentDownloadImageCorrect;
}
+ (instancetype)shareInstance
{
    static MTTAvatarManager* g_avatarManager;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_avatarManager = [[MTTAvatarManager alloc] init];
    });
    return g_avatarManager;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _avatars = [[NSMutableDictionary alloc]init];
        _downloadImages = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)addKey:(NSString*)key Avatar:(NSArray *)avatar forLayout:(NSArray *)layout
{
    if (![[_avatars allKeys] containsObject:key])
    {
        NSDictionary* object = @{DD_AVATAR_ARRAY : avatar,
                                 DD_AVATAR_LAYOUT : layout};
        [_avatars setObject:object forKey:key];
        [self p_startOneGroupAvatarCombination];
    }
}

-(UIImage *)getAvatar:(NSString *)avatarUrl
{
    UIImage* image = [[UIImage alloc] init];
    if (avatarUrl) {
        if ([avatarUrl hasSuffix:@";"]) {
            avatarUrl = [avatarUrl substringToIndex:avatarUrl.length-1];
        }
        NSArray *avatars = [avatarUrl componentsSeparatedByString:@";"];
        NSMutableString* imageKey = [[NSMutableString alloc] init];
        for (NSUInteger i=0; i < [avatars count]; i++) {
            [imageKey stringByAppendingString:@"_"];
            [imageKey stringByAppendingString:[avatars objectAtIndex:i]];
        }
        if ([[SDImageCache sharedImageCache] diskImageExistsWithKey:imageKey]) {
            image = [[SDImageCache sharedImageCache] imageFromDiskCacheForKey:imageKey];
        }
    }
    return image;
}


#pragma mark -
#pragma mark PrivateAPI
- (void)p_startOneGroupAvatarCombination
{
    if ([_avatars count] > 0 && !_currentAvatarKey)
    {
        _currentAvatarKey = [_avatars allKeys][0];
        NSArray* avatarUrls = _avatars[_currentAvatarKey][DD_AVATAR_ARRAY];
        UIGraphicsBeginImageContext(CGSizeMake(100,100));
        _currentLeftImages = [avatarUrls count];
        _currentDownloadImageCorrect = YES;
        __weak MTTAvatarManager* weakSelf = self;
        [avatarUrls enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString* urlString = obj;
            if (![urlString hasSuffix:@"_100x100.jpg"] && [urlString length] > 0)
            {
                urlString = [urlString stringByAppendingString:@"_100x100.jpg"];
            }
            NSURL* url = [NSURL safeURLWithString:obj];
            [[SDWebImageManager sharedManager] downloadImageWithURL:url options:SDWebImageLowPriority progress:^(NSInteger receivedSize, NSInteger expectedSize) {
                
            } completed:^(UIImage *image, NSError *error, SDImageCacheType cacheType, BOOL finished, NSURL *imageURL) {
                _currentLeftImages --;
                if (!image)
                {
//                    image = [UIImage imageNamed:@"user_placeholder"];
                    image = [UIImage initWithColor:RGB(228, 227, 230) rect:CGRectMake(0, 0, 100, 100)];
                    if (url && [url absoluteString].length > 0)
                    {
                        _currentDownloadImageCorrect = NO;
                    }
                }
                [_downloadImages setObject:image forKey:@(idx)];
                if (_currentLeftImages <= 0)
                {
                    [weakSelf p_combinationCompletion];
                }
            }];
        }];
    }
}

- (void)p_combinationCompletion
{
    if (_currentDownloadImageCorrect)
    {
        UIGraphicsBeginImageContext(CGSizeMake(100,100));
        NSArray* frames = _avatars[_currentAvatarKey][DD_AVATAR_LAYOUT];
        [_downloadImages enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            NSUInteger index = [key integerValue];
            UIImage* image = obj;
            image = [UIImage roundCorners:image corner:10];
            CGRect frame = [frames[index] CGRectValue];
            [image drawInRect:frame];
        }];
        UIImage* combinationImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        [[SDImageCache sharedImageCache] storeImage:combinationImage forKey:_currentAvatarKey toDisk:YES];
    }
    [self p_endCurrentGroupAvatarTaskAndStartNextTask];
}

- (void)p_endCurrentGroupAvatarTaskAndStartNextTask
{
    if ([_avatars count] > 0 && _currentAvatarKey)
    {
        [_avatars removeObjectForKey:_currentAvatarKey];
        _downloadImages = nil;
        _downloadImages = [[NSMutableDictionary alloc] init];
        _currentAvatarKey = nil;
        [self p_startOneGroupAvatarCombination];
    }
}
@end
