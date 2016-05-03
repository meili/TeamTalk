//
//  PlayerManager.h
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "Decapsulator.h"

typedef NS_ENUM(NSUInteger, PlayerType)
{
    DDEarPhone,
    DDSpeaker
};

@protocol PlayingDelegate <NSObject>

- (void)playingStoped;

@end

@interface PlayerManager : NSObject <DecapsulatingDelegate, AVAudioPlayerDelegate> {
    Decapsulator *decapsulator;
    AVAudioPlayer *avAudioPlayer;
    
}
@property (nonatomic, strong) Decapsulator *decapsulator;
@property (nonatomic, strong) AVAudioPlayer *avAudioPlayer;
@property (nonatomic, weak)  id<PlayingDelegate> delegate;

+ (PlayerManager *)sharedManager;

- (void)playAudioWithFileName:(NSString *)filename delegate:(id<PlayingDelegate>)newDelegate;
- (void)stopPlaying;

- (void)playAudioWithFileName:(NSString *)filename playerType:(PlayerType)type delegate:(id<PlayingDelegate>)newDelegate;
- (BOOL)playingFileName:(NSString*)fileName;
@end
