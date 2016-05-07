//
//  CONSTANT.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-23.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

/**
 *  Debug模式和Release模式不同的宏定义
 */

//-------------------打印--------------------
#ifdef DEBUG
#define NEED_OUTPUT_LOG             0
#define Is_CanSwitchServer          1
#else
#define NEED_OUTPUT_LOG             0
#define Is_CanSwitchServer          0
#endif

#if NEED_OUTPUT_LOG
#define DDLog(xx, ...)                      NSLog(@"%s(%d): " xx, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DDLog(xx, ...)                 nil
#endif

#define IM_PDU_HEADER_LEN   16
#define IM_PDU_VERSION      13

#define SERVER_ADDR                             @"http://61.153.100.221:8080/msg_server"

#define _(x)                                NSLocalizedString(x,@"")