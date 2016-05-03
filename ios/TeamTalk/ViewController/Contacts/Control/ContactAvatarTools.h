//
//  DDContactAvatarTools.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-25.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef void(^ButtonSelectBlock)(int buttonIndex) ;
@class MTTUserEntity;
@interface ContactAvatarTools : UIView
@property(strong)UIButton *item1;
@property(strong)UIButton *item2;
@property(strong)UIButton *item3;
@property(strong)MTTUserEntity *user;
@property(copy)ButtonSelectBlock block;
@property(assign)BOOL isShow;
-(void)hiddenSelf;
@end
