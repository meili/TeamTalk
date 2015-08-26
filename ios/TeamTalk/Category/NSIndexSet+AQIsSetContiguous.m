//
//  NSIndexSet+AQIsSetContiguous.m
//  AQGridView
//
//  Created by Jim Dovey on 10-04-17.
//  Copyright 2010 Kobo Inc. All rights reserved.
//

#import "NSIndexSet+AQIsSetContiguous.h"

@implementation NSIndexSet (AQIsSetContiguous)

- (BOOL) aq_isSetContiguous
{
    return ( (([self lastIndex] - [self firstIndex]) + 1) == [self count] );
}

@end
