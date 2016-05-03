//
//  MTTPCLoginViewController.m
//  TeamTalk
//
//  Created by scorpio on 15/7/17.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTPCLoginViewController.h"
#import "MTTKickPCClientAPI.h"
#import <Masonry.h>

@interface MTTPCLoginViewController ()

@end

@implementation MTTPCLoginViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.view setBackgroundColor:[UIColor whiteColor]];
    
    UILabel *close = [UILabel new];
    [close setText:@"关闭"];
    [close setTextColor:TTBLUE];
    [close setFont:systemFont(18)];
    [self.view addSubview:close];
    [close mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.mas_equalTo(32);
        make.left.mas_equalTo(15);
        make.size.mas_equalTo(CGSizeMake(200, 18));
    }];
    
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(goBack)];
    close.userInteractionEnabled=YES;
    [close addGestureRecognizer:tap];
    
    UILabel *closeLine = [UILabel new];
    [closeLine setBackgroundColor:RGB(202, 202, 202)];
    [self.view addSubview:closeLine];
    [closeLine mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(close.mas_bottom).offset(15);
        make.left.mas_equalTo(0);
        make.right.mas_equalTo(0);
        make.height.mas_equalTo(1);
    }];
    
    UIImageView *macImage = [UIImageView new];
    [self.view addSubview:macImage];
    [macImage setImage:[UIImage imageNamed:@"connect_alert_mac"]];
    [macImage mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(closeLine.mas_bottom).offset(80);
        make.centerX.equalTo(self.view);
        make.size.mas_equalTo(CGSizeMake(130, 100));
    }];
    
    UILabel *macTTOnline = [UILabel new];
    [self.view addSubview:macTTOnline];
    [macTTOnline setText:@"Mac TT已登录"];
    [macTTOnline setFont:systemFont(18)];
    [macTTOnline setTextAlignment:NSTextAlignmentCenter];
    [macTTOnline mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(macImage.mas_bottom).offset(50);
        make.left.mas_equalTo(0);
        make.right.mas_equalTo(0);
        make.height.mas_equalTo(18);
    }];
    
    UILabel *logOut = [UILabel new];
    [self.view addSubview:logOut];
    [logOut setText:@"退出Mac TT"];
    [logOut setFont:systemFont(15)];
    [logOut setTextColor:TTGRAY];
    [logOut setTextAlignment:NSTextAlignmentCenter];
    [logOut.layer setBorderWidth:1];
    [logOut.layer setBorderColor:TTGRAY.CGColor];
    [logOut setClipsToBounds:YES];
    [logOut.layer setCornerRadius:5];
    [logOut mas_makeConstraints:^(MASConstraintMaker *make) {
        make.bottom.equalTo(self.view).offset(-75);
        make.height.mas_equalTo(40);
        make.width.mas_equalTo(150);
        make.centerX.equalTo(self.view);
    }];
    
    UITapGestureRecognizer *kickTap = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(kickPC)];
    logOut.userInteractionEnabled=YES;
    [logOut addGestureRecognizer:kickTap];
    
}

- (void)kickPC{
    MTTKickPCClientAPI *request = [MTTKickPCClientAPI new];
    NSMutableArray *array = [NSMutableArray new];
    [request requestWithObject:array Completion:^(NSArray *response, NSError *error) {
        [self dismissViewControllerAnimated:YES completion:^{}];
    }];
}

- (void)goBack{
    [self dismissViewControllerAnimated:YES completion:^{
        
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
