//
//  MTTDepartment.m
//  IOSDuoduo
//
//  Created by Michael Scofield on 2014-08-06.
//  Copyright (c) 2014 dujia. All rights reserved.
//

#import "MTTDepartment.h"

@implementation MTTDepartment
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.ID = @"";
        self.parentID=@"";
        self.title=@"";
        self.description=@"";
        self.leader=@"";
        self.status=0;
        self.count=0;

    }
    return self;
}
+(id)departmentFromDic:(NSDictionary *)dic
{
    MTTDepartment *department = [MTTDepartment new];
    department.ID = [dic objectForKey:@"departID"];
    department.title = [dic objectForKey:@"title"];
    department.description = [dic objectForKey:@"description"];
    department.leader = [dic objectForKey:@"leader"];
    department.parentID = [dic objectForKey:@"parentID"];
    department.status = [[dic objectForKey:@"status"] integerValue];
    department.count = [[dic objectForKey:@"departCount"] integerValue];
    return department;
}
@end
