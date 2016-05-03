//
//  MTTBubble.m
//  TeamTalk
//
//  Created by scorpio on 15/7/2.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBubbleModule.h"
#import "MTTUtil.h"

@implementation MTTBubbleModule
{
    MTTBubbleConfig* _left_config;
    MTTBubbleConfig* _right_config;
}
+ (instancetype)shareInstance
{
    static MTTBubbleModule* g_bubbleModule;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_bubbleModule = [[MTTBubbleModule alloc] init];
    });
    return g_bubbleModule;
}

- (instancetype)init
{
    //
    self = [super init];
    if (self)
    {
        NSString* leftBubbleType = [MTTUtil getBubbleTypeLeft:YES];
        NSString* rightBubbleType = [MTTUtil getBubbleTypeLeft:NO];
        NSString* leftBubblePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/config.json", leftBubbleType];
        NSString* rightBubblePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/config.json", rightBubbleType];
        NSString* leftPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:leftBubblePath];
        NSString* rightPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:rightBubblePath];

        _left_config = [[MTTBubbleConfig alloc] initWithConfig:leftPath left:YES];
        _right_config = [[MTTBubbleConfig alloc] initWithConfig:rightPath left:NO];
        
    }
    return self;
    
}

- (MTTBubbleConfig*)getBubbleConfigLeft:(BOOL)left
{
    if(left){
        return _left_config;
    }
    return _right_config;
}

- (void)selectBubbleTheme:(NSString *)bubbleType left:(BOOL)left
{
    [MTTUtil setBubbleTypeLeft:bubbleType left:left];
    NSString* path = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/config.json", bubbleType];
    NSString* realPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:path];
    if(left){
        _left_config = [[MTTBubbleConfig alloc] initWithConfig:realPath left:(BOOL)left];
    }else{
        _right_config = [[MTTBubbleConfig alloc] initWithConfig:realPath left:(BOOL)left];
    }
}

@end

@implementation MTTBubbleConfig

- (instancetype)initWithConfig:(NSString*)string left:(BOOL)left
{
    self = [super init];
    if (self)
    {
        NSString* textBgImagePath;
        NSString* picBgImagePath;
        
        NSData* data = [NSData dataWithContentsOfFile:string];
        NSDictionary* dic = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        
        MTTBubbleContentInset insetTemp;
        insetTemp.top = [dic[@"contentInset"][@"top"] floatValue];
        insetTemp.bottom = [dic[@"contentInset"][@"bottom"] floatValue];
        if(left){
            insetTemp.left = [dic[@"contentInset"][@"left"] floatValue];
            insetTemp.right = [dic[@"contentInset"][@"right"] floatValue];
        }else{
            insetTemp.left = [dic[@"contentInset"][@"right"] floatValue];
            insetTemp.right = [dic[@"contentInset"][@"left"] floatValue];
        }
        self.inset = insetTemp;
        MTTBubbleVoiceInset voiceInsetTemp;
        voiceInsetTemp.top = [dic[@"voiceInset"][@"top"] floatValue];
        voiceInsetTemp.bottom = [dic[@"voiceInset"][@"bottom"] floatValue];
        if(left){
            voiceInsetTemp.left = [dic[@"voiceInset"][@"left"] floatValue];
            voiceInsetTemp.right = [dic[@"voiceInset"][@"right"] floatValue];
        }else{
            voiceInsetTemp.left = [dic[@"voiceInset"][@"right"] floatValue];
            voiceInsetTemp.right = [dic[@"voiceInset"][@"left"] floatValue];
        }
        self.voiceInset = voiceInsetTemp;
        MTTBubbleStretchy stretchyTemp;
        stretchyTemp.left = [dic[@"stretchy"][@"left"] floatValue];
        stretchyTemp.top = [dic[@"stretchy"][@"top"] floatValue];
        self.stretchy = stretchyTemp;
        MTTBubbleStretchy imgStretchyTemp;
        imgStretchyTemp.left = [dic[@"imgStretchy"][@"left"] floatValue];
        imgStretchyTemp.top = [dic[@"imgStretchy"][@"top"] floatValue];
        self.imgStretchy = imgStretchyTemp;
        NSArray *textColorTemp = [dic[@"textColor"] componentsSeparatedByString:@","];
        self.textColor = RGB([textColorTemp[0] floatValue], [textColorTemp[1] floatValue], [textColorTemp[2] floatValue]);
        NSArray *linkColorTemp = [dic[@"linkColor"] componentsSeparatedByString:@","];
        self.linkColor = RGB([linkColorTemp[0] floatValue], [linkColorTemp[1] floatValue], [linkColorTemp[2] floatValue]);
        
        NSString* bubbleType = [MTTUtil getBubbleTypeLeft:left];
        if(left){
            textBgImagePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/textLeftBubble", bubbleType];
            picBgImagePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/picLeftBubble", bubbleType];
        }else{
            textBgImagePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/textBubble", bubbleType];
            picBgImagePath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/picBubble", bubbleType];
        }
        
//        self.textBgImage = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:textBgImagePath];
        self.textBgImage = textBgImagePath;
//        self.picBgImage = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:picBgImagePath];
        self.picBgImage = picBgImagePath;
    }
    return self;
}

@end
