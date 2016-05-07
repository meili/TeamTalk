//
//  userInfoCell.h
//  TeamTalk
//
//  Created by scorpio on 15/6/19.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MTTUserInfoCell : UITableViewCell
@property(strong)UIImageView *avatar;
@property(strong)UILabel *nameLabel;
@property(strong)UILabel *cnameLabel;
-(void)setCellContent:(NSString *)avater Name:(NSString *)name Cname:(NSString *)cname;
@end