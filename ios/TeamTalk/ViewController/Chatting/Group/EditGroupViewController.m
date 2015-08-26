//
//  EditGroupViewController.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-09-01.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "EditGroupViewController.h"
#import "EditGroupViewCell.h"
#import "EditContactsCell.h"
#import "DDAddMemberToGroupAPI.h"
#import "DDCreateGroupAPI.h"

#import "RuntimeStatus.h"
#import "DDSearch.h"
#import "ChattingMainViewController.h"
#import "MBProgressHUD.h"
#import "UIImageView+WebCache.h"
#import "DDGroupModule.h"
#import "ContactsModule.h"
#import "DDUserModule.h"
#import "DDDeleteMemberFromGroupAPI.h"
#import "NSDictionary+Safe.h"
#import "SessionModule.h"
#import "MTTDatabaseUtil.h"
#import "SpellLibrary.h"
@interface EditGroupViewController ()
@property(weak)IBOutlet UITableView *tableView;
@property(weak)IBOutlet UISearchBar *searchBar;
@property(strong)NSDictionary *items;
@property(strong)NSMutableArray *backupArray;
@property(strong)NSMutableArray *editArray;
@property(strong)NSMutableArray *searchResult;
@property(strong)UISearchDisplayController *searchController;
@property(strong)ContactsModule*model;
@property(strong)MBProgressHUD *hud;
@property(strong)NSMutableArray *addArray;
@end

@implementation EditGroupViewController

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

    if (isIOS7)
    {
        self.edgesForExtendedLayout = UIRectEdgeNone;
        self.extendedLayoutIncludesOpaqueBars = NO;
    }
    self.title=@"选择联系人";
    self.searchResult = [NSMutableArray new];
    [self.users removeLastObject];
    self.backupArray = [NSMutableArray arrayWithArray:self.users];
    self.editArray = [NSMutableArray arrayWithArray:self.users];
    self.sessionID=self.editControll.session.sessionID;
    self.items = [NSMutableDictionary new];
    self.model = [ContactsModule new];
    self.addArray = [NSMutableArray new];
    UIBarButtonItem *item = [[UIBarButtonItem alloc] initWithTitle:@"确定" style:UIBarButtonItemStylePlain target:self action:@selector(saveSelectItems)];
    [item setTitle:@"确定"];
    super.navigationItem.rightBarButtonItem=item;
   // self.searchBar.showsCancelButton=YES;
    self.searchController = [[UISearchDisplayController alloc] initWithSearchBar:self.searchBar contentsController:self];
    self.searchController.delegate=self;
    self.searchController.searchResultsDataSource=self;
    self.searchController.searchResultsDelegate=self;

    NSArray *users = [[DDUserModule shareInstance] getAllMaintanceUser];
    self.items = [NSDictionary dictionaryWithDictionary:[self sortByContactFirstLetter:users]];
    // 右侧索引颜色透明
    self.tableView.sectionIndexBackgroundColor = [UIColor clearColor];
    [self.searchBar setBarTintColor:RGB(242, 242, 244)];
    [self.searchBar becomeFirstResponder];
    [self.searchBar setSearchFieldBackgroundImage:[UIImage imageNamed:@"search_bar"] forState:UIControlStateNormal];

    if ([[SpellLibrary instance] isEmpty]) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            [[[DDUserModule shareInstance] getAllMaintanceUser] enumerateObjectsUsingBlock:^(MTTUserEntity *obj, NSUInteger idx, BOOL *stop) {
                [[SpellLibrary instance] addSpellForObject:obj];
                [[SpellLibrary instance] addDeparmentSpellForObject:obj];
            }];
            
        });
    }
    self.tableView.sectionIndexBackgroundColor = [UIColor clearColor];
    self.tableView.sectionIndexColor=RGB(102, 102, 102);
    self.tableView.separatorStyle = 0;
}
-(NSMutableDictionary *)sortByContactFirstLetter:(NSArray *)users
{
    NSMutableDictionary *dic = [NSMutableDictionary new];
    for (MTTUserEntity * user in [[DDUserModule shareInstance] getAllMaintanceUser]) {
        NSString *fl = [[user.pyname substringWithRange:NSMakeRange(0, 1)] uppercaseString];
        if ([dic safeObjectForKey:fl]) {
            NSMutableArray *arr = [dic safeObjectForKey:fl];
            [arr addObject:user];
        }else
        {
            NSMutableArray *arr = [[NSMutableArray alloc] initWithArray:@[user]];
            [dic safeSetObject:arr forKey:fl];
        }
    }
    return dic;
}
-(void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [self.tabBarController.tabBar setHidden:YES];
    [self.editArray removeAllObjects];
}
-(void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];


}
-(void)saveSelectItems
{
    if (self.isCreat) {
        [self createGroup];
    }else
    {
        if ([self.editArray count] != 0) {
            [self.editArray removeObjectsInArray:self.users];
           [self addUsersToGroup:self.editArray];
        }else{
            [MTTUtil showAlertWithTitle:@" " message:@"你没有选择要添加的联系人"];
        }
        
    }
}
-(BOOL)searchDisplayController:(UISearchDisplayController *)controller shouldReloadTableForSearchString:(NSString *)searchString {
    if ([searchString isEqualToString:@""]) {
        return NO;
    }
    [[DDSearch instance] searchContent:searchString completion:^(NSArray *result, NSError *error) {
        [self.searchResult removeAllObjects];
        [result enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            if ([obj isKindOfClass:[MTTUserEntity class]]) {
                [self.searchResult addObject:obj];
            }
        }];
        [self.self.searchDisplayController.searchResultsTableView reloadData];
    }];
    
    return YES;
}
- (void) searchDisplayControllerDidEndSearch:(UISearchDisplayController *)controller
{
    [self.tableView reloadData];
}
- (BOOL)searchBarShouldBeginEditing:(UISearchBar *)searchBar{
    if ( [[self allKeys] count] == 0) {
        return NO;
    }
 //   [self.searchBar setShowsCancelButton:YES animated:YES];
    [self.searchController setActive:YES animated:YES];

    return YES;
    
}
- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar{
    [self.searchBar resignFirstResponder];
   // [self.searchBar setShowsCancelButton:NO animated:YES];
    [self.tableView reloadData];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    if (tableView.tag == 100) {
        return [[self.items allKeys] count];
    }
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
   
    if (tableView.tag == 100) {
        NSString *keyStr = [self allKeys][(NSUInteger) (section)];
        NSArray *arr = (self.items)[keyStr];
        return [arr count];
    }
    
  return [self.searchResult count];
}
- (UIView*)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    UIView *sectionHeadView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, SCREEN_WIDTH, 22)];
    [sectionHeadView setBackgroundColor:RGB(241, 240, 246)];
    UILabel *sectionHeaderLabel = [[UILabel alloc] initWithFrame:CGRectMake(10, 0, 100, 22)];
    [sectionHeaderLabel setText:[self allKeys][section]];
    [sectionHeadView addSubview:sectionHeaderLabel];
    return sectionHeadView;
}

