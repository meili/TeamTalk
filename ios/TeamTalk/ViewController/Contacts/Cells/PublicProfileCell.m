//
//  PublicProfileCell.m
//  TeamTalk
//
//  Created by scorpio on 14/12/18.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "PublicProfileCell.h"

#import <Masonry/Masonry.h>

@implementation PublicProfileCell

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        UIView *contentView =self.contentView;
        
        _descLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, 15, 30, 15)];
        [_descLabel setFont:systemFont(15)];
        [_descLabel setTextColor:RGB(164, 165, 169)];
        [contentView addSubview:_descLabel];
        
        [_descLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.top.equalTo(contentView).offset(0);
            make.bottom.equalTo(contentView).offset(0);
            make.leading.mas_equalTo(20);
            make.trailing.mas_equalTo(-15);
        }];
        
        _detailLabel = [[UILabel alloc]initWithFrame:CGRectMake(70, 12, SCREEN_WIDTH -85, 20)];
        _detailLabel.numberOfLines =4;
        [_detailLabel setFont:systemFont(15)];
        [contentView addSubview:_detailLabel];
        
        [_detailLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.top.equalTo(contentView).offset(0);
            make.bottom.equalTo(contentView).offset(0);
            make.leading.mas_equalTo(85);
            make.trailing.mas_equalTo(-15);
        }];
        
    }
    return self;
}

- (void)setDesc:(NSString *)desc detail:(NSString *)detail{
    [_descLabel setText:desc];
    [_detailLabel setText:detail];
}

- (void)awakeFromNib {
    // Initialization code
}

+(CGFloat)cellHeightForDetailString:(NSString *)string{

    UIFont *font =systemFont(15);
    
    CGRect rect =[string boundingRectWithSize:CGSizeMake(SCREEN_WIDTH -100, 1000)  options:NSStringDrawingUsesLineFragmentOrigin attributes:[NSDictionary dictionaryWithObjectsAndKeys:font,NSFontAttributeName, nil] context:nil];
    
    CGFloat contentH = rect.size.height +24;
    
    if (contentH <44) {
        contentH =44;
    }
    
    return contentH;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
