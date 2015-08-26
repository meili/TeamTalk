//
//  DataOutputStream+Addition.m
//  Duoduo
//
//  Created by 独嘉 on 14-5-6.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "DataOutputStream+Addition.h"
#import "NSStream+NSStreamAddition.h"
@implementation DDDataOutputStream (Addition)
-(void)writeTcpProtocolHeader:(int16_t)sId cId:(int16_t)cId seqNo:(uint16_t)seqNo
{
    [self writeShort:IM_PDU_VERSION];
    [self writeShort:0]; //默认值
    [self writeShort:sId];
    [self writeShort:cId];
//    [self writeShort:0]; //默认值
    [self writeShort:seqNo]; //默认值
    [self writeShort:1];
}
@end
