//
//  RawAudioDataPlayer.h
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioFile.h>

#define QUEUE_BUFFER_SIZE 4 //队列缓冲个数
#define EVERY_READ_LENGTH 1000 //每次从文件读取的长度
#define MIN_SIZE_PER_FRAME 2000 //每侦最小数据长

#define NOTIFICATION_PLAY_OVER @"playingOver"

@interface RawAudioDataPlayer : NSObject {
    AudioStreamBasicDescription audioDescription;///音频参数
    AudioQueueRef audioQueue;//音频播放队列
    AudioQueueBufferRef audioQueueBuffers[QUEUE_BUFFER_SIZE];//音频缓存
    NSLock *synlock ;///同步控制
    Byte *pcmDataBuffer;//pcm的读文件数据区
    
    BOOL isDataInputOver;
    NSMutableData *mPcmData;
    NSUInteger readedBytes;
    NSMutableArray *emptyAudioQueueBufferIndexs;
}

@property (nonatomic, assign) BOOL isDataInputOver;

//static void AudioPlayerAQInputCallback(void *input, AudioQueueRef inQ, AudioQueueBufferRef outQB);

- (void)startPlay;
- (void)stopPlay;
- (void)inputNewDataFromBuffer:(Byte *)buffer size:(int)bufferSize;

@end
