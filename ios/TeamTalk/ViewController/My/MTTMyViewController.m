//
//  MTTMyViewController.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-15.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTMyViewController.h"
#import "PublicProfileViewControll.h"
#import "RuntimeStatus.h"
#import "UIImageView+WebCache.h"
#import "DDUserDetailInfoAPI.h"
#import "MTTPhotosCache.h"
#import "LogoutAPI.h"
#import "MTTLoginViewController.h"
#import "DDClientState.h"
#import "DDUserModule.h"
#import "MTTDatabaseUtil.h"
#import "NSString+Additions.h"
#import "SCLAlertView.h"
#import "MTTNotification.h"
#import "MTTUserInfoCell.h"
#import "MTTBaseCell.h"
#import <Masonry/Masonry.h>
#import "MTTNightModeAPI.h"
#import "MTTChangeNightModeAPI.h"
#import "UIView+PointBadge.h"
#import "MTTBubbleShowViewControll.h"
#import "LCActionSheet.h"

@interface MTTMyViewController ()
@property(assign)NSInteger hadUpdate;

@end

@implementation MTTMyViewController

- (id)init
{
    self = [super init];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    MTT_WEAKSELF(ws);
    
    // 默认设置夜间模式关闭
    _pushShiledStatus = 0;
    
    [super viewDidLoad];
    
    self.title=@"我";
    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
    [self.tableView setDelegate:self];
    [self.tableView setDataSource:self];
    [self.view addSubview:self.tableView];
    
    [self.view setBackgroundColor:TTBG];
    [self.tableView setBackgroundColor:TTBG];
    
    [self.tableView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.equalTo(ws.view);
        make.center.equalTo(ws.view);
    }];
    
    [self.tableView registerClass:[MTTUserInfoCell class] forCellReuseIdentifier:@"myInfoCellIdentifier"];
    [self.tableView registerClass:[MTTBaseCell class] forCellReuseIdentifier:@"myFuncIdentifier"];
    [self.tableView registerClass:[MTTBaseCell class] forCellReuseIdentifier:@"logoutIdentifier"];
    [self.tableView registerClass:[MTTBaseCell class] forCellReuseIdentifier:@"extraIdentifier"];
    
    // 获取夜间模式状态
    MTTNightModeAPI *request = [MTTNightModeAPI new];
    NSMutableArray *array = [NSMutableArray new];
    [request requestWithObject:array Completion:^(NSArray *response, NSError *error) {
        if(response){
            NSMutableArray *narray = [NSMutableArray arrayWithArray:response];
            _pushShiledStatus = [narray[0] boolValue];
            [self.tableView reloadData];
        }
    }];
    
    self.hadUpdate=0;
    
    if([TheRuntime.updateInfo[@"haveupdate"] boolValue])
    {
        self.hadUpdate =1;
    }else{
        self.hadUpdate = 0;
    }
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    self.title=@"我";
    
    [self.tableView reloadData];
}

-(void)viewDidDisappear:(BOOL)animated{
    
    [super viewDidDisappear:animated];
    
    self.tableView.contentInset =UIEdgeInsetsMake(0,0,0,0);
}

-(void)viewDidAppear:(BOOL)animated{

    [super viewDidAppear:animated];
    
    self.tableView.contentInset =UIEdgeInsetsMake(64, 0, 49, 0);
}

-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 16;
}

