//
//  MTTDetailSearchViewControll.m
//  TeamTalk
//
//  Created by scorpio on 15/7/8.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTDetailSearchViewControll.h"
#import "MTTSearchResultCell.h"
#import <Masonry.h>
#import "SearchContentViewController.h"

@interface MTTDetailSearchViewControll()
@property(nonatomic,strong)UITableView* searchTableView;
@property(nonatomic,strong)UIView* searchPlaceholderView;
@end
@implementation MTTDetailSearchViewControll

#pragma mark - 初始化

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.hasBack = 0;
//    UIView *top = [UIView new];
//    [self.view addSubview:top];
//    [top mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.top.mas_equalTo(STATUSBAR_HEIGHT);
//        make.left.mas_equalTo(0);
//        make.right.mas_equalTo(0);
//        make.height.mas_equalTo(44);
//    }];
    
//    UIView *back = [UIView new];
//    back.userInteractionEnabled = YES;
//    UITapGestureRecognizer *tap=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(goBack:)];
//    [back addGestureRecognizer:tap];
//    [self.view addSubview:back];
//    [back setBackgroundColor:RGB(236, 236, 238)];
//    [back mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.top.mas_equalTo(-STATUSBAR_HEIGHT);
//        make.left.mas_equalTo(0);
//        make.width.mas_equalTo(40);
//        make.height.mas_equalTo(44+STATUSBAR_HEIGHT*2);
//    }];
    
//    UILabel *backBtnTopLine = [UILabel new];
//    [backBtnTopLine setBackgroundColor:RGB(100, 100, 102)];
//    [back addSubview:backBtnTopLine];
//    [backBtnTopLine mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.bottom.mas_equalTo(0);
//        make.left.mas_equalTo(0);
//        make.width.mas_equalTo(40);
//        make.height.mas_equalTo(0.5);
//    }];
    
//    UIImageView *backBtn = [UIImageView new];
//    [backBtn setImage:[UIImage imageNamed:@"search_back"]];
//    [back addSubview:backBtn];
//    [backBtn mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.centerX.equalTo(back);
//        make.size.mas_equalTo(CGSizeMake(10.5, 18));
//        make.centerY.equalTo(back).offset(STATUSBAR_HEIGHT);
//    }];
    
    UISearchBar *searchBar = [UISearchBar new];
    [self.view addSubview:searchBar];
    [searchBar mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.mas_equalTo(64);
        make.left.mas_equalTo(0);
        make.right.mas_equalTo(0);
        make.height.mas_equalTo(44);
    }];
    switch (self.searchType) {
        case MTTSearchAll:
            searchBar.placeholder = @"搜索";
            self.title = @"搜索";
            break;
        case MTTSearchUser:
            searchBar.placeholder = @"搜索用户";
            self.title = @"搜索用户";
            break;
        case MTTSearchDepartment:
            searchBar.placeholder = @"搜索部门";
            self.title = @"搜索部门";
            break;
        case MTTSearchGroup:
            searchBar.placeholder = @"搜索群组";
            self.title = @"搜索群组";
            break;
        case MTTSearchChatHistory:
            searchBar.placeholder = @"搜索聊天记录";
            self.title = @"搜索聊天记录";
            break;
        default:
            searchBar.placeholder = @"搜索";
            self.title = @"搜索";
            break;
    }
    searchBar.text = self.searchKey;
    searchBar.delegate = self;
    searchBar.barTintColor = TTBG;
    searchBar.layer.borderWidth = 0.5;
    searchBar.layer.borderColor = RGB(204, 204, 204).CGColor;
    [searchBar becomeFirstResponder];
    [self addSearchTableView];
    MTT_WEAKSELF(ws);
    self.searchContent.didScrollViewScrolled = ^(){
        [ws.view endEditing:YES];
        [ws enableControlsInView:searchBar];
    };
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

-(void)addSearchTableView{
    self.searchTableView = [[UITableView alloc]initWithFrame:CGRectMake(0, 109, SCREEN_WIDTH, SCREEN_HEIGHT-109)];
    [self.view addSubview:self.searchTableView];
    [self.searchTableView setHidden:YES];
    [self.searchTableView setSeparatorStyle:UITableViewCellSeparatorStyleNone];
    [self.searchTableView setBackgroundColor:TTBG];
    self.searchContent = [SearchContentViewController new];
    self.searchContent.viewController=self;
    self.searchContent.searchType = self.searchType;
    self.searchContent.searchKey = self.searchKey;
    self.searchTableView.delegate = self.searchContent.delegate;
    self.searchTableView.dataSource = self.searchContent.dataSource;
    
    self.searchPlaceholderView = [[UIView alloc]initWithFrame:CGRectMake(0, 105, SCREEN_WIDTH, SCREEN_HEIGHT-105)];
    [self.view addSubview:self.searchPlaceholderView];
    [self.searchPlaceholderView setHidden:YES];
    [self.searchPlaceholderView setBackgroundColor:TTBG];
    
    [self.searchContent searchTextDidChanged:self.searchKey Block:^(bool done) {
        [self.searchTableView setHidden:NO];
        [self.searchPlaceholderView setHidden:YES];
        [self.searchTableView reloadData];
    }];
}

- (void)viewDidAppear:(BOOL)animated
{
    MTT_WEAKSELF(ws);
    self.searchContent.searchType = self.searchType;
    [self.searchContent searchTextDidChanged:self.searchKey Block:^(bool done) {
        [ws.searchTableView reloadData];
    }];
}

#pragma mark - UISearchBarDelegate

- (BOOL)searchBarShouldBeginEditing:(UISearchBar *)searchBar
{
    self.searchContent.searchKey = searchBar.text;
    if(searchBar.text.length){
        [self.searchPlaceholderView setHidden:YES];
    }else{
        // 显示取消按钮
        [searchBar setShowsCancelButton:YES animated:YES];
        // 显示空白view
        [self.searchPlaceholderView setHidden:NO];
    }
    return YES;
}

- (BOOL)searchBarShouldEndEditing:(UISearchBar *)searchBar{
    return YES;
}

-(void)searchDisplayControllerDidEndSearch:(UISearchDisplayController *)controller
{
}

- (void)searchDisplayControllerWillBeginSearch:(UISearchDisplayController *)controller {
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText
{
    if(searchText.length){
        [self.searchPlaceholderView setHidden:YES];
        [self.searchTableView setHidden:NO];
        MTT_WEAKSELF(ws)
        
        [self.searchContent searchTextDidChanged:searchText Block:^(bool done) {
            [ws.searchTableView reloadData];
        }];
    }else{
        [self.searchPlaceholderView setHidden:NO];
        [self.searchTableView setHidden:YES];
    }

}

// 取消按钮被按下时，执行的方法
- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar{
    if(self.hasBack == 0){
        [self.navigationController popViewControllerAnimated:YES];
        self.hasBack = 1;
    }
}

- (void)searchDisplayControllerWillEndSearch:(UISearchDisplayController *)controller{
    if(self.hasBack == 0){
        [self.navigationController popViewControllerAnimated:YES];
        self.hasBack = 1;
    }
}

- (void)goBack:(UITapGestureRecognizer *)gesture
{
    if(self.hasBack == 0){
        [self.navigationController popViewControllerAnimated:YES];
        self.hasBack = 1;
    }
}
@end
