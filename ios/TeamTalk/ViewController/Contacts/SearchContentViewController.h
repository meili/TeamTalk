//
//  SearchContentViewController.h
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTBaseViewController.h"

typedef  void (^DidScrollViewScrolled)();

@interface SearchContentViewController : UITableView<UISearchBarDelegate,UITableViewDelegate,UITableViewDataSource,UIScrollViewDelegate>
typedef NS_ENUM(NSUInteger, MTTSearchType)
{
    MTTSearchUser           = 0,
    MTTSearchGroup          = 1,
    MTTSearchDepartment     = 2,
    MTTSearchChatHistory    = 3,
    MTTSearchAll            = 10
};
-(void)searchTextDidChanged:(NSString *)searchText Block:(void(^)(bool done)) block;
@property(nonatomic,strong)MTTBaseViewController *viewController;
@property(nonatomic)NSInteger searchType;
@property(nonatomic,strong)NSString *searchKey;
@property(nonatomic,strong)DidScrollViewScrolled didScrollViewScrolled;
// @功能搜索
@property(nonatomic,assign)BOOL isFromAt;

@property (nonatomic,copy) void(^selectUser)(MTTUserEntity *userEntity);
@end
