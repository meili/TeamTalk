//
//  DDEmotionsModule.h
//  Mogujie4iPhone
//
//  Created by 独嘉 on 14-6-23.
//  Copyright (c) 2014年 juangua. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface EmotionsModule : NSObject
@property (nonatomic,readonly)NSMutableArray* emotions;
@property (nonatomic,readonly)NSDictionary* emotionUnicodeDic;
@property (nonatomic,readonly)NSDictionary* unicodeEmotionDic;
@property (nonatomic,readonly)NSDictionary* emotionLength;
+ (instancetype)shareInstance;
@end
