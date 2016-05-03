//
//  DDEmotionCell.h
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-22.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import "DDChatImageCell.h"

@class MTTMessageEntity;
@interface DDEmotionCell : DDChatImageCell<DDChatCellProtocol>
-(void)sendTextAgain:(MTTMessageEntity *)msg;
@end
