//
//  DDRecentUserCell.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "RecentUserCell.h"
#import <SDWebImage/UIImageView+WebCache.h>
#import "NSDate+DDAddition.h"
#import "UIView+Addition.h"
#import "RuntimeStatus.h"
#import "MTTUserEntity.h"
#import "DDMessageModule.h"
#import "DDUserModule.h"
#import "MTTSessionEntity.h"
#import "DDGroupModule.h"
#import <QuartzCore/QuartzCore.h>
#import "MTTPhotosCache.h"
#import "MTTDatabaseUtil.h"
#import <Masonry/Masonry.h>
#import "MTTAvatarImageView.h"
@implementation RecentUserCell

-(instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if(self){
        _avatarImageView = [MTTAvatarImageView new];
        [self.contentView addSubview:_avatarImageView];
        [_avatarImageView setClipsToBounds:YES];
        [_avatarImageView.layer setCornerRadius:4.0];
        [_avatarImageView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(50, 50));
            make.top.mas_equalTo(10);
            make.left.mas_equalTo(10);
        }];
        
        _unreadMessageCountLabel = [UILabel new];
        [_unreadMessageCountLabel setBackgroundColor:RGB(242, 49, 54)];
        [_unreadMessageCountLabel setClipsToBounds:YES];
        [_unreadMessageCountLabel.layer setCornerRadius:9];
        [_unreadMessageCountLabel setTextColor:[UIColor whiteColor]];
        [_unreadMessageCountLabel setFont:systemFont(12)];
        [_unreadMessageCountLabel setTextAlignment:NSTextAlignmentCenter];
        [self.contentView addSubview:_unreadMessageCountLabel];
        [_unreadMessageCountLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(18, 18));
            make.top.mas_equalTo(2);
            make.right.equalTo(_avatarImageView.mas_right).offset(9);
        }];
        
        _shiledUnreadMessageCountLabel = [UILabel new];
        [_shiledUnreadMessageCountLabel setBackgroundColor:RGB(242, 49, 54)];
        [_shiledUnreadMessageCountLabel setClipsToBounds:YES];
        [_shiledUnreadMessageCountLabel.layer setCornerRadius:5];
        [_shiledUnreadMessageCountLabel setTextAlignment:NSTextAlignmentCenter];
        [self.contentView addSubview:_shiledUnreadMessageCountLabel];
        [_shiledUnreadMessageCountLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(10, 10));
            make.top.mas_equalTo(6);
            make.right.equalTo(_avatarImageView.mas_right).offset(4);
        }];
        [_shiledUnreadMessageCountLabel setHidden:YES];
        
        _nameLabel = [UILabel new];
        [_nameLabel setFont:systemFont(17)];
        [self.contentView addSubview:_nameLabel];
        [_nameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatarImageView.mas_right).offset(10);
            make.right.mas_equalTo(self.contentView).offset(-70);
            make.top.mas_equalTo(15);
            make.height.mas_equalTo(17);
        }];
        
        _dateLabel = [UILabel new];
        [_dateLabel setFont:systemFont(12)];
        [_dateLabel setTextAlignment:NSTextAlignmentRight];
        [self.contentView addSubview:_dateLabel];
        [_dateLabel setTextColor:RGB(170, 170, 170)];
        [_dateLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.right.mas_equalTo(self.contentView).offset(-10);
            make.top.mas_equalTo(15);
            make.height.mas_equalTo(12);
            make.width.mas_equalTo(60);
        }];
        
        _shiledImageView = [UIImageView new];
        UIImage* shieldImg = [UIImage imageNamed:@"shielded"];
        [_shiledImageView setImage:shieldImg];
        [self.contentView addSubview:_shiledImageView];
        [_shiledImageView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(14, 14));
            make.right.mas_equalTo(self.contentView).offset(-10);
            make.top.mas_equalTo(_dateLabel.mas_bottom).offset(15);
        }];
        
        _lastmessageLabel = [UILabel new];
        [_lastmessageLabel setFont:systemFont(14)];
        [_lastmessageLabel setTextColor:TTGRAY];
        [self.contentView addSubview:_lastmessageLabel];
        [_lastmessageLabel mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(_avatarImageView.mas_right).offset(10);
            make.right.mas_equalTo(self.contentView).offset(-70);
            make.top.mas_equalTo(_nameLabel.mas_bottom).offset(10);
            make.height.mas_equalTo(16);
        }];
        
        UILabel *bottomLine = [UILabel new];
        [bottomLine setBackgroundColor:TTCELLGRAY];
        [self.contentView addSubview:bottomLine];
        [bottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
            make.height.mas_equalTo(0.5);
            make.left.mas_equalTo(10);
            make.right.mas_equalTo(0);
            make.bottom.mas_equalTo(0);
        }];
        
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];
    if (selected)
    {
        [_nameLabel setTextColor:[UIColor whiteColor]];
        [_lastmessageLabel setTextColor:[UIColor whiteColor]];
        [_dateLabel setTextColor:[UIColor whiteColor]];
    }
    else
    {
        [_nameLabel setTextColor:[UIColor blackColor]];
        [_lastmessageLabel setTextColor:RGB(135, 135, 135)];
        [_dateLabel setTextColor:RGB(135, 135, 135)];
    }
    // Configure the view for the selected state
}

