//
//  DDChatBaseCell.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-28.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDChatBaseCell.h"
#import "UIView+Addition.h"
#import "DDUserModule.h"
#import "ChattingMainViewController.h"
#import "PublicProfileViewControll.h"
#import "UILabel+VerticalAlign.h"
#import "MTTBubbleModule.h"
#import <UIImageView+WebCache.h>
#import <Masonry/Masonry.h>

CGFloat const dd_avatarEdge = 10;                 //头像到边缘的距离
CGFloat const dd_avatarBubbleGap = 5;             //头像和气泡之间的距离
CGFloat const dd_bubbleUpDown = 20;                //气泡到上下边缘的距离
@interface DDChatBaseCell ()
@property(copy)NSString *currentUserID;

@end
@implementation DDChatBaseCell

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        _contentLabel = [[TTTAttributedLabel alloc] initWithFrame:CGRectZero];
        _contentLabel.userInteractionEnabled = YES;
        UILongPressGestureRecognizer *press = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(longPress:)];
        press.minimumPressDuration = 0.5;
        [_contentLabel addGestureRecognizer:press];
                
//        [self.contentView setBackgroundColor:TTBG];
        [self.contentView setBackgroundColor:[UIColor clearColor]];
        [self setBackgroundColor:[UIColor clearColor]];
        
        self.userAvatar = [UIImageView new];
        [self.userAvatar setUserInteractionEnabled:YES];
        [self.userAvatar setContentMode:UIViewContentModeScaleAspectFill];
        [self.userAvatar setClipsToBounds:YES];
        [self.userAvatar.layer setCornerRadius:20];
        [self.contentView addSubview:self.userAvatar];
        
        self.userName = [UILabel new];
        [self.userName setBackgroundColor:[UIColor clearColor]];
        [self.userName setFont:systemFont(11)];
        [self.userName setTextColor:RGB(102, 102, 102)];
        [self.userName alignTop];
        [self.contentView addSubview:self.userName];
        [self.userName mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(200, 15));
            make.top.mas_equalTo(0);
            make.left.equalTo(self.userAvatar.mas_right).offset(13);
        }];
        
        self.bubbleImageView = [MenuImageView new];
        self.bubbleImageView.tag = 1000;
        self.bubbleImageView.delegate = self;
        [self.bubbleImageView setUserInteractionEnabled:YES];
        [self.contentView addSubview:self.bubbleImageView];
        
        self.activityView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
        [self.activityView setHidesWhenStopped:YES];
        [self.activityView setHidden:YES];
        [self.contentView addSubview:self.activityView];
        
        self.sendFailuredImageView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, 20, 20)];
        [self.sendFailuredImageView setHidden:YES];
        [self.contentView setAutoresizesSubviews:NO];
        self.sendFailuredImageView.userInteractionEnabled=YES;
        [self.contentView addSubview:self.sendFailuredImageView];
        [self.sendFailuredImageView setImage:[UIImage imageNamed:@"dd_send_failed"]];
        UITapGestureRecognizer *pan = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(clickTheSendAgain)];
        [self.sendFailuredImageView addGestureRecognizer:pan];
        UITapGestureRecognizer *openProfile = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(openProfilePage)];
        [self.userAvatar addGestureRecognizer:openProfile];
    }
    return self;
}
-(void)openProfilePage
{
    if (self.currentUserID) {
        [[DDUserModule shareInstance] getUserForUserID:self.currentUserID Block:^(MTTUserEntity *user) {
            PublicProfileViewControll *public = [PublicProfileViewControll new];
            public.user=user;
            [[ChattingMainViewController shareInstance] pushViewController:public animated:YES];
        }];
    }
}
-(void)clickTheSendAgain
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"重发" message:@"是否重新发送此消息" delegate:self cancelButtonTitle:@"取消" otherButtonTitles:@"确定", nil];
    [alert show];
    
}
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1) {
        [self clickTheSendAgain:nil];
    }
}
- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];
}

