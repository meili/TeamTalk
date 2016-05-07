//
//  DDChattingEditModel.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-07-17.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ChattingEditModule : NSObject
-(void)removePersonFromGroup:(NSArray *)userIDs Block:(void(^)(BOOL success))block;
- (instancetype)initChattingEditModel:(NSString *)sessionID;

@end