-(CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    if(section != 0){
        return 20;
    }else{
        return 0.1;
    }
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    UIView *footerView = [[UIView alloc]initWithFrame:CGRectMake(0, 0, SCREEN_WIDTH, 16)];
    UILabel *detail = [[UILabel alloc]initWithFrame:CGRectMake(20, 5, SCREEN_WIDTH-40, 16)];
    [detail setFont:systemFont(12)];
    [detail setTextColor:TTGRAY];
    if(section == 1){
        [detail setText:@"开启后,在22:00-8:00时间段收到消息不会有推送."];
        [footerView addSubview:detail];
        return footerView;
    }else if(section == 2){
//        [detail setText:[NSString stringWithFormat:@"版本：%@",MTTVerison]];
        [detail setText:@""];
        [detail setTextAlignment:NSTextAlignmentCenter];
        [footerView addSubview:detail];
        return footerView;
    }else{
        return footerView;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(indexPath.section == 0){
        return 100;
    }else{
        return 43;
    }
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 3;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section{
    if(section == 0 || section == 2){
        return 1;
    }else{
        return 3;
    }
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0)
    {
        static NSString* identifier = @"myInfoCellIdentifier";
        MTTUserInfoCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier forIndexPath:indexPath];
        
        [[DDUserModule shareInstance] getUserForUserID:[RuntimeStatus instance].user.objID Block:^(MTTUserEntity *user) {
            self.user=user;
            [cell setCellContent:[user getAvatarUrl] Name:user.name Cname:user.nick];
        }];
        [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
        return cell;
        
    }else if(indexPath.section == 1){
        static NSString* identifier = @"myFuncIdentifier";
        MTTBaseCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier forIndexPath:indexPath];
        NSInteger row = [indexPath row];
        if (row == 0)
        {
            [cell.textLabel setText:@"清理缓存"];
        }
        else if (row == 1)
        {
            [cell.textLabel setText:@"检查更新"];
            [cell setDetail:[NSString stringWithFormat:@"%@",MTTVerison]];
            [cell setUserInteractionEnabled:NO];
            if (self.hadUpdate) {
                [cell setUserInteractionEnabled:YES];
                [cell showPointBadge:NO];
                UIView *pointView =[cell pointBadgeView];
                [pointView mas_makeConstraints:^(MASConstraintMaker *make) {
                    make.left.mas_equalTo(90);
                    make.centerY.equalTo(cell);
                    make.width.mas_equalTo(PointBadgeDiameter);
                    make.height.mas_equalTo(PointBadgeDiameter);
                }];
            }
        }
        else if(row == 2)
        {
            [cell.textLabel setText:@"夜间模式"];
            [cell setUserInteractionEnabled:YES];
            [cell showSwitch];
            [cell opSwitch:_pushShiledStatus];
            [cell setChangeSwitch:^(BOOL on){
                MTTChangeNightModeAPI *changeRequest = [MTTChangeNightModeAPI new];
                NSMutableArray *array = [NSMutableArray new];
                if(on){
                    [array addObject:@(1)];
                }else{
                    [array addObject:@(0)];
                }
                [changeRequest requestWithObject:array Completion:^(NSArray *response, NSError *error) {
                    _pushShiledStatus = on;
                }];
            }];

        }
        return cell;
    }else if(indexPath.section == 2){
        static NSString* identifier = @"logoutIdentifier";
        MTTBaseCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier forIndexPath:indexPath];
        [cell.textLabel setText:@"退出登录"];
        [cell.textLabel setTextAlignment:NSTextAlignmentCenter];
        return cell;
    }else{
        static NSString* identifier = @"extraIdentifier";
        UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier forIndexPath:indexPath];
        return cell;
    }
}
-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setSelected:NO];
    if (indexPath.section == 0){
        [self goPersonalProfile];
    }else if(indexPath.section == 1){
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        if (indexPath.row==0) {
            [self clearCacheButtonPressed:nil];
        }else if(indexPath.row == 1){
            [self gotoUpdatePage];
        }
    }else{
        [self logoutButtonPressed:nil];
    }
}

-(void)clearCacheButtonPressed:(id)sender
{
    LCActionSheet *sheet = [[LCActionSheet alloc] initWithTitle:@"是否清理图片缓存?"
                                                   buttonTitles:@[@"确定"]
                                                 redButtonIndex:0
                                                       delegate:self];
    sheet.tag = 10001;
    [sheet show];
}


-(void)logoutButtonPressed:(id)sender
{
    LCActionSheet *sheet = [[LCActionSheet alloc] initWithTitle:@"退出不会删除任何历史数据,TT等你归来!"
                                                   buttonTitles:@[@"退出登陆"]
                                                 redButtonIndex:0
                                                       delegate:self];
    sheet.tag = 10000;
    [sheet show];
}

#pragma mark - LCActionSheetDelegate
- (void)actionSheet:(LCActionSheet *)actionSheet didClickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(actionSheet.tag == 10000){
        if(buttonIndex == 0){
            LogoutAPI *logout = [LogoutAPI new];
            [logout requestWithObject:nil Completion:NULL];
            [MTTNotification postNotification:DDNotificationLogout userInfo:nil object:nil];
        }
    }
    if(actionSheet.tag == 10001){
        if(buttonIndex == 0){
            [[MTTPhotosCache sharedPhotoCache] clearAllCache:^(bool isfinish) {
                if (isfinish) {
                    NSLog(@"11");
                }
            }];
        }
    }
}

-(void)gotoUpdatePage
{
    NSString *url = TheRuntime.updateInfo[@"url"];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:url]];
}

-(void)goPersonalProfile
{
    PublicProfileViewControll *public = [PublicProfileViewControll new] ;
    public.user = self.user;
    [self pushViewController:public animated:YES];
}

-(void)gotoSelectBubble
{
    MTTBubbleShowViewControll *bubble = [MTTBubbleShowViewControll new] ;
    [self pushViewController:bubble animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
