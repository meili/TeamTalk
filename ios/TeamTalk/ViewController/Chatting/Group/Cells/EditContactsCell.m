//
//  EditContactsCell.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-09-10.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "EditContactsCell.h"
#import <Masonry.h>
@interface EditContactsCell()
@property(strong)UIImageView *selectView;
@end
@implementation EditContactsCell
- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        self.selectView = [UIImageView new];
        [self addSubview:self.selectView];
        [self.selectView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(22, 22));
            make.left.mas_equalTo(10);
            make.centerY.mas_equalTo(self.contentView);
        }];
        [self.selectView setImage:[UIImage imageNamed:@"unselected"]];
        [self.selectView setHighlightedImage:[UIImage imageNamed:@"select"]];
        [self.avatar mas_remakeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(35, 35));
            make.left.equalTo(self.selectView.mas_right).offset(10);
            make.centerY.mas_equalTo(self.contentView);
        }];
    }
    return self;
}
- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        
    }
    return self;
}

-(void)setCellToSelected:(BOOL)select
{
    [self.selectView setHighlighted:select];
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
