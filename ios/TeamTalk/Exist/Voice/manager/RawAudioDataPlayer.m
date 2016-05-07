//
//  RawAudioDataPlayer.m
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import "RawAudioDataPlayer.h"

static void AudioPlayerAQInputCallback(void *input, AudioQueueRef inQ, AudioQueueBufferRef outQB);

@interface RawAudioDataPlayer()

- (void)prepare;
- (void)initAudio;
- (void)readPCMAndPlay:(AudioQueueRef)outQ buffer:(AudioQueueBufferRef)outQB;
- (int)checkUsedQueueBuffer:(AudioQueueBufferRef) qbuf;
- (void)putEmptyBuffer:(AudioQueueBufferRef)buffer;
- (void)removeEmptyBuffer:(AudioQueueBufferRef)buffer;

@end

@implementation RawAudioDataPlayer

@synthesize isDataInputOver;

- (id)init {
    self = [super init];
    if (self) {
        mPcmData = [NSMutableData data];
        readedBytes = 0;
        pcmDataBuffer = malloc(EVERY_READ_LENGTH);
        synlock = [[NSLock alloc] init];
        emptyAudioQueueBufferIndexs = [NSMutableArray arrayWithCapacity:QUEUE_BUFFER_SIZE];
    }
    return self;
}

- (void)prepare {
    isDataInputOver = NO;
    readedBytes = 0;
    [emptyAudioQueueBufferIndexs removeAllObjects];
}

