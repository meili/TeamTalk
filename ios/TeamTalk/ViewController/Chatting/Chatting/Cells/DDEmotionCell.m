//
//  DDEmotionCell.m
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-22.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import "DDEmotionCell.h"
#import "EmotionsModule.h"
#import "DDMessageSendManager.h"
#import "UIView+Addition.h"
#import "SessionModule.h"
#import "UIImage+GIF.h"
#import <Masonry/Masonry.h>
@implementation DDEmotionCell
- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
        self.msgImgView.userInteractionEnabled=NO;
        [self.msgImgView setClipsToBounds:YES];
        [self.msgImgView setContentMode:UIViewContentModeScaleAspectFill];
        
    }
    return self;
}
#pragma mark -
#pragma mark DDChatCellProtocol Protocol
- (CGSize)sizeForContent:(MTTMessageEntity*)content
{
    float height = 133;
    float width = 100;
    return CGSizeMake(width, height);
}

- (float)contentUpGapWithBubble
{
    return 1;
}

- (float)contentDownGapWithBubble
{
    return 1;
}

- (float)contentLeftGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return 1;
        case DDBubbleLeft:
            return 8.5;
    }
    return 0;
}

- (float)contentRightGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return 6.5;
            break;
        case DDBubbleLeft:
            return 1;
            break;
    }
    return 0;
}

- (void)layoutContentView:(MTTMessageEntity*)content
{
    CGSize size = [self sizeForContent:content];
    [self.msgImgView mas_remakeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(size);
        make.top.equalTo(self.bubbleImageView.mas_top).offset([self contentUpGapWithBubble]);
        make.left.equalTo(self.bubbleImageView.mas_left).offset([self contentLeftGapWithBubble]);
    }];
}

- (float)cellHeightForMessage:(MTTMessageEntity*)message
{
    return 133 +  dd_bubbleUpDown;
}
- (void)setContent:(MTTMessageEntity*)content
{
    [super setContent:content];
    NSString *emotionStr = content.msgContent;
    NSString *emotionImageStr = [[EmotionsModule shareInstance].emotionUnicodeDic objectForKey:emotionStr];
    emotionImageStr = [emotionImageStr stringByDeletingPathExtension];
    UIImage *emotion = [UIImage sd_animatedGIFNamed:emotionImageStr];
    if (emotion) {
        [self.msgImgView setImage:emotion];
        [self.bubbleImageView setHidden:YES];
    }
    
}
-(void)sendTextAgain:(MTTMessageEntity *)message
{
    message.state = DDMessageSending;
    [self showSending];
    [[DDMessageSendManager instance] sendMessage:message isGroup:[message isGroupMessage]  Session:[[SessionModule instance] getSessionById:message.sessionId] completion:^(MTTMessageEntity* theMessage,NSError *error) {
        
        [self showSendSuccess];
        
        
    } Error:^(NSError *error) {
        [self showSendFailure];
    }];
    
}
@end
