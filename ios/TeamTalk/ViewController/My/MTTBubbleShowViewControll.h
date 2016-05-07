//
//  BubbleShowViewControll.h
//  TeamTalk
//
//  Created by scorpio on 15/7/3.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBaseViewController.h"

@interface MTTBubbleShowViewControll : MTTBaseViewController<UICollectionViewDataSource,UICollectionViewDelegate>
@property (nonatomic, strong) UICollectionView *collectionView;
@property (nonatomic, strong) NSDictionary *bubbles;
@property (nonatomic, strong) NSMutableArray *bubbleKeys;
@property (nonatomic, strong) UIView *headerView;
@property (nonatomic, strong) UILabel *leftBottomLine;
@property (nonatomic, strong) UILabel *rightBottomLine;
@property (nonatomic, strong) UILabel *leftLabel;
@property (nonatomic, strong) UILabel *rightLabel;
@property (nonatomic) BOOL isMine;
@end
