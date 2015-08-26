//
//  NSString+Additions.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-23.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (mogujieString)

+(NSString *)documentPath;
+(NSString *)cachePath;
+(NSString *)formatCurDate;
+(NSString *)formatCurDay;
+(NSString *)getAppVer;
- (NSString*)removeAllSpace;
+(NSString *)formatCurDayForVersion;
- (NSURL *) toURL;
- (BOOL) isEmail;
- (BOOL) isEmpty;
- (NSString *) MD5;
-(NSString *)trim;

-(BOOL) isOlderVersionThan:(NSString*)otherVersion;
-(BOOL) isNewerVersionThan:(NSString*)otherVersion;
@end
