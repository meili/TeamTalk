//
//  MTTPCStatusCell.m
//  TeamTalk
//
//  Created by scorpio on 15/7/17.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTPCStatusCell.h"
#import <Masonry.h>

@implementation MTTPCStatusCell

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if(self){
        UIImageView *pcImage = [UIImageView new];
        [self setBackgroundColor:TTBG];
        [self.contentView addSubview:pcImage];
        [pcImage mas_makeConstraints:^(MASConstraintMaker *make) {
            make.centerY.equalTo(self.contentView);
            make.left.mas_equalTo(15);
            make.size.mas_equalTo(CGSizeMake(40, 40));
        }];
        [pcImage setImage:[UIImage imageNamed:@"mac_logined"]];
        UILabel *label = [UILabel new];
        [self.contentView addSubview:label];
        [label setFont:systemFont(15)];
        [label setTextColor:RGB(136, 136, 136)];
        [label setText:@"Mac TT已登录"];
        [label mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(pcImage.mas_right).offset(5);
            make.centerY.equalTo(self.contentView);
            make.right.mas_equalTo(15);
            make.height.mas_equalTo(15);
        }];
        UILabel *bottomLine = [UILabel new];
        [bottomLine setBackgroundColor:TTCELLGRAY];
        [self.contentView addSubview:bottomLine];
        [bottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
            make.height.mas_equalTo(0.5);
            make.left.mas_equalTo(0);
            make.right.mas_equalTo(0);
            make.bottom.mas_equalTo(0);
        }];
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