- (void)setContent:(MTTMessageEntity*)content
{
    // 获取气泡设置
    _leftConfig = [[MTTBubbleModule shareInstance] getBubbleConfigLeft:YES];
    _rightConfig = [[MTTBubbleModule shareInstance] getBubbleConfigLeft:NO];
    
    id<DDChatCellProtocol> cell = (id<DDChatCellProtocol>)self;
    //设置头像位置
    switch (self.location) {
        case DDBubbleLeft:
            [self.userAvatar mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(40, 40));
                make.top.mas_equalTo(0);
                make.left.mas_equalTo(dd_avatarEdge);
            }];
            break;
        case DDBubbleRight:
            [self.userAvatar mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.size.mas_equalTo(CGSizeMake(40, 40));
                make.top.mas_equalTo(0);
                make.right.mas_equalTo(-dd_avatarEdge);
            }];
            break;
        default:
            break;
    }
    // 设置头像和昵称
    self.currentUserID=content.senderId;
    [[DDUserModule shareInstance] getUserForUserID:content.senderId Block:^(MTTUserEntity *user) {
        NSURL* avatarURL = [NSURL URLWithString:[user getAvatarUrl]];
        [self.userAvatar sd_setImageWithURL:avatarURL placeholderImage:[UIImage imageNamed:@"user_placeholder"]];
        [self.userName setText:user.nick];
    }];
    
    //设置昵称是否隐藏
    if (self.session.sessionType == SessionTypeSessionTypeSingle || self.location == DDBubbleRight) {
        [self.userName mas_updateConstraints:^(MASConstraintMaker *make) {
            make.height.mas_equalTo(0);
        }];
        [self.userName setHidden:YES];
    }else{
        [self.userName mas_updateConstraints:^(MASConstraintMaker *make) {
            make.height.mas_equalTo(15);
        }];
        [self.userName setHidden:NO];
    }
    
    CGSize size = [cell sizeForContent:content];
    float bubbleheight = [cell contentUpGapWithBubble] + size.height + [cell contentDownGapWithBubble];
    float bubbleWidth = [cell contentLeftGapWithBubble] + size.width + [cell contentRightGapWithBubble];
    
    //设置气泡相关
    UIImage* bubbleImage = nil;
    switch (self.location)
    {
        case DDBubbleLeft:
        {
            bubbleImage = [UIImage imageNamed:_leftConfig.textBgImage];
            bubbleImage = [bubbleImage stretchableImageWithLeftCapWidth:_leftConfig.stretchy.left topCapHeight:_leftConfig.stretchy.top];
            [self.bubbleImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.left.mas_equalTo(self.userAvatar.mas_right).offset(dd_avatarBubbleGap);
                make.top.mas_equalTo(self.userName.mas_bottom).offset(0);
                make.size.mas_equalTo(CGSizeMake(bubbleWidth, bubbleheight));
            }];
        }
            break;
        case DDBubbleRight:
        {
            bubbleImage = [UIImage imageNamed:_rightConfig.textBgImage];
            bubbleImage = [bubbleImage stretchableImageWithLeftCapWidth:_rightConfig.stretchy.left topCapHeight:_rightConfig.stretchy.top];
            [self.bubbleImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.right.mas_equalTo(self.userAvatar.mas_left).offset(-dd_avatarBubbleGap);
                make.top.mas_equalTo(self.userName.mas_bottom).offset(0);
                make.size.mas_equalTo(CGSizeMake(bubbleWidth, bubbleheight));
            }];
            break;
        }
        default:
            break;
    }
    [self.bubbleImageView setImage:bubbleImage];
    
    //设置菊花位置
    switch (self.location)
    {
        case DDBubbleLeft:
        {
            [self.activityView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.left.mas_equalTo(self.bubbleImageView.mas_right).offset(10);
                make.bottom.mas_equalTo(self.bubbleImageView.mas_bottom);
            }];
            [self.sendFailuredImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.left.mas_equalTo(self.bubbleImageView.mas_right).offset(10);
                make.bottom.mas_equalTo(self.bubbleImageView.mas_bottom);
            }];
        }
            break;
        case DDBubbleRight:
        {
            
            [self.activityView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.right.mas_equalTo(self.bubbleImageView.mas_left).offset(-10);
                make.bottom.mas_equalTo(self.bubbleImageView.mas_bottom);
            }];
            [self.sendFailuredImageView mas_remakeConstraints:^(MASConstraintMaker *make) {
                make.right.mas_equalTo(self.bubbleImageView.mas_left).offset(-10);
                make.bottom.mas_equalTo(self.bubbleImageView.mas_bottom);
            }];
        }
            break;
        default:
            break;
    }
    
    DDImageShowMenu showMenu = 0;
    
    switch (content.state)
    {
        case DDMessageSending:
            [self.activityView startAnimating];
            self.sendFailuredImageView.hidden = YES;
            break;
        case DDMessageSendFailure:
            [self.activityView stopAnimating];
            self.sendFailuredImageView.hidden = NO;
            showMenu = DDShowSendAgain;
            break;
        case DDmessageSendSuccess:
            [self.activityView stopAnimating];
            self.sendFailuredImageView.hidden = YES;
            break;
    }
    
    //设置菜单
    switch (content.msgContentType) {
        case DDMessageTypeImage:
            showMenu = showMenu | DDShowPreview;
            break;
        case DDMessageTypeText:
            showMenu = showMenu | DDShowCopy;
            break;
        case DDMessageTypeVoice:
            showMenu = showMenu | DDShowEarphonePlay | DDShowSpeakerPlay;
            break;
        default:
            break;
    }
    [self.bubbleImageView setShowMenu:showMenu];
    
    //设置内容位置
    [cell layoutContentView:content];
    
}