- (void)setHighlighted:(BOOL)highlighted animated:(BOOL)animated               // animate between regular and highlighted state
{
    if (highlighted && self.selected)
    {
        [_nameLabel setTextColor:[UIColor whiteColor]];
        [_lastmessageLabel setTextColor:[UIColor whiteColor]];
        [_dateLabel setTextColor:[UIColor whiteColor]];
    }
    else
    {
        [_nameLabel setTextColor:[UIColor blackColor]];
        [_lastmessageLabel setTextColor:RGB(135, 135, 135)];
        [_dateLabel setTextColor:RGB(135, 135, 135)];
    }
}

#pragma mark - public
- (void)setName:(NSString*)name
{
    if (!name)
    {
        [_nameLabel setText:@""];
    }
    else
    {
        [_nameLabel setText:name];
    }
}

- (void)setTimeStamp:(NSUInteger)timeStamp
{
    NSDate* date = [NSDate dateWithTimeIntervalSince1970:timeStamp];
    NSString* dateString = [date transformToFuzzyDate];
    [_dateLabel setText:dateString];
}

- (void)setLastMessage:(NSString*)message
{
    if (!message)
    {
        [_lastmessageLabel setText:@"."];
    }
    else
    {
        [_lastmessageLabel setText:message];
    }
}

- (void)setAvatar:(NSString*)avatar
{
    
    [[_avatarImageView subviews] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [(UIView*)obj removeFromSuperview];
    }];
    
    NSURL* avatarURL = [NSURL URLWithString:avatar];
    UIImage* placeholder = [UIImage imageNamed:@"user_placeholder"];
    [_avatarImageView sd_setImageWithURL:avatarURL placeholderImage:placeholder];
}

- (void)setShiledUnreadMessage
{
    [self.unreadMessageCountLabel setHidden:YES];
    [self.shiledUnreadMessageCountLabel setHidden:NO];
}

