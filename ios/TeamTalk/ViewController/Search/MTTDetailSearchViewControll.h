//
//  MTTDetailSearchViewControll.h
//  TeamTalk
//
//  Created by scorpio on 15/7/8.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTPullScrollViewController.h"
#import "SearchContentViewController.h"

@interface MTTDetailSearchViewControll : MTTPullScrollViewController<UISearchBarDelegate,UIGestureRecognizerDelegate>
@property(nonatomic,strong)SearchContentViewController *searchContent;
@property(nonatomic,strong)UISearchDisplayController * searchController;
@property(assign)NSInteger hasBack;
@property(assign)NSInteger searchType;
@property(nonatomic,strong)NSString* searchKey;
@end
