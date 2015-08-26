//
//  DDChatVoiceCell.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-5.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDChatVoiceCell.h"
#import "MTTMessageEntity.h"
#import "UIView+Addition.h"
#import "PlayerManager.h"
#import "RecorderManager.h"
#import "MTTDatabaseUtil.h"
#import "DDMessageSendManager.h"
#import "SessionModule.h"
#import <Masonry/Masonry.h>
static float const maxCellLength = 180;
static float const minCellLength = 50;

@interface DDChatVoiceCell(privateAPI)

- (float)lengthForVoiceLength:(float)voiceLength;

@end

@implementation DDChatVoiceCell
{
    NSString* _voicePath;
}

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self)
    {
        _voiceImageView = [UIImageView new];
        [self.contentView addSubview:_voiceImageView];
        
        _timeLabel = [UILabel new];
        [_timeLabel setFont:systemFont(15)];
        [_timeLabel setBackgroundColor:[UIColor clearColor]];
        [_timeLabel setTextColor:RGB(149, 149, 149)];
        [self.contentView addSubview:_timeLabel];
        
        _playedLabel = [UILabel new];
        [_playedLabel setBackgroundColor:[UIColor redColor]];
        [_playedLabel.layer setCornerRadius:5];
        [_playedLabel setClipsToBounds:YES];
        [self.contentView addSubview:_playedLabel];
    }
    return self;
}

- (void)setContent:(MTTMessageEntity *)content
{
    [super setContent:content];
    
    if ([content.info[DDVOICE_PLAYED] intValue])
    {
        [_playedLabel setHidden:YES];
    }
    else
    {
        [_playedLabel setHidden:NO];
    }
    
    _voicePath = [content.msgContent copy];
    NSArray* imageArray = nil;
    
    switch (self.location) {
        case DDBubbleLeft:
        {
            imageArray = @[[UIImage imageNamed:@"dd_left_voice_one"],[UIImage imageNamed:@"dd_left_voice_two"],[UIImage imageNamed:@"dd_left_voice_three"]];
            [_voiceImageView setContentMode:UIViewContentModeLeft];
            [_voiceImageView setImage:[UIImage imageNamed:@"dd_left_voice_three"]];
            [self.activityView mas_updateConstraints:^(MASConstraintMaker *make) {
                make.left.mas_equalTo(self.bubbleImageView.mas_right).offset(20);
            }];
        }
            break;
        case DDBubbleRight:
        {
            imageArray = @[[UIImage imageNamed:@"dd_right_voice_one"],[UIImage imageNamed:@"dd_right_voice_two"],[UIImage imageNamed:@"dd_right_voice_three"]];
            [_voiceImageView setContentMode:UIViewContentModeRight];
            [_voiceImageView setImage:[UIImage imageNamed:@"dd_right_voice_three"]];
            [_playedLabel setHidden:YES];
            [self.activityView mas_updateConstraints:^(MASConstraintMaker *make) {
                make.right.mas_equalTo(self.bubbleImageView.mas_left).offset(-30);
            }];
        }
            break;
    }
    float voiceLength = [content.info[VOICE_LENGTH] floatValue];
    [_voiceImageView setAnimationImages:imageArray];
    [_voiceImageView setAnimationRepeatCount:voiceLength];
    [_voiceImageView setAnimationDuration:1];
    
    NSUInteger timeLength = [content.info[VOICE_LENGTH] longValue];
    NSString* lengthString = [NSString stringWithFormat:@"%ld\"",timeLength];
    [_timeLabel setText:lengthString];
}

- (void)showVoicePlayed
{
    [_playedLabel setHidden:YES];
}

- (void)stopVoicePlayAnimation
{
    [_voiceImageView stopAnimating];
}

#pragma mark -
#pragma mark DDChatCellProtocol Protocol
- (CGSize)sizeForContent:(MTTMessageEntity*)content
{
    float voiceLength = [content.info[VOICE_LENGTH] floatValue];
    float width = [self lengthForVoiceLength:voiceLength];
    return CGSizeMake(width, 17);
}

- (float)contentUpGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return self.rightConfig.voiceInset.top;
            break;
        case DDBubbleLeft:
            return self.leftConfig.voiceInset.top;
            break;
    }
    return 0;
}

- (float)contentDownGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return self.rightConfig.voiceInset.bottom;
            break;
        case DDBubbleLeft:
            return self.leftConfig.voiceInset.bottom;
            break;
    }
    return 0;
}

- (float)contentLeftGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return self.rightConfig.voiceInset.left;
            break;
        case DDBubbleLeft:
            return self.leftConfig.voiceInset.left;
            break;
    }
    return 0;
}