-(void)startPlay {
    [self initAudio];
    [self prepare];
    OSStatus status = AudioQueueStart(audioQueue, NULL);
    if (status) {
        [[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_PLAY_OVER object:nil];
        return;
    }
    for(int i=0;i<QUEUE_BUFFER_SIZE;i++) {
        [self readPCMAndPlay:audioQueue buffer:audioQueueBuffers[i]];
    }
    /*
     audioQueue使用的是驱动回调方式，即通过AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);传入一个buff去播放，播放完buffer区后通过回调通知用户,
     用户得到通知后再重新初始化buff去播放，周而复始,当然，可以使用多个buff提高效率(测试发现使用单个buff会小卡)
     */
}

-(void)stopPlay {
    AudioQueueStop(audioQueue, YES);
    AudioQueueDispose(audioQueue, YES);
}


-(void)initAudio {
    ///设置音频参数
    audioDescription.mSampleRate = 8000;//采样率
    audioDescription.mFormatID = kAudioFormatLinearPCM;
    audioDescription.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    audioDescription.mChannelsPerFrame = 1;///单声道
    audioDescription.mFramesPerPacket = 1;//每一个packet一侦数据
    audioDescription.mBitsPerChannel = 16;//每个采样点16bit量化
    audioDescription.mBytesPerFrame = (audioDescription.mBitsPerChannel/8) * audioDescription.mChannelsPerFrame;
    audioDescription.mBytesPerPacket = audioDescription.mBytesPerFrame ;
    ///创建一个新的从audioqueue到硬件层的通道
    //  AudioQueueNewOutput(&audioDescription, AudioPlayerAQInputCallback, self, CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &audioQueue);///使用当前线程播
    AudioQueueNewOutput(&audioDescription, AudioPlayerAQInputCallback, (__bridge void *)(self), nil, nil, 0, &audioQueue);//使用player的内部线程播
    
    ////添加buffer区
    for(int i=0;i<QUEUE_BUFFER_SIZE;i++) {
        int result =  AudioQueueAllocateBuffer(audioQueue, MIN_SIZE_PER_FRAME, &audioQueueBuffers[i]);///创建buffer区，MIN_SIZE_PER_FRAME为每一侦所需要的最小的大小，该大小应该比每次往buffer里写的最大的一次还大
        NSLog(@"AudioQueueAllocateBuffer i = %d,result = %d",i,result);
    }
}

- (void)inputNewDataFromBuffer:(Byte *)buffer size:(int)bufferSize {
    [synlock lock];
    [mPcmData appendBytes:buffer length:bufferSize];
    [synlock unlock];
    
    if ([emptyAudioQueueBufferIndexs count] > 0) {      //如果有空闲的audio queue buffer就尝试填入
        [self readPCMAndPlay:audioQueue buffer:audioQueueBuffers[[[emptyAudioQueueBufferIndexs objectAtIndex:0] intValue]]];
    }
    
}

- (void)setIsDataInputOver:(BOOL)dataInputOver {
    isDataInputOver = dataInputOver;
    if (dataInputOver) {
        if ([emptyAudioQueueBufferIndexs count] == QUEUE_BUFFER_SIZE) {
            NSLog(@"audio queue play over");
            [self stopPlay];
            [[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_PLAY_OVER object:nil];
        }
    }
}

-(void)readPCMAndPlay:(AudioQueueRef)outQ buffer:(AudioQueueBufferRef)outQB
{
    [synlock lock];
    
    NSUInteger lengthLeft = [mPcmData length] - readedBytes;   //能够读取的数据长度
    if ((lengthLeft < EVERY_READ_LENGTH) && ( ! isDataInputOver)) {     //数据不足以填充queue buffer
        [self putEmptyBuffer:outQB];
    }
    else {
        if ([emptyAudioQueueBufferIndexs count] > 0) {
            if (lengthLeft == 0 && isDataInputOver) {
                //所有数据都输入并且读取完了
                if ([emptyAudioQueueBufferIndexs count] == QUEUE_BUFFER_SIZE) {
                    NSLog(@"audio queue play over");
                    [[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_PLAY_OVER object:nil];
                }
            }
            else {
                [self removeEmptyBuffer:outQB];
                
                NSUInteger readLength = (lengthLeft > EVERY_READ_LENGTH) ? EVERY_READ_LENGTH : lengthLeft;
                outQB->mAudioDataByteSize = readLength;
                Byte *audioData = (Byte *)outQB->mAudioData;
                memcpy(audioData, [mPcmData bytes] + readedBytes, readLength);
                readedBytes += readLength;
                /*
                 将创建的buffer区添加到audioqueue里播放
                 AudioQueueBufferRef用来缓存待播放的数据区，AudioQueueBufferRef有两个比较重要的参数，AudioQueueBufferRef->mAudioDataByteSize用来指示数据区大小，AudioQueueBufferRef->mAudioData用来保存数据区
                 */
                AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
            }
            
        }
    }
    [synlock unlock];
}

- (void)putEmptyBuffer:(AudioQueueBufferRef)buffer {
    BOOL isInArray = NO;
    int indexValue = [self checkUsedQueueBuffer:buffer];
    for (NSNumber *index in emptyAudioQueueBufferIndexs) {
        if ([index intValue] == indexValue) {
            isInArray = YES;
        }
    }
    if ( ! isInArray) {
        [emptyAudioQueueBufferIndexs addObject:[NSNumber numberWithInt:indexValue]];
    }
}

- (void)removeEmptyBuffer:(AudioQueueBufferRef)buffer {
    int indexValue = [self checkUsedQueueBuffer:buffer];
    for (NSNumber *index in emptyAudioQueueBufferIndexs) {
        if ([index intValue] == indexValue) {
            [emptyAudioQueueBufferIndexs removeObject:index];
            return;
        }
    }
}

-(int)checkUsedQueueBuffer:(AudioQueueBufferRef) qbuf {
    int bufferIndex = 0;
    if(qbuf == audioQueueBuffers[0]) {
        bufferIndex = 0;
    }
    if(qbuf == audioQueueBuffers[1]) {
        bufferIndex = 1;
    }
    if(qbuf == audioQueueBuffers[2]) {
        bufferIndex = 2;
    }
    if(qbuf == audioQueueBuffers[3]) {
        bufferIndex = 3;
    }
    return bufferIndex;
}


- (void)dealloc {
    free(pcmDataBuffer);
}

#pragma mark -
#pragma mark player call back
/*
 试了下其实可以不用静态函数，但是c写法的函数内是无法调用[self ***]这种格式的写法，所以还是用静态函数通过void *input来获取原类指针
 这个回调存在的意义是为了重用缓冲buffer区，当通过AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);函数放入queue里面的音频文件播放完以后，通过这个函数通知
 调用者，这样可以重新再使用回调传回的AudioQueueBufferRef
 */
static void AudioPlayerAQInputCallback(void *input, AudioQueueRef outQ, AudioQueueBufferRef outQB) {
    @autoreleasepool {
        RawAudioDataPlayer *player = (__bridge RawAudioDataPlayer *)input;
        [player putEmptyBuffer:outQB];
        [player readPCMAndPlay:outQ buffer:outQB];
    }
}

@end
