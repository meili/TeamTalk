//
//  DDRecentUsersViewController.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "RecentUsersViewController.h"
#import "RecentUserCell.h"
#import "DDUserModule.h"
#import "DDMessageModule.h"
#import "ChattingMainViewController.h"
#import "MTTSessionEntity.h"
#import "MTTDatabaseUtil.h"
#import "LoginModule.h"
#import "DDClientState.h"
#import "RuntimeStatus.h"
#import "DDUserModule.h"
#import "DDGroupModule.h"
#import "DDFixedGroupAPI.h"
#import "SearchContentViewController.h"
#import "MBProgressHUD.h"
#import "SessionModule.h"
#import "MTTLoginViewController.h"
#import "MTTPCStatusCell.h"
#import "MTTPCLoginViewController.h"
#import "MTTUsersStatAPI.h"
#import "UIImageView+WebCache.h"
#import <Masonry.h>

@interface RecentUsersViewController ()
@property(nonatomic,strong)MBProgressHUD *hud;
@property(nonatomic,strong)NSMutableDictionary *lastMsgs;
@property(nonatomic,strong)UISearchBar *searchBar;
@property(nonatomic,strong)SearchContentViewController *searchContent;
@property(nonatomic,assign)NSInteger fixedCount;
@property(nonatomic,strong)UITableView* searchTableView;
@property(nonatomic,strong)UIView* searchPlaceholderView;
@property(nonatomic,assign)BOOL isMacOnline;
- (void)n_receiveStartLoginNotification:(NSNotification*)notification;
- (void)n_receiveLoginFailureNotification:(NSNotification*)notification;
@end

@implementation RecentUsersViewController

+ (instancetype)shareInstance
{
    static RecentUsersViewController* g_recentUsersViewController;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_recentUsersViewController = [RecentUsersViewController new];
    });
    return g_recentUsersViewController;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(n_receiveLoginFailureNotification:)
                                                     name:DDNotificationUserLoginFailure
                                                   object:nil];
    }
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(n_receiveLoginNotification:)
                                                 name:DDNotificationUserLoginSuccess
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(refreshData)
                                                 name:@"RefreshRecentData"
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(n_receiveReLoginSuccessNotification)
                                                 name:@"ReloginSuccess"
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(refreshData)
                                                 name:MTTNotificationSessionShieldAndFixed
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(pcLoginNotification:)
                                                 name:DDNotificationPCLoginStatusChanged
                                               object:nil];
    
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title=@"消息";
    self.navigationItem.title=APP_NAME;
    
    self.items=[NSMutableArray new];
    [self.tableView setSeparatorStyle:UITableViewCellSeparatorStyleNone];
    [self.tableView setBackgroundColor:TTBG];
    self.searchBar = [[UISearchBar alloc] initWithFrame:CGRectMake(0, 0, FULL_WIDTH, 40)];
    self.searchBar.placeholder = @"搜索";
    self.searchBar.delegate = self;
    self.searchBar.barTintColor = TTBG;
    self.searchBar.layer.borderWidth = 0.5;
    self.searchBar.layer.borderColor = RGB(204, 204, 204).CGColor;
    self.tableView.tableHeaderView = self.searchBar;
    
    self.lastMsgs = [NSMutableDictionary new];
    self.isMacOnline = 0;
    
    /*
     [[SessionModule instance] loadLocalSession:^(bool isok) {
     
     if (isok) {
     [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
     [self sortItems];
     
     dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
     
     [[SessionModule instance] getRecentSession:^(NSUInteger count) {
     
     [self.items removeAllObjects];
     [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
     [self sortItems];
     NSUInteger unreadcount =  [[self.items valueForKeyPath:@"@sum.unReadMsgCount"] integerValue];
     
     [self setToolbarBadge:unreadcount];
     
     }];
     });
     }
     }];
     */
    [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
    [self sortItems];
    
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        [[SessionModule instance] getRecentSession:^(NSUInteger count) {
            
            [self.items removeAllObjects];
            [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
            
            [self sortItems];
            
            //            NSUInteger unreadcount =  [[self.items valueForKeyPath:@"@sum.unReadMsgCount"] integerValue];
            NSUInteger unreadcount =  [[SessionModule instance]getAllUnreadMessageCount];
            
            [self setToolbarBadge:unreadcount];
            
        }];
    });
    
    
    [SessionModule instance].delegate=self;
    
