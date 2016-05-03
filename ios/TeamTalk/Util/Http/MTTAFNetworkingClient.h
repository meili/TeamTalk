//
//  MTTAFNetworkingClient.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-29.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "AFNetworking.h"

#define BLOCK_SAFE_RUN(block, ...) block ? block(__VA_ARGS__) : nil;

@interface MTTAFNetworkingClient : NSObject

+(void) jsonFormPOSTRequest:(NSString *)url param:(NSDictionary *)param success:(void (^)(id))success failure:(void (^)(NSError *))failure;

+(void) jsonFormRequest:(NSString *)url param:(NSDictionary *)param fromBlock:(void (^)(id <AFMultipartFormData> formData))block success:(void (^)(id))success failure:(void (^)(NSError *))failure;
@end

