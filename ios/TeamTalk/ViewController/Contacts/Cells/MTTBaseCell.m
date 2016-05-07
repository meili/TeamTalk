//
//  MTTBaseCell.m
//  TeamTalk
//
//  Created by scorpio on 15/6/23.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBaseCell.h"
#import <Masonry/Masonry.h>

@implementation MTTBaseCell

- (void)awakeFromNib {
    // Initialization code
}

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        MTT_WEAKSELF(ws);
        _topBorder = [UIView new];
        [_topBorder setBackgroundColor:RGB(229, 229, 229)];
        [self.contentView addSubview:_topBorder];
        [self.textLabel setBackgroundColor:[UIColor clearColor]];
        
        [_topBorder mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.mas_equalTo(15);
            make.top.mas_equalTo(0);
            make.width.equalTo(ws.contentView);
            make.height.mas_equalTo(0.5);
        }];
        
        [_topBorder setHidden:YES];
        
        _bottomBorder = [UIView new];
        [_bottomBorder setBackgroundColor:RGB(229, 229, 229)];
        [self.contentView addSubview:_bottomBorder];
        
        [_bottomBorder mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.mas_equalTo(15);
            make.bottom.mas_equalTo(0);
            make.width.equalTo(ws.contentView);
            make.height.mas_equalTo(0.5);
        }];
        
        [_bottomBorder setHidden:YES];
        
        _detail = [UILabel new];
        [self.contentView addSubview:_detail];
        [_detail setFont:systemFont(15)];
        [_detail setTextColor:TTGRAY];
        [_detail mas_makeConstraints:^(MASConstraintMaker *make) {
            make.right.equalTo(ws.contentView).offset(-15);
            make.top.mas_equalTo(15);
            make.left.equalTo(ws.textLabel.mas_left).offset(100);
            make.height.mas_equalTo(15);
        }];
        [_detail setTextAlignment:NSTextAlignmentRight];
        
        _switchIcon = [UISwitch new];
        [_switchIcon setOnTintColor:TTBLUE];
        
        self.selectedBackgroundView = [[UIView alloc] initWithFrame:self.frame];
        self.selectedBackgroundView.backgroundColor = RGB(244, 245, 246);
    }
    return self;
}

- (void)showTopBorder
{
    [_topBorder setHidden:NO];
}

- (void)showBottomBorder
{
    [_bottomBorder setHidden:NO];
}

- (void)opSwitch:(BOOL)status
{
    [_switchIcon setOn:status];
}

- (void)showSwitch
{
    [self.contentView addSubview:_switchIcon];
    
    [_switchIcon mas_makeConstraints:^(MASConstraintMaker *make) {
        make.right.mas_equalTo(-15);
        make.top.mas_equalTo(6);
        make.width.mas_equalTo(53);
        make.height.mas_equalTo(30);
    }];
    
    [_switchIcon addTarget:self action:@selector(changeNightMode:) forControlEvents:UIControlEventValueChanged];
}

- (void)setDetail:(NSString *)detail{
    [_detail setText:detail];
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    // Configure the view for the selected state
}

- (void)changeNightMode:(UISwitch*)sender
{
    if (self.changeSwitch)
    {
        self.changeSwitch(sender.on);
    }
}

@end
