//
//  PlayerManager.m
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import "PlayerManager.h"

@interface PlayerManager ()

- (void)startProximityMonitering;  //开启距离感应器监听(开始播放时)
- (void)stopProximityMonitering;   //关闭距离感应器监听(播放完成时)

@end

@implementation PlayerManager
{
    NSString* _playingFileName;
}
@synthesize decapsulator;
@synthesize avAudioPlayer;

static PlayerManager *mPlayerManager = nil;

+ (PlayerManager *)sharedManager {
    static PlayerManager *g_playerManager;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_playerManager = [[PlayerManager alloc] init];
    });
    return g_playerManager;
//    @synchronized(self) {
//        if (mPlayerManager == nil)
//        {
//            mPlayerManager = [[PlayerManager alloc] init];
//            
//            [[NSNotificationCenter defaultCenter] addObserver:mPlayerManager
//                                                     selector:@selector(sensorStateChange:)
//                                                         name:@"UIDeviceProximityStateDidChangeNotification"
//                                                       object:nil];
//        }
//    }
//    return mPlayerManager;
}

+ (id)allocWithZone:(NSZone *)zone
{
    @synchronized(self)
    {
        if(mPlayerManager == nil)
        {
            mPlayerManager = [super allocWithZone:zone];
            return mPlayerManager;
        }
    }
    
    return nil;
}

- (id)init {
    if (self = [super init]) {
        
        [[NSNotificationCenter defaultCenter] addObserver:mPlayerManager
                                                 selector:@selector(sensorStateChange:)
                                                     name:@"UIDeviceProximityStateDidChangeNotification"
                                                   object:nil];
        
        //初始化播放器的时候如下设置
        UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
        AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
        UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
        AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride), &audioRouteOverride);
        
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        //默认情况下扬声器播放
        [audioSession setCategory:AVAudioSessionCategoryPlayback error:nil];
        [audioSession setActive:YES error:nil];
    }
    return self;
}

- (void)playAudioWithFileName:(NSString *)filename delegate:(id<PlayingDelegate>)newDelegate {
    if ( ! filename) {
        return;
    }
    if ([filename rangeOfString:@".spx"].location != NSNotFound) {
        [[AVAudioSession sharedInstance] setActive:YES error:nil];
        
        [self stopPlaying];
        
        self.delegate = newDelegate;
        
        self.decapsulator = [[Decapsulator alloc] initWithFileName:filename];
        self.decapsulator.delegate = self;
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
        [self startProximityMonitering];
        _playingFileName = [filename copy];
        [self.decapsulator play];
        
    }
    else if ([filename rangeOfString:@".mp3"].location != NSNotFound) {
        if ( ! [[NSFileManager defaultManager] fileExistsAtPath:filename]) {
            NSLog(@"要播放的文件不存在:%@", filename);
            _playingFileName = nil;
            [self.delegate playingStoped];
            [newDelegate playingStoped];
            return;
        }
        [self.delegate playingStoped];
        self.delegate = newDelegate;
        
        NSError *error;
        self.avAudioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:filename] error:&error];
        if (self.avAudioPlayer) {
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
            [[AVAudioSession sharedInstance] setActive:YES error:nil];
            self.avAudioPlayer.delegate = self;
            _playingFileName = [filename copy];
            [self.avAudioPlayer play];
            [self startProximityMonitering];
        }
        else {
            [self.delegate playingStoped];
        }
    }
    else {
        [self.delegate playingStoped];
    }
}

