//
//  NSString+Additions.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-23.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "NSString+Additions.h"

#import <sys/xattr.h>

#import <CommonCrypto/CommonDigest.h>

@implementation NSString (TTString)

+(NSString *)documentPath {
    static NSString * path = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)
                 objectAtIndex:0] copy];
        [NSString addSkipBackupAttributeToItemAtURL:[NSURL fileURLWithPath:path]];
    });
    return path;
}
+(BOOL)addSkipBackupAttributeToItemAtURL:(NSURL *)URL
{
    if (URL==nil) {
        return NO;
    }
    NSString *systemVersion=[[UIDevice currentDevice] systemVersion];
    float version=[systemVersion floatValue];
    if (version<5.0) {
        return YES;
    }
    if ( version>=5.1) {
        assert([[NSFileManager defaultManager] fileExistsAtPath: [URL path]]);
        
        NSError *error = nil;
        BOOL success = [URL setResourceValue: [NSNumber numberWithBool: YES]
                                      forKey: NSURLIsExcludedFromBackupKey error: &error];
        if(!success){
        }
        return success;
    }
    
    if ([systemVersion isEqual:@"5.0"]) {
        return NO;
    }else{
        assert([[NSFileManager defaultManager] fileExistsAtPath: [URL path]]);
        
        const char* filePath = [[URL path] fileSystemRepresentation];
        
        const char* attrName = "com.apple.MobileBackup";
        u_int8_t attrValue = 1;
        
        int result = setxattr(filePath, attrName, &attrValue, sizeof(attrValue), 0, 0);
        return result == 0;
    }
    return YES;
}
+(NSString *)cachePath {
    static NSString * path = nil;
    if (!path) {
        path = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)
                 objectAtIndex:0] copy];
    }
    return path;
}

+(NSString *)formatCurDate {
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss"];
    NSString *result = [dateFormatter stringFromDate:[NSDate date]];
    
    return result;
}
+(NSString *)formatCurDay {
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd"];
    NSString *result = [dateFormatter stringFromDate:[NSDate date]];
    
    return result;
}
+(NSString *)formatCurDayForVersion {
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy.MM.dd"];
    NSString *result = [dateFormatter stringFromDate:[NSDate date]];
    
    return result;
}
+(NSString *)getAppVer {
    return [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
}
- (NSURL *) toURL {
    return [NSURL URLWithString:[self stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
}

- (BOOL) isEmpty {
    return nil == self
    || 0 == [[self stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length];
}


- (NSString *) MD5 {
    // Create pointer to the string as UTF8
	const char* ptr = [self UTF8String];
	unsigned char md5Buffer[CC_MD5_DIGEST_LENGTH];
    
	// Create 16 byte MD5 hash value, store in buffer
	CC_MD5(ptr, strlen(ptr), md5Buffer);
    
	// Convert MD5 value in the buffer to NSString of hex values
	NSMutableString* output = [NSMutableString stringWithCapacity:CC_MD5_DIGEST_LENGTH * 2];
	for (int i = 0; i < CC_MD5_DIGEST_LENGTH; i++) {
		[output appendFormat:@"%02x",md5Buffer[i]];
	}
    
	return output;
}
-(NSString *)trim{
    return  [self stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}


-(BOOL) isOlderVersionThan:(NSString*)otherVersion
{
	return ([self compare:otherVersion options:NSNumericSearch] == NSOrderedAscending);
}

-(BOOL) isNewerVersionThan:(NSString*)otherVersion
{
	return ([self compare:otherVersion options:NSNumericSearch] == NSOrderedDescending);
}
- (NSString*)removeAllSpace
{
    NSString* result = [self stringByReplacingOccurrencesOfString:@" " withString:@""];
    result = [result stringByReplacingOccurrencesOfString:@"    " withString:@""];
    return result;
}
@end