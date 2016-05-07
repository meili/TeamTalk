//
//  DDAlbumDetailsBottomBar.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-6-4.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef void(^ButtonSelectBlock)(int buttonIndex) ;
@interface AlbumDetailsBottomBar : UIView
@property(nonatomic,strong)UIButton *send;
@property(nonatomic,copy)ButtonSelectBlock Block;

-(void)setSendButtonTitle:(int)num;
@end
