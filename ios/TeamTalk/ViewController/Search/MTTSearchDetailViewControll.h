//
//  MTTSearchDetailViewControll.h
//  TeamTalk
//
//  Created by scorpio on 15/7/12.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTPullScrollViewController.h"
#import "MTTSessionEntity.h"

@interface MTTSearchDetailViewControll : MTTPullScrollViewController<UITableViewDelegate,UITableViewDataSource>
@property(nonatomic,strong)MTTSessionEntity* session;
@property(nonatomic,strong)UITableView* tableView;
@property(nonatomic,strong)NSString* searchKey;
@property(nonatomic,strong)NSArray* searchResult;
@end
