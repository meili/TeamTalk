//
//  DDAblumViewController.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-6-4.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"
#import <AssetsLibrary/AssetsLibrary.h>
@interface AlbumViewController : MTTBaseViewController<UITableViewDataSource,UITableViewDelegate>
@property(nonatomic,strong) ALAssetsLibrary * assetsLibrary;
@property(nonatomic,strong) NSMutableArray *albumsArray;
@end
