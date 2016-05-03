//
//  MTTSearchResultCell.h
//  TeamTalk
//
//  Created by scorpio on 15/7/7.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTGroupEntity.h"
#import "MTTAvatarImageView.h"

@interface MTTSearchResultCell : UITableViewCell

@property(nonatomic,strong)MTTAvatarImageView *avatar;
@property(nonatomic,strong)UILabel *title;
@property(nonatomic,strong)UILabel *detail;

- (void)setCellContent:(NSString*)avatar title:(NSString*)title detail:(NSString*)detail key:(NSString *)key;
- (void)setGroupAvatar:(MTTGroupEntity *)group;

@end
