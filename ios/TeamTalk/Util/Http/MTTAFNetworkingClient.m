//
//  MTTAFNetworkingClient.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-29.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTAFNetworkingClient.h"
#import "NSDictionary+Safe.h"

@implementation MTTAFNetworkingClient

static NSString* const DD_URL_BASE = @"http://www.mogujie.com/";

+(void) handleRequest:(id)result
              success:(void (^)(id))success
              failure:(void (^)(NSError *))failure
{
    
    if (![result isKindOfClass:[NSDictionary class]]) {
        NSError * error = [NSError errorWithDomain:@"data formate is invalid" code:-1000 userInfo:nil];
        BLOCK_SAFE_RUN(failure, error);
        return;
    }
    NSInteger code =[[[result safeObjectForKey:@"status"] objectForKey:@"code"] integerValue];
    NSString *msg =[[result safeObjectForKey:@"status"] objectForKey:@"msg"];
    if (1001 == code)
    {
        id object = [result valueForKey:@"result"];
        object = isNull(object) ? result : object;
        BLOCK_SAFE_RUN(success,object);
    }
    else
    {
        
        if (msg)
        {
            NSError* error = [NSError errorWithDomain:msg code:code userInfo:nil];
            failure(error);
        }
        else
        {
            failure(nil);
        }
    }
    
    
}
+(void) jsonFormRequest:(NSString *)url param:(NSDictionary *)param fromBlock:(void (^)(id <AFMultipartFormData> formData))block success:(void (^)(id))success failure:(void (^)(NSError *))failure
{
    AFHTTPRequestOperationManager *manager = [AFHTTPRequestOperationManager manager];
    [manager GET:url parameters:param success:^(AFHTTPRequestOperation *operation, id responseObject) {
        if ([responseObject respondsToSelector:@selector(objectForKey:)]) {
            [MTTAFNetworkingClient handleRequest:(NSDictionary *)responseObject success:success failure:failure];
        }else
        {
            NSDictionary *responseDictionary = [NSJSONSerialization JSONObjectWithData:responseObject options:0 error:nil];
            [MTTAFNetworkingClient handleRequest:responseDictionary success:success failure:failure];
        }
        
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        BLOCK_SAFE_RUN(failure,error);
    }];
}
+(void) jsonFormPOSTRequest:(NSString *)url param:(NSDictionary *)param success:(void (^)(id))success failure:(void (^)(NSError *))failure{
    
    AFHTTPRequestOperationManager *manager = [AFHTTPRequestOperationManager manager];
    manager.responseSerializer = [AFHTTPResponseSerializer serializer];
    NSString *fullPath = [NSString stringWithFormat:@"%@%@",DD_URL_BASE,url];
    
    [manager POST:fullPath parameters:param success:^(AFHTTPRequestOperation *operation, id responseObject) {
        
        NSDictionary *responseDictionary = [NSJSONSerialization JSONObjectWithData:responseObject options:0 error:nil];
        NSString *string = [[NSString alloc] initWithData:responseObject encoding:NSUTF8StringEncoding];
        NSLog(@"%@<------",string);
        [MTTAFNetworkingClient handleRequest:responseDictionary success:success failure:failure];
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        if([error.domain isEqualToString:NSURLErrorDomain])
            error = [NSError errorWithDomain:@"没有网络连接。" code:-100 userInfo:nil];
        BLOCK_SAFE_RUN(failure,error);
    }];
}

@end
