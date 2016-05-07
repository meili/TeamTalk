//
//  MTTBaseCell.h
//  TeamTalk
//
//  Created by scorpio on 15/6/23.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef void(^ChangeSwitch)(BOOL on);

@interface MTTBaseCell : UITableViewCell
@property(nonatomic,strong)UILabel *desc;
@property(nonatomic,strong)UILabel *detail;
@property(nonatomic,strong)UIView *topBorder;
@property(nonatomic,strong)UIView *bottomBorder;
@property(nonatomic,strong)UISwitch *switchIcon;

@property(nonatomic,copy)ChangeSwitch changeSwitch;

-(void)showSwitch;
-(void)opSwitch:(BOOL)status;
-(void)showTopBorder;
-(void)showBottomBorder;
-(void)setDetail:(NSString *)detail;
@end
