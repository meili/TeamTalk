//
//  MTTBubbleCollectionViewCell.h
//  TeamTalk
//
//  Created by scorpio on 15/7/3.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTBubbleModule.h"

@interface MTTBubbleCollectionViewCell : UICollectionViewCell

@property (nonatomic,retain)UILabel* bubbleContent;
@property (nonatomic,retain)UIImageView* bubbleImage;
@property (nonatomic,retain)UIImageView* isSelected;
- (void)setcontent:(NSString*)key value:(NSString *)value;
- (void)setCellSelected:(BOOL)selected;

@end

