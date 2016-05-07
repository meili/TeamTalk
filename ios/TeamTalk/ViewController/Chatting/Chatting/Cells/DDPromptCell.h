//
//  DDPromptCell.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-9.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DDPromptCell : UITableViewCell
{
    UILabel* _promptLabel;
}

- (void)setprompt:(NSString*)prompt;
@end
