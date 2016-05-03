//
//  LogoutAPI.m
//  TeamTalk
//
//  Created by Michael Scofield on 2014-10-20.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "LogoutAPI.h"
#import "IMLogin.pb.h"
@implementation LogoutAPI
- (int)requestTimeOutTimeInterval
{
    return 5;
}

- (int)requestServiceID
{
    return SID_LOGIN;
}

- (int)responseServiceID
{
    return SID_LOGIN;
}

- (int)requestCommendID
{
    return IM_LOGOUT_REQ;
}

- (int)responseCommendID
{
    return IM_LOGOUT_RES;
}

- (Analysis)analysisReturnData
{
    Analysis analysis = (id)^(NSData* data)
    {
        
        IMLogoutRsp *rsp =[IMLogoutRsp parseFromData:data];
        int isok =rsp.resultCode;
        return isok;
    };
    return analysis;
}

- (Package)packageRequestObject
{
    Package package = (id)^(id object,uint32_t seqNo)
    {
        IMLogoutReqBuilder *logoutbuilder = [IMLogoutReq builder];
        DDDataOutputStream *dataout = [[DDDataOutputStream alloc] init];
        [dataout writeInt:0];
        [dataout writeTcpProtocolHeader:SID_LOGIN cId:IM_LOGOUT_REQ seqNo:seqNo];
        [dataout directWriteBytes:[logoutbuilder build].data];
        [dataout writeDataCount];
        return [dataout toByteArray];
    };
    return package;
}
@end
