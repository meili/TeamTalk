//
//  DDRecentUsersViewController.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTPullScrollViewController.h"
#import "SessionModule.h"
@class RecentUserVCModule;
@interface RecentUsersViewController : MTTPullScrollViewController<UITableViewDataSource,UITableViewDelegate,UIAlertViewDelegate,UISearchBarDelegate,SessionModuelDelegate,UIScrollViewDelegate>

@property(nonatomic,weak)IBOutlet UITableView* tableView;
@property(nonatomic,strong)RecentUserVCModule* module;
@property(nonatomic,strong)UIView* placeholderView;
@property(strong)NSMutableArray *items;
+ (instancetype)shareInstance;
@end