- (void)playAudioWithFileName:(NSString *)filename playerType:(PlayerType)type delegate:(id<PlayingDelegate>)newDelegate
{
    [[AVAudioSession sharedInstance] setActive:YES error:nil];
    if ([filename rangeOfString:@".spx"].location != NSNotFound) {
        [self stopPlaying];
        self.delegate = newDelegate;
        
        self.decapsulator = [[Decapsulator alloc] initWithFileName:filename];
        self.decapsulator.delegate = self;
        [self.decapsulator play];
        
    }
    else if ([filename rangeOfString:@".mp3"].location != NSNotFound) {
        if ( ! [[NSFileManager defaultManager] fileExistsAtPath:filename]) {
            NSLog(@"要播放的文件不存在:%@", filename);
            _playingFileName = nil;
            [self.delegate playingStoped];
            [newDelegate playingStoped];
            return;
        }
        _playingFileName = nil;
        [self.delegate playingStoped];
        self.delegate = newDelegate;
        
        NSError *error;
        self.avAudioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:filename] error:&error];
        if (self.avAudioPlayer) {
            self.avAudioPlayer.delegate = self;
            [self.avAudioPlayer play];
        }
        else {
            _playingFileName = nil;
            [self.delegate playingStoped];
        }
    }else if([filename rangeOfString:@".caf"].location != NSNotFound)
    {
        //        if ( ! [[NSFileManager defaultManager] fileExistsAtPath:filename]) {
        //            NSLog(@"要播放的文件不存在:%@", filename);
        //            [self.delegate playingStoped];
        //            [newDelegate playingStoped];
        //            return;
        //        }
        _playingFileName = nil;
        [self.delegate playingStoped];
        self.delegate = newDelegate;
        
        NSError *error;
        NSArray *array  =[filename componentsSeparatedByString:@"."];
        NSString *bundlePath=[[NSBundle mainBundle]pathForResource:@"Resource" ofType:@"bundle"];
        NSBundle *bundle=[NSBundle bundleWithPath:bundlePath];
        NSString *soundPath=[bundle pathForResource:array[0] ofType:@"caf"inDirectory:nil];
        if (soundPath ==nil) {
            NSLog(@"要播放的文件不存在:%@", filename);
            [self.delegate playingStoped];
            [newDelegate playingStoped];
            return;
        }
        NSURL *soundUrl=[[NSURL alloc] initFileURLWithPath:soundPath];
        self.avAudioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:soundUrl error:&error];
        if (self.avAudioPlayer) {
            self.avAudioPlayer.delegate = self;
            [self.avAudioPlayer play];
        }
        else {
            _playingFileName = nil;
            [self.delegate playingStoped];
        }
    }
    else {
        _playingFileName = nil;
        [self.delegate playingStoped];
    }
    
    switch (type)
    {
        case DDEarPhone:
            //听筒
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
            break;
        case DDSpeaker:
            //扬声器
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
            break;
        default:
            break;
    }
}

- (void)stopPlaying {
    _playingFileName = nil;
    [self stopProximityMonitering];

    if (self.decapsulator) {
        [self.decapsulator stopPlaying];
//        self.decapsulator.delegate = nil;   //此行如果放在上一行之前会导致回调问题
        self.decapsulator = nil;
    }
    if (self.avAudioPlayer) {
        [self.avAudioPlayer stop];
        self.avAudioPlayer = nil;
        
//        [self.delegate playingStoped];
    }
    
    [self.delegate playingStoped];
}

- (BOOL)playingFileName:(NSString *)fileName
{
    return [_playingFileName isEqualToString:fileName];
}

- (void)decapsulatingAndPlayingOver {
    _playingFileName = nil;
    [self.delegate playingStoped];
    [self stopProximityMonitering];
}

- (void)sensorStateChange:(NSNotification *)notification {
    //如果此时手机靠近面部放在耳朵旁，那么声音将通过听筒输出，并将屏幕变暗
    if ([[UIDevice currentDevice] proximityState] == YES) {
        NSLog(@"Device is close to user");
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
    }
    else {
        NSLog(@"Device is not close to user");
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
    }
}

- (void)startProximityMonitering {
    [[UIDevice currentDevice] setProximityMonitoringEnabled:YES];
    NSLog(@"开启距离监听");
}

- (void)stopProximityMonitering {
//    dispatch_async(dispatch_get_main_queue(), ^{
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
        [[UIDevice currentDevice] setProximityMonitoringEnabled:NO];
        NSLog(@"关闭距离监听");
//    });
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
