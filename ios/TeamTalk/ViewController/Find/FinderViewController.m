//
//  FinderViewController.m
//  TeamTalk
//
//  Created by 独嘉 on 14-10-22.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "FinderViewController.h"
#import "MTTWebViewController.h"
#import "ScanQRCodePage.h"
#import "RuntimeStatus.h"
#import "MTTUserEntity.h"
#import "AFHTTPRequestOperationManager.h"
#import "MTTURLProtocal.h"
#import <SVWebViewController.h>
#import <Masonry/Masonry.h>

@interface FinderViewController ()
@property(strong)NSURLConnection *connection;
@property(assign)NSInteger hadUpdate;
@property(copy)NSString *url;
@end

@implementation FinderViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    MTT_WEAKSELF(ws);
    
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
}

-(void)viewWillAppear:(BOOL)animated{
    
    [super viewWillAppear:animated];
    
//    self.tableView.contentInset =UIEdgeInsetsMake(0, 0, 0, 0);
    
    self.title = @"发现";
}

-(void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    self.tableView.contentInset =UIEdgeInsetsMake(64, 0, 49, 0);
}

-(void)viewDidDisappear:(BOOL)animated{
    
    [super viewDidDisappear:animated];
    
    self.tableView.contentInset =UIEdgeInsetsMake(0,0,0,0);
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark -
#pragma mark UITableViewDataSource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 2;
}

-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 16;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 44;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString* identifier = @"FinderCellIdentifier";
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
    }
    cell.selectedBackgroundView = [[UIView alloc] initWithFrame:cell.frame];
    cell.selectedBackgroundView.backgroundColor = RGB(244, 245, 246);
    NSInteger row = [indexPath row];
    if (row == 0)
    {
        [cell.textLabel setText:@"TT开源"];
    }
    else if(row == 1)
    {
        [cell.textLabel setText:@"扫一扫"];
    }
    else if (row == 2)
    {
        [cell.textLabel setText:@"街利贷"];
    }
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    return cell;
}

#pragma mark -
#pragma mark UITableViewDelegate
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    NSInteger row = indexPath.row;
    
    UIViewController *viewController =nil;
    
    if (row == 0)
    {
        SVWebViewController *webViewController = [[SVWebViewController alloc] initWithURL:[NSURL URLWithString:@"http://tt.mogu.io/home/introduce?type=mobile"]];
        [self.navigationController pushViewController:webViewController animated:YES];
    }else if (row == 1)
    {
        NSString* nick = [RuntimeStatus instance].user.nick;
        NSDictionary *dict = @{@"nick" : nick};
        viewController =[ScanQRCodePage new];
    }
    
    if (viewController) {
        [self pushViewController:viewController animated:YES];
    }
}

@end
