//
//  DDAPISchedule.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-24.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDAPIScheduleProtocol.h"
#import "DDAPIUnrequestScheduleProtocol.h"
typedef struct Response_Server_Data_Head {
    int serviceID;
    int commandID;
    int seqNo;
}ServerDataType;

NS_INLINE ServerDataType DDMakeServerDataType(int serviceID, int commendID,int seqNo)
{
    ServerDataType type;
    type.serviceID = serviceID;
    type.commandID = commendID;
    type.seqNo = seqNo;
    return type;
}


//TODO:应该有自己的专属线程

@interface DDAPISchedule : NSObject

@property (nonatomic,readonly)dispatch_queue_t apiScheduleQueue;

+ (instancetype)instance;

/**
 *  注册接口，此接口只应该在DDSuperAPI中被使用
 *
 *  @param api 接口
 */
- (BOOL)registerApi:(id<DDAPIScheduleProtocol>)api;

/**
 *  注册超时的查询表，此接口只应该在DDSuperAPI中被使用
 *
 *  @param api 接口
 */
- (void)registerTimeoutApi:(id<DDAPIScheduleProtocol>)api;

/**
 *  接收到服务器端的数据进行解析，对外的接口
 *
 *  @param data 服务器端的数据
 */
- (void)receiveServerData:(NSData*)data forDataType:(ServerDataType)dataType;

/**
 *  注册没有请求，只有返回的api
 *
 *  @param api api
 */
- (BOOL)registerUnrequestAPI:(id<DDAPIUnrequestScheduleProtocol>)api;

/**
 *  发送数据包
 *
 *  @param data 数据包
 */
- (void)sendData:(NSMutableData*)data;
@end
