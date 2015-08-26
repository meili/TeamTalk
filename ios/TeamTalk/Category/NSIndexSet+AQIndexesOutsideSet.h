//
//  NSIndexSet+AQIndexesOutsideSet.h
//  Kobov3
//
//  Created by Jim Dovey on 10-06-22.
//  Copyright 2010 Kobo Inc. All rights reserved.
//

#import <Foundation/NSIndexSet.h>

@interface NSIndexSet (AQIndexesOutsideSet)
- (NSIndexSet *) aq_indexesOutsideIndexSet: (NSIndexSet *) otherSet;
@end
