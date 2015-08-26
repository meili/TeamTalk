/*
 * AQGridViewData.h
 * AQGridView
 * 
 * Created by Jim Dovey on 1/3/2010.
 * Copyright (c) 2010 Kobo Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "AQGridView.h"

@interface AQGridViewData : NSObject <NSCopying, NSMutableCopying>
{
	AQGridView *				__unsafe_unretained _gridView;				// weak reference
	CGSize						_boundsSize;
	AQGridViewLayoutDirection	_layoutDirection;
	CGSize						_desiredCellSize;		// NB: 'cell' here means a grid cell -- i.e. constant size, tessellating
	CGSize						_actualCellSize;
	
	CGFloat						_topPadding;
	CGFloat						_bottomPadding;
	CGFloat						_leftPadding;
	CGFloat						_rightPadding;
	
	NSUInteger					_numberOfItems;
	NSUInteger					_reorderedIndex;
}

- (id) initWithGridView: (AQGridView *) gridView;

@property (nonatomic) NSUInteger numberOfItems;

@property (nonatomic) CGFloat topPadding, bottomPadding, leftPadding, rightPadding;
@property (nonatomic) AQGridViewLayoutDirection layoutDirection;

// notify this object of changes to the layout parameters
- (void) gridViewDidChangeBoundsSize: (CGSize) boundsSize;

// nabbed from UITableViewRowData-- will we need something like this?
@property (nonatomic) NSUInteger reorderedIndex;

// Turning view locations into item indices
- (NSUInteger) itemIndexForPoint: (CGPoint) point;
- (BOOL) pointIsInLastRow: (CGPoint) point;

// grid cell sizes-- for the layout calculations
- (void) setDesiredCellSize: (CGSize) desiredCellSize;
- (CGSize) cellSize;

// metrics used within the scroll view
- (CGRect) rectForEntireGrid;
- (CGSize) sizeForEntireGrid;
- (NSUInteger) numberOfItemsPerRow;

- (CGRect) cellRectAtIndex: (NSUInteger) index;
- (CGRect) cellRectForPoint: (CGPoint) point;
- (NSIndexSet *) indicesOfCellsInRect: (CGRect) rect;		// NB: Grid Cells only-- AQGridViewCells might not actually intersect

@end
