//
//  FinderViewController.h
//  TeamTalk
//
//  Created by 独嘉 on 14-10-22.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"

@interface FinderViewController : MTTBaseViewController<UITableViewDataSource,UITableViewDelegate,UIWebViewDelegate,NSURLConnectionDelegate>
@property(nonatomic,strong)UITableView* tableView;
@end
