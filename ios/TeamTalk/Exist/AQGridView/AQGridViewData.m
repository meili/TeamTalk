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

#import "AQGridViewData.h"
#import "AQGridView.h"

@interface AQGridViewData (AQGridViewDataPrivate)
- (void) fixDesiredCellSizeForWidth: (CGFloat) width;
@end

@implementation AQGridViewData

@synthesize reorderedIndex=_reorderedIndex, numberOfItems=_numberOfItems, topPadding=_topPadding, bottomPadding=_bottomPadding, leftPadding=_leftPadding, rightPadding=_rightPadding, layoutDirection=_layoutDirection;

- (id) initWithGridView: (AQGridView *) gridView
{
	self = [super init];
	if ( self == nil )
		return ( nil );
	
	_gridView = gridView;
	_boundsSize = gridView.bounds.size;
	
	return ( self );
}

- (id) copyWithZone: (NSZone *) zone
{
	AQGridViewData * theCopy = [[AQGridViewData allocWithZone: zone] initWithGridView: _gridView];
	theCopy->_desiredCellSize = _desiredCellSize;
	theCopy->_actualCellSize = _actualCellSize;
	theCopy->_layoutDirection = _layoutDirection;
	theCopy->_topPadding = _topPadding;
	theCopy->_bottomPadding = _bottomPadding;
	theCopy->_leftPadding = _leftPadding;
	theCopy->_rightPadding = _rightPadding;
	theCopy->_numberOfItems = _numberOfItems;
	theCopy->_reorderedIndex = _reorderedIndex;
	return ( theCopy );
}

- (id) mutableCopyWithZone: (NSZone *) zone
{
	return ( [self copyWithZone: zone] );
}

- (void) gridViewDidChangeBoundsSize: (CGSize) boundsSize
{
	_boundsSize = boundsSize;
	if ( _layoutDirection == AQGridViewLayoutDirectionVertical )
		[self fixDesiredCellSizeForWidth: boundsSize.width];
}

- (NSUInteger) itemIndexForPoint: (CGPoint) point
{
	// adjust for top padding
	point.y -= _topPadding;
	point.x -= _leftPadding;
	
	// get a count of all rows before the one containing the point
	NSUInteger y = (NSUInteger)floorf(point.y);
	NSUInteger row = y / (NSUInteger)_actualCellSize.height;
	
	// now column
	NSUInteger x = (NSUInteger)floorf(point.x);
	NSUInteger col = x / (NSUInteger)_actualCellSize.width;
	
	NSUInteger result = (row * [self numberOfItemsPerRow]) + col;
	if ( result >= self.numberOfItems )
		result = NSNotFound;
	
	return ( result );
}

- (BOOL) pointIsInLastRow: (CGPoint) point
{
	CGRect rect = [self rectForEntireGrid];
	if ( _layoutDirection == AQGridViewLayoutDirectionVertical )
		return ( point.y >= (rect.size.height - _actualCellSize.height) );
	
	// 'else'
	return ( point.x >= (rect.size.width - _actualCellSize.width) );
}

- (CGRect) cellRectForPoint: (CGPoint) point
{
	return ( [self cellRectAtIndex: [self itemIndexForPoint: point]] );
}

- (void) setDesiredCellSize: (CGSize) desiredCellSize
{
	_desiredCellSize = desiredCellSize;
	if ( _layoutDirection == AQGridViewLayoutDirectionVertical )
		[self fixDesiredCellSizeForWidth: _boundsSize.width];
	else
		_actualCellSize = _desiredCellSize;
}

- (void) setLayoutDirection: (AQGridViewLayoutDirection) direction
{
	if ( direction == AQGridViewLayoutDirectionVertical )
		[self fixDesiredCellSizeForWidth: _boundsSize.width];
	else
		_actualCellSize = _desiredCellSize;
	_layoutDirection = direction;
}

- (CGSize) cellSize
{
	return ( _actualCellSize );
}

