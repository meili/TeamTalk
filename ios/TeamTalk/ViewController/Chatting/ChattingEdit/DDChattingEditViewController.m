//
//  DDChattingEditViewController.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-17.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "DDChattingEditViewController.h"
#import "ChattingEditModule.h"
#import "DDUserModule.h"
#import "DDCreateGroupAPI.h"
#import "RuntimeStatus.h"
#import "DDGroupModule.h"
#import "EditGroupViewController.h"
#import "PublicProfileViewControll.h"
#import "DDPersonEditCollectionCell.h"
#import "ShieldGroupMessageAPI.h"

#import "DDDeleteMemberFromGroupAPI.h"
#import "MBProgressHUD.h"
#import "MTTDatabaseUtil.h"
#import "MTTGroupInfoCell.h"
#import "MTTNotification.h"
#import <Masonry/Masonry.h>

@interface DDChattingEditViewController ()
@property(nonatomic,strong)ChattingEditModule *model;
@property(nonatomic,strong)NSMutableArray *temp;
@property(nonatomic,strong) MTTUserEntity *edit;
@property(nonatomic,strong) MTTUserEntity *delete;
@property(strong)NSMutableArray *willDeleteItems;
@property(strong)MBProgressHUD *hud;
@property(strong)UISwitch *shieldingOn;
@property(strong)UISwitch *topOn;
@property(assign)BOOL isShowEdit;


@property(strong)UIButton *btn;

@end

@implementation DDChattingEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title=@"聊天详情";
    self.items = [NSMutableArray new];
    self.willDeleteItems = [NSMutableArray new];
    self.groupName=@"";
    
    self.edit = [MTTUserEntity new];
    self.edit.avatar=@"tt_group_manager_add_user";
    self.edit.position=@"99999";
    self.edit.nick=@"";
    
    self.delete = [MTTUserEntity new];
    self.delete.avatar =@"tt_group_manager_delete_user";
    self.delete.position=@"00000";
    self.delete.nick=@"";
    [self loadGroupUsers];
    // 固定群 不能编辑
    // 普通可以加,自建可以编辑
    // 个人只可以加.
    if(self.session.isGroup){
        if (self.group.groupType == GROUP_TYPE_FIXED) {
            self.temp = [[NSMutableArray alloc]init];
        }else{
            if ([self.group.groupCreatorId isEqual:TheRuntime.user.objID]){
                self.temp = [NSMutableArray arrayWithArray:@[self.edit,self.delete]];
            }else{
                self.temp = [NSMutableArray arrayWithArray:@[self.edit]];
            }
        }
    }else{
        self.temp = [NSMutableArray arrayWithArray:@[self.edit]];
    }
    
    [self.items addObjectsFromArray:self.temp];
    
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    layout.minimumInteritemSpacing = 20;
    layout.minimumLineSpacing = 10;
    layout.itemSize = CGSizeMake(65, 85);
    
    layout.sectionInset = UIEdgeInsetsMake(15, 15, 15, 15);
    
    [layout setFooterReferenceSize:CGSizeMake(SCREEN_WIDTH, 165)];
    
    self.collectionView = [[UICollectionView alloc]initWithFrame:CGRectZero collectionViewLayout:layout];
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    self.collectionView.backgroundColor = [UIColor whiteColor];
    [_collectionView registerClass:[DDPersonEditCollectionCell class] forCellWithReuseIdentifier:@"PersonCollectionCell"];
    [_collectionView registerClass:[UICollectionReusableView class] forSupplementaryViewOfKind:UICollectionElementKindSectionFooter withReuseIdentifier:@"FooterView"];
    
    [self.view addSubview:self.collectionView];
    [_collectionView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH, SCREEN_HEIGHT));
        make.top.mas_equalTo(0);
        make.left.mas_equalTo(0);
    }];
    
    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
    [self.tableView setDelegate:self];
    [self.tableView setDataSource:self];
    self.tableView.scrollEnabled = NO;
    
    [self.view setBackgroundColor:TTBG];
    [self.tableView setBackgroundColor:TTBG];
    
    self.hud = [[MBProgressHUD alloc] initWithView:self.view];
    [self.view addSubview:self.hud];
    self.hud.dimBackground = YES;
    self.hud.labelText=@"正在删除...";
    
    [self addHiddenDelete];
}

#pragma mark - tableview
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (self.session.sessionType == SessionTypeSessionTypeGroup){
        return 3;
    }else{
        return 1;
    }
}

-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 16;
}

