//
//  PublicProfileCell.h
//  TeamTalk
//
//  Created by scorpio on 14/12/18.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface PublicProfileCell : UITableViewCell

@property (nonatomic,strong)UILabel* descLabel;
@property (nonatomic,strong)UILabel* detailLabel;

- (void)setDesc:(NSString *)desc detail:(NSString *)detail;

+(CGFloat)cellHeightForDetailString:(NSString *)string;

@end