- (float)contentRightGapWithBubble
{
    switch (self.location)
    {
        case DDBubbleRight:
            return self.rightConfig.voiceInset.right;
            break;
        case DDBubbleLeft:
            return self.leftConfig.voiceInset.right;
            break;
    }
    return 0;
}

- (void)layoutContentView:(MTTMessageEntity*)content
{
    switch (self.location)
    {
        case DDBubbleLeft:
        {
            [_voiceImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(11, 17));
                make.left.equalTo(self.bubbleImageView.mas_left).offset(15);
                make.top.equalTo(self.bubbleImageView.mas_top).offset([self contentUpGapWithBubble]);
            }];
            
            [_timeLabel setTextAlignment:NSTextAlignmentLeft];
            [_timeLabel mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(20, 15));
                make.bottom.equalTo(self.bubbleImageView.mas_bottom).offset(0);
                make.left.equalTo(self.bubbleImageView.mas_right).offset(10);
            }];
            
            [_playedLabel mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(10, 10));
                make.top.equalTo(self.bubbleImageView.mas_top).offset(2);
                make.left.equalTo(self.bubbleImageView.mas_right).offset(10);
            }];
        }
            break;
        case DDBubbleRight:
        {
            [_voiceImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(11, 17));
                make.right.equalTo(self.bubbleImageView.mas_right).offset(-15);
                make.top.equalTo(self.bubbleImageView.mas_top).offset([self contentUpGapWithBubble]);
            }];
            
            [_timeLabel setTextAlignment:NSTextAlignmentRight];
            [_timeLabel mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(20, 15));
                make.bottom.equalTo(self.bubbleImageView.mas_bottom).offset(0);
                make.right.equalTo(self.bubbleImageView.mas_left).offset(-10);
            }];
            
            [_playedLabel mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(10, 10));
                make.top.equalTo(self.bubbleImageView.mas_top).offset(2);
                make.right.equalTo(self.bubbleImageView.mas_left).offset(-10);
            }];
        }
            break;
    }
}

- (float)cellHeightForMessage:(MTTMessageEntity*)message
{
    return 27 + dd_bubbleUpDown;
}


#pragma mark - 
#pragma mark PrivateAPI
- (float)lengthForVoiceLength:(float)voiceLength
{
    float gap = maxCellLength - minCellLength;
    if (voiceLength > 10)
    {
        return maxCellLength;
    }
    else
    {
        float length = (gap / 10) * voiceLength + minCellLength;
        return length;
    }
}

#pragma mark -
#pragma mark DDMenuImageView Delegate
- (void)clickTheCopy:(MenuImageView*)imageView
{
    //子类去继承
}

- (void)clickTheEarphonePlay:(MenuImageView*)imageView
{
    //子类去继承
    [_voiceImageView startAnimating];
    if (self.earphonePlay)
    {
        self.earphonePlay();
    }
}

- (void)clickTheSpeakerPlay:(MenuImageView*)imageView
{
    //子类去继承
    [_voiceImageView startAnimating];
    if (self.speakerPlay)
    {
        self.speakerPlay();
    }
}

- (void)clickTheSendAgain:(MenuImageView*)imageView
{
    //子类去继承
    if (self.sendAgain)
    {
        self.sendAgain();
    }
}

- (void)tapTheImageView:(MenuImageView*)imageView
{
    if (![_voiceImageView isAnimating])
    {
        [_voiceImageView startAnimating];
        [super tapTheImageView:imageView];
    }else{
        [_voiceImageView stopAnimating];
        [super tapTheImageView:imageView];
    }
}

-(void)sendVoiceAgain:(MTTMessageEntity *)message
{
    [self showSending];
    NSString* filePath = message.msgContent;
    NSMutableData* muData = [[NSMutableData alloc] init];
    NSData* data = [NSData dataWithContentsOfFile:filePath];
    int length = [RecorderManager sharedManager].recordedTimeInterval;
    int8_t ch[4];
    for(int32_t i = 0;i<4;i++){
        ch[i] = ((length >> ((3 - i)*8)) & 0x0ff);
    }
    [muData appendBytes:ch length:4];
    [muData appendData:data];
    [[DDMessageSendManager instance] sendVoiceMessage:muData filePath:filePath forSessionID:message.sessionId isGroup:[message isGroupMessage] Message:message Session:[[SessionModule instance] getSessionById:message.sessionId] completion:^(MTTMessageEntity *theMessage, NSError *error) {
        if (!error)
        {
              [self showSendSuccess];
        }
        else
        {
              [self showSendFailure];
        }
    }];
}
@end
