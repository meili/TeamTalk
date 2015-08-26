//
//  DDDDChatUtilityViewController.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-23.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"
@interface ChatUtilityViewController : MTTBaseViewController<UIImagePickerControllerDelegate,UINavigationControllerDelegate>
@property(nonatomic,strong) UIImagePickerController *imagePicker;
@property(nonatomic) NSInteger userId;
-(void)setShakeHidden;
@end
