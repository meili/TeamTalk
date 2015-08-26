//
//  DDContactsCell.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-22.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTGroupEntity.h"
#import "MTTAvatarImageView.h"
@interface DDContactsCell : UITableViewCell
@property(strong)MTTAvatarImageView *avatar;
@property(strong)UILabel *nameLabel;
-(void)setCellContent:(NSString *)avater Name:(NSString *)name;
-(void)setGroupAvatar:(MTTGroupEntity*)group;
@end
