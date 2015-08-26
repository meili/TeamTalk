//
//  DDSendPhotoMessageAPI.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-6-6.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDSuperAPI.h"

@interface DDSendPhotoMessageAPI : NSObject
+ (DDSendPhotoMessageAPI *)sharedPhotoCache;
- (void)uploadImage:(NSString*)imagePath success:(void(^)(NSString* imageURL))success failure:(void(^)(id error))failure;
@end
