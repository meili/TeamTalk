//
//  DDContactAvatarTools.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-25.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "ContactAvatarTools.h"
#import "MTTUserEntity.h"
#import "PublicProfileViewControll.h"
#define TOPOINTX 50
#define TOPOINTY 0
#define DEFAULTRECT CGRectMake(0, self.frame.size.height/2, 0, 0)
@implementation ContactAvatarTools

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        self.item1 = [UIButton buttonWithType:UIButtonTypeCustom];
        self.item1.tag=1;
        [self.item1 setImage:[UIImage imageNamed:@"tel"] forState:UIControlStateNormal];
        [self.item1 addTarget:self action:@selector(clickItem:) forControlEvents:UIControlEventTouchUpInside];
        self.item1.frame=DEFAULTRECT;
        [self addSubview:self.item1];
        
        self.item2 = [UIButton buttonWithType:UIButtonTypeCustom];
        self.item2.tag=2;
         [self.item2 setImage:[UIImage imageNamed:@"email"] forState:UIControlStateNormal];
        [self.item2 addTarget:self action:@selector(clickItem:) forControlEvents:UIControlEventTouchUpInside];
        self.item2.frame=DEFAULTRECT;
        [self addSubview:self.item2];
        
        
        self.item3 = [UIButton buttonWithType:UIButtonTypeCustom];
        self.item3.tag=3;
        [self.item3 setImage:[UIImage imageNamed:@"chat"] forState:UIControlStateNormal];
        [self.item3 addTarget:self action:@selector(clickItem:) forControlEvents:UIControlEventTouchUpInside];
        self.item3.frame=DEFAULTRECT;
        [self addSubview:self.item3];
        [self showItem];
        UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(hiddenSelf)];
        [self addGestureRecognizer:tap];
    }
    return self;
}

-(void)hiddenSelf
{
    [UIView animateWithDuration:0.5 animations:^{
        self.item1.frame=DEFAULTRECT;
    }];
    [UIView animateWithDuration:0.4 animations:^{
        self.item2.frame=DEFAULTRECT;
    }];
    [UIView animateWithDuration:0.3 animations:^{
        self.item3.frame=DEFAULTRECT;
    } completion:^(BOOL finished) {
            self.isShow=NO;
           [self removeFromSuperview];
    }];

}
-(void)showItem
{
    self.isShow=YES;
    [UIView animateWithDuration:0.3 animations:^{
        self.item1.frame=CGRectMake(TOPOINTX-40, TOPOINTY, 30, 30);
    }];
    [UIView animateWithDuration:0.4 animations:^{
         self.item2.frame=CGRectMake(TOPOINTX-25, TOPOINTY+35, 30, 30);
    }];
    [UIView animateWithDuration:0.5 animations:^{
         self.item3.frame=CGRectMake(TOPOINTX-40, TOPOINTY+70, 30, 30);
    }];
   
}

-(IBAction)clickItem:(id)sender
{
    UIButton *button =(UIButton *)sender;
    CGRect rect=button.frame;
    rect.size.width=70;
    rect.size.height=30;
    [UIView animateWithDuration:0.2 animations:^{
        button.frame=rect;
    } completion:^(BOOL finished) {
        [self hiddenSelf];
        switch (button.tag) {
            case 1:
                self.block(1);
                break;
            case 2:
               self.block(2);
                break;
            case 3:
                self.block(3);
                break;
            default:
                break;
        }
    }];
    
}
-(void)goToUserInfo
{

    PublicProfileViewControll *public = [PublicProfileViewControll new];
    public.user=self.user;

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