-(void)longPress:(UIGestureRecognizer*) recognizer{
    [self.bubbleImageView showTheMenu];

}


- (void)showSendFailure
{
    [self.activityView stopAnimating];
    self.sendFailuredImageView.hidden = NO;
    DDImageShowMenu showMenu = self.bubbleImageView.showMenu | DDShowSendAgain;
    [self.bubbleImageView setShowMenu:showMenu];
}

- (void)showSendSuccess
{
    [self.activityView stopAnimating];
    self.sendFailuredImageView.hidden = YES;
}

- (void)showSending
{
    [self.activityView startAnimating];
    self.sendFailuredImageView.hidden = YES;
}

#pragma mark -
#pragma mark Protocol
- (CGSize)sizeForContent:(MTTMessageEntity*)message
{
    return CGSizeZero;
}

- (float)contentUpGapWithBubble
{
    return 0;
}

- (float)contentDownGapWithBubble
{
    return 0;
}

- (float)contentLeftGapWithBubble
{
    return 0;
}

- (float)contentRightGapWithBubble
{
    return 0;
}

- (void)layoutContentView:(MTTMessageEntity*)content
{
}

- (float)cellHeightForMessage:(MTTMessageEntity*)message
{
    _leftConfig = [[MTTBubbleModule shareInstance] getBubbleConfigLeft:YES];
    _rightConfig = [[MTTBubbleModule shareInstance] getBubbleConfigLeft:NO];
    return 0;
}


#pragma mark -
#pragma mark DDMenuImageView Delegate
- (void)clickTheCopy:(MenuImageView*)imageView
{
    //子类去继承
}

- (void)clickTheEarphonePlay:(MenuImageView*)imageView
{
    //子类去继承
}

- (void)clickTheSpeakerPlay:(MenuImageView*)imageView
{
    //子类去继承
}

- (void)clickTheSendAgain:(MenuImageView*)imageView
{
    //子类去继承
}

- (void)tapTheImageView:(MenuImageView*)imageView
{
    if (self.tapInBubble)
    {
        self.tapInBubble();
    }
}

- (void)clickThePreview:(MenuImageView *)imageView
{
    //子类去继承
}
@end
