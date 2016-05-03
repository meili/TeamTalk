//
//  MTTUtil.m
//  TeamTalk
//
//  Created by 宪法 on 15/6/18.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTUtil.h"

#import "IMBaseDefine.pb.h"
#import "MTTUserEntity.h"
#import "MTTGroupEntity.h"
#import "DDTcpClientManager.h"
#import "DDClientState.h"
#import "MTTPinyinConstant.h"
#import "SessionModule.h"
#import "DDMessageModule.h"


@implementation MTTUtil

#pragma mark - 拼音

+(char)pinyinFirstLetter:(unsigned short)hanzi{
    int index = hanzi - HANZI_START;
    if (index >= 0 && index <= HANZI_COUNT)
    {
        return firstLetterArray[index];
    }
    else
    {
        return '#';
    }
}

+(char)getFirstChar:(const NSString *)str{
    if (nil == str || 0 == [str length]) {
        return '#';
    }
    const char * firstChar = [str UTF8String];
    if ( ('a'<=  *firstChar && *firstChar <= 'z')
        || ('A' <= *firstChar && *firstChar <= 'Z')) {
        return *firstChar;
    }
    else {
        return [MTTUtil pinyinFirstLetter:[str characterAtIndex:0]];
    }
}

#pragma mark - loginOut

+(void)loginOut{
    
    
    TheRuntime.user =nil;
    TheRuntime.userID =nil;
    [[DDMessageModule shareInstance] removeAllUnreadMessages];
    [[SessionModule instance] clearSession];
    [[DDTcpClientManager instance] disconnect];
    [DDClientState shareInstance].userState = DDUserOffLineInitiative;
}

#pragma mark - show alert

+(void)showAlertWithTitle:(NSString *)title message:(NSString *)message
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title message:message delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
    [alert show];
}

#pragma mark - OriginalID & sessionID

+(UInt32)changeIDToOriginal:(NSString *)sessionID
{
    NSArray *array = [sessionID componentsSeparatedByString:@"_"];
    if (array[1]) {
        return [array[1] unsignedIntValue];
    }
    return 0;
}

+(NSString *)changeOriginalToLocalID:(UInt32)orignalID SessionType:(int)sessionType
{
    if(sessionType == SessionTypeSessionTypeSingle)
    {
        return [MTTUserEntity pbUserIdToLocalID:orignalID];
    }
    return [MTTGroupEntity pbGroupIdToLocalID:orignalID];
}

#pragma mark - new function

+(BOOL)isUseFunctionBubble{
    
    NSNumber *number =[[NSUserDefaults standardUserDefaults] objectForKey:@"UseFunctionBubble"];
    return [number boolValue];
}

+(void)useFunctionBubble{
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:YES] forKey:@"UseFunctionBubble"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

