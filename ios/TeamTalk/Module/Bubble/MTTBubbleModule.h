//
//  MTTBubble.h
//  TeamTalk
//
//  Created by scorpio on 15/7/2.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <Foundation/Foundation.h>

struct MTTBubbleContentInset {
    float top;
    float left;
    float bottom;
    float right;
};
typedef struct MTTBubbleContentInset MTTBubbleContentInset;

struct MTTBubbleVoiceInset {
    float top;
    float left;
    float bottom;
    float right;
};
typedef struct MTTBubbleVoiceInset MTTBubbleVoiceInset;

struct MTTBubbleStretchy {
    float left;
    float top;
};
typedef struct MTTBubbleStretchy MTTBubbleStretchy;

@class MTTBubbleConfig;
@interface MTTBubbleModule : NSObject

+ (instancetype)shareInstance;

- (MTTBubbleConfig*)getBubbleConfigLeft:(BOOL)left;

- (void)selectBubbleTheme:(NSString*)bubbleType left:(BOOL)left;

@end

@interface MTTBubbleConfig : NSObject

@property(nonatomic,assign)MTTBubbleContentInset inset;
@property(nonatomic,assign)MTTBubbleVoiceInset voiceInset;
@property(nonatomic,assign)MTTBubbleStretchy stretchy;
@property(nonatomic,assign)MTTBubbleStretchy imgStretchy;
@property(nonatomic,retain)UIColor* textColor;
@property(nonatomic,retain)UIColor* linkColor;
@property(nonatomic,retain)NSString* textBgImage;
@property(nonatomic,retain)NSString* picBgImage;

- (instancetype)initWithConfig:(NSString*)string left:(BOOL)left;

@end