//    [self addCustomSearchControll];
    
    // 获取mac登陆状态
    [self getMacLoginStatus];
    // 初始化searchTableView
    [self addSearchTableView];
}

-(void)addSearchTableView{
    self.searchTableView = [[UITableView alloc]initWithFrame:CGRectMake(0, 105, SCREEN_WIDTH, SCREEN_HEIGHT-105)];
    [self.view addSubview:self.searchTableView];
    [self.searchTableView setHidden:YES];
    [self.searchTableView setSeparatorStyle:UITableViewCellSeparatorStyleNone];
    [self.searchTableView setBackgroundColor:TTBG];
    self.searchContent = [SearchContentViewController new];
    self.searchContent.viewController=self;
    self.searchTableView.delegate = self.searchContent.delegate;
    self.searchTableView.dataSource = self.searchContent.dataSource;
    
    __weak __typeof(self)weakSelf = self;
    self.searchContent.didScrollViewScrolled = ^(){
        [weakSelf.view endEditing:YES];
        [weakSelf enableControlsInView:weakSelf.searchBar];
    };
    
    self.searchPlaceholderView = [[UIView alloc]initWithFrame:CGRectMake(0, 105, SCREEN_WIDTH, SCREEN_HEIGHT-105)];
    [self.view addSubview:self.searchPlaceholderView];
    [self.searchPlaceholderView setHidden:YES];
    [self.searchPlaceholderView setBackgroundColor:[UIColor whiteColor]];
    
    // 点击取消
    self.searchPlaceholderView.userInteractionEnabled = YES;
    UITapGestureRecognizer *tapGesture=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(endSearch)];
    [self.searchPlaceholderView addGestureRecognizer:tapGesture];
    
    // 添加其他元素
    UILabel *searchMore = [[UILabel alloc]initWithFrame:CGRectMake(0, 60, SCREEN_WIDTH, 20)];
    [self.searchPlaceholderView addSubview:searchMore];
    [searchMore setTextAlignment:NSTextAlignmentCenter];
    [searchMore setText:@"搜索更多内容"];
    [searchMore setFont:systemFont(20)];
    [searchMore setTextColor:RGB(129, 129, 131)];
    
    UILabel *searchMoreLine = [[UILabel alloc]initWithFrame:CGRectMake((SCREEN_WIDTH-200)/2, 95, 200, 0.5)];
    [self.searchPlaceholderView addSubview:searchMoreLine];
    [searchMoreLine setBackgroundColor:RGB(230, 230, 232)];
    
    UIView *searchMoreContent = [[UIView alloc]initWithFrame:CGRectMake((SCREEN_WIDTH-200)/2, 110, 200, 50)];
    [self.searchPlaceholderView addSubview:searchMoreContent];
    
    UIImageView *searchUser = [[UIImageView alloc]initWithFrame:CGRectMake(0, 0, 25, 25)];
    [searchUser setImage:[UIImage imageNamed:@"search_user"]];
    [searchMoreContent addSubview:searchUser];
    
    UILabel *searchUserLabel = [[UILabel alloc]initWithFrame:CGRectMake(0, 35, 25, 25)];
    [searchUserLabel setText:@"用户"];
    [searchUserLabel setTextColor:RGB(170, 170, 171)];
    [searchUserLabel setFont:systemFont(12)];
    [searchUserLabel setTextAlignment:NSTextAlignmentCenter];
    [searchMoreContent addSubview:searchUserLabel];
    
    UIImageView *searchGroup = [[UIImageView alloc]initWithFrame:CGRectMake(25+33, 0, 25, 25)];
    [searchGroup setImage:[UIImage imageNamed:@"search_group"]];
    [searchMoreContent addSubview:searchGroup];
    
    UILabel *searchGroupLabel = [[UILabel alloc]initWithFrame:CGRectMake(25+33, 35, 25, 25)];
    [searchGroupLabel setText:@"群组"];
    [searchGroupLabel setTextColor:RGB(170, 170, 171)];
    [searchGroupLabel setFont:systemFont(12)];
    [searchGroupLabel setTextAlignment:NSTextAlignmentCenter];
    [searchMoreContent addSubview:searchGroupLabel];
    
    UIImageView *searchDepartment = [[UIImageView alloc]initWithFrame:CGRectMake((25+33)*2, 0, 25, 25)];
    [searchDepartment setImage:[UIImage imageNamed:@"search_department"]];
    [searchMoreContent addSubview:searchDepartment];
    
    UILabel *searchDepartmentLabel = [[UILabel alloc]initWithFrame:CGRectMake((25+33)*2, 35, 25, 25)];
    [searchDepartmentLabel setText:@"部门"];
    [searchDepartmentLabel setTextColor:RGB(170, 170, 171)];
    [searchDepartmentLabel setFont:systemFont(12)];
    [searchDepartmentLabel setTextAlignment:NSTextAlignmentCenter];
    [searchMoreContent addSubview:searchDepartmentLabel];
    
    UIImageView *searchChat = [[UIImageView alloc]initWithFrame:CGRectMake((25+33)*3, 0, 25, 25)];
    [searchChat setImage:[UIImage imageNamed:@"search_chat"]];
    [searchMoreContent addSubview:searchChat];
    
    UILabel *searchChatLabel = [[UILabel alloc]initWithFrame:CGRectMake((25+33)*3, 35, 25, 25)];
    [searchChatLabel setText:@"聊天"];
    [searchChatLabel setTextColor:RGB(170, 170, 171)];
    [searchChatLabel setFont:systemFont(12)];
    [searchChatLabel setTextAlignment:NSTextAlignmentCenter];
    [searchMoreContent addSubview:searchChatLabel];
}

