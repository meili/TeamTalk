//
//  MTTDepartment.h
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-06.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MTTDepartment : NSObject

@property(nonatomic ,copy)NSString *ID;
@property(nonatomic ,copy)NSString *parentID;
@property(nonatomic ,copy)NSString *title;
@property(nonatomic ,copy)NSString *description;
@property(nonatomic ,copy)NSString *leader;
@property(nonatomic ,assign)NSInteger status;
@property(nonatomic ,assign)NSInteger count;

+(id)departmentFromDic:(NSDictionary *)dic;
@end
