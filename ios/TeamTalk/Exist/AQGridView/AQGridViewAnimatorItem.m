//
//  AQGridViewAnimatorItem.m
//  Kobov3
//
//  Created by Jim Dovey on 10-06-29.
//  Copyright 2010 Kobo Inc. All rights reserved.
//

#import "AQGridViewAnimatorItem.h"

@implementation AQGridViewAnimatorItem

@synthesize animatingView, index;

+ (AQGridViewAnimatorItem *) itemWithView: (UIView *) aView index: (NSUInteger) anIndex
{
	AQGridViewAnimatorItem * result = [[self alloc] init];
	result.animatingView = aView;
	result.index = anIndex;
	return ( result );
}


- (NSUInteger) hash
{
	return ( self.index );
}

- (BOOL) isEqual: (AQGridViewAnimatorItem *) o
{
	if ( [o isKindOfClass: [self class]] == NO )
		return ( NO );
	
	return ( o.index == self.index );
}

- (NSComparisonResult) compare: (id) obj
{
	if ( [obj isKindOfClass: [self class]] == NO )
	{
		if ( (void *)objc_unretainedPointer(self) < (void *)objc_unretainedPointer(obj) )
			return ( NSOrderedAscending );
		if ( (void *)objc_unretainedPointer(self) > (void *)objc_unretainedPointer(obj) )
			return ( NSOrderedDescending );
		return ( NSOrderedSame );			// how ??!?!?
	}
	
	AQGridViewAnimatorItem * item = (AQGridViewAnimatorItem *) obj;
	if ( self.index < item.index )
		return ( NSOrderedAscending );
	if ( self.index > item.index )
		return ( NSOrderedDescending );
	
	return ( NSOrderedSame );
}

@end
