//
//  NSData+Conversion.m
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-06.
//  Copyright (c) 2015 dujia. All rights reserved.
//

#import "NSData+Conversion.h"

@implementation NSData (Conversion)
#pragma mark - String Conversion
- (NSString *)hexadecimalString {
    const unsigned char *dataBuffer = (const unsigned char *)[self bytes];
    
    if (!dataBuffer)
        return [NSString string];
    
    NSUInteger          dataLength  = [self length];
    NSMutableString     *hexString  = [NSMutableString stringWithCapacity:(dataLength * 2)];
    
    for (int i = 0; i < dataLength; ++i)
        [hexString appendString:[NSString stringWithFormat:@"%02lx", (unsigned long)dataBuffer[i]]];
    
    return [NSString stringWithString:hexString];
}

@end