- (CGRect) rectForEntireGrid
{
	CGRect rect;
	rect.origin.x = _leftPadding;
	rect.origin.y = _topPadding;
	rect.size = [self sizeForEntireGrid];
	return ( rect );
}

- (CGSize) sizeForEntireGrid
{
	NSUInteger numPerRow = [self numberOfItemsPerRow];
    if ( numPerRow == 0 )       // avoid a divide-by-zero exception
        return ( CGSizeZero );
	NSUInteger numRows = _numberOfItems / numPerRow;
	if ( _numberOfItems % numPerRow != 0 )
		numRows++;
	
	CGFloat height = ( ((CGFloat)ceilf((CGFloat)numRows * _actualCellSize.height)) + _topPadding + _bottomPadding );
	if (height < _gridView.bounds.size.height)
		height = _gridView.bounds.size.height;
	
	return ( CGSizeMake(((CGFloat)ceilf(_actualCellSize.width * numPerRow)) + _leftPadding + _rightPadding, height) );
}

- (NSUInteger) numberOfItemsPerRow
{
	if ( _layoutDirection == AQGridViewLayoutDirectionVertical )
		return ( (NSUInteger)floorf(_boundsSize.width / _actualCellSize.width) );
	
	// work out how many rows we can fit
	NSUInteger rows = (NSUInteger)floorf(_boundsSize.height / _actualCellSize.height);
	if (0 == rows) {
		rows = 1;
	}
	NSUInteger cols = _numberOfItems / rows;
	if ( _numberOfItems % rows != 0 )
		cols++;
	
	return ( cols );	
}

- (CGRect) cellRectAtIndex: (NSUInteger) index
{
	NSUInteger numPerRow = [self numberOfItemsPerRow];
    if ( numPerRow == 0 )       // avoid a divide-by-zero exception
        return ( CGRectZero );
	NSUInteger skipRows = index / numPerRow;
	NSUInteger skipCols = index % numPerRow;
	
	CGRect result = CGRectZero;
	result.origin.x = _actualCellSize.width * (CGFloat)skipCols + _leftPadding;
	result.origin.y = (_actualCellSize.height  * (CGFloat)skipRows) + _topPadding;
	result.size = _actualCellSize;
	
	return ( result );
}

- (NSIndexSet *) indicesOfCellsInRect: (CGRect) aRect
{
	NSMutableIndexSet * result = [NSMutableIndexSet indexSet];
	NSUInteger numPerRow = [self numberOfItemsPerRow];
	
	for ( NSUInteger i = 0; i < _numberOfItems; i++ )
	{
		CGRect cellRect = [self cellRectAtIndex: i];
		
        // numPerRow must be > 0 or you get an infinite loop
        // (i becomes 0 - 1 => UINT_MAX; loop UINT_MAX+1 => 0, etc.)
		if ( CGRectGetMaxY(cellRect) < CGRectGetMinY(aRect) && numPerRow > 0 )
		{
			// jump forward to the next row
			i += (numPerRow - 1);
			continue;
		}
		
		if ( CGRectIntersectsRect(cellRect, aRect) )
		{
			[result addIndex: i];
			if ( (CGRectGetMaxY(cellRect) > CGRectGetMaxY(aRect)) &&
				 (CGRectGetMaxX(cellRect) > CGRectGetMaxX(aRect)) )
			{
				// passed the bottom-right edge of the given rect
				break;
			}
		}
	}
	
	return ( result );
}

@end

@implementation AQGridViewData (AQGridViewDataPrivate)

- (void) fixDesiredCellSizeForWidth: (CGFloat) width
{
    // Much thanks to Brandon Sneed (@bsneed) for the following new algorithm, reduced to two floating-point divisions -- that's O(1) folks!
	CGFloat w = floorf(width - _leftPadding - _rightPadding);
	CGFloat dw = floorf(_desiredCellSize.width);
    CGFloat multiplier = floorf( w / dw );
	
	_actualCellSize.width = floorf( w / multiplier );
	_actualCellSize.height = _desiredCellSize.height;
}

@end
