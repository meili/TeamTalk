//
//  MTTBubbleCollectionViewCell.m
//  TeamTalk
//
//  Created by scorpio on 15/7/3.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBubbleCollectionViewCell.h"
#import "MTTBubbleModule.h"
#import <Masonry/Masonry.h>

@implementation MTTBubbleCollectionViewCell

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        _bubbleImage = [UIImageView new];
        [self.contentView addSubview:_bubbleImage];
        [_bubbleImage mas_makeConstraints:^(MASConstraintMaker *make) {
            make.center.equalTo(self.contentView);
            make.size.equalTo(self.contentView);
        }];
        
        _isSelected = [UIImageView new];
        [self.contentView addSubview:_isSelected];
        [_isSelected mas_makeConstraints:^(MASConstraintMaker *make) {
            make.right.mas_equalTo(5);
            make.top.mas_equalTo(-5);
            make.size.mas_equalTo(CGSizeMake(22, 22));
        }];
        [_isSelected setImage:[UIImage imageNamed:@"select"]];
        [_isSelected setHidden:YES];
        
        _bubbleContent = [UILabel new];
        [_bubbleContent setTextColor:[UIColor blackColor]];
        [self.contentView addSubview:_bubbleContent];
        [_bubbleContent mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.mas_equalTo(30);
            make.center.equalTo(self.contentView);
        }];
        
        [self.contentView setBackgroundColor:RGB(240, 239, 244)];
    }
    return self;
}

- (void)setcontent:(NSString*)key value:(NSString *)value
{
    NSString *bgPath = [[NSString alloc]initWithFormat:@"bubble.bundle/%@/thumb", value];
    UIImage *bg = [UIImage imageNamed:bgPath];
    [_bubbleImage setImage:bg];
}

- (void)setCellSelected:(BOOL)selected
{
    [_isSelected setHidden:!selected];
}

@end
