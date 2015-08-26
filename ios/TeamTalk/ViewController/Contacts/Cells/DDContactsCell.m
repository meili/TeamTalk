//
//  DDContactsCell.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-22.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "DDContactsCell.h"
#import "UIImageView+WebCache.h"
#import "MTTGroupEntity.h"
#import "DDUserModule.h"
#import "UIView+Addition.h"
#import "DDGroupModule.h"
#import "MTTAvatarImageView.h"
#import <Masonry.h>
@implementation DDContactsCell

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
        self.avatar = [[MTTAvatarImageView alloc] init];
        [self.contentView addSubview:self.avatar];
        [self.avatar mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(35, 35));
            make.centerY.equalTo(self.contentView);
            make.left.mas_equalTo(10);
        }];
        [self.avatar setContentMode:UIViewContentModeScaleAspectFill];
        [self.avatar setClipsToBounds:YES];
        [self.avatar.layer setCornerRadius:2.0];
        
        self.nameLabel = [UILabel new];
        [self.contentView addSubview:self.nameLabel];
        [self.nameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self.avatar.mas_right).offset(10);
            make.right.mas_equalTo(10);
            make.centerY.equalTo(self.contentView);
        }];
        [self.nameLabel setFont:[UIFont systemFontOfSize:17.0]];
        [self.nameLabel setTextColor:[UIColor blackColor]];
        
    
        UILabel *bottomLine = [UILabel new];
        [self.contentView addSubview:bottomLine];
        [bottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.mas_equalTo(10);
            make.bottom.mas_equalTo(self.contentView.mas_bottom).offset(-0.5);
            make.right.mas_equalTo(0);
            make.height.mas_equalTo(0.5);
        }];
        [bottomLine setBackgroundColor:RGB(229, 229, 229)];
        
        self.selectedBackgroundView = [[UIView alloc] initWithFrame:self.frame];
        self.selectedBackgroundView.backgroundColor = RGB(244, 245, 246);
    }
    return self;
}

- (void)awakeFromNib
{
    // Initialization code
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}
-(void)setCellContent:(NSString *)avatar Name:(NSString *)name
{
    self.nameLabel.text=name;
    UIImage* placeholder = [UIImage imageNamed:@"user_placeholder"];    
    [self.avatar sd_setImageWithURL:[NSURL URLWithString:avatar] placeholderImage:placeholder];
}

- (void)setGroupAvatar:(MTTGroupEntity *)group
{
    NSMutableArray *ids = [[NSMutableArray alloc]init];
    NSMutableArray *avatars = [[NSMutableArray alloc]init];
    NSArray* data = [[group.groupUserIds reverseObjectEnumerator] allObjects];
    if(data.count>=9){
        for (int i=0; i<9; i++) {
            [ids addObject:[data objectAtIndex:i]];
        }
    }else{
        for (int i=0;i<data.count;i++) {
            [ids addObject:[data objectAtIndex:i]];
        }
    }
    [ids enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString* userID = (NSString*)obj;
        [[DDUserModule shareInstance] getUserForUserID:userID Block:^(MTTUserEntity *user) {
            if (user)
            {
                NSString* avatarTmp = [user getAvatarUrl];
                [avatars addObject:avatarTmp];
            }
        }];
    }];
    
    [self.avatar setAvatar:[avatars componentsJoinedByString:@";"] group:1];
}

@end
