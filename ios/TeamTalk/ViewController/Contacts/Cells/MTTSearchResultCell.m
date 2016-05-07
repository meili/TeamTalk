//
//  MTTSearchResultCell.m
//  TeamTalk
//
//  Created by scorpio on 15/7/7.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTSearchResultCell.h"
#import <Masonry.h>
#import "UIImageView+WebCache.h"
#import "MTTGroupEntity.h"
#import "DDUserModule.h"

@implementation MTTSearchResultCell

- (void)awakeFromNib {
    // Initialization code
}

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        _avatar = [MTTAvatarImageView new];
        [_avatar setClipsToBounds:YES];
        [_avatar.layer setCornerRadius:4.0];
        [self.contentView addSubview:_avatar];
        [_avatar mas_makeConstraints:^(MASConstraintMaker *make) {
            make.centerY.equalTo(self.contentView);
            make.left.mas_equalTo(15);
            make.size.mas_equalTo(CGSizeMake(45, 45));
        }];
        
        _title = [UILabel new];
        [_title setFont:systemFont(16)];
        [self.contentView addSubview:_title];
        
        _detail = [UILabel new];
        [_detail setFont:systemFont(15)];
        [_detail setTextColor:TTGRAY];
        [self.contentView addSubview:_detail];
        
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

- (void)setCellContent:(NSString *)avatar title:(NSString *)title detail:(NSString *)detail key:(NSString *)key{
    UIImage* placeholder = [UIImage imageNamed:@"user_placeholder"];
    [_avatar sd_setImageWithURL:[NSURL URLWithString:avatar] placeholderImage:placeholder];
    
    if(detail.length>0){
        [_title mas_remakeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(self.contentView).offset(-10);
            make.right.mas_equalTo(-15);
            make.height.mas_equalTo(18);
        }];
        [_detail mas_remakeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(self.contentView).offset(10);
            make.right.mas_equalTo(-15);
            make.height.mas_equalTo(17);
        }];
    }else{
        [_title mas_remakeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(self.contentView);
            make.right.mas_equalTo(-15);
            make.height.mas_equalTo(18);
        }];
        [_detail mas_remakeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatar.mas_right).offset(10);
            make.centerY.equalTo(self.contentView).offset(10);
            make.right.mas_equalTo(-15);
            make.height.mas_equalTo(0);
        }];
    }
    if(title.length > 0){
        NSMutableAttributedString *attrTitle = [[NSMutableAttributedString alloc] initWithString:title];
        NSRange titleRange = [title rangeOfString:key options:NSCaseInsensitiveSearch];
        [attrTitle addAttribute:NSForegroundColorAttributeName
                          value:TTBLUE
                          range:titleRange];
        _title.attributedText = attrTitle;
    }else{
        _title.text = @"";
    }

    // 如果detail就是"相关的聊天记录"直接返回
    if(detail.length > 0){
        NSRange ifDetail = [detail rangeOfString:@"相关聊天记录"];
        if(ifDetail.length>0){
            _detail.text = detail;
        }else{
            //让搜索到的关键字显示在屏幕上
            NSRange range = [detail rangeOfString:key options:NSCaseInsensitiveSearch];
            NSInteger keyLocation = range.location;
            if(range.length>0 && keyLocation > 20)
            {
                NSRange range = NSMakeRange(keyLocation-10,detail.length-keyLocation+7>50?50:detail.length-keyLocation+10);
                NSString *subContent  = [NSString stringWithFormat:@"%@",[detail substringWithRange:range]];
                subContent =[subContent stringByReplacingOccurrencesOfString:@"\n" withString:@""];
                subContent =[subContent stringByReplacingOccurrencesOfString:@"\U00002028" withString:@""];
                NSMutableString *subContent1 = [[NSMutableString alloc]init];
                [subContent1 appendString:@"..."];
                [subContent1 appendString:subContent];
                detail = subContent1;
            }
            NSError *error;
            NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:key
                                                                                   options:NSRegularExpressionCaseInsensitive
                                                                                     error:&error];
            NSArray *matches = [regex matchesInString:detail options:0 range:NSMakeRange(0, [detail length])];
            NSMutableAttributedString *attrDetail = [[NSMutableAttributedString alloc] initWithString:detail];
            for (NSTextCheckingResult *per in matches)
            {
                [attrDetail addAttribute:NSForegroundColorAttributeName
                                   value:TTBLUE
                                   range:per.range];
            }
            NSMutableParagraphStyle *paragraph = [[NSMutableParagraphStyle alloc] init];
            paragraph.lineSpacing = 0;
            paragraph.paragraphSpacing = 0;
            [attrDetail addAttribute:NSParagraphStyleAttributeName
                               value:paragraph
                               range:NSMakeRange(0, [detail length])];
            _detail.attributedText = attrDetail;
        }
    }else{
        _detail.text = @"";
    }
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

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    // Configure the view for the selected state
}

@end
