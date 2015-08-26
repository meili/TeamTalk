//
//  MTTSearchShowMore.m
//  TeamTalk
//
//  Created by scorpio on 15/7/7.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTSearchShowMoreCell.h"
#import <Masonry.h>

@implementation MTTSearchShowMoreCell

- (void)awakeFromNib {
    // Initialization code
}

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        UIImageView *search = [UIImageView new];
        [search setImage:[UIImage imageNamed:@"search_icon"]];
        [self.contentView addSubview:search];
        [search mas_makeConstraints:^(MASConstraintMaker *make) {
            make.centerY.equalTo(self.contentView);
            make.left.mas_equalTo(15);
            make.size.mas_equalTo(CGSizeMake(13, 13));
        }];
        
        UILabel *moreLabel = [UILabel new];
        [self.contentView addSubview:moreLabel];
        [moreLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.centerY.equalTo(self.contentView);
            make.left.mas_equalTo(search.mas_right).offset(10);
            make.right.mas_equalTo(15);
            make.height.mas_equalTo(13);
        }];
        [moreLabel setText:@"显示更多查询结果"];
        [moreLabel setTextColor:RGB(113, 130, 164)];
        [moreLabel setFont:systemFont(13)];
        
        UILabel *bottomLine = [UILabel new];
        [bottomLine setBackgroundColor:RGB(229, 229, 229)];
        [self.contentView addSubview:bottomLine];
        [bottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
            make.top.equalTo(self.contentView.mas_bottom).offset(-0.5);
            make.left.mas_equalTo(15);
            make.right.mas_equalTo(0);
            make.height.mas_equalTo(0.5);
        }];
        
        self.selectedBackgroundView = [[UIView alloc] initWithFrame:self.frame];
        self.selectedBackgroundView.backgroundColor = RGB(244, 245, 246);
    }
    return self;
}


- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