-(CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    if(section == 1){
        return 15;
    }else{
        return 0.1;
    }
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{

    return 44;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    UITableViewCell *customCell =nil;
    
    static NSString *cellIdentifier = @"ChatEditCellIdentifier";
    MTTGroupInfoCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier ];
    if (cell == nil) {
        cell = [[MTTGroupInfoCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
    }
    
    if (self.session.sessionType == SessionTypeSessionTypeGroup){
        if (indexPath.row == 0) {
            [cell showDesc:@"群聊名称"];
            [cell showDetail:self.session.name];
            [cell setUserInteractionEnabled:NO];
        }
        if (indexPath.row == 1) {
            [cell showDesc:@"屏蔽通知"];
            [cell showSwitch];
            [cell opSwitch:self.group.isShield];
            [cell setChangeSwitch:^(BOOL on){
                [self switchIsAddShielding:on];
            }];
        }
        if (indexPath.row == 2) {
            [cell showDesc:@"置顶聊天"];
            [cell showSwitch];
            if([MTTUtil checkFixedTop:self.session.sessionID]){
                [cell opSwitch:YES];
            }else{
                [cell opSwitch:NO];
            }
            [cell setChangeSwitch:^(BOOL on){
                if(on){
                    [MTTUtil setFixedTop:self.session.sessionID];
                }else{
                    [MTTUtil removeFixedTop:self.session.sessionID];
                }
                self.session.isFixedTop = on;
                [MTTNotification postNotification:MTTNotificationSessionShieldAndFixed userInfo:nil object:nil];
            }];
        }
    }else{
        if (indexPath.row == 0) {
            [cell showDesc:@"置顶聊天"];
            [cell showSwitch];
            if([MTTUtil checkFixedTop:self.session.sessionID]){
                [cell opSwitch:YES];
            }else{
                [cell opSwitch:NO];
            }
            [cell setChangeSwitch:^(BOOL on){
                if(on){
                    [MTTUtil setFixedTop:self.session.sessionID];
                }else{
                    [MTTUtil removeFixedTop:self.session.sessionID];
                }
                self.session.isFixedTop = on;
                [MTTNotification postNotification:MTTNotificationSessionShieldAndFixed userInfo:nil object:nil];
            }];
        }
    }
    customCell =cell;
    //    }
    return customCell;
    
}

#pragma mark - collectionview

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView
{
    return 1;
}

-(NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [self.items count];
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section {
    return 5;
}

-(UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    DDPersonEditCollectionCell *cell =[collectionView dequeueReusableCellWithReuseIdentifier:@"PersonCollectionCell" forIndexPath:indexPath];
    MTTUserEntity *user = [self.items objectAtIndex:indexPath.row];
    [cell.personIcon setHidden:NO];
    if (self.isShowEdit) {
        [cell.delImg setHidden:NO];
    }else{
        [cell.delImg setHidden:YES];
    }
    cell.delImg.tag=indexPath.row;
    [cell.delImg addTarget:self action:@selector(clickDeleteUser:) forControlEvents:UIControlEventTouchUpInside];
    
    if (![user.position isEqualToString:@"99999"] && ![user.position isEqualToString:@"00000"]) {
        if ([user.objID isEqualToString:TheRuntime.user.objID]) {
            [cell.delImg setHidden:YES];
        }
        [cell setContent:user.nick AvatarImage:[user getAvatarUrl]];
    }else{
        if (self.group.groupType == GROUP_TYPE_FIXED) {
            [cell setContent:@"  " AvatarImage:@"  "];
            [cell.personIcon setHidden:YES];
            [cell setUserInteractionEnabled:NO];
            [cell.delImg setHidden:YES];
        }else
        {
            [cell setContent:user.nick AvatarImage:[user getAvatarUrl]];
            [cell.delImg setHidden:YES];
        }
    }
    
    return cell ;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    if (self.isShowEdit) {
        [self hiddenAlldelete];
        return;
    }
    MTTUserEntity *user = self.items[indexPath.row];
    if ([user.position isEqualToString:@"99999"]) {
        //添加联系人
        EditGroupViewController *newEdit = [EditGroupViewController new];
        newEdit.session=self.session;
        newEdit.group=self.group;
        newEdit.isCreat=self.group.objID?NO:YES;
        newEdit.users=self.items;
        newEdit.editControll=self;
        [self pushViewController:newEdit animated:YES];
    }
    else if ([user.position isEqualToString:@"00000"])
    {
        // 删除联系人
        if ([self.group.groupCreatorId isEqual:TheRuntime.user.objID]) {
            self.isShowEdit=YES;
            [self.collectionView reloadData];
        }
    }
    else if (user)
    {
        PublicProfileViewControll *public = [PublicProfileViewControll new];
        public.user=user;
        [self pushViewController:public animated:YES];
        
    }
}

-(UICollectionReusableView *)collectionView:(UICollectionView *)collectionView viewForSupplementaryElementOfKind:(NSString *)kind atIndexPath:(NSIndexPath *)indexPath
{
    UICollectionReusableView *reusableView =nil;
    
    if (kind ==UICollectionElementKindSectionFooter){
        UICollectionReusableView *footerV = (UICollectionReusableView *)[collectionView dequeueReusableSupplementaryViewOfKind:UICollectionElementKindSectionFooter withReuseIdentifier: @"FooterView" forIndexPath:indexPath];
        
        [footerV addSubview:_tableView];
        [_tableView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH, 800));
            make.top.mas_equalTo(0);
            make.left.mas_equalTo(0);
        }];
        reusableView = footerV;
    }
    return reusableView;
    
}

