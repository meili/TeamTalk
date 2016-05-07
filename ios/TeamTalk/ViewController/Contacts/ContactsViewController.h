//
//  ContactsViewController.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-15.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTPullScrollViewController.h"
#import "MTTSessionEntity.h"
@interface ContactsViewController : MTTPullScrollViewController<UITableViewDataSource,UITableViewDelegate,UISearchBarDelegate,UISearchDisplayDelegate,UIScrollViewDelegate>
// 搜索关键字
@property(nonatomic,strong)NSString *searchKey;
// 主页面搜索
@property(nonatomic,assign)BOOL isFromSearch;
// @功能搜索
@property(nonatomic,assign)BOOL isFromAt;

@property (nonatomic,copy) void(^selectUser)(MTTUserEntity *userEntity);

@end
