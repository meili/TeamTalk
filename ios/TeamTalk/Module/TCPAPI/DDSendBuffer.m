//
//  SendBuffer.m
//  mtalk
//
//  Created by maye on 13-10-23.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "DDSendBuffer.h"

@implementation DDSendBuffer
@synthesize sendPos;

+ (id)dataWithNSData:(NSData *)newData;
{
	DDSendBuffer *obj = [DDSendBuffer alloc];
	return [obj initWithData:newData];
}

- (id)initWithData:(NSData *)newData
{
	self = [super init];
    if (self) {
		embeddedData = [NSMutableData dataWithData:newData];
		sendPos = 0;
	}
	
	return self;
}

- (void)consumeData:(NSInteger)length {
	sendPos += length;
}



- (const void *)bytes
{
	return [embeddedData bytes];
}

- (NSUInteger)length
{
	return [embeddedData length];
}

- (void *)mutableBytes
{
	return [embeddedData mutableBytes];
}

- (void)setLength:(NSUInteger)length
{
	[embeddedData setLength:length];
}
@end
