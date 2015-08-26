//
//  DDEmotionsModule.m
//  Mogujie4iPhone
//
//  Created by 独嘉 on 14-6-23.
//  Copyright (c) 2014年 juangua. All rights reserved.
//

#import "EmotionsModule.h"

@implementation EmotionsModule
{
//    NSDictionary* _emotionUnicodeDic;
//    NSDictionary* _unicodeEmotionDic;
//    NSArray* _emotions;
}

+ (instancetype)shareInstance
{
    static EmotionsModule* g_emotionsModule;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_emotionsModule = [[EmotionsModule alloc] init];
        
    });
    return g_emotionsModule;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        _emotionUnicodeDic = @{@"[牙牙撒花]":@"emotions.bundle/221.gif",
                               @"[牙牙尴尬]":@"emotions.bundle/222.gif",
                               @"[牙牙大笑]":@"emotions.bundle/223.gif",
                               @"[牙牙组团]":@"emotions.bundle/224.gif",
                               @"[牙牙凄凉]":@"emotions.bundle/225.gif",
                               @"[牙牙吐血]":@"emotions.bundle/226.gif",
                               @"[牙牙花痴]":@"emotions.bundle/227.gif",
                               @"[牙牙疑问]":@"emotions.bundle/228.gif",
                               @"[牙牙爱心]":@"emotions.bundle/229.gif",
                               @"[牙牙害羞]":@"emotions.bundle/230.gif",
                               @"[牙牙牙买碟]":@"emotions.bundle/231.gif",
                               @"[牙牙亲一下]":@"emotions.bundle/232.gif",
                               @"[牙牙大哭]":@"emotions.bundle/233.gif",
                               @"[牙牙愤怒]":@"emotions.bundle/234.gif",
                               @"[牙牙挖鼻屎]":@"emotions.bundle/235.gif",
                               @"[牙牙嘻嘻]":@"emotions.bundle/236.gif",
                               @"[牙牙漂漂]":@"emotions.bundle/237.gif",
                               @"[牙牙冰冻]":@"emotions.bundle/238.gif",
                               @"[牙牙傲娇]":@"emotions.bundle/239.gif",
                             
                               };
        _unicodeEmotionDic = [[NSMutableDictionary alloc] init];
        [_emotionUnicodeDic enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            [_unicodeEmotionDic setValue:key forKey:obj];
        }];
        _emotions = [[NSMutableArray alloc] initWithArray:[_emotionUnicodeDic allKeys]];
        
        _emotionLength = [[NSMutableDictionary alloc] init];
        [_emotions enumerateObjectsUsingBlock:^(NSString *obj, NSUInteger idx, BOOL *stop) {
            NSString *string = _emotionUnicodeDic[obj];
            [_emotionLength setValue:@([string length]) forKeyPath:obj];
        }];
    }
    return self;
}
@end
