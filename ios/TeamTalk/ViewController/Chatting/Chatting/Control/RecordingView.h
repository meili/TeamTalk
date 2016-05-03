//
//  DDRecordingView.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-6.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSUInteger, DDRecordingState)
{
    DDShowVolumnState,
    DDShowCancelSendState,
    DDShowRecordTimeTooShort
};

@interface RecordingView : UIView
@property (nonatomic,assign)DDRecordingState recordingState;

- (instancetype)initWithState:(DDRecordingState)state;
- (void)setVolume:(float)volume;

@end
