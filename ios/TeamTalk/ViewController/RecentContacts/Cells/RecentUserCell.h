//
//  DDRecentUserCell.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTGroupEntity.h"
#import "MTTUserEntity.h"
#import "MTTAvatarImageView.h"
@class MTTSessionEntity;
@interface RecentUserCell : UITableViewCell
@property (nonatomic,strong) MTTAvatarImageView* avatarImageView;
@property (nonatomic,strong) UIImageView* shiledImageView;
@property (nonatomic,strong) UILabel* nameLabel;
@property (nonatomic,strong) UILabel* dateLabel;
@property (nonatomic,strong) UILabel* lastmessageLabel;
@property (nonatomic,strong) UILabel* unreadMessageCountLabel;
@property (nonatomic,strong) UILabel* shiledUnreadMessageCountLabel;
@property (assign)NSInteger time_sort;
- (void)setName:(NSString*)name;
- (void)setTimeStamp:(NSUInteger)timeStamp;
- (void)setLastMessage:(NSString*)message;
- (void)setAvatar:(NSString*)avatar;
- (void)setUnreadMessageCount:(NSUInteger)messageCount;
-(void)setShowSession:(MTTSessionEntity *)session;
@end
