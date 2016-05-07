
//
//  SearchContentViewController.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "SearchContentViewController.h"
#import "DDSearch.h"
#import "DDContactsCell.h"
#import "MTTUserEntity.h"
#import "PublicProfileViewControll.h"
#import "MTTSessionEntity.h"
#import "ContactsViewController.h"
#import "MBProgressHUD.h"
#import "ContactsModule.h"
#import "MTTDatabaseUtil.h"
#import "SpellLibrary.h"
#import "DDGroupModule.h"
#import "DDUserModule.h"
#import "SessionModule.h"
#import "ChattingMainViewController.h"
#import <Masonry/Masonry.h>
#import "MTTDatabaseUtil.h"
#import "RecentUserCell.h"
#import "MTTSearchShowMoreCell.h"
#import "MTTSearchResultCell.h"
#import "MTTAvatarImageView.h"
#import "DDUserModule.h"
#import "MTTDetailSearchViewControll.h"
#import "MTTSearchDetailViewControll.h"
NSInteger const MAX_RESULT_SHOW = 3;
NSInteger const MAX_LINE_LENGTH = 15;
@interface SearchContentViewController ()
@property(strong)NSMutableArray *groups;
@property(strong)NSString *keyString;
@property(strong)ContactsViewController *contact;
@property(strong)NSMutableArray *searchResult;
@property(strong)NSMutableArray *department;
@property(strong)NSMutableArray *chatHistory;
@end

@implementation SearchContentViewController
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.groups = [NSMutableArray new];
        self.searchResult = [NSMutableArray new];
        self.department = [NSMutableArray new];
        self.chatHistory = [NSMutableArray new];
        self.keyString=@"";
        self.searchType = MTTSearchAll;
        self.dataSource=self;
        self.delegate=self;
    }
    return self;
}


-(void)searchTextDidChanged:(NSString *)searchText Block:(void(^)(bool done)) block
{
    self.searchKey = searchText;
    [self.department removeAllObjects];
    [self.searchResult removeAllObjects];
    [self.groups removeAllObjects];
    [self.chatHistory removeAllObjects];
    if ([searchText isEqualToString:@""]) {
        [self.department removeAllObjects];
        [self.searchResult removeAllObjects];
        [self.groups removeAllObjects];
        [self.chatHistory removeAllObjects];
        block(YES);
        return ;
    }

    if(self.searchType == MTTSearchAll || self.searchType == MTTSearchDepartment){
        [[DDSearch instance] searchDepartment:searchText completion:^(NSArray *result, NSError *error) {
            if ([result count] >0) {
                [self.department removeAllObjects];
                [result enumerateObjectsUsingBlock:^(MTTUserEntity *obj, NSUInteger idx, BOOL *stop) {
                    if (![self.department containsObject:obj.department]) {
                        [self.department addObject:obj.department];
                    }
                }];
                block(YES);
            }
        }];
    }
    
    [[DDSearch instance] searchContent:searchText completion:^(NSArray *result, NSError *error) {
        self.keyString=searchText;
        
        if ([result count] >0) {
            [self.searchResult removeAllObjects];
            [self.groups removeAllObjects];
            
            [result enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if ([obj isKindOfClass:[MTTUserEntity class]]) {
                    [self.searchResult addObject:obj];
                }else if ([obj isKindOfClass:[MTTGroupEntity class]])
                {
                    [self.groups addObject:obj];
                }
            }];
            if(self.searchType == MTTSearchUser || self.searchType == MTTSearchChatHistory){
                [self.groups removeAllObjects];
            }
            if(self.searchType == MTTSearchGroup || self.searchType == MTTSearchChatHistory){
                [self.searchResult removeAllObjects];
            }
            if(self.searchType == MTTSearchDepartment){
                [self.searchResult removeAllObjects];
                [self.groups removeAllObjects];
            }
        }
        block(YES);
    }];
    
    if(self.searchType == MTTSearchAll || self.searchType == MTTSearchChatHistory){
        [[MTTDatabaseUtil instance] searchHistory:searchText completion:^(NSArray *messages, NSError *error) {
            [messages enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [self.chatHistory addObject:obj];
            }];
            block(YES);
        }];
    }
}

- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar
{
    [self.searchResult removeAllObjects];
    [self.groups removeAllObjects];
    [self.department removeAllObjects];
    [self.chatHistory removeAllObjects];
    [self reloadData];
}
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    NSInteger max = 0;
    if(self.searchType == MTTSearchAll){
        max = MAX_RESULT_SHOW;
    }else{
        max = 10000;
    }
    if (section == 0) {
        return [self.searchResult count]>max?max+1:[self.searchResult count];
    }else if(section == 1){
        return [self.groups count]>max?max+1:[self.groups count];
    }else if(section == 2){
        return [self.department count]>max?max+1:[self.department count];
    }else{
        return [self.chatHistory count]>max?max+1:[self.chatHistory count];
    }
    
}
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 4;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"contactsCell";
    static NSString *moreCellIdentifier = @"showMoreCell";
    static NSString *resultCellIdentifier = @"resultCell";
    DDContactsCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    MTTSearchShowMoreCell *moreCell = [tableView dequeueReusableCellWithIdentifier:moreCellIdentifier];
    MTTSearchResultCell *resultCell = [tableView dequeueReusableCellWithIdentifier:resultCellIdentifier];
    if (cell == nil) {
        cell = [[DDContactsCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
    }
    if(moreCell == nil){
        moreCell = [[MTTSearchShowMoreCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:moreCellIdentifier];
    }
    if(resultCell == nil){
        resultCell = [[MTTSearchResultCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:resultCellIdentifier];
    }
    MTTUserEntity *user=nil;
    NSInteger max = 0;
    if(self.searchType == MTTSearchAll){
        max = MAX_RESULT_SHOW;
    }else{
        max = 10000;
    }
    if(indexPath.row == max){
        return moreCell;
    }else{
        if (indexPath.section == 0) {
            user = [self.searchResult objectAtIndex:indexPath.row];
            [resultCell setCellContent:[user getAvatarUrl] title:user.nick detail:@"" key:self.searchKey];
        }else if(indexPath.section == 1)
        {
            MTTGroupEntity *group = [self.groups objectAtIndex:indexPath.row];
            [resultCell setCellContent:@"" title:group.name detail:@"" key:self.searchKey];
            [resultCell setGroupAvatar:group];
        }else if(indexPath.section == 2)
        {
            NSString *string = [self.department objectAtIndex:indexPath.row];
            [resultCell setCellContent:[user getAvatarUrl] title:string detail:@"" key:self.searchKey];
        }else{
            NSArray *data = [self.chatHistory objectAtIndex:indexPath.row];
            MTTMessageEntity *msg = (MTTMessageEntity*)data[1];
            NSInteger count= [data[0] integerValue];
            if(msg.msgType == MsgTypeMsgTypeSingleText){
                // 个人聊天
                [[DDUserModule shareInstance] getUserForUserID:msg.sessionId Block:^(MTTUserEntity *user) {
                    if(count == 1){
                        NSString *content = [self highlightText:msg.msgContent key:self.searchKey];
                        [resultCell setCellContent:[user getAvatarUrl] title:user.nick detail:content key:self.searchKey];
                    }else{
                        [resultCell setCellContent:[user getAvatarUrl] title:user.nick detail:[NSString stringWithFormat:@"%li条相关聊天记录",(long)count] key:self.searchKey];
                    }
                }];
            }else{
                // 群聊天
                MTTGroupEntity *group = [[DDGroupModule instance] getGroupByGId:msg.sessionId];
                if(count == 1){
                    NSString *content = [self highlightText:msg.msgContent key:self.searchKey];
                    [resultCell setCellContent:@"" title:group.name detail:content key:self.searchKey];
                }else{
                    [resultCell setCellContent:@"" title:group.name detail:[NSString stringWithFormat:@"%li条相关聊天记录",(long)count]  key:self.searchKey];
                }
                [resultCell setGroupAvatar:group];
            }
            return resultCell;
        }
        return resultCell;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section{
    if(self.searchType != MTTSearchAll){
        return 0.1;
    }
    CGFloat height;
    if (section == 0) {
        height = [self.searchResult count]?10:0;
    } else if(section == 1){
        height = [self.groups count]?10:0;
    }else if(section == 2){
        height = [self.department count]?10:0;
    }else{
        height = 0;
    }
    return height;
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section{
    UIView *footView = [[UIView alloc]init];
    [footView setBackgroundColor:TTBG];
    return footView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section{
    CGFloat height;
    NSInteger count;
    if (section == 0) {
        count = [self.searchResult count];
    }else if(section == 1){
        count = [self.groups count];
    }else if(section == 2){
        count = [self.department count];
    }else{
        count = [self.chatHistory count];
    }
    if(count){
        height = 40;
    }else{
        height = 0;
    }
    return height;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section{
    UIView *headView = [[UIView alloc]init];
    [headView setBackgroundColor:[UIColor whiteColor]];
    UILabel *title = [UILabel new];
    [headView addSubview:title];
    [title setTextColor:TTGRAY];
    [title setFont:systemFont(14)];
    [title mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.mas_equalTo(15);
        make.left.mas_equalTo(15);
        make.size.mas_equalTo(CGSizeMake(200, 16));
    }];
    UILabel *bottomLine = [UILabel new];
    [bottomLine setBackgroundColor:RGB(229, 229, 229)];
    [headView addSubview:bottomLine];
    [bottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(headView.mas_bottom).offset(-0.5);
        make.left.mas_equalTo(15);
        make.right.mas_equalTo(0);
        make.height.mas_equalTo(0.5);
    }];
    NSString *data;
    if (section == 0) {
         data = [self.searchResult count]?@"联系人":@"";
    }else if(section == 1){
        data = [self.groups count]?@"群组":@"";
    }else if(section == 2){
        data = [self.department count ]?@"部门":@"";
    }else{
        data = [self.chatHistory count ]?@"聊天记录":@"";
    }
    [title setText:data];
    return headView;
}

-(void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    if (self.didScrollViewScrolled) {
        self.didScrollViewScrolled();
    }
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{
    if(self.searchType != MTTSearchAll){
        return 55;
    }
    if(indexPath.row == MAX_RESULT_SHOW){
        return 40;
    }
    return 55;
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:NO];
    if(self.searchType == MTTSearchAll){
        if(indexPath.row == MAX_RESULT_SHOW){
            if(indexPath.section == 3){
                NSString* nick = [RuntimeStatus instance].user.nick;
                NSDictionary *dict = @{@"nick" : nick};
            }
            MTTDetailSearchViewControll *searchDetail = [MTTDetailSearchViewControll new];
            searchDetail.searchType = indexPath.section;
            searchDetail.searchKey = self.searchKey;
            [self.viewController.navigationController pushViewController:searchDetail animated:YES];
            return;
        }
    }
    if (indexPath.section == 0) {
        MTTUserEntity *user;
        user = self.searchResult[indexPath.row];
        PublicProfileViewControll *public = [PublicProfileViewControll new];
        public.user=user;
        if (self.selectUser && self.isFromAt) {
            self.selectUser(user);
//            [self.viewController.navigationController popViewControllerAnimated:YES];
            return;
        }
        if ([self.viewController isKindOfClass:[MTTBaseViewController class]]) {
            [self.viewController pushViewController:public animated:YES];
        }
        else{
            [self.viewController.navigationController pushViewController:public animated:YES];
        }
        return;
    }else if(indexPath.section == 1){
        MTTGroupEntity *group = [self.groups objectAtIndex:indexPath.row];
        MTTSessionEntity *session;
        if (![[SessionModule instance] getSessionById:group.objID]) {
            session = [[MTTSessionEntity alloc] initWithSessionID:group.objID SessionName:group.name type:SessionTypeSessionTypeGroup];
        }else{
            session = [[SessionModule instance] getSessionById:group.objID];
        }
        
        [[ChattingMainViewController shareInstance] showChattingContentForSession:session];
        [self.viewController.navigationController  pushViewController:[ChattingMainViewController shareInstance] animated:YES];
    }else if(indexPath.section == 2){
        NSString *string = [self.department objectAtIndex:indexPath.row];
        ContactsViewController *contact = [ContactsViewController new];
        contact.searchKey=string;
        contact.isFromSearch=YES;
        [self.viewController.navigationController pushViewController:contact animated:YES];
    }else if(indexPath.section == 3){
        NSArray *data = [self.chatHistory objectAtIndex:indexPath.row];
        MTTMessageEntity *msg = (MTTMessageEntity*)data[1];
        NSInteger count= [data[0] integerValue];
        if(count == 1){
            MTTSessionEntity *session = [[SessionModule instance] getSessionById:msg.sessionId];
            [[ChattingMainViewController shareInstance] loadChattingContentFromSearch:session message:msg];
            [self.viewController.navigationController pushViewController:[ChattingMainViewController shareInstance] animated:YES];
        }else{
            // 继续进入一个列表页
            MTTSearchDetailViewControll *detail = [MTTSearchDetailViewControll new];
            detail.searchKey = self.searchKey;
            MTTSessionEntity *session = [[SessionModule instance] getSessionById:msg.sessionId];
            detail.session = session;
            [self.viewController.navigationController pushViewController:detail animated:YES];
        }
    }
}

- (NSString*)highlightText:(NSString*)originText key:(NSString*)key
{
    NSRange range = [originText rangeOfString:key];
    NSInteger length = [originText length];
    if (range.location != NSNotFound) {
        if(range.location+range.length>=MAX_LINE_LENGTH){
            NSString *text;
            NSInteger tmp = MAX_LINE_LENGTH-length+range.length;
            if(tmp>0){
                text = [NSString stringWithFormat:@"...%@",[originText substringFromIndex:length-MAX_LINE_LENGTH]];
            }else{
                text = [NSString stringWithFormat:@"...%@",[originText substringFromIndex:range.location]];
            }
            return text;
        }else{
            return originText;
        }
    }
    return originText;
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar
{
}


@end