- (void)enableControlsInView:(UIView *)view
{
    for (id subview in view.subviews) {
        if ([subview isKindOfClass:[UIControl class]]) {
            [subview setEnabled:YES];
        }
        [self enableControlsInView:subview];
    }
}

-(void)getMacLoginStatus{
    MTTUsersStatAPI *request = [MTTUsersStatAPI new];
    NSMutableArray *array = [NSMutableArray new];
    UInt32 uid = [MTTUserEntity localIDTopb:TheRuntime.user.objID];
    [array addObject:@(uid)];
    [request requestWithObject:array Completion:^(NSArray *response, NSError *error) {
        if(response){
            NSMutableArray *narray = [NSMutableArray arrayWithArray:response];
            if([narray[0][1] intValue]== UserStatTypeUserStatusOnline){
                self.isMacOnline = 1;
            }
            [self.tableView reloadData];
        }
    }];
}

-(void)sortItems
{
    [self.items removeAllObjects];
    [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
    NSSortDescriptor *sortDescriptor = [[NSSortDescriptor alloc] initWithKey:@"timeInterval" ascending:NO];
    NSSortDescriptor *sortFixed = [[NSSortDescriptor alloc] initWithKey:@"isFixedTop" ascending:NO];
    [self.items sortUsingDescriptors:[NSArray arrayWithObject:sortDescriptor]];
    [self.items sortUsingDescriptors:[NSArray arrayWithObject:sortFixed]];
    [self.tableView reloadData];
}

-(void)refreshData
{
    [self setToolbarBadge:0];
    [self sortItems];
}

-(void)setToolbarBadge:(NSUInteger)count
{
    
    if (count !=0) {
        if (count > 99)
        {
            [self.tabBarItem setBadgeValue:@"99+"];
        }
        else
        {
            [self.tabBarItem setBadgeValue:[NSString stringWithFormat:@"%ld",count]];
        }
    }else
    {
        [[UIApplication sharedApplication] setApplicationIconBadgeNumber:0];
        [self.tabBarItem setBadgeValue:nil];
    }
}


-(void)searchContact
{
    
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    NSUInteger count = [[SessionModule instance]getAllUnreadMessageCount];
    [self setToolbarBadge:count];
    [self.tableView reloadData];
    self.title=APP_NAME;
}


- (void)viewDidAppear:(BOOL)animated{
    
    [super viewDidAppear:animated];
    [ChattingMainViewController shareInstance].module.MTTSessionEntity=nil;
    self.tableView.contentInset =UIEdgeInsetsMake(64, 0, 49, 0);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - UITableView DataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if(section == 0){
        return 1;
    }else{
        return [self.items count];
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CGFloat height;
    if(indexPath.section == 0){
        if(self.isMacOnline){
            height = 45;
        }else{
            height = 0;
        }
    }else{
        height = 72;
    }
    return height;
}


- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(indexPath.section == 0){
        static NSString* cellIdentifier = @"MTTPCStatusCellIdentifier";
        MTTPCStatusCell* cell = (MTTPCStatusCell*)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
        if (!cell)
        {
            cell = [[MTTPCStatusCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
        }
        if(!self.isMacOnline){
            [cell setHidden:YES];
        }
        return cell;
    }else{
        static NSString* cellIdentifier = @"MTTRecentUserCellIdentifier";
        RecentUserCell* cell = (RecentUserCell*)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
        if (!cell)
        {
            cell = [[RecentUserCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
        }
        NSInteger row = [indexPath row];
        UIView *view = [[UIView alloc] initWithFrame:cell.bounds];
        view.backgroundColor=RGB(229, 229, 229);
        MTTSessionEntity *session = self.items[row];
        if(session.isFixedTop){
            [cell setBackgroundColor:RGB(243, 243, 247)];
        }else{
            [cell setBackgroundColor:[UIColor whiteColor]];
        }
        view.backgroundColor=RGB(229, 229, 229);
        cell.selectedBackgroundView=view;
        [cell setShowSession:self.items[row]];
        [self preLoadMessage:self.items[row]];
        return cell;
    }
    
}
-(void)sessionUpdate:(MTTSessionEntity *)session Action:(SessionAction)action
{
    if (![self.items containsObject:session]) {
        [self.items insertObject:session atIndex:0];
    }
    [self sortItems];
    [self.tableView reloadData];
    //    NSUInteger count =  [[self.items valueForKeyPath:@"@sum.unReadMsgCount"] integerValue];
    NSUInteger count = [[SessionModule instance]getAllUnreadMessageCount];
    [self setToolbarBadge:count];
}
#pragma mark - UITableView Delegate
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(indexPath.section == 0){
        MTTPCLoginViewController *pcLogin = [MTTPCLoginViewController new];
        [self presentViewController:pcLogin animated:YES completion:^{
            
        }];
    }else{
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        NSInteger row = [indexPath row];
        MTTSessionEntity *session = self.items[row];
        [ChattingMainViewController shareInstance].title=session.name;
        [[ChattingMainViewController shareInstance] showChattingContentForSession:session];
        [self pushViewController:[ChattingMainViewController shareInstance] animated:YES];
    }
}


-(void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger row = [indexPath row];
    MTTSessionEntity *session = self.items[row];
    [[SessionModule instance] removeSessionByServer:session];
    [self.items removeObjectAtIndex:row];
    [self setToolbarBadge:[[SessionModule instance]getAllUnreadMessageCount]];
    [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationRight];
}
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath{
    return YES;
}

- (NSString *)tableView:(UITableView *)tableView titleForDeleteConfirmationButtonForRowAtIndexPath:(NSIndexPath *)indexPath{
    return @"删除";
}

#pragma mark - UISearchBarDelegate

- (BOOL)searchBarShouldBeginEditing:(UISearchBar *)searchBar
{
    NSString* nick = [RuntimeStatus instance].user.nick;
    NSDictionary *dict = @{@"nick" : nick};
    self.searchContent.searchKey = searchBar.text;
    // 原tableview不允许滚动
    self.tableView.scrollEnabled = NO;
    if(searchBar.text.length){
        [self enableControlsInView:searchBar];
        // 显示空白view
        [self.searchPlaceholderView setHidden:YES];
        // 隐藏tabbar
        self.tabBarController.tabBar.hidden = YES;
    }else{
        // 显示取消按钮
        [self.searchBar setShowsCancelButton:YES animated:YES];
        // 显示空白view
        [self.searchPlaceholderView setHidden:NO];
        // 隐藏tabbar
        self.tabBarController.tabBar.hidden = YES;
    }
    return YES;
}

- (BOOL)searchBarShouldEndEditing:(UISearchBar *)searchBar{
    return YES;
}

- (void)endSearch{
    // 原tableview允许滚动
    self.tableView.scrollEnabled = YES;
    self.searchBar.text = @"";
    [self.searchPlaceholderView setHidden:YES];
    [self.searchTableView setHidden:YES];
    // 显示tabbar
    self.tabBarController.tabBar.hidden = NO;
    [self.searchBar resignFirstResponder];
    [self.searchBar setShowsCancelButton:NO animated:YES];
}

- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar{
    [self endSearch];
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText
{
    if(searchText.length){
        [self.searchPlaceholderView setHidden:YES];
        [self.searchTableView setHidden:NO];
        __weak RecentUsersViewController *weakSelf =self;
        
        [self.searchContent searchTextDidChanged:searchText Block:^(bool done) {
            [weakSelf.searchTableView reloadData];
        }];
    }else{
        [self.searchPlaceholderView setHidden:NO];
        [self.searchTableView setHidden:YES];
    }
}

#pragma mark -  SNotification

- (void)n_receiveLoginFailureNotification:(NSNotification*)notification
{
    self.title = @"未连接";
}

- (void)n_receiveStartLoginNotification:(NSNotification*)notification
{
    self.title = APP_NAME;
}

- (void)n_receiveLoginNotification:(NSNotification*)notification
{
    self.title = APP_NAME;
}

-(void)n_receiveReLoginSuccessNotification
{
    self.title = APP_NAME;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [[SessionModule instance] getRecentSession:^(NSUInteger count) {
            
            [self.items removeAllObjects];
            [self.items addObjectsFromArray:[[SessionModule instance] getAllSessions]];
            [self sortItems];
            [self setToolbarBadge:count];
            
        }];
    });
}
-(void)preLoadMessage:(MTTSessionEntity *)session
{
    [[MTTDatabaseUtil instance] getLastestMessageForSessionID:session.sessionID completion:^(MTTMessageEntity *message, NSError *error) {
        if (message) {
            if (message.msgID != session.lastMsgID ) {
                [[DDMessageModule shareInstance] getMessageFromServer:session.lastMsgID currentSession:session count:20 Block:^(NSMutableArray *array, NSError *error) {
                    [[MTTDatabaseUtil instance] insertMessages:array success:^{
                        
                    } failure:^(NSString *errorDescripe) {
                        
                    }];
                }];
            }
        }else{
            if (session.lastMsgID !=0) {
                [[DDMessageModule shareInstance] getMessageFromServer:session.lastMsgID currentSession:session count:20 Block:^(NSMutableArray *array, NSError *error) {
                    [[MTTDatabaseUtil instance] insertMessages:array success:^{
                        
                    } failure:^(NSString *errorDescripe) {
                        
                    }];
                }];
            }
        }
    }];
}
-(void)pcLoginNotification:(NSNotification*)notification
{
    if([[[notification object]objectForKey: @"loginStat"] intValue]== UserStatTypeUserStatusOffline){
        self.isMacOnline = 0;
    }else{
        self.isMacOnline = 1;
    }
    [self.tableView reloadData];
}
@end
