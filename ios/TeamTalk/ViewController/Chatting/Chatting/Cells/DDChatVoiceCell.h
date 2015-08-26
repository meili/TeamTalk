//
//  DDChatVoiceCell.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-5.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDChatBaseCell.h"

typedef void(^DDEarphonePlay)();
typedef void(^DDSpearerPlay)();
@interface DDChatVoiceCell : DDChatBaseCell<DDChatCellProtocol>
{
    UIImageView* _voiceImageView;
    UILabel* _timeLabel;
    UILabel* _playedLabel;
}
@property (nonatomic,copy)DDEarphonePlay earphonePlay;
@property (nonatomic,copy)DDSpearerPlay speakerPlay;

- (void)showVoicePlayed;
- (void)stopVoicePlayAnimation;
-(void)sendVoiceAgain:(MTTMessageEntity *)message;
@end
