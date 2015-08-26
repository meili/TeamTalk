//
//  userInfoCell.m
//  TeamTalk
//
//  Created by scorpio on 15/6/19.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTUserInfoCell.h"
#import "UIImageView+WebCache.h"
#import <Masonry/Masonry.h>

@implementation MTTUserInfoCell

- (void)awakeFromNib {
    // Initialization code
}

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        MTT_WEAKSELF(ws);
        _avatar = [UIImageView new];
        [_avatar setContentMode:UIViewContentModeScaleAspectFit];
        [_avatar setClipsToBounds:YES];
//        [_avatar.layer setBorderWidth:0.3];
//        [_avatar.layer setBorderColor:RGB(153,153,153).CGColor];
        [_avatar.layer setCornerRadius:4.0];
        [self.contentView addSubview:_avatar];

        [_avatar mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.mas_equalTo(15);
            make.centerY.equalTo(ws.contentView);
            make.size.mas_equalTo(CGSizeMake(70, 70));
        }];
        
        _nameLabel = [UILabel new];
        [self.contentView addSubview:_nameLabel];
        [_nameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(ws.contentView).offset(-16);
            make.size.mas_equalTo(CGSizeMake(100, 16));
        }];
        
        _cnameLabel = [UILabel new];
        [_cnameLabel setTextColor:TTGRAY];
        [self.contentView addSubview:_cnameLabel];
        [_cnameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(ws.contentView).offset(16);
            make.size.mas_equalTo(CGSizeMake(100, 16));
        }];
        
        self.selectedBackgroundView = [[UIView alloc] initWithFrame:self.frame];
        self.selectedBackgroundView.backgroundColor = RGB(244, 245, 246);
    }
    return self;
}

-(void)setCellContent:(NSString *)avatar Name:(NSString *)name Cname:(NSString *)cname
{
    UIImage* placeholder = [UIImage imageNamed:@"user_placeholder"];
    [_avatar sd_setImageWithURL:[NSURL URLWithString:avatar] placeholderImage:placeholder];
    [_nameLabel setText:cname];
    [_cnameLabel setText:name];
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