#pragma mark - data

-(void)loadGroupUsers
{
    
    if([self.items count] > 2)
    {
        [self.items removeObjectsInRange:NSMakeRange(0, [self.items count]-2)];
    }
    if (self.session.sessionType == SessionTypeSessionTypeGroup) {
        self.group = [[DDGroupModule instance] getGroupByGId:self.session.sessionID];
        self.groupName = self.group.name;
        if (!self.group)
        {
            MTTSessionEntity* session = self.session;
            [[DDGroupModule instance] getGroupInfogroupID:session.sessionID completion:^(MTTGroupEntity *group) {
                self.group =group;
                self.groupName = self.group.name;
                [self loadUserToView:self.group.groupUserIds];
            }];
        }else{
            [self loadUserToView:self.group.groupUserIds];
        }
    }else
    {
        //加载对方的头像上去
        [self loadUserToView:@[self.session.sessionID]];
    }
    [self.collectionView reloadData];
    [self.tableView reloadData];
}

-(void)loadUserToView:(NSArray *)users
{
    NSMutableArray *tmpArray = [[NSMutableArray alloc]initWithArray:users];
    // 对users排序.群主第一个!
    if (self.session.sessionType == SessionTypeSessionTypeGroup) {
        MTTGroupEntity *tmpGroup = [[DDGroupModule instance] getGroupByGId:self.session.sessionID];
        [tmpArray removeObject:tmpGroup.groupCreatorId];
        [tmpArray addObject:tmpGroup.groupCreatorId];
    }
    if ([tmpArray count] >0) {
        [tmpArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString *userID = (NSString *)obj;
            [[DDUserModule shareInstance] getUserForUserID:userID Block:^(MTTUserEntity *user) {
                if (user) {
                    [self.items insertObject:user atIndex:0];
                }
            }];
            
        }];
    }
}

-(void)addHiddenDelete
{
    UITapGestureRecognizer *hiddenDelete = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(hiddenAlldelete)];
    UIView *bgview = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.collectionView.contentSize.width, self.collectionView.contentSize.height)];
    [bgview addGestureRecognizer:hiddenDelete];
    [self.collectionView setBackgroundView:bgview];
    
}

-(void)hiddenAlldelete
{
    self.isShowEdit=NO;
    [self.collectionView reloadData];
}

-(void)refreshUsers:(NSMutableArray *)array
{
    
    [self.items removeAllObjects];
    [self.items addObjectsFromArray:array];
    [self.items addObject:self.edit];
    [self.collectionView reloadData];
    [self.tableView reloadData];
}

-(void)clickDeleteUser:(id)sender
{
    [self.hud show:YES];
    UIButton *btn = (UIButton *)sender;
    MTTUserEntity *user = [self.items objectAtIndex:btn.tag];
    DDDeleteMemberFromGroupAPI* deleteMemberAPI = [[DDDeleteMemberFromGroupAPI alloc] init];
    [deleteMemberAPI requestWithObject:@[self.session.sessionID, user.objID] Completion:^(MTTGroupEntity *response, NSError *error) {
        [self.hud hide:YES afterDelay:1];
        if (error) {
            [MTTUtil showAlertWithTitle:@" " message:error.domain?error.domain:@"未知错误"];
            return ;
        }
        if (response)
        {
            [self.items removeObject:user];
            [self.collectionView reloadData];
            self.group=response;
            [[MTTDatabaseUtil instance] updateRecentGroup:response completion:^(NSError *error) {
                
            }];
        }
    }];
}

-(void)switchIsAddShielding:(BOOL)on
{
    ShieldGroupMessageAPI *request = [ShieldGroupMessageAPI new];
    NSMutableArray *array = [NSMutableArray new];
    [array addObject:self.session.sessionID];
    if (on) {
        [array addObject:@(1)];
    }else
    {
        [array addObject:@(0)];
    }
    [request requestWithObject:array Completion:^(id response, NSError *error) {
        if (error) {
            return ;
        }
        self.group.isShield=!self.group.isShield;
        [MTTNotification postNotification:MTTNotificationSessionShieldAndFixed userInfo:nil object:nil];
        [[MTTDatabaseUtil instance] updateRecentGroup:self.group completion:^(NSError *error) {
            
        }];
    }];
    
}


-(void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    MTTUserEntity *user = [self.items lastObject];
    
    if (![user.position isEqualToString:@"99999"]) {
        [self.items removeObject:self.edit];
        [self.items addObject:self.edit];
    }
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
