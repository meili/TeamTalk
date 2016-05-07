//
//  DDReceiveMessageAPI.m
//  IOSDuoduo
//
//  Created by 独嘉 on 14-6-5.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "DDReceiveMessageAPI.h"
#import "MTTMessageEntity.h"
#import "Encapsulator.h"
#import "DDMessageModule.h"
#import "RuntimeStatus.h"
#import "IMMessage.pb.h"
@implementation DDReceiveMessageAPI
- (int)responseServiceID
{
    return SID_MSG;
}

- (int)responseCommandID
{
    return IM_MSG_DATA;
}

- (UnrequestAPIAnalysis)unrequestAnalysis
{
    UnrequestAPIAnalysis analysis = (id)^(NSData *data)
    {
        IMMsgData *msgdata = [IMMsgData parseFromData:data];
        MTTMessageEntity *msg = [MTTMessageEntity makeMessageFromPBData:msgdata];
        msg.state=DDmessageSendSuccess;
        return msg;
    };
    return analysis;
}
@end