- (void)setUnreadMessageCount:(NSUInteger)messageCount
{
    if (messageCount == 0)
    {
        [self.unreadMessageCountLabel setHidden:YES];
    }
    else if (messageCount < 10)
    {
        [self.unreadMessageCountLabel setHidden:NO];
        CGPoint center = self.unreadMessageCountLabel.center;
        NSString* title = [NSString stringWithFormat:@"%li",messageCount];
        [self.unreadMessageCountLabel setText:title];
        [self.unreadMessageCountLabel setWidth:16];
        [self.unreadMessageCountLabel setCenter:center];
        [self.unreadMessageCountLabel.layer setCornerRadius:8];
    }
    else if (messageCount < 99)
    {
        [self.unreadMessageCountLabel setHidden:NO];
        CGPoint center = self.unreadMessageCountLabel.center;
        NSString* title = [NSString stringWithFormat:@"%li",messageCount];
        [self.unreadMessageCountLabel setText:title];
        [self.unreadMessageCountLabel setWidth:25];
        [self.unreadMessageCountLabel setCenter:center];
        [self.unreadMessageCountLabel.layer setCornerRadius:8];
    }
    else
    {
        [self.unreadMessageCountLabel setHidden:NO];
        CGPoint center = self.unreadMessageCountLabel.center;
        NSString* title = @"99+";
        [self.unreadMessageCountLabel setText:title];
        [self.unreadMessageCountLabel setWidth:34];
        [self.unreadMessageCountLabel setCenter:center];
        [self.unreadMessageCountLabel.layer setCornerRadius:8];
    }
}


-(void)setShowSession:(MTTSessionEntity *)session
{
    [self setName:session.name];
    if ([session.lastMsg isKindOfClass:[NSString class]]) {
        if ([session.lastMsg rangeOfString:DD_MESSAGE_IMAGE_PREFIX].location != NSNotFound) {
            NSArray *array = [session.lastMsg componentsSeparatedByString:DD_MESSAGE_IMAGE_PREFIX];
            NSString *string = [array lastObject];
            if ([string rangeOfString:DD_MESSAGE_IMAGE_SUFFIX].location != NSNotFound) {
                [self setLastMessage:@"[图片]"];
            }else{
                [self setLastMessage:string];
            }
            
        }else if ([session.lastMsg hasSuffix:@".spx"])
        {
            [self setLastMessage:@"[语音]"];
        }
        else{
            [self setLastMessage:session.lastMsg];
            
        }
    }
    
    
    if (session.sessionType == SessionTypeSessionTypeSingle) {
        [_avatarImageView setBackgroundColor:[UIColor clearColor]];
        [[DDUserModule shareInstance] getUserForUserID:session.sessionID Block:^(MTTUserEntity *user) {
            [[_avatarImageView subviews] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [(UIView*)obj removeFromSuperview];
            }];
            [_avatarImageView setImage:nil];
            [self setAvatar:[user getAvatarUrl]];
        }];
    }else{
        [_avatarImageView setBackgroundColor:RGB(228, 227, 230)];
        [_avatarImageView setImage:nil];
        [[_avatarImageView subviews] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            [(UIView*)obj removeFromSuperview];
        }];
        
        [self loadGroupIcon:session];
    }
    [self.shiledUnreadMessageCountLabel setHidden:YES];
    [self setUnreadMessageCount:session.unReadMsgCount];
    [self.shiledImageView setHidden:YES];
    if(session.isGroup){
        MTTGroupEntity *group = [[DDGroupModule instance] getGroupByGId:session.sessionID];
        if (group) {
            if(group.isShield){
                if(session.unReadMsgCount){
                    [self setShiledUnreadMessage];
                }
                [self.shiledImageView setHidden:NO];
            }
        }
    }
    [self setTimeStamp:session.timeInterval];
    if(session.unReadMsgCount)
    {
        //实时获取未读消息从接口
    }
}

-(void)loadGroupIcon:(MTTSessionEntity *)session
{
    [[DDGroupModule instance] getGroupInfogroupID:session.sessionID completion:^(MTTGroupEntity *group) {
        [self setName:group.name];
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
                    NSString* avatar = [user getAvatarUrl];
                    [avatars addObject:avatar];
                }
            }];
        }];

        [_avatarImageView setAvatar:[avatars componentsJoinedByString:@";"] group:1];
    }];
}

-(UIImage *)getImageFromView:(UIView *)orgView{
    CGSize s = orgView.bounds.size;
    UIGraphicsBeginImageContextWithOptions(s, NO, [UIScreen mainScreen].scale);
    [orgView.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage*image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return image;
}
@end