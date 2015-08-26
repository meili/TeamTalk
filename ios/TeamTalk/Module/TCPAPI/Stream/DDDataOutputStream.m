//
//  DataOutputStream.m
//  mtalk
//
//  Created by maye on 13-10-24.
//  Copyright (c) 2013年 zuoye. All rights reserved.
//

#import "DDDataOutputStream.h"

@implementation DDDataOutputStream
- (id)init{
    self = [super init];
    if(self != nil){
        data = [[NSMutableData alloc] init];
        length = 0;
    }
    return self;
}

- (void)writeChar:(int8_t)v {
    int8_t ch[1];
    ch[0] = (v & 0x0ff);
    [data appendBytes:ch length:1];
    length++;
}

- (void)writeShort:(int16_t)v {
    int8_t ch[2];
    ch[0] = (v & 0x0ff00)>>8;
    ch[1] = (v & 0x0ff);
    [data appendBytes:ch length:2];
    length = length + 2;
}

- (void)writeInt:(int32_t)v {
    int8_t ch[4];
    for(int32_t i = 0;i<4;i++){
        ch[i] = ((v >> ((3 - i)*8)) & 0x0ff);
    }
    [data appendBytes:ch length:4];
    length = length + 4;
}

- (void)writeLong:(int64_t)v {
    int8_t ch[8];
    for(int32_t i = 0;i<8;i++){
        ch[i] = ((v >> ((7 - i)*8)) & 0x0ff);
    }
    [data appendBytes:ch length:8];
    length = length + 8;
}

- (void)writeUTF:(NSString *)v {
    NSData *d = [v dataUsingEncoding:NSUTF8StringEncoding];
    uint32_t len = (uint32_t)[d length];
    
    [self writeInt:len];
    [data appendData:d];
    length = length + len;
}

- (void)writeBytes:(NSData *)v {
    int32_t len = (int32_t)[v length];
    [self writeInt:len];
    [data appendData:v];
    
    length = length + len;
}
-(void)directWriteBytes:(NSData *)v
{
    int32_t len = (int32_t)[v length];
    [data appendData:v];
    length = length + len;
}
-(void)writeDataCount
{
    int8_t ch[4];
    for(int32_t i = 0;i<4;i++){
        ch[i] = ((length >> ((3 - i)*8)) & 0x0ff);
    }
    
    [data replaceBytesInRange:NSMakeRange(0, 4) withBytes:ch];
}

- (NSMutableData *)toByteArray{
    return [[NSMutableData alloc] initWithData:data];
}
@end
