//
//  DDPersonEditCollectionCell.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DDPersonEditCollectionCell : UICollectionViewCell
@property(strong)UIImageView *personIcon;
@property(strong)UIButton *delImg;
@property(strong)UILabel *name;
@property(strong) UIButton *button;
-(void)setContent:(NSString *)name AvatarImage:(NSString *)urlString;
@end