#pragma mark - fiexed top
+(void)setFixedTop:(NSString *)sessionID{
    NSArray *allUser = [[NSUserDefaults standardUserDefaults] objectForKey:@"fixedTopUsers"];
    NSMutableArray *allUserTmp =[NSMutableArray arrayWithArray:allUser];
    [allUserTmp addObject:sessionID];
    [[NSUserDefaults standardUserDefaults] setObject:allUserTmp forKey:@"fixedTopUsers"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+(NSArray *)getFixedTop{
    NSArray *allUser = [[NSUserDefaults standardUserDefaults] objectForKey:@"fixedTopUsers"];
    return allUser;
}

+(void)removeFixedTop:(NSString *)sessionID{
    NSArray *allUser = [[NSUserDefaults standardUserDefaults] objectForKey:@"fixedTopUsers"];
    NSMutableArray *allUserTmp =[NSMutableArray arrayWithArray:allUser];
    [allUserTmp removeObject:sessionID];
    [[NSUserDefaults standardUserDefaults] setObject:allUserTmp forKey:@"fixedTopUsers"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+(BOOL)checkFixedTop:(NSString *)sessionID{
    NSArray *allUser = [[NSUserDefaults standardUserDefaults] objectForKey:@"fixedTopUsers"];
    return [allUser containsObject:sessionID];
}

+(CGSize)sizeTrans:(CGSize)size{
    float width;
    float height;
    float imgWidth = size.width;
    float imgHeight = size.height;
    float radio = size.width/size.height;
    if(radio>=1){
        width = imgWidth > MAX_CHAT_TEXT_WIDTH ? MAX_CHAT_TEXT_WIDTH : imgWidth;
        height = imgWidth > MAX_CHAT_TEXT_WIDTH ? (imgHeight * MAX_CHAT_TEXT_WIDTH / imgWidth):imgHeight;
    }else{
        height = imgHeight > MAX_CHAT_TEXT_WIDTH ? MAX_CHAT_TEXT_WIDTH : imgHeight;
        width = imgHeight > MAX_CHAT_TEXT_WIDTH ? (imgWidth * MAX_CHAT_TEXT_WIDTH / imgHeight):imgWidth;
    }
    return CGSizeMake(width, height);
}

#pragma mark - 气泡功能
+(NSString *)getBubbleTypeLeft:(BOOL)left
{
    NSString *bubbleType;
    if(left){
        bubbleType = [[NSUserDefaults standardUserDefaults] objectForKey:@"userLeftCustomerBubble"];
        if(!bubbleType){
            bubbleType = @"default_white";
        }
    }else{
        bubbleType = [[NSUserDefaults standardUserDefaults] objectForKey:@"userRightCustomerBubble"];
        if(!bubbleType){
            bubbleType = @"default_blue";
        }
    }
    return bubbleType;
}

+(void)setBubbleTypeLeft:(NSString *)bubbleType left:(BOOL)left
{
    if(left){
        [[NSUserDefaults standardUserDefaults] setObject:bubbleType forKey:@"userLeftCustomerBubble"];
    }else{
        [[NSUserDefaults standardUserDefaults] setObject:bubbleType forKey:@"userRightCustomerBubble"];
    }
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+(void)setLastPhotoTime:(NSDate *)date
{
    [[NSUserDefaults standardUserDefaults] setObject:date forKey:@"preShowImageTime"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

+(NSDate *)getLastPhotoTime
{
    NSDate *lastDate = [[NSDate alloc] initWithTimeInterval:-90 sinceDate:[NSDate date]];
    NSDate *date = [[NSUserDefaults standardUserDefaults] objectForKey:@"preShowImageTime"];
    if(date){
        return date;
    }else{
        return lastDate;
    }
}

+(void)setLastShakeTime:(NSDate *)date
{
    [[NSUserDefaults standardUserDefaults] setObject:date forKey:@"shakePcTime"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}
+(BOOL)ifCanShake
{
    NSDate *date = [[NSDate alloc] initWithTimeInterval:-10 sinceDate:[NSDate date]];
    NSDate *preDate = [[NSUserDefaults standardUserDefaults] objectForKey:@"shakePcTime"];
    if(!preDate){
        return YES;
    }
    if([date compare:preDate] == NSOrderedDescending){
        return YES;
    }else{
        return NO;
    }
}
+(void)setDBVersion:(NSInteger)version
{
    [[NSUserDefaults standardUserDefaults] setInteger:version forKey:@"dbVersion"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}
+(NSInteger)getDBVersion
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:@"dbVersion"];
}
+(void)setLastDBVersion:(NSInteger)version
{
    [[NSUserDefaults standardUserDefaults] setInteger:version forKey:@"lastDbVersion"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}
+(NSInteger)getLastDBVersion
{
    return [[NSUserDefaults standardUserDefaults] integerForKey:@"lastDbVersion"];
}
+(void)setMsfsUrl:(NSString*)url
{
    [[NSUserDefaults standardUserDefaults] setObject:url forKey:@"msfsurl"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}
+(NSString*)getMsfsUrl
{
     return [[NSUserDefaults standardUserDefaults] objectForKey:@"msfsurl"];
}

@end
