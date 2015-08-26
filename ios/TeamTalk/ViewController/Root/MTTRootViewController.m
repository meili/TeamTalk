//
//  MTTRootViewController.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-15.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTRootViewController.h"
#import "RecentUsersViewController.h"
#import "ContactsViewController.h"
#import "MTTMyViewController.h"
#import "FinderViewController.h"
#import "MTTLoginViewController.h"
#import "MTTSessionEntity.h"
#import <Masonry/Masonry.h>
#import "UIView+PointBadge.h"
#import "UITabBar+SubView.h"
#import "DDUserModule.h"
#import "UIAlertView+Block.h"
#import "DDClientState.h"

@interface MTTRootViewController ()<UITabBarControllerDelegate,UITabBarDelegate>
@property(assign) NSUInteger clickCount;
@property (nonatomic,strong) UIPanGestureRecognizer *panGestureRecognizer;
@end

@implementation MTTRootViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(kickOffUserNotification:) name:DDNotificationUserKickouted object:nil];

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(logoutNotification:) name:DDNotificationLogout object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(signChangeNotification:) name:DDNotificationUserSignChanged object:nil];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    
    RecentUsersViewController *recentVC= [RecentUsersViewController shareInstance];
    UIImage* conversationSelected = [[UIImage imageNamed:@"conversation_selected"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    recentVC.tabBarItem = [[UITabBarItem alloc]initWithTitle:@"消息" image:[UIImage imageNamed:@"conversation"] selectedImage:conversationSelected];
    recentVC.tabBarItem.tag=100;
    [recentVC.tabBarItem setTitleTextAttributes:[NSDictionary dictionaryWithObject:RGB(26, 140, 242) forKey:NSForegroundColorAttributeName] forState:UIControlStateSelected];
    recentVC.hidesBottomBarWhenPushed =YES;
    
    UIImage* contactSelected = [[UIImage imageNamed:@"contact_selected"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    ContactsViewController *contactVC =[ContactsViewController new];
    contactVC.tabBarItem = [[UITabBarItem alloc]initWithTitle:@"通讯录" image:[UIImage imageNamed:@"contact"] selectedImage:contactSelected];
    contactVC.tabBarItem.tag=101;
    [contactVC.tabBarItem setTitleTextAttributes:[NSDictionary dictionaryWithObject:RGB(26, 140, 242) forKey:NSForegroundColorAttributeName] forState:UIControlStateSelected];
    contactVC.hidesBottomBarWhenPushed =YES;
    
    UIImage* findSelected = [[UIImage imageNamed:@"tab_nav_selected"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    FinderViewController *findVC =[[FinderViewController alloc] init];
    findVC.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"发现" image:[UIImage imageNamed:@"tab_nav"] selectedImage:findSelected];
    findVC.tabBarItem.tag = 102;
    [findVC.tabBarItem setTitleTextAttributes:[NSDictionary dictionaryWithObject:RGB(26, 140, 242) forKey:NSForegroundColorAttributeName] forState:UIControlStateSelected];
    findVC.hidesBottomBarWhenPushed =YES;
    
    UIImage* myProfileSelected = [[UIImage imageNamed:@"myprofile_selected"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    MTTMyViewController *myVC =[MTTMyViewController new];
    myVC.tabBarItem = [[UITabBarItem alloc]initWithTitle:@"我" image:[UIImage imageNamed:@"myprofile"] selectedImage:myProfileSelected];
    myVC.tabBarItem.tag=103;
    [myVC.tabBarItem setTitleTextAttributes:[NSDictionary dictionaryWithObject:RGB(26, 140, 242) forKey:NSForegroundColorAttributeName] forState:UIControlStateSelected];
    findVC.hidesBottomBarWhenPushed =YES;
    
    self.viewControllers=@[recentVC,contactVC,findVC,myVC];
    self.delegate=self;
    self.title=APP_NAME;
//    self.tabBar.translucent = YES;
    
//    [[UIApplication sharedApplication] setStatusBarHidden:YES];
}

-(void)viewWillAppear:(BOOL)animated{

    [super viewWillAppear:animated];
    
    self.navigationItem.hidesBackButton =YES;
    self.navigationController.navigationBarHidden =NO;
    
    BOOL update = [TheRuntime.updateInfo[@"haveupdate"] boolValue];
    
    if (![MTTUtil isUseFunctionBubble] || update) {
        UIImageView *imgView =[self.tabBar tabBarButtonImageViewWithTitle:@"我"];
        [imgView showPointBadge:NO];
    }
}

-(void)viewWillDisappear:(BOOL)animated{

    [super viewWillDisappear:animated];
    
    self.navigationController.interactivePopGestureRecognizer.enabled =YES;
}

-(void)viewDidAppear:(BOOL)animated{

    [super viewDidAppear:animated];
    
    self.navigationController.interactivePopGestureRecognizer.enabled =NO;
    
    UIImageView *imgView =[self.tabBar tabBarButtonImageViewWithTitle:@"我"];
    
    BOOL update = [TheRuntime.updateInfo[@"haveupdate"] boolValue];
    
    if (![MTTUtil isUseFunctionBubble] || update) {
        
        UIView *pointBadgeView =[imgView pointBadgeView];
        if (![[self.tabBar.superview subviews] containsObject:pointBadgeView]) {
            CGRect rect =[imgView convertRect:pointBadgeView.frame toView:self.tabBar.superview];
            pointBadgeView.frame =rect;
            [self.tabBar.superview addSubview:pointBadgeView];
        }
    }
    else{
        [imgView removePointBadge:YES];
    }
}

#pragma mark -


#pragma mark - 退出登录和被踢登录 通知

-(void)logoutNotification:(NSNotification*)notification{

    [MTTUtil loginOut];
    
    [self.navigationController popToRootViewControllerAnimated:YES];
}

-(void)kickOffUserNotification:(NSNotification*)notification
{
    DDClientState* clientState = [DDClientState shareInstance];
    clientState.userState = DDUserKickout;
    [[NSUserDefaults standardUserDefaults] setObject:@(false) forKey:@"autologin"];
    [MTTUtil loginOut];
    [self.navigationController popToRootViewControllerAnimated:YES];
    UIAlertView *alert =[UIAlertView alertWithTitle:@"注意" message:@"你的账号在其他设备登陆了" buttonIndex:^(NSInteger index){
    } cancelButtonTitle:@"确定" otherButtonTitles:nil];
    [alert show];
}

-(void)signChangeNotification:(NSNotification*)notification
{
    NSString *sign = [[notification object] objectForKey:@"sign"];
    NSString* uid = [[notification object] objectForKey:@"uid"];
    NSString* sessionId = [MTTUserEntity pbUserIdToLocalID:[uid integerValue]];
    [[DDUserModule shareInstance] getUserForUserID:sessionId Block:^(MTTUserEntity *user) {
        user.signature = sign;
    }];
    
}

#pragma mark - UITabBarDelegate

- (void)tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item
{
//    [tabBar.items indexOfObject:item]
    
    if (item.tag ==100)
    {
        self.clickCount=self.clickCount+1;
        if (self.clickCount==2) {
            if ([[[RecentUsersViewController shareInstance].tableView visibleCells] count] > 0)
            {
                __block BOOL allStop = NO;
                [[RecentUsersViewController shareInstance].items enumerateObjectsUsingBlock:^(MTTSessionEntity *obj, NSUInteger idx, BOOL *stop) {
                    if (obj.unReadMsgCount) {
                        [[RecentUsersViewController shareInstance].tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:idx inSection:1] atScrollPosition:UITableViewScrollPositionTop animated:YES];
                        *stop = YES;
                        allStop = YES;
                    }
                }];
                if(!allStop){
                    [[RecentUsersViewController shareInstance].tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:1] atScrollPosition:UITableViewScrollPositionTop animated:YES];
                }
            }
            self.clickCount=0;
        }
        
    }else{
        self.clickCount=0;
    }
    
}

#pragma mark - memory manage

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
    
//    self.navigationController.navigationBarHidden
}


-(void)dealloc{

    self.navigationController.interactivePopGestureRecognizer.delegate =nil;
}

@end
