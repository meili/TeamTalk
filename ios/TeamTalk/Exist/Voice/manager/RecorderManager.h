//
//  RecorderManager.h
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import <Foundation/Foundation.h>
//#import "AQRecorder.h"
#import "Encapsulator.h"

@protocol RecordingDelegate <NSObject>

@optional
- (void)levelMeterChanged:(float)levelMeter;
- (void)recordingFinishedWithFileName:(NSString *)filePath time:(NSTimeInterval)interval;
- (void)recordingTimeout;
- (void)recordingStopped;  //录音机停止采集声音
- (void)recordingFailed:(NSString *)failureInfoString;

@end

@interface RecorderManager : NSObject <EncapsulatingDelegate> {
    
    Encapsulator *encapsulator;
    NSString *filename;
    NSDate *dateStartRecording;
    NSDate *dateStopRecording;
    NSTimer *timerLevelMeter;
    NSTimer *timerTimeout;
}

@property (nonatomic, weak)  id<RecordingDelegate> delegate;
@property (nonatomic, strong) Encapsulator *encapsulator;
@property (nonatomic, strong) NSDate *dateStartRecording, *dateStopRecording;
@property (nonatomic, strong) NSTimer *timerLevelMeter;
@property (nonatomic, strong) NSTimer *timerTimeout;

+ (RecorderManager *)sharedManager;

- (void)startRecording;

- (void)stopRecording;

- (void)cancelRecording;

- (NSTimeInterval)recordedTimeInterval;

@end
