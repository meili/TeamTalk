//
//  MTTGroupInfoCell.h
//  TeamTalk
//
//  Created by scorpio on 15/6/25.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef void(^ChangeSwitch)(BOOL on);

@interface MTTGroupInfoCell : UITableViewCell

@property(strong)UILabel *desc;
@property(strong)UILabel *detail;
@property(strong)UISwitch *switchIcon;

@property(nonatomic,copy)ChangeSwitch changeSwitch;

-(void)showSwitch;
-(void)opSwitch:(BOOL)status;
-(void)showDesc:(NSString *)desc;
-(void)showDetail:(NSString *)detail;

@end