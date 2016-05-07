//
//  MTTWebViewController.h
//  TeamTalk
//
//  Created by 独嘉 on 14-10-22.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"

@interface MTTWebViewController : MTTBaseViewController<UIWebViewDelegate>

@property (nonatomic,copy) NSString *urlString;

@end
