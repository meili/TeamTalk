//
//  EditGroupViewCell.h
//  TeamTalk
//
//  Created by Michael Scofield on 2014-09-01.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTTUserEntity.h"

@interface EditGroupViewCell : UICollectionViewCell
@property(strong)UILabel *name;
@property(strong)UIImageView *personIcon;
@property(strong)UIButton *button;

-(void)showDeleteActionView;
@end
