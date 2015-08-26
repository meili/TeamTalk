//
//  MTTMyViewController.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-15.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"
#import "LCActionSheet.h"
@class MTTUserEntity;
@interface MTTMyViewController : MTTBaseViewController<UITableViewDataSource,UITableViewDelegate,LCActionSheetDelegate>
@property(nonatomic,weak)MTTUserEntity *user;
@property(nonatomic,strong)UITableView* tableView;
@property(assign)BOOL pushShiledStatus;

@end
