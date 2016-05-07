//
//  MTTEditSignViewController.h
//  TeamTalk
//
//  Created by scorpio on 15/6/19.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBaseViewController.h"

@interface MTTEditSignViewController : MTTBaseViewController<UITextViewDelegate,UITextViewDelegate>
@property(strong)UITextView *signText;
@property(nonatomic,weak)MTTUserEntity *user;
@end
