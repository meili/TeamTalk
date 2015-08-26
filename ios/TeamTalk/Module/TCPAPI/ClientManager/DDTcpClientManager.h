//
//  DDTcpClientManager.h
//  Duoduo
//
//  Created by 独嘉 on 14-4-12.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>
@class DDSendBuffer;
@interface DDTcpClientManager : NSObject<NSStreamDelegate>
{
@private
    NSInputStream *_inStream;
    NSOutputStream *_outStream;
    NSLock *_receiveLock;
	NSMutableData *_receiveBuffer;
    NSLock *_sendLock;
	NSMutableArray *_sendBuffers;
	DDSendBuffer *_lastSendBuffer;
	BOOL _noDataSent;
    int32_t cDataLen;

}

+ (instancetype)instance;

-(void)connect:(NSString *)ipAdr port:(NSInteger)port status:(NSInteger)status;
-(void)disconnect;
-(void)writeToSocket:(NSMutableData *)data;


@end
