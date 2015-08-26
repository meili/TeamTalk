//
//  DMTTDepartment.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-05.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "DDDepartmentAPI.h"

@implementation DMTTDepartmentAPI

/**
 *  请求超时时间
 *
 *  @return 超时时间
 */
- (int)requestTimeOutTimeInterval
{
    return TimeOutTimeInterval;
}

/**
 *  请求的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)requestServiceID
{
    return 2;
}

/**
 *  请求返回的serviceID
 *
 *  @return 对应的serviceID
 */
- (int)responseServiceID
{
    return 2;
}

/**
 *  请求的commendID
 *
 *  @return 对应的commendID
 */
- (int)requestCommendID
{
    return 18;
}

/**
 *  请求返回的commendID
 *
 *  @return 对应的commendID
 */
- (int)responseCommendID
{
    return 19;
}

/**
 *  解析数据的block
 *
 *  @return 解析数据的block
 */
- (Analysis)analysisReturnData
{
    
    Analysis analysis = (id)^(NSData* data)
    {
        DDDataInputStream* bodyData = [DDDataInputStream dataInputStreamWithData:data];
        NSInteger departCount = [bodyData readInt];
        NSMutableArray *array = [NSMutableArray new];
        for (int i = 0 ; i<departCount; i++) {
            NSString *departID = [bodyData readUTF];
            NSString *title = [bodyData readUTF];
            NSString *description = [bodyData readUTF];
            NSString *parentID = [bodyData readUTF];
            NSString *leader = [bodyData readUTF];
            NSInteger isDelete = [bodyData readInt];
            NSDictionary *result = @{@"departCount": @(departCount),
                                     @"departID":departID,
                                     @"title":title,
                                     @"description":description,
                                     @"parentID":parentID,
                                     @"leader":leader,
                                     @"isDelete":@(isDelete)
                                     };
            [array addObject:result];

        }
    
        return array;
    };
    return analysis;
}

/**
 *  打包数据的block
 *
 *  @return 打包数据的block
 */
- (Package)packageRequestObject
{
    Package package = (id)^(id object,uint32_t seqNo)
    {
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:IM_PDU_HEADER_LEN];
        [dataout writeTcpProtocolHeader:2
                                    cId:18
                                  seqNo:seqNo];
    
        return [dataout toByteArray];
    };
    return package;
}

@end