-(NSArray*)sectionIndexTitlesForTableView:(UITableView *)tableView{
        NSMutableArray *arr = [NSMutableArray new];
        [[self allKeys] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
//            char firstLetter = getFirstChar((NSString *)obj);
            char firstLetter =[MTTUtil getFirstChar:obj];
            [arr addObject:[NSString stringWithFormat:@"%c",firstLetter]];
        }];
        return arr;
}

-(NSArray*)allKeys{
    return [[self.items allKeys] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [obj1 compare:obj2];
    }];
}
-(CGFloat )tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 55.0;
}
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"contactsCell";
    EditContactsCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier ];
    if (cell == nil) {
        cell = [[EditContactsCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
    }
    cell.selectionStyle=UITableViewCellSelectionStyleNone;
    if (tableView.tag == 100) {
        NSString *keyStr = [[self allKeys] objectAtIndex:indexPath.section];
        NSArray *userArray =[self.items objectForKey:keyStr];
        MTTUserEntity *user = [userArray objectAtIndex:indexPath.row];
        if ([self.users containsObject:user]) {
            cell.userInteractionEnabled=NO;
        }else{
            cell.userInteractionEnabled=YES;
        }
        if ([self.editArray containsObject:user]) {
            [cell setCellToSelected:YES];
        }else
        {
            [cell setCellToSelected:NO];
        }
        [cell setCellContent:[user getAvatarUrl] Name:user.nick];
        
        return cell;
    }
    else
    {
        MTTUserEntity *user =self.searchResult[indexPath.row];
        if ([self.editArray containsObject:user]) {
           [cell setCellToSelected:YES];
        }else
        {
           [cell setCellToSelected:NO];
        }
        cell.nameLabel.text=user.nick;
        UIImage* placeholder = [UIImage imageNamed:@"user_placeholder"];
        [cell.avatar sd_setImageWithURL:[NSURL URLWithString:[user getAvatarUrl]] placeholderImage:placeholder];
        return cell;
        
    }
    
}
-(void)tableView:(UITableView *)tableView didDeselectRowAtIndexPath:(NSIndexPath *)indexPath
{
    EditContactsCell *oneCell =(EditContactsCell *) [tableView cellForRowAtIndexPath: indexPath];
    NSString *keyStr = [[self allKeys] objectAtIndex:indexPath.section];
    NSArray *userArray =[self.items objectForKey:keyStr];
    MTTUserEntity *user = [userArray objectAtIndex:indexPath.row];
    if(tableView.tag != 100)
    {
        user =self.searchResult[indexPath.row];
    }
    if ([self.editArray containsObject:user]) {
        [oneCell setCellToSelected:YES];
        
    }else
    {
        [oneCell setCellToSelected:NO];
    }
}
-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
        MTTUserEntity *user;
        NSString *keyStr = [[self allKeys] objectAtIndex:indexPath.section];
        NSArray *userArray =[self.items objectForKey:keyStr];
        user = [userArray objectAtIndex:indexPath.row];
    if(tableView.tag != 100)
    {
        user =self.searchResult[indexPath.row];
    }
     EditContactsCell *oneCell =(EditContactsCell *) [tableView cellForRowAtIndexPath: indexPath];

    if (![self.editArray containsObject:user])
    {
       [oneCell setCellToSelected:YES];
        [self.editArray addObject:user];

    }
    else
    {
        [oneCell setCellToSelected:NO];
        [self.editArray removeObject:user];

    }
    
}

