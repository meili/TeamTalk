//
//  MTTSearchDetailViewControll.m
//  TeamTalk
//
//  Created by scorpio on 15/7/12.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTSearchDetailViewControll.h"
#import "MTTSearchResultCell.h"
#import "DDUserModule.h"
#import "MTTDatabaseUtil.h"
#import "DDGroupModule.h"
#import "MTTMessageEntity.h"
#import "MTTUserEntity.h"
#import "ChattingMainViewController.h"
#import <Masonry.h>

@implementation MTTSearchDetailViewControll
#pragma mark - 初始化

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    MTT_WEAKSELF(ws);
    self.title = self.session.name;
    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStylePlain];
    self.tableView.separatorStyle = 0;
    [self.tableView setDelegate:self];
    [self.tableView setDataSource:self];
    [self.view addSubview:self.tableView];
    [self.view setBackgroundColor:TTBG];
    [self.tableView setBackgroundColor:TTBG];
    [self.tableView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.equalTo(ws.view);
        make.center.equalTo(ws.view);
    }];
    [[MTTDatabaseUtil instance] searchHistoryBySessionId:self.searchKey sessionId:self.session.sessionID completion:^(NSArray *messages, NSError *error) {
        self.searchResult = messages;
        [self.tableView reloadData];
    }];
}

#pragma mark - UITableViewDelegate
- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 40;
}

-(UIView*)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    UIView *headView = [[UIView alloc]init];
    [headView setBackgroundColor:TTBG];
    UILabel *label = [UILabel new];
    [headView addSubview:label];
    [label setText:[NSString stringWithFormat:@"共%li条与\"%@\"相关的聊天记录",(long)[self.searchResult count],self.searchKey]];
    [label setFont:systemFont(15)];
    [label setTextColor:TTGRAY];
    [label mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.mas_equalTo(15);
        make.centerY.equalTo(headView).offset(5);
        make.right.mas_equalTo(15);
        make.height.mas_equalTo(15);
    }];
    return headView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 55;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.searchResult count];
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *resultCellIdentifier = @"resultCell";
    MTTSearchResultCell *resultCell = [tableView dequeueReusableCellWithIdentifier:resultCellIdentifier];
    if(resultCell == nil){
        resultCell = [[MTTSearchResultCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:resultCellIdentifier];
    }
    [resultCell setSelectionStyle:UITableViewCellSelectionStyleNone];
    if(self.session.sessionType == MsgTypeMsgTypeSingleText){
        [[DDUserModule shareInstance] getUserForUserID:self.session.sessionID Block:^(MTTUserEntity *user) {
            MTTMessageEntity *msg = [self.searchResult objectAtIndex:indexPath.row];
            [resultCell setCellContent:[user getAvatarUrl] title:user.nick detail:msg.msgContent key:self.searchKey];
        }];
    }else{
        MTTGroupEntity *group = [[DDGroupModule instance] getGroupByGId:self.session.sessionID];
        MTTMessageEntity *msg = [self.searchResult objectAtIndex:indexPath.row];
        [resultCell setCellContent:@"" title:group.name detail:msg.msgContent key:self.searchKey];
        [resultCell setGroupAvatar:group];
    }
    return resultCell;
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    MTTMessageEntity *msg = [self.searchResult objectAtIndex:indexPath.row];
    [[ChattingMainViewController shareInstance] loadChattingContentFromSearch:self.session message:msg];
    [self pushViewController:[ChattingMainViewController shareInstance] animated:YES];
}

@end
