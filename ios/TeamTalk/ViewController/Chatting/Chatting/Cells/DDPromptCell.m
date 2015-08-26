//
//  DDPromptCell.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-9.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDPromptCell.h"
#import "UIView+Addition.h"
#import <Masonry/Masonry.h>
@implementation DDPromptCell

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        _promptLabel = [UILabel new];
        [_promptLabel setClipsToBounds:YES];
        [_promptLabel setFont:systemFont(11)];
        [_promptLabel.layer setCornerRadius:5];
        [_promptLabel setTextColor:RGB(255, 255, 255)];
        [_promptLabel setBackgroundColor:RGBA(100, 100, 100, 0.2)];
        [_promptLabel setTextAlignment:NSTextAlignmentCenter];
        [self.contentView addSubview:_promptLabel];
        [self.contentView setBackgroundColor:[UIColor clearColor]];
        [self setBackgroundColor:[UIColor clearColor]];
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void)setprompt:(NSString*)prompt
{
    CGRect size = [prompt boundingRectWithSize:CGSizeMake(SCREEN_WIDTH-30, 1000000) options:NSStringDrawingUsesLineFragmentOrigin attributes:[NSDictionary dictionaryWithObjectsAndKeys:systemFont(11),NSFontAttributeName, nil] context:nil];
    [_promptLabel mas_remakeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(self.contentView);
        make.top.mas_equalTo(0);
        make.size.mas_equalTo(CGSizeMake(size.size.width+30, size.size.height + 6));
    }];
    [_promptLabel setText:prompt];
}

@end
