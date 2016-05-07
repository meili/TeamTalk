//
//  BubbleShowViewControll.m
//  TeamTalk
//
//  Created by scorpio on 15/7/3.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTBubbleShowViewControll.h"
#import "MTTBubbleModule.h"
#import "MTTBubbleCollectionViewCell.h"
#import "RuntimeStatus.h"
#import "MTTUserEntity.h"
#import <Masonry/Masonry.h>

@implementation MTTBubbleShowViewControll

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title=@"个性气泡选择";
    
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    layout.minimumInteritemSpacing = 10;
    layout.minimumLineSpacing = 10;
    layout.itemSize = CGSizeMake((SCREEN_WIDTH-40)/2, (SCREEN_WIDTH-40)/2*85/140);
    layout.sectionInset = UIEdgeInsetsMake(15, 15, 15, 15);
    [layout setHeaderReferenceSize:CGSizeMake(SCREEN_WIDTH, 0)];
    
    self.collectionView = [[UICollectionView alloc]initWithFrame:CGRectZero collectionViewLayout:layout];
    [self.view addSubview:self.collectionView];
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    self.collectionView.showsHorizontalScrollIndicator = NO;
    self.collectionView.showsVerticalScrollIndicator = NO;
    self.collectionView.backgroundColor = [UIColor whiteColor];
    [_collectionView registerClass:[MTTBubbleCollectionViewCell class] forCellWithReuseIdentifier:@"BubbleShowCell"];
    [_collectionView registerClass:[UICollectionReusableView class] forSupplementaryViewOfKind:UICollectionElementKindSectionHeader withReuseIdentifier:@"HeaderView"];
    
    [_collectionView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH, SCREEN_HEIGHT-50));
        make.top.mas_equalTo(50);
        make.left.mas_equalTo(0);
    }];
    
    self.bubbles = [NSDictionary dictionaryWithObjectsAndKeys:@"default_blue",@"默认蓝",@"default_white",@"默认白",@"doraemon",@"哆啦A梦",@"mengmengda",@"萌萌哒",@"pineapple",@"新鲜菠萝",@"qjnn",@"奇迹暖暖",@"transformer",@"变形金刚",@"wave",@"浪花朵朵",@"wechat_green",@"清新绿",@"beach",@"阳光沙滩",nil];
    self.bubbleKeys = [[NSMutableArray alloc]initWithArray:[self.bubbles allKeys]];
    [self.bubbleKeys removeObject:@"默认蓝"];
    [self.bubbleKeys removeObject:@"默认白"];
    [self.bubbleKeys removeObject:@"清新绿"];
    [self.bubbleKeys insertObject:@"默认蓝" atIndex:0];
    [self.bubbleKeys insertObject:@"默认白" atIndex:1];
    [self.bubbleKeys insertObject:@"清新绿" atIndex:2];
    
    self.isMine = YES;
    NSString* nick = [RuntimeStatus instance].user.nick;
    NSDictionary *dict = @{@"nick" : nick};
    [self renderHeaderView];
}

- (void)renderHeaderView{
    _headerView = [UIView new];
    [_headerView setBackgroundColor:[UIColor whiteColor]];
    [self.view addSubview:_headerView];
    [_headerView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH, 50));
        make.top.mas_equalTo(NAVBAR_HEIGHT);
        make.left.mas_equalTo(0);
    }];
    UIView *leftView = [UIView new];
    UIView *rightView = [UIView new];
    
    [_headerView addSubview:leftView];
    [_headerView addSubview:rightView];
    [leftView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 50));
        make.top.mas_equalTo(0);
        make.left.mas_equalTo(0);
    }];
    [rightView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 50));
        make.top.mas_equalTo(0);
        make.right.mas_equalTo(0);
    }];
    leftView.tag = 10001;
    rightView.tag = 10002;
    [leftView setUserInteractionEnabled:YES];
    [rightView setUserInteractionEnabled:YES];
    UITapGestureRecognizer *lefttap = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(changeIfMine:)];
    UITapGestureRecognizer *righttap = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(changeIfMine:)];
    [leftView addGestureRecognizer:lefttap];
    [rightView addGestureRecognizer:righttap];
    
    _leftLabel = [UILabel new];
    _rightLabel = [UILabel new];
    [_headerView addSubview:_leftLabel];
    [_headerView addSubview:_rightLabel];
    [_leftLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 17));
        make.center.equalTo(leftView);
    }];
    [_rightLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 17));
        make.center.equalTo(rightView);
    }];
    [_leftLabel setText:@"自己的气泡"];
    [_leftLabel setTextColor:TTBLUE];
    [_leftLabel setTextAlignment:NSTextAlignmentCenter];
    [_rightLabel setText:@"别人的气泡"];
    [_rightLabel setTextColor:RGB(127, 127, 127)];
    [_rightLabel setTextAlignment:NSTextAlignmentCenter];
    
    _leftBottomLine = [UILabel new];
    _rightBottomLine = [UILabel new];
    [_headerView addSubview:_leftBottomLine];
    [_headerView addSubview:_rightBottomLine];
    [_leftBottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 2));
        make.left.mas_equalTo(0);
        make.bottom.equalTo(_headerView.mas_bottom);
    }];
    [_rightBottomLine mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 1));
        make.right.mas_equalTo(0);
        make.bottom.equalTo(_headerView.mas_bottom);
    }];
    [_leftBottomLine setBackgroundColor:TTBLUE];
    [_rightBottomLine setBackgroundColor:RGB(243,242,246)];
    [_headerView addSubview:_leftBottomLine];
    [_headerView addSubview:_rightBottomLine];
}