-(void)addUsersToGroup:(NSMutableArray *)users
{
    DDAddMemberToGroupAPI *addMember = [[DDAddMemberToGroupAPI alloc] init];
    __block NSMutableArray *userIDs = [NSMutableArray new];

    [users enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        MTTUserEntity *user = (MTTUserEntity *)obj;
        if (user.objID) {
            [userIDs addObject:user.objID];
        }
    }];
    [addMember requestWithObject:@[self.sessionID,userIDs] Completion:^(NSMutableArray * response, NSError *error) {
        if (response != nil) {
            self.editControll.group.groupUserIds=response;
            [[MTTDatabaseUtil instance] updateRecentGroup: self.editControll.group completion:^(NSError *error) {
                DDLog(@"update group error , group id is : %@", self.editControll.group.objID);
            }];
            [self.navigationController popToViewController:self.editControll animated:YES];
            [self.editArray addObjectsFromArray:self.users];
            [self.editControll refreshUsers:self.editArray];
        }else
        {
            [self showAlert:error.domain?error.domain:@"未知错误"];
        }
    }];
}
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1) {
        UITextField *tf=[alertView textFieldAtIndex:0];
        if(tf.text.length !=0)
        {
            DDCreateGroupAPI *creatGroup = [[DDCreateGroupAPI alloc] init];
            __block NSMutableArray *userIDs = [NSMutableArray new];
            [userIDs addObject:TheRuntime.user.objID];
            [self.editArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                MTTUserEntity *user = (MTTUserEntity *)obj;
                if (user.objID) {
                    [userIDs addObject:user.objID];
                }
            }];
            NSString *groupName = tf.text.length !=0?tf.text:[self creatGroupName];
            NSArray *array =@[groupName,@"",userIDs];
            [creatGroup requestWithObject:array Completion:^(MTTGroupEntity * response, NSError *error) {
                if (response !=nil) {
                    response.groupCreatorId=TheRuntime.user.objID;
                    [[DDGroupModule instance] addGroup:response];
                    [self.editControll refreshUsers:self.editArray];
                    self.editControll.group=response;
                    self.editControll.session.sessionID=response.objID;
                    self.editControll.session.sessionType=SessionTypeSessionTypeGroup;
                    MTTSessionEntity *session = [[MTTSessionEntity alloc] initWithSessionID:response.objID type:SessionTypeSessionTypeGroup];
                    session.lastMsg=@" ";
                    [[MTTDatabaseUtil instance] updateRecentSession:session completion:^(NSError *error) {
                        
                    }];
                    [[MTTDatabaseUtil instance] updateRecentGroup:response completion:^(NSError *error) {
                        
                    }];
                    [[ChattingMainViewController shareInstance] showChattingContentForSession:session];
//                    [[ChattingMainViewController shareInstance].module.showingMessages removeAllObjects];
                    [ChattingMainViewController shareInstance].title=response.name;
                    [self.navigationController popToViewController:[ChattingMainViewController shareInstance] animated:YES];
                    [[SessionModule instance] addToSessionModel:session];
                    if ([SessionModule instance].delegate && [[SessionModule instance].delegate respondsToSelector:@selector(sessionUpdate:Action:)]) {
                        [[SessionModule instance].delegate sessionUpdate:session Action:ADD];
                    }
                     [[SpellLibrary instance] addSpellForObject:response];
                }else
                {
                    [self showAlert:error.domain?error.domain:@"未知错误"];
                }
                
                
            }];

        }
        
    }
}
-(void)createGroup
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"创建讨论组" message:nil delegate:self cancelButtonTitle:@"取消" otherButtonTitles:@"确定", nil];
    alert.alertViewStyle=UIAlertViewStylePlainTextInput;
    [alert show];
   
}

-(void)showAlert:(NSString *)string
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"" message:string delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
    [alert show];
}

-(NSString *)creatGroupName
{
    NSMutableString *string= [NSMutableString new];
    [self.editArray enumerateObjectsUsingBlock:^(MTTUserEntity *obj, NSUInteger idx, BOOL *stop) {
        [string appendFormat:@"%@,",obj.name];
        if (idx == 3) {
            *stop=YES;
        }
    }];

    return string;
}
@end
