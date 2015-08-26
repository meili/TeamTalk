//
//  DataOutputStream.h
//  mtalk
//
//  Created by maye on 13-10-24.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import <Foundation/Foundation.h>

// 数据输出流允许应用程序以适当方式将基本数据类型写入输出流中
@interface DDDataOutputStream : NSObject{
    NSMutableData *data;
    NSInteger length;
}
// 将一个 char 值以 1-byte 值形式写入基础输出流中，先写入高字节。
- (void)writeChar:(int8_t)v;

//将一个 short 值以 2-byte 值形式写入基础输出流中，先写入高字节。
- (void)writeShort:(int16_t)v;

//将一个 int 值以 4-byte 值形式写入基础输出流中，先写入高字节。
- (void)writeInt:(int32_t)v;

//将一个 long 值以 8-byte 值形式写入基础输出流中，先写入高字节。
- (void)writeLong:(int64_t)v;

//以与机器无关方式使用 UTF-8 修改版编码将一个字符串写入基础输出流。
- (void)writeUTF:(NSString *)v;

//将一个 NSData byte数组写入输出流中，先写入高字节。
- (void)writeBytes:(NSData *)v;
-(void)directWriteBytes:(NSData *)v;
-(void)writeDataCount;

//将此转换为 byte 序列。
- (NSMutableData *)toByteArray;

@end
