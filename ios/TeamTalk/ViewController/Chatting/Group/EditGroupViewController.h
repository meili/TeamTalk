//
//  EditGroupViewController.h
//  TeamTalk
//
//  Created by Michael Scofield on 2014-09-01.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"
#import "MTTGroupEntity.h"
#import "DDChattingEditViewController.h"
@class MTTSessionEntity;
typedef void(^RefreshBlock)(NSString *sessionID);
@interface EditGroupViewController : MTTBaseViewController<UISearchBarDelegate,UISearchDisplayDelegate,UITableViewDataSource,UITableViewDelegate,UIAlertViewDelegate>
@property(strong)NSMutableArray *users;
@property(copy)NSString *sessionID;
@property(strong)MTTSessionEntity *session;
@property(assign)BOOL isGroupCreator;
@property(assign)BOOL isCreat;
@property(weak)MTTGroupEntity *group;
@property(strong)DDChattingEditViewController *editControll;
@end
