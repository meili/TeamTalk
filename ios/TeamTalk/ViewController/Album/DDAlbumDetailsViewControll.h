//
//  DDAlbumDetailsViewControllViewController.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-6-4.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "AQGridView.h"
#import "MTTBaseViewController.h"
#import <AssetsLibrary/AssetsLibrary.h>
@class AlbumDetailsBottomBar;
@interface DDAlbumDetailsViewControll : MTTBaseViewController<AQGridViewDataSource,AQGridViewDelegate>
@property(nonatomic,strong)ALAssetsGroup *assetsGroup;
@property(nonatomic,strong)NSMutableArray *assetsArray;
@property(nonatomic,strong)NSMutableArray *choosePhotosArray;
@property(nonatomic,strong)AQGridView *gridView;
@property(nonatomic,strong)AlbumDetailsBottomBar *bar;

@end
