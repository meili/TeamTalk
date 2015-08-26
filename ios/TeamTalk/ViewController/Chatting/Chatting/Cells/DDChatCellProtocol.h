//
//  DDChatCellProtocol.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-28.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
@class MTTMessageEntity;
@protocol DDChatCellProtocol <NSObject>

- (CGSize)sizeForContent:(MTTMessageEntity*)content;

- (float)contentUpGapWithBubble;

- (float)contentDownGapWithBubble;

- (float)contentLeftGapWithBubble;

- (float)contentRightGapWithBubble;

- (void)layoutContentView:(MTTMessageEntity*)content;

- (float)cellHeightForMessage:(MTTMessageEntity*)message;
@end
