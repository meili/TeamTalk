//
//  DDBaseEntity.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-16.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MTTBaseEntity : NSObject
@property(assign)long lastUpdateTime;
@property(copy)NSString *objID;
@property(assign)NSInteger objectVersion;
-(NSUInteger)getOriginalID;
@end
