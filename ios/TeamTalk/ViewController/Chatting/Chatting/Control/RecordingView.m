//
//  DDRecordingView.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-6.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "RecordingView.h"
#import "UIView+Addition.h"

#define DDVIEW_RECORDING_WIDTH                          196
#define DDVIEW_RECORDING_HEIGHT                         196

#define DDVOLUMN_VIEW_TAG                               10
@interface RecordingView(PrivateAPI)

- (void)setupCancelSendView;
- (void)setupShowVolumnState;
- (void)setupShowRecordingTooShort;

- (void)showCancelSendState;
- (void)showVolumnState;
- (void)showRecordingTooShort;


- (float)heightForVolumn:(float)vomlun;
@end

@implementation RecordingView

- (instancetype)initWithState:(DDRecordingState)state
{
    self = [super init];
    if (self)
    {
        self.frame = CGRectMake(0, 0, DDVIEW_RECORDING_WIDTH, DDVIEW_RECORDING_HEIGHT);
        [self.layer setCornerRadius:10];
        [self setClipsToBounds:YES];
        [self setBackgroundColor:[UIColor clearColor]];
        UIView* backroundView = [[UIView alloc]initWithFrame:CGRectMake(0, 0, DDVIEW_RECORDING_WIDTH, DDVIEW_RECORDING_HEIGHT)];
        [backroundView setBackgroundColor:[UIColor blackColor]];
        [backroundView setAlpha:0.7];
        backroundView.tag = 100;
        [self addSubview:backroundView];
        _recordingState = DDShowVolumnState;
        [self setupShowVolumnState];
    
    }
    return self;
}


- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)setRecordingState:(DDRecordingState)recordingState
{
    switch (recordingState)
    {
        case DDShowCancelSendState:
            [self showCancelSendState];
            break;
        case DDShowVolumnState:
            [self showVolumnState];
            break;
        case DDShowRecordTimeTooShort:
            [self showRecordingTooShort];
            break;
    }
}

- (void)setVolume:(float)volume
{
    if (_recordingState != DDShowVolumnState)
    {
        return;
    }
    
    UIImageView* volumnImageView = [self subviewWithTag:DDVOLUMN_VIEW_TAG];
    float height = [self heightForVolumn:volume];
    [volumnImageView setHeight:height];
    volumnImageView.bottom = 126;
}

#pragma mark privateAPI
- (void)setupCancelSendView
{
    [self.subviews enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([(UIView*)obj tag] != 100)
        {
            [(UIView*)obj removeFromSuperview];
        }
    }];
    UIImage* image = [UIImage imageNamed:@"dd_cancel_send_record"];
    UIImageView* imageView = [[UIImageView alloc] initWithImage:image];
    [imageView setFrame:CGRectMake(74, 53, 45, 59)];
    [self addSubview:imageView];
    
    UIView* backgrounView = [[UIView alloc] initWithFrame:CGRectMake(28, 152, 140, 23)];
    [backgrounView setBackgroundColor:RGB(176, 34, 33)];
    [backgrounView setAlpha:0.8];
    [backgrounView.layer setCornerRadius:2];
    [backgrounView setClipsToBounds:YES];
    [self addSubview:backgrounView];
    
    UILabel* prompt = [[UILabel alloc] initWithFrame:CGRectMake(28, 152, 140, 23)];
    [prompt setBackgroundColor:[UIColor clearColor]];
    [prompt setTextColor:[UIColor whiteColor]];
    [prompt setText:@"松开手指，取消发送"];
    [prompt setFont:[UIFont systemFontOfSize:15]];
    [prompt setTextAlignment:NSTextAlignmentCenter];
    [self addSubview:prompt];
}

- (void)setupShowVolumnState
{
    [self.subviews enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([(UIView*)obj tag] != 100)
        {
            [(UIView*)obj removeFromSuperview];
        }
    }];
    UIImage* image = [UIImage imageNamed:@"dd_recording"];
    UIImageView* imageView = [[UIImageView alloc] initWithImage:image];
    [imageView setFrame:CGRectMake(60, 42, 53, 83)];
    [self addSubview:imageView];
    
    UILabel* prompt = [[UILabel alloc] initWithFrame:CGRectMake(0, 152, DDVIEW_RECORDING_WIDTH, 23)];
    [prompt setBackgroundColor:[UIColor clearColor]];
    [prompt setTextColor:[UIColor whiteColor]];
    [prompt.layer setCornerRadius:2];
    [prompt setTextAlignment:NSTextAlignmentCenter];
    [prompt setFont:[UIFont systemFontOfSize:15]];
    [prompt setText:@"手指上滑,取消发送"];
    
    [self addSubview:prompt];

    UIImage* volumnImage = [UIImage imageNamed:@"dd_volumn"];
    UIImageView* volumnImageView = [[UIImageView alloc] initWithImage:volumnImage];
    [volumnImageView setFrame:CGRectMake(119, 83, 20, 43)];
    [volumnImageView setContentMode:UIViewContentModeBottom];
    [volumnImageView setClipsToBounds:YES];
    [volumnImageView setTag:DDVOLUMN_VIEW_TAG];
    [self addSubview:volumnImageView];
    
}

- (void)setupShowRecordingTooShort
{
    [self.subviews enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([(UIView*)obj tag] != 100)
        {
            [(UIView*)obj removeFromSuperview];
        }
    }];
    UIImage* image = [UIImage imageNamed:@"dd_record_too_short"];
    UIImageView* imageView = [[UIImageView alloc] initWithImage:image];
    [imageView setFrame:CGRectMake(85, 42, 22, 83)];
    [self addSubview:imageView];
    
    UILabel* prompt = [[UILabel alloc] initWithFrame:CGRectMake(0, 152, DDVIEW_RECORDING_WIDTH, 23)];
    [prompt setBackgroundColor:[UIColor clearColor]];
    [prompt setTextColor:[UIColor whiteColor]];
    [prompt.layer setCornerRadius:2];
    [prompt setTextAlignment:NSTextAlignmentCenter];
    [prompt setFont:[UIFont systemFontOfSize:15]];
    [prompt setText:@"说话时间太短"];
    
    [self addSubview:prompt];
}

- (void)showCancelSendState
{
    if (self.recordingState != DDShowCancelSendState)
    {
        [self setupCancelSendView];
    }
    _recordingState = DDShowCancelSendState;
}

- (void)showVolumnState
{
    if (self.recordingState != DDShowVolumnState)
    {
        [self setupShowVolumnState];
    }
    _recordingState = DDShowVolumnState;
}

- (void)showRecordingTooShort
{
    if (self.recordingState != DDShowRecordTimeTooShort)
    {
        [self setupShowRecordingTooShort];
    }
    _recordingState = DDShowRecordTimeTooShort;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if (self.recordingState == DDShowRecordTimeTooShort)
        {
            [self setHidden:YES];
        }
    });
}

- (float)heightForVolumn:(float)vomlun
{
    //0-1.6 volumn
    float height = 43.0 / 1.6 * vomlun;
    return height;
}
@end