#pragma mark - collectionview

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView
{
    return 1;
}

-(NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [self.bubbles count];
}

-(UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    MTTBubbleCollectionViewCell *cell =[collectionView dequeueReusableCellWithReuseIdentifier:@"BubbleShowCell" forIndexPath:indexPath];
    NSString *key = [self.bubbleKeys objectAtIndex:indexPath.row];
    NSString *value = [self.bubbles objectForKey:key];
    [cell setcontent:key value:value];
    // 如果是自己的.选中
    if([[MTTUtil getBubbleTypeLeft:!self.isMine] isEqualToString:value]){
        [cell setCellSelected:YES];
    }else{
        [cell setCellSelected:NO];
    }
    return cell ;
}

-(UICollectionReusableView *)collectionView:(UICollectionView *)collectionView viewForSupplementaryElementOfKind:(NSString *)kind atIndexPath:(NSIndexPath *)indexPath
{
    UICollectionReusableView *reusableView =nil;
    
    if (kind ==UICollectionElementKindSectionHeader){
        UICollectionReusableView *headerV = (UICollectionReusableView *)[collectionView dequeueReusableSupplementaryViewOfKind:UICollectionElementKindSectionHeader withReuseIdentifier: @"HeaderView" forIndexPath:indexPath];
        [headerV addSubview:_headerView];
        [_headerView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.size.equalTo(headerV);
            make.top.mas_equalTo(0);
            make.left.mas_equalTo(0);
        }];
        reusableView = headerV;
    }
    return reusableView;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *key = [self.bubbleKeys objectAtIndex:indexPath.row];
    NSString *value = [self.bubbles objectForKey:key];
    [[MTTBubbleModule shareInstance] selectBubbleTheme:value left:!self.isMine];
    NSString* nick = [RuntimeStatus instance].user.nick;
    NSString* eventMine;
    if(self.isMine){
        eventMine = @"1";
    }else{
        eventMine = @"0";
    }
    NSDictionary *dict = @{@"nick" : nick,@"theme":key,@"mine":eventMine};
    [self.collectionView reloadData];
}

- (void)changeIfMine:(UITapGestureRecognizer *)gesture
{
    if(gesture.view.tag == 10001){
        [_leftBottomLine mas_updateConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 2));
        }];
        [_leftBottomLine setBackgroundColor:TTBLUE];
        
        [_rightBottomLine mas_updateConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 1));
        }];
        [_rightBottomLine setBackgroundColor:RGB(243,242,246)];
        [_leftLabel setTextColor:TTBLUE];
        [_rightLabel setTextColor:RGB(127, 127, 127)];
        self.isMine = YES;
    }else{
        [_rightBottomLine mas_updateConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 2));
        }];
        [_rightBottomLine setBackgroundColor:TTBLUE];
        
        [_leftBottomLine mas_updateConstraints:^(MASConstraintMaker *make) {
            make.size.mas_equalTo(CGSizeMake(SCREEN_WIDTH/2, 1));
        }];
        [_leftBottomLine setBackgroundColor:RGB(243,242,246)];
        [_rightLabel setTextColor:TTBLUE];
        [_leftLabel setTextColor:RGB(127, 127, 127)];
        self.isMine = NO;
    }
    [self.collectionView reloadData];
}

@end
