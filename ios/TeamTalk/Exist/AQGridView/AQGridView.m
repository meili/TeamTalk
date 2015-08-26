/*
 * AQGridView.m
 * AQGridView
 *
 * Created by Jim Dovey on 10/2/2010.
 * Copyright 2010 Kobo Inc. All rights reserved.
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

#import "AQGridView.h"
#import "AQGridViewUpdateItem.h"
#import "AQGridViewAnimatorItem.h"
#import "AQGridViewData.h"
#import "AQGridViewUpdateInfo.h"
#import "AQGridViewCell+AQGridViewCellPrivate.h"
#import "AQGridView+CellLocationDelegation.h"
#import "NSIndexSet+AQIsSetContiguous.h"
#import "NSIndexSet+AQIndexesOutsideSet.h"

#import <libkern/OSAtomic.h>

// see _basicHitTest:withEvent: below
#import <objc/objc.h>
#import <objc/runtime.h>

// Lightweight object class for touch selection parameters
@interface UserSelectItemIndexParams : NSObject
{
  NSUInteger _indexNum;
  NSUInteger _numFingers;
};

@property (nonatomic, assign) NSUInteger indexNum;
@property (nonatomic, assign) NSUInteger numFingers;
@end

@implementation UserSelectItemIndexParams

@synthesize indexNum = _indexNum;
@synthesize numFingers = _numFingers;

@end


NSString * const AQGridViewSelectionDidChangeNotification = @"AQGridViewSelectionDidChangeNotification";

@interface AQGridView (AQCellGridMath)
- (NSUInteger) visibleCellListIndexForItemIndex: (NSUInteger) itemIndex;
@end

@interface AQGridView (AQCellLayout)
- (void) layoutCellsInVisibleCellRange: (NSRange) range;
- (void) layoutAllCells;
- (CGRect) fixCellFrame: (CGRect) cellFrame forGridRect: (CGRect) gridRect;
- (void) updateVisibleGridCellsNow;
//- (void) updateForwardCellsForVisibleIndices: (NSIndexSet *) newVisibleIndices;
- (AQGridViewCell *) createPreparedCellForIndex: (NSUInteger) index;
- (void) insertVisibleCell: (AQGridViewCell *) cell atIndex: (NSUInteger) visibleCellListIndex;
- (void) deleteVisibleCell: (AQGridViewCell *) cell atIndex: (NSUInteger) visibleCellListIndex appendingNewCell: (AQGridViewCell *) newLastCell;
@end

@interface AQGridView ()
@property (nonatomic, copy) NSIndexSet * animatingIndices;
@end


@implementation AQGridView

@synthesize dataSource=_dataSource, backgroundView=_backgroundView, separatorColor=_separatorColor, animatingCells=_animatingCells, animatingIndices=_animatingIndices;

- (void) _sharedGridViewInit
{
	_gridData = [[AQGridViewData alloc] initWithGridView: self];
	[_gridData setDesiredCellSize: CGSizeMake(96.0, 128.0)];

	_visibleBounds = self.bounds;
	_visibleCells = [[NSMutableArray alloc] init];
	_reusableGridCells = [[NSMutableDictionary alloc] init];
	_highlightedIndices = [[NSMutableIndexSet alloc] init];
	_updateInfoStack = [[NSMutableArray alloc] init];

	self.clipsToBounds = YES;
	self.separatorColor = [UIColor colorWithWhite: 0.85 alpha: 1.0];
    self.canCancelContentTouches = YES;

	_selectedIndex = NSNotFound;
	_pendingSelectionIndex = NSNotFound;

	_flags.resizesCellWidths = 0;
	_flags.numColumns = [_gridData numberOfItemsPerRow];
	_flags.separatorStyle = AQGridViewCellSeparatorStyleEmptySpace;
	_flags.allowsSelection = 1;
	_flags.usesPagedHorizontalScrolling = NO;
	_flags.contentSizeFillsBounds = 1;
}

- (id)initWithFrame: (CGRect) frame
{
    self = [super initWithFrame:frame];
	if ( self == nil )
		return ( nil );

	[self _sharedGridViewInit];

	return ( self );
}

- (id) initWithCoder: (NSCoder *) aDecoder
{
	self = [super initWithCoder: aDecoder];
	if ( self == nil )
		return ( nil );

	[self _sharedGridViewInit];

	return ( self );
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/


#pragma mark -
#pragma mark Properties

- (void) setDelegate: (id<AQGridViewDelegate>) obj
{
	if ( (obj != nil) && ([obj conformsToProtocol: @protocol(AQGridViewDelegate)] == NO ))
		[NSException raise: NSInvalidArgumentException format: @"Argument to -setDelegate must conform to the AQGridViewDelegate protocol"];
	[super setDelegate: obj];

	_flags.delegateWillDisplayCell = [obj respondsToSelector: @selector(gridView:willDisplayCell:forItemAtIndex:)];
	_flags.delegateWillSelectItem = [obj respondsToSelector: @selector(gridView:willSelectItemAtIndex:)];
  _flags.delegateWillSelectItemMultiTouch = [obj respondsToSelector: @selector(gridView:willSelectItemAtIndex:numFingersTouch:)];
	_flags.delegateWillDeselectItem = [obj respondsToSelector: @selector(gridView:willDeselectItemAtIndex:)];
	_flags.delegateDidSelectItem = [obj respondsToSelector: @selector(gridView:didSelectItemAtIndex:)];
  _flags.delegateDidSelectItemMultiTouch = [obj respondsToSelector: @selector(gridView:didSelectItemAtIndex:numFingersTouch:)];
	_flags.delegateDidDeselectItem = [obj respondsToSelector: @selector(gridView:didDeselectItemAtIndex:)];
	_flags.delegateGestureRecognizerActivated = [obj respondsToSelector: @selector(gridView:gestureRecognizer:activatedForItemAtIndex:)];
	_flags.delegateAdjustGridCellFrame = [obj respondsToSelector: @selector(gridView:adjustCellFrame:withinGridCellFrame:)];
	_flags.delegateDidEndUpdateAnimation = [obj respondsToSelector:@selector(gridViewDidEndUpdateAnimation:)];
}

- (id<AQGridViewDelegate>) delegate
{
	id obj = [super delegate];
	if ( [obj conformsToProtocol: @protocol(AQGridViewDelegate)] == NO )
		return ( nil );
	return ( obj );
}

- (void) setDataSource: (id<AQGridViewDataSource>) obj
{
	if ((obj != nil) && ([obj conformsToProtocol: @protocol(AQGridViewDataSource)] == NO ))
		[NSException raise: NSInvalidArgumentException format: @"Argument to -setDataSource must conform to the AQGridViewDataSource protocol"];

	_dataSource = obj;

	_flags.dataSourceGridCellSize = [obj respondsToSelector: @selector(portraitGridCellSizeForGridView:)];
}

- (AQGridViewLayoutDirection) layoutDirection
{
	return ( _gridData.layoutDirection );
}

- (void) setLayoutDirection: (AQGridViewLayoutDirection) direction
{
	_gridData.layoutDirection = direction;
}

- (NSUInteger) numberOfItems
{
	return ( _gridData.numberOfItems );
}

- (NSUInteger) numberOfColumns
{
	if ( _flags.numColumns == 0 )
		_flags.numColumns = 1;
	return ( _flags.numColumns );
}

- (NSUInteger) numberOfRows
{
	return ( _gridData.numberOfItems / _flags.numColumns );
}

- (BOOL) allowsSelection
{
	return ( _flags.allowsSelection );
}

- (void) setAllowsSelection: (BOOL) value
{
	_flags.allowsSelection = (value ? 1 : 0);
}

- (BOOL) backgroundViewExtendsDown
{
	return ( _flags.backgroundViewExtendsDown);
}

- (void) setBackgroundViewExtendsDown: (BOOL) value
{
	_flags.backgroundViewExtendsDown = (value ? 1 : 0);
}

- (BOOL) backgroundViewExtendsUp
{
	return ( _flags.backgroundViewExtendsUp);
}

- (void) setBackgroundViewExtendsUp: (BOOL) value
{
	_flags.backgroundViewExtendsUp = (value ? 1 : 0);
}

- (BOOL) requiresSelection
{
	return ( _flags.requiresSelection );
}

- (void) setRequiresSelection: (BOOL) value
{
	_flags.requiresSelection = (value ? 1 : 0);
}

- (BOOL) resizesCellWidthToFit
{
	return ( _flags.resizesCellWidths );
}

- (void) setResizesCellWidthToFit: (BOOL) value
{
	int i = (value ? 1 : 0);
	if ( _flags.resizesCellWidths == i )
		return;

	_flags.resizesCellWidths = i;
	[self setNeedsLayout];
}

- (BOOL) clipsContentWidthToBounds
{
	return ( self.layoutDirection == AQGridViewLayoutDirectionVertical );
}

- (void) setClipsContentWidthToBounds: (BOOL) value
{
	self.layoutDirection = (value ? AQGridViewLayoutDirectionVertical : AQGridViewLayoutDirectionHorizontal);
}

- (BOOL) usesPagedHorizontalScrolling
{
	return ( _flags.usesPagedHorizontalScrolling );
}

- (void) setUsesPagedHorizontalScrolling: (BOOL) value
{
	int i = (value ? 1 : 0);
	if ( _flags.usesPagedHorizontalScrolling == i )
		return;

	_flags.usesPagedHorizontalScrolling = i;
	[self setNeedsLayout];
}

- (AQGridViewCellSeparatorStyle) separatorStyle
{
	return ( _flags.separatorStyle );
}

- (void) setSeparatorStyle: (AQGridViewCellSeparatorStyle) style
{
	if ( style == _flags.separatorStyle )
		return;

	_flags.separatorStyle = style;

	for ( AQGridViewCell * cell in _visibleCells )
	{
		cell.separatorStyle = style;
	}

	[self setNeedsLayout];
}

- (CGFloat) leftContentInset
{
	return ( _gridData.leftPadding );
}

- (void) setLeftContentInset: (CGFloat) inset
{
	_gridData.leftPadding = inset;
}

- (CGFloat) rightContentInset
{
	return ( _gridData.rightPadding );
}

- (void) setRightContentInset: (CGFloat) inset
{
	_gridData.rightPadding = inset;
}

- (CGSize) gridCellSize
{
	return ( [_gridData cellSize] );
}

- (UIView *) gridHeaderView
{
	return ( _headerView );
}

- (void) setGridHeaderView: (UIView *) newHeaderView
{
	if ( newHeaderView == _headerView )
		return;

	[_headerView removeFromSuperview];

	_headerView = newHeaderView;
	if ( _headerView == nil )
	{
		_gridData.topPadding = 0.0;
	}
	else
	{
		[self addSubview: _headerView];
		_gridData.topPadding = _headerView.frame.size.height;
	}

	[self setNeedsLayout];
}

- (UIView *) gridFooterView
{
	return ( _footerView );
}

- (void) setGridFooterView: (UIView *) newFooterView
{
	if ( newFooterView == _footerView )
		return;

	[_footerView removeFromSuperview];

	_footerView = newFooterView;
	if ( _footerView == nil )
	{
		_gridData.bottomPadding = 0.0;
	}
	else
	{
		[self addSubview: _footerView];
		_gridData.bottomPadding = _footerView.frame.size.height;
	}

	[self setNeedsLayout];
}

- (BOOL) contentSizeGrowsToFillBounds
{
	return ( _flags.contentSizeFillsBounds == 1 );
}

- (void) setContentSizeGrowsToFillBounds: (BOOL) value
{
	_flags.contentSizeFillsBounds = (value ? 1 : 0);
}

- (void) setAnimatingCells: (NSSet *) set
{
	_animatingCells = set;

	NSMutableIndexSet * indices = [[NSMutableIndexSet alloc] init];
	for ( AQGridViewAnimatorItem * item in set )
	{
		if ( item.index != NSNotFound )
			[indices addIndex: item.index];
	}

	self.animatingIndices = indices;
}

- (BOOL) isAnimatingUpdates
{
    return ( _animationCount > 0 );
}

- (void) updateContentRectWithOldMaxLocation: (CGPoint) oldMaxLocation gridSize: (CGSize) gridSize
{
    // The following line prevents an update leading to unneccessary auto-scrolling
    // Before this fix, AQGridView animation always caused scrolling to the most bottom line
    if (CGSizeEqualToSize(self.contentSize, gridSize)) return;

	// update content size
	self.contentSize = gridSize;

	// fix content offset if applicable
	CGPoint offset = self.contentOffset;
	CGPoint oldOffset = offset;

	if ( offset.y + self.bounds.size.height > gridSize.height )
	{
		offset.y = MAX(0.0, self.contentSize.height - self.bounds.size.height);
	}
	else if ( !CGPointEqualToPoint(oldOffset, CGPointZero) )	// stick-to-top takes precedence
	{
		if ( [_gridData pointIsInLastRow: oldMaxLocation] )
		{
			// we were scrolled to the bottom-- stay there as our height decreases
			if ( self.layoutDirection == AQGridViewLayoutDirectionVertical )
				offset.y = MAX(0.0, self.contentSize.height - self.bounds.size.height);
			else
				offset.x = MAX(0.0, self.contentSize.width - self.bounds.size.width);
		}
	}

	//NSLog( @"Resetting offset from %@ to %@", NSStringFromCGPoint(oldOffset), NSStringFromCGPoint(offset) );
	self.contentOffset = offset;
}

- (void) handleGridViewBoundsChanged: (CGRect) oldBounds toNewBounds: (CGRect) bounds
{
	CGSize oldGridSize = [_gridData sizeForEntireGrid];
	BOOL wasAtBottom = ((oldGridSize.height != 0.0) && (CGRectGetMaxY(oldBounds) == oldGridSize.height));

	[_gridData gridViewDidChangeBoundsSize: bounds.size];
    _flags.numColumns = [_gridData numberOfItemsPerRow];
	CGSize newGridSize = [_gridData sizeForEntireGrid];

	CGPoint oldMaxLocation = CGPointMake(CGRectGetMaxX(oldBounds), CGRectGetMaxY(oldBounds));
	[self updateContentRectWithOldMaxLocation: oldMaxLocation gridSize: newGridSize];

	if ( (wasAtBottom) && (!CGPointEqualToPoint(oldBounds.origin, CGPointZero)) && (newGridSize.height > oldGridSize.height) )
	{
		CGRect contentRect = self.bounds;
		if ( CGRectGetMaxY(contentRect) < newGridSize.height )
		{
			contentRect.origin.y += (newGridSize.height - oldGridSize.height);
			self.contentOffset = contentRect.origin;
		}
	}

	[self updateVisibleGridCellsNow];
	_flags.allCellsNeedLayout = 1;
}

- (void) setContentOffset:(CGPoint) offset
{
	[super setContentOffset: offset];
}

- (void)setContentOffset: (CGPoint) contentOffset animated: (BOOL) animate
{
	// Call our super duper method
	[super setContentOffset: contentOffset animated: animate];

	// for long grids, ensure there are visible cells when scrolled to
	if (!animate)
	{
		[self updateVisibleGridCellsNow];
		/*if (![_visibleCells count])
		{
			NSIndexSet * newIndices = [_gridData indicesOfCellsInRect: [self gridViewVisibleBounds]];
			[self updateForwardCellsForVisibleIndices: newIndices];
		}*/
	}
}

- (void) setContentSize: (CGSize) newSize
{
	if ( (_flags.contentSizeFillsBounds == 1) && (newSize.height < self.bounds.size.height) )
		newSize.height = self.bounds.size.height;

	if (self.gridFooterView)
	{
	    // In-call status bar influences footer position
		CGRect statusRect = [UIApplication sharedApplication].statusBarFrame;
	    CGFloat statusHeight = MIN(CGRectGetWidth(statusRect), CGRectGetHeight(statusRect))  - 20;

	    CGFloat footerHeight = CGRectGetHeight(self.gridFooterView.bounds);
	    CGFloat minimumHeight = statusHeight + CGRectGetHeight(self.bounds) + footerHeight;
	    if (newSize.height < footerHeight + minimumHeight)
	        newSize.height = minimumHeight;
	}

	newSize.height = fmax(newSize.height, self.frame.size.height);

	CGSize oldSize = self.contentSize;
	[super setContentSize: newSize];

	if ( oldSize.width != newSize.width )
		[_gridData gridViewDidChangeBoundsSize: newSize];

	if ( CGRectGetMaxY(self.bounds) > newSize.height )
	{
		CGRect b = self.bounds;
		CGFloat diff = CGRectGetMaxY(b) - newSize.height;
		b.origin.y = MAX(0.0, b.origin.y - diff);
		self.bounds = b;
	}
}

- (void) setFrame: (CGRect) newFrame
{
	CGRect oldBounds = self.bounds;
	[super setFrame: newFrame];
	CGRect newBounds = self.bounds;

	if ( newBounds.size.width != oldBounds.size.width )
		[self handleGridViewBoundsChanged: oldBounds toNewBounds: newBounds];
}

- (void) setBounds: (CGRect) bounds
{
	CGRect oldBounds = self.bounds;
	[super setBounds: bounds];
	bounds = self.bounds;		// in case it was modified

	if ( !CGSizeEqualToSize(bounds.size, oldBounds.size) )
		[self handleGridViewBoundsChanged: oldBounds toNewBounds: bounds];
}

- (BOOL) isEditing
{
	return ( _flags.isEditing == 1 );
}

- (void) setEditing: (BOOL) value
{
	[self setEditing:value animated:NO];
}

#pragma mark -
#pragma mark Data Management

- (AQGridViewCell *) dequeueReusableCellWithIdentifier: (NSString *) reuseIdentifier
{
	NSMutableSet * cells = [_reusableGridCells objectForKey: reuseIdentifier];
	AQGridViewCell * cell = [cells anyObject];
	if ( cell == nil )
		return ( nil );

	[cell prepareForReuse];

	[cells removeObject: cell];
	return ( cell );
}

- (void) enqueueReusableCells: (NSArray *) reusableCells
{
	for ( AQGridViewCell * cell in reusableCells )
	{
		NSMutableSet * reuseSet = [_reusableGridCells objectForKey: cell.reuseIdentifier];
		if ( reuseSet == nil )
		{
			reuseSet = [[NSMutableSet alloc] initWithCapacity: 32];
			[_reusableGridCells setObject: reuseSet forKey: cell.reuseIdentifier];
		}
		else if ( [reuseSet member: cell] == cell )
		{
			NSLog( @"Warning: tried to add duplicate gridview cell" );
			continue;
		}

		[reuseSet addObject: cell];
	}
}

- (CGRect) gridViewVisibleBounds
{
	CGRect result = CGRectZero;
	result.origin = self.contentOffset;
	result.size   = self.bounds.size;
	return ( result );
}

- (void) reloadData
{
	if ( _reloadingSuspendedCount != 0 )
		return;

	if ( _flags.dataSourceGridCellSize == 1 )
	{
		[_gridData setDesiredCellSize: [_dataSource portraitGridCellSizeForGridView: self]];
		_flags.numColumns = [_gridData numberOfItemsPerRow];
	}

	_gridData.numberOfItems = [_dataSource numberOfItemsInGridView: self];

	// update our content size as appropriate
	self.contentSize = [_gridData sizeForEntireGrid];

    // fix up the visible index list
    NSUInteger cutoff = MAX(0, _gridData.numberOfItems-_visibleIndices.length);
    _visibleIndices.location = MIN(_visibleIndices.location, cutoff);
	_visibleIndices.length = 0;

	// remove all existing cells
	[_visibleCells makeObjectsPerformSelector: @selector(removeFromSuperview)];
	[self enqueueReusableCells: _visibleCells];
	[_visibleCells removeAllObjects];

	// -layoutSubviews will update the visible cell list

	// layout -- no animation
	[self setNeedsLayout];
	_flags.allCellsNeedLayout = 1;
}

#define MAX_BOUNCE_DISTANCE (500.0f)

- (void) layoutSubviews
{
	if ( (_flags.needsReload == 1) && (_animationCount == 0) && (_reloadingSuspendedCount == 0) )
		[self reloadData];

	if ( (_reloadingSuspendedCount == 0) && (!CGRectIsEmpty([self gridViewVisibleBounds])) )
	{
        [self updateVisibleGridCellsNow];
	}

	if ( _flags.allCellsNeedLayout == 1 )
	{
		_flags.allCellsNeedLayout = 0;
		if ( _visibleIndices.length != 0 )
			[self layoutAllCells];
	}

	CGRect rect = CGRectZero;
	rect.size.width = self.bounds.size.width;
	rect.size.height = self.contentSize.height -  (_gridData.topPadding + _gridData.bottomPadding);
	rect.origin.y += _gridData.topPadding;

	// Make sure background is an integral number of rows tall. That way, it draws patterned colours correctly on all OSes.
	CGRect backgroundRect = rect;

	if ([self backgroundViewExtendsUp]) {
		backgroundRect.origin.y = backgroundRect.origin.y - MAX_BOUNCE_DISTANCE;
		backgroundRect.size.height += MAX_BOUNCE_DISTANCE;	// don't just move it, grow it
	}

	if ([self backgroundViewExtendsDown]) {
		backgroundRect.size.height = backgroundRect.size.height + MAX_BOUNCE_DISTANCE;
	}

	CGFloat minimumHeight = rect.size.height,
		actualHeight = 0;

	if (([_gridData numberOfItems] == 0) || ([_gridData numberOfItemsPerRow] == 0)) {

		actualHeight = [_gridData cellSize].height;

	} else {

		actualHeight = [_gridData cellSize].height * ([_gridData numberOfItems] / [_gridData numberOfItemsPerRow] + 1);

	}
	for (; actualHeight < minimumHeight; actualHeight += [_gridData cellSize].height) {
	}
	backgroundRect.size.height = actualHeight;


	self.backgroundView.frame = backgroundRect;

	if ( _headerView != nil )
	{
		rect = _headerView.frame;
		rect.origin = CGPointZero;
		rect.size.width = self.bounds.size.width;
		_headerView.frame = rect;
	}

	if ( _footerView != nil )
	{
		rect = _footerView.frame;
		rect.origin.x = 0.0;
		rect.origin.y  = self.contentSize.height - rect.size.height;
		rect.size.width = self.bounds.size.width;
		_footerView.frame = rect;
		[self bringSubviewToFront:_footerView];
	}
}

- (CGRect) rectForItemAtIndex: (NSUInteger) index
{
	// simple case -- there's a cell already, we can just ask for its frame
	if ( NSLocationInRange(index, _visibleIndices) )
		return ( [[_visibleCells objectAtIndex: [self visibleCellListIndexForItemIndex: index]] frame] );

	// complex case-- compute the frame manually
	return ( [self fixCellFrame: CGRectZero forGridRect: [_gridData cellRectAtIndex: index]] );
}

- (AQGridViewCell *) cellForItemAtIndex: (NSUInteger) index
{
	//if ( NSLocationInRange(index, _visibleIndices) == NO )
	//	return ( nil );

	// we don't clip to visible range-- when animating edits the visible cell list can contain extra items
	NSUInteger visibleCellListIndex = [self visibleCellListIndexForItemIndex: index];
	if ( visibleCellListIndex < [_visibleCells count] )
		return ( [_visibleCells objectAtIndex: visibleCellListIndex] );
	return ( nil );
}

- (NSUInteger) indexForItemAtPoint: (CGPoint) point
{
	return ( [_gridData itemIndexForPoint: point] );
}

- (NSUInteger) indexForCell: (AQGridViewCell *) cell
{
	NSUInteger index = [_visibleCells indexOfObject:cell];
	if (index == NSNotFound)
		return NSNotFound;

	return _visibleIndices.location + index;
}

- (AQGridViewCell *) cellForItemAtPoint: (CGPoint) point
{
	return ( [self cellForItemAtIndex: [_gridData itemIndexForPoint: point]] );
}

- (NSArray *) visibleCells
{
	return ( [_visibleCells copy] );
}

- (NSIndexSet *) visibleCellIndices
{
	return ( [NSIndexSet indexSetWithIndexesInRange: _visibleIndices] );
}

- (void) scrollToItemAtIndex: (NSUInteger) index atScrollPosition: (AQGridViewScrollPosition) scrollPosition
					animated: (BOOL) animated
{
	CGRect gridRect = [_gridData cellRectAtIndex: index];
	CGRect targetRect = self.bounds;

	switch ( scrollPosition )
	{
		case AQGridViewScrollPositionNone:
		default:
			targetRect = gridRect;		// no special coordinate handling
			break;

		case AQGridViewScrollPositionTop:
			targetRect.origin.y = gridRect.origin.y;	// set target y origin to cell's y origin
			break;

		case AQGridViewScrollPositionMiddle:
			targetRect.origin.y = MAX(gridRect.origin.y - (CGFloat)ceilf((targetRect.size.height - gridRect.size.height) * 0.5), 0.0);
			break;

		case AQGridViewScrollPositionBottom:
			targetRect.origin.y = MAX((CGFloat)floorf(gridRect.origin.y - (targetRect.size.height - gridRect.size.height)), 0.0);
			break;
	}

	[self scrollRectToVisible: targetRect animated: animated];

	// for long grids, ensure there are visible cells when scrolled to
	if (!animated) {
		[self updateVisibleGridCellsNow];
		/*if (![_visibleCells count]) {
			NSIndexSet * newIndices = [_gridData indicesOfCellsInRect: [self gridViewVisibleBounds]];
			[self updateForwardCellsForVisibleIndices: newIndices];
		}*/
	}
}

#pragma mark -
#pragma mark Cell Updates

- (BOOL) isRectVisible: (CGRect) frameRect
{
	return ( CGRectIntersectsRect(frameRect, self.bounds) );
}

- (void) fixCellsFromAnimation
{
	// the visible cell list might contain hidden cells-- make them visible now
	for ( AQGridViewCell * cell in _visibleCells )
	{
		if ( cell.hiddenForAnimation )
		{
			cell.hiddenForAnimation = NO;

			if ( _flags.delegateWillDisplayCell == 1 )
				[self delegateWillDisplayCell: cell atIndex: cell.displayIndex];

			cell.hidden = NO;
		}
	}

	// update the visible item list appropriately
	NSIndexSet * indices = [_gridData indicesOfCellsInRect: self.bounds];
	if ( [indices count] == 0 )
	{
		_visibleIndices.location = 0;
		_visibleIndices.length = 0;

		[_visibleCells makeObjectsPerformSelector: @selector(removeFromSuperview)];
		[self enqueueReusableCells: _visibleCells];
		[_visibleCells removeAllObjects];

		// update the content size/offset based on the new grid data
		CGPoint oldMaxLocation = CGPointMake(CGRectGetMaxX(self.bounds), CGRectGetMaxY(self.bounds));
		[self updateContentRectWithOldMaxLocation: oldMaxLocation gridSize: [_gridData sizeForEntireGrid]];
		return;
	}

	_visibleIndices.location = [indices firstIndex];
	_visibleIndices.length = [indices count];

	NSMutableArray * newVisibleCells = [[NSMutableArray alloc] initWithCapacity: _visibleIndices.length];
	for ( AQGridViewAnimatorItem * item in self.animatingCells )
	{
		if ( [item.animatingView isKindOfClass: [AQGridViewCell class]] == NO )
		{
			[item.animatingView removeFromSuperview];
			continue;
		}

		if ( [self isRectVisible: [_gridData cellRectForPoint: item.animatingView.center]] == NO )
		{
			[item.animatingView removeFromSuperview];
			continue;
		}

		[newVisibleCells addObject: item.animatingView];
	}

	//NSAssert([newVisibleCells count] == _visibleIndices.length, @"visible cell count after animation doesn't match visible indices");

	[newVisibleCells sortUsingSelector: @selector(compareOriginAgainstCell:)];
	[_visibleCells removeObjectsInArray: newVisibleCells];
	[_visibleCells makeObjectsPerformSelector: @selector(removeFromSuperview)];
	[_visibleCells setArray: newVisibleCells];
	self.animatingCells = nil;

	NSMutableSet * removals = [[NSMutableSet alloc] init];
	for ( UIView * view in self.subviews )
	{
		if ( [view isKindOfClass: [AQGridViewCell class]] == NO )
			continue;

		if ( [_visibleCells containsObject: view] == NO )
			[removals addObject: view];
	}

	[removals makeObjectsPerformSelector: @selector(removeFromSuperview)];

	// update the content size/offset based on the new grid data
	CGPoint oldMaxLocation = CGPointMake(CGRectGetMaxX(self.bounds), CGRectGetMaxY(self.bounds));
	[self updateContentRectWithOldMaxLocation: oldMaxLocation gridSize: [_gridData sizeForEntireGrid]];
}

- (void) setupUpdateAnimations
{
	_reloadingSuspendedCount++;

	AQGridViewUpdateInfo * info = [[AQGridViewUpdateInfo alloc] initWithOldGridData: _gridData forGridView: self];
	[_updateInfoStack addObject: info];
}

- (void) endUpdateAnimations
{
	NSAssert([_updateInfoStack lastObject] != nil, @"_updateInfoStack should not be empty at this point" );
    
	__block AQGridViewUpdateInfo * info = [_updateInfoStack lastObject];

	if ( info.numberOfUpdates == 0 )
	{
		[_updateInfoStack removeObject: info];
		_reloadingSuspendedCount--;
		return;
	}

	NSUInteger expectedItemCount = [info numberOfItemsAfterUpdates];
	NSUInteger actualItemCount = [_dataSource numberOfItemsInGridView: self];
	if ( expectedItemCount != actualItemCount )
	{
		NSUInteger numAdded = [[info sortedInsertItems] count];
		NSUInteger numDeleted = [[info sortedDeleteItems] count];

		[_updateInfoStack removeObject: info];
		_reloadingSuspendedCount--;

		[NSException raise: NSInternalInconsistencyException format: @"Invalid number of items in AQGridView: Started with %u, added %u, deleted %u. Expected %u items after changes, but got %u", (unsigned)_gridData.numberOfItems, (unsigned)numAdded, (unsigned)numDeleted, (unsigned)expectedItemCount, (unsigned)actualItemCount];
	}

	[info cleanupUpdateItems];
	_animationCount++;
	//NSAssert(_animationCount == 1, @"Stacked animations occurring!!");
    
    [UIView animateWithDuration:0.3 delay:0 options:UIViewAnimationOptionCurveEaseInOut
                     animations:^(void) {
                         self.animatingCells = [info animateCellUpdatesUsingVisibleContentRect: [self gridViewVisibleBounds]];
                         
                         
                         _gridData = [info newGridViewData];
                         if ( _selectedIndex != NSNotFound )
                             _selectedIndex = [info newIndexForOldIndex: _selectedIndex];
                         
                         _reloadingSuspendedCount--;
                     }
                     completion:^(BOOL finished) {
                         // if nothing was animated, we don't have to do anything at all
                         //	if ( self.animatingCells.count != 0 )
                         [self fixCellsFromAnimation];
                         
                         // NB: info becomes invalid at this point
                         [_updateInfoStack removeObject: info];
                         _animationCount--;
                         
                         //_reloadingSuspendedCount--;
                         if ( _flags.delegateDidEndUpdateAnimation == 1 )
                             [self.delegate gridViewDidEndUpdateAnimation: self];
                     }];
}

- (void) beginUpdates
{
	if ( _updateCount++ == 0 )
		[self setupUpdateAnimations];
}

- (void) endUpdates
{
	if ( --_updateCount == 0 )
		[self endUpdateAnimations];
}

- (void) _updateItemsAtIndices: (NSIndexSet *) indices updateAction: (AQGridViewUpdateAction) action withAnimation: (AQGridViewItemAnimation) animation
{
	BOOL needsAnimationSetup = ([_updateInfoStack count] <= _animationCount);

	// not in the middle of an update loop -- start animations here
	if ( needsAnimationSetup )
		[self setupUpdateAnimations];

	[[_updateInfoStack lastObject] updateItemsAtIndices: indices updateAction: action withAnimation: animation];

	// not in the middle of an update loop -- commit animations here
	if ( needsAnimationSetup )
		[self endUpdateAnimations];
}

- (void) insertItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation
{
	[self _updateItemsAtIndices: indices updateAction: AQGridViewUpdateActionInsert withAnimation: animation];
}

- (void) deleteItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation
{
	[self _updateItemsAtIndices: indices updateAction: AQGridViewUpdateActionDelete withAnimation: animation];
}

- (void) reloadItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation
{
	[self _updateItemsAtIndices: indices updateAction: AQGridViewUpdateActionReload withAnimation: animation];
}

- (void) moveItemAtIndex: (NSUInteger) index toIndex: (NSUInteger) newIndex withAnimation: (AQGridViewItemAnimation) animation
{
	BOOL needsAnimationSetup = ([_updateInfoStack count] <= _animationCount);

	if ( needsAnimationSetup )
		[self setupUpdateAnimations];

	[[_updateInfoStack lastObject] moveItemAtIndex: index toIndex: newIndex withAnimation: animation];

	if ( needsAnimationSetup )
		[self endUpdateAnimations];
}

- (void)setEditing:(BOOL)editing animated:(BOOL)animated
{
	_flags.isEditing = (editing ? 1 : 0);

	NSArray *visibleCells = [self visibleCells];
	for (AQGridViewCell *aCell in visibleCells) {
		[aCell setEditing:editing animated:animated];
	}
}

#pragma mark -
#pragma mark Selection

- (NSUInteger) indexOfSelectedItem
{
	return ( _selectedIndex );
}

- (void) highlightItemAtIndex: (NSUInteger) index animated: (BOOL) animated scrollPosition: (AQGridViewScrollPosition) position
{
	if ( [_highlightedIndices containsIndex: index] )
	{
		if ( position != AQGridViewScrollPositionNone )
			[self scrollToItemAtIndex: index atScrollPosition: position animated: animated];
		return;
	}

	if ( index == NSNotFound )
	{
		NSUInteger i = [_highlightedIndices firstIndex];
		while ( i != NSNotFound )
		{
			AQGridViewCell * cell = [self cellForItemAtIndex: i];
			[cell setHighlighted: NO animated: animated];
			i = [_highlightedIndices indexGreaterThanIndex: i];
		}

		[_highlightedIndices removeAllIndexes];
		return;
	}

	AQGridViewCell * cell = [self cellForItemAtIndex: index];
	[cell setHighlighted: YES animated: animated];
	[_highlightedIndices addIndex: index];

	if ( position != AQGridViewScrollPositionNone )
		[self scrollToItemAtIndex: index atScrollPosition: position animated: animated];
}

- (void) unhighlightItemAtIndex: (NSUInteger) index animated: (BOOL) animated
{
	if ( [_highlightedIndices containsIndex: index] == NO )
		return;

	[_highlightedIndices removeIndex: index];

	// don't remove highlighting if the cell is actually the selected cell
	if ( index == _selectedIndex )
		return;

	AQGridViewCell * cell = [self cellForItemAtIndex: index];
	if ( cell != nil )
		[cell setHighlighted: NO animated: animated];
}

- (void) _deselectItemAtIndex: (NSUInteger) index animated: (BOOL) animated notifyDelegate: (BOOL) notifyDelegate
{
	if ( _selectedIndex != index )
		return;

	if ( notifyDelegate && _flags.delegateWillDeselectItem )
		[self.delegate gridView: self willDeselectItemAtIndex: index];

	_selectedIndex = NSNotFound;
	[[self cellForItemAtIndex: index] setSelected: NO animated: animated];

	if ( notifyDelegate && _flags.delegateDidDeselectItem )
		[self.delegate gridView: self didDeselectItemAtIndex: index];

	if ( notifyDelegate )
	{
		[[NSNotificationCenter defaultCenter] postNotificationName: AQGridViewSelectionDidChangeNotification
															object: self];
	}
}

- (void) _selectItemAtIndex: (NSUInteger) index animated: (BOOL) animated
			 scrollPosition: (AQGridViewScrollPosition) position notifyDelegate: (BOOL) notifyDelegate
       numFingersTouch: (NSUInteger) numFingers
{
	if ( _selectedIndex == index )
		return;		// already selected this item

	if ( _selectedIndex != NSNotFound )
		[self _deselectItemAtIndex: _selectedIndex animated: animated notifyDelegate: notifyDelegate];

	if ( _flags.allowsSelection == 0 )
		return;

	if ( notifyDelegate && _flags.delegateWillSelectItem )
		index = [self.delegate gridView: self willSelectItemAtIndex: index];

  if ( notifyDelegate && _flags.delegateWillSelectItemMultiTouch )
		index = [self.delegate gridView: self willSelectItemAtIndex: index
                    numFingersTouch:numFingers];

	_selectedIndex = index;
	[[self cellForItemAtIndex: index] setSelected: YES animated: animated];

	if ( position != AQGridViewScrollPositionNone )
		[self scrollToItemAtIndex: index atScrollPosition: position animated: animated];

	if ( notifyDelegate )
	{
		[[NSNotificationCenter defaultCenter] postNotificationName: AQGridViewSelectionDidChangeNotification
															object: self];
	}

	if ( notifyDelegate && _flags.delegateDidSelectItem )
		[self.delegate gridView: self didSelectItemAtIndex: index];

  if ( notifyDelegate && _flags.delegateDidSelectItemMultiTouch )
		[self.delegate gridView: self didSelectItemAtIndex: index numFingersTouch:numFingers];

	// ensure that the selected item is no longer marked as just 'highlighted' (that's an intermediary state)
	[_highlightedIndices removeIndex: index];
}

- (void) selectItemAtIndex: (NSUInteger) index animated: (BOOL) animated
			scrollPosition: (AQGridViewScrollPosition) scrollPosition
{
	[self _selectItemAtIndex: index animated: animated scrollPosition: scrollPosition notifyDelegate: NO
           numFingersTouch: 1];
}

- (void) deselectItemAtIndex: (NSUInteger) index animated: (BOOL) animated
{
	[self _deselectItemAtIndex: index animated: animated notifyDelegate: NO];
}

#pragma mark -
#pragma mark Appearance

- (UIView *) backgroundView
{
	return ( _backgroundView );
}

- (void) setBackgroundView: (UIView *) newView
{
	if ( newView == _backgroundView )
		return;

	[_backgroundView removeFromSuperview];

	_backgroundView = newView;
	_backgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
	CGRect frame = self.bounds;
	frame.size = self.contentSize;

	CGRect backgroundRect = CGRectMake(0.0f, 0.0f, self.bounds.size.width, self.bounds.size.height);

	if ([self backgroundViewExtendsUp]) {
		backgroundRect.origin.y = backgroundRect.origin.y - MAX_BOUNCE_DISTANCE;
		backgroundRect.size.height += MAX_BOUNCE_DISTANCE;		// don't just move it, grow it
	}

	if ([self backgroundViewExtendsDown]) {
		backgroundRect.size.height = backgroundRect.size.height + MAX_BOUNCE_DISTANCE;
	}

	_backgroundView.frame = backgroundRect;

	[self insertSubview: _backgroundView atIndex: 0];

	// this view is already laid out nicely-- no need to call -setNeedsLayout at all
}

- (UIColor *) separatorColor
{
	return ( _separatorColor );
}

- (void) setSeparatorColor: (UIColor *) color
{
	if ( color == _separatorColor )
		return;

	_separatorColor = color;

	for ( AQGridViewCell * cell in _visibleCells )
	{
		cell.separatorColor = _separatorColor;
	}
}

#pragma mark -
#pragma mark Touch Events

- (UIView *) _basicHitTest: (CGPoint) point withEvent: (UIEvent *) event
{
	// STUPID STUPID RAT CREATURES
	// ===========================
	//
	// Problem: we want to do a default hit-test without UIScrollView's processing getting in the way.
	// UIScrollView implements _defaultHitTest:withEvent: for this, but we can't call that due to it
	//  being a private API.
	// Instead, we have to manufacture a call to our super-super class here, grr
	Method method = class_getInstanceMethod( [UIView class], @selector(hitTest:withEvent:) );
	IMP imp = method_getImplementation( method );
	return ( (UIView *)imp(self, @selector(hitTest:withEvent:), point, event) ); // -[UIView hitTest:withEvent:]
}

- (BOOL) _canSelectItemContainingHitView: (UIView *) hitView
{
	if ( [hitView isKindOfClass: [UIControl class]] )
		return ( NO );


//	Simply querying the superview will not work if the hit view is a subview of the contentView, e.g. its superview is a plain UIView *inside* a cell

	if ( [[hitView superview] isKindOfClass: [AQGridViewCell class]] )
		return ( YES );

	if ( [hitView isKindOfClass: [AQGridViewCell class]] )
		return ( YES );

	CGPoint hitCenter = [self convertPoint:[hitView center] fromView:hitView];

	for ( AQGridViewCell *aCell in [[self visibleCells] copy])
	{

		if ( CGRectContainsPoint( aCell.frame, hitCenter ) )
		return ( YES );

	}

	return ( NO );
}

- (void) _gridViewDeferredTouchesBegan: (NSNumber *) indexNum
{
	if ( (self.dragging == NO) && (_flags.ignoreTouchSelect == 0) && (_pendingSelectionIndex != NSNotFound) )
		[self highlightItemAtIndex: _pendingSelectionIndex animated: NO scrollPosition: AQGridViewScrollPositionNone];
	//_pendingSelectionIndex = NSNotFound;
}

- (void) _userSelectItemAtIndex: (UserSelectItemIndexParams*) params
{
	NSUInteger index = params.indexNum;
  NSUInteger numFingersCount = params.numFingers;
	[self unhighlightItemAtIndex: index animated: NO];
	if ( ([[self cellForItemAtIndex: index] isSelected]) && (self.requiresSelection == NO) )
		[self _deselectItemAtIndex: index animated: NO notifyDelegate: YES];
	else
		[self _selectItemAtIndex: index animated: NO scrollPosition: AQGridViewScrollPositionNone notifyDelegate: YES
             numFingersTouch: numFingersCount];
	_pendingSelectionIndex = NSNotFound;
}

- (BOOL) _gestureRecognizerIsHandlingTouches: (NSSet *) touches
{
	// see if the touch is (possibly) being tracked by a gesture recognizer
	for ( UIGestureRecognizer *recognizer in self.gestureRecognizers )
	{
		switch ( [recognizer state] )
		{
			case UIGestureRecognizerStateEnded:
			case UIGestureRecognizerStateCancelled:
			case UIGestureRecognizerStateFailed:
				continue;

			default:
				break;
		}

		if ( [recognizer numberOfTouches] == [touches count] )
		{
			// simple version:
			// pick a touch from our event's set, and see if it's in the recognizer's set
			UITouch * touch = [touches anyObject];
			CGPoint touchLocation = [touch locationInView: self];

			for ( NSUInteger i = 0; i < [recognizer numberOfTouches]; i++ )
			{
				CGPoint test = [recognizer locationOfTouch: i inView: self];
				if ( CGPointEqualToPoint(test, touchLocation) )
				{
					return ( YES );
				}
			}
		}
	}

	return ( NO );
}

- (void) touchesBegan: (NSSet *) touches withEvent: (UIEvent *) event
{
	_flags.ignoreTouchSelect = ([self isDragging] ? 1 : 0);

	UITouch * touch = [touches anyObject];
	_touchBeganPosition = [touch locationInView: nil];
	if ( (touch != nil) && (_pendingSelectionIndex == NSNotFound) )
	{
		CGPoint pt = [touch locationInView: self];
		UIView * hitView = [self _basicHitTest: pt withEvent: event];
		_touchedContentView = hitView;

		// unhighlight anything not here
		if ( hitView != self )
			[self highlightItemAtIndex: NSNotFound animated: NO scrollPosition: AQGridViewScrollPositionNone];

		if ( [self _canSelectItemContainingHitView: hitView] )
		{
			NSUInteger index = [self indexForItemAtPoint: pt];
			if ( index != NSNotFound )
			{
				if ( _flags.allowsSelection == 1 )
				{
					_pendingSelectionIndex = index;

					// NB: In UITableView:
					// if ( [self usesGestureRecognizers] && [self isDragging] ) skip next line
					[self performSelector: @selector(_gridViewDeferredTouchesBegan:)
							   withObject: [NSNumber numberWithUnsignedInteger: index]
							   afterDelay: 0.0];
				}
			}
		}
	}

	[super touchesBegan: touches withEvent: event];
}
/*
- (void) _cancelContentTouchUsingEvent: (UIEvent *) event forced: (BOOL) forced
{
	static char * name = "_cancelContentTouchWithEvent:forced:";

	// more manual ObjC runtime calls...
	SEL selector = sel_getUid( name );
	objc_msgSend( self, selector, event, forced );
}
*/
- (void) touchesMoved: (NSSet *) touches withEvent: (UIEvent *) event
{
	if ( _flags.ignoreTouchSelect == 0 )
	{
		Class cls = NSClassFromString(@"UILongPressGestureRecognizer");
		if ( (cls != Nil) && ([cls instancesRespondToSelector: @selector(setNumberOfTouchesRequired:)]) )
		{
			if ( [self _gestureRecognizerIsHandlingTouches: touches] )
				goto passToSuper;			// I feel all icky now
		}

		//[self _cancelContentTouchUsingEvent: event forced: NO];
		[self highlightItemAtIndex: NSNotFound animated: NO scrollPosition: AQGridViewScrollPositionNone];
		_flags.ignoreTouchSelect = 1;
		_touchedContentView = nil;
	}

passToSuper:
	[super touchesMoved: touches withEvent: event];
}

- (void) touchesEnded: (NSSet *) touches withEvent: (UIEvent *) event
{
  [[self class] cancelPreviousPerformRequestsWithTarget: self
												 selector: @selector(_gridViewDeferredTouchesBegan:)
												   object: nil];

	UIView * hitView = _touchedContentView;
	_touchedContentView = nil;

	[super touchesEnded: touches withEvent: event];
	if ( _touchedContentView != nil )
	{
		hitView = _touchedContentView;
	}

	if ( [hitView superview] == nil )
	{
		hitView = nil;
	}

	// poor-man's goto
	do
	{
		if ( self.dragging )
			break;

		UITouch * touch = [touches anyObject];
		if ( touch == nil )
			break;

		CGPoint pt = [touch locationInView: self];
		if ( (hitView != nil) && ([self _canSelectItemContainingHitView: hitView] == NO) )
			break;

		if ( _pendingSelectionIndex != [self indexForItemAtPoint: pt] )
			break;

		if ( _flags.allowsSelection == 0 )
			break;

    NSSet *touchEventSet = [event allTouches];

		// run this on the next runloop tick
    UserSelectItemIndexParams* selectorParams = [[UserSelectItemIndexParams alloc] init];
    selectorParams.indexNum = _pendingSelectionIndex;
    selectorParams.numFingers = [touchEventSet count];
		[self performSelector: @selector(_userSelectItemAtIndex:)
				   withObject: selectorParams
           afterDelay:0.0];


	} while (0);

	if ( _pendingSelectionIndex != NSNotFound )
		[self unhighlightItemAtIndex: _pendingSelectionIndex animated: NO];
	_pendingSelectionIndex = NSNotFound;
}

- (void) touchesCancelled: (NSSet *) touches withEvent: (UIEvent *) event
{
    _pendingSelectionIndex = NSNotFound;
    [self highlightItemAtIndex: NSNotFound animated: NO scrollPosition: AQGridViewScrollPositionNone];
    [super touchesCancelled: touches withEvent: event];
    
    _touchedContentView = nil;
}

- (void)doAddVisibleCell: (UIView *)cell
{
	[_visibleCells addObject: cell];
	// updated: if we're adding it to our visibleCells collection, really it should be in the gridview.
	if ( cell.superview == nil )
	{
		NSLog( @"Visible cell not in gridview - adding" );
		if ( _backgroundView.superview == self )
			[self insertSubview: cell aboveSubview: _backgroundView];
		else
			[self insertSubview: cell atIndex: 0];
	}
}


@end

#pragma mark -

@implementation AQGridView (AQCellGridMath)

- (NSUInteger) visibleCellListIndexForItemIndex: (NSUInteger) itemIndex
{
	return ( itemIndex - _visibleIndices.location );
}

@end

#pragma mark -

@implementation AQGridView (AQCellLayout)

NSArray * __sortDescriptors;

- (void) sortVisibleCellList
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
		__sortDescriptors = [[NSArray alloc] initWithObjects: [[NSSortDescriptor alloc] initWithKey: @"displayIndex" ascending: YES], nil];
    });

	[_visibleCells sortUsingDescriptors: __sortDescriptors];
}

- (void) updateGridViewBoundsForNewGridData: (AQGridViewData *) newGridData
{
	CGPoint oldMaxLocation = CGPointMake(CGRectGetMaxX(self.bounds), CGRectGetMaxY(self.bounds));
	[self updateContentRectWithOldMaxLocation: oldMaxLocation gridSize: [newGridData sizeForEntireGrid]];
}

- (void) updateVisibleGridCellsNow
{
	if ( _reloadingSuspendedCount > 0 )
		return;

	_reloadingSuspendedCount++;

    @autoreleasepool {
        
        NSIndexSet * newVisibleIndices = [_gridData indicesOfCellsInRect: [self gridViewVisibleBounds]];
        
        BOOL enableAnim = [UIView areAnimationsEnabled];
        [UIView setAnimationsEnabled: NO];
        
        @try
        {
            // a couple of simple tests
            // TODO: if we replace _visibleIndices with an index set, this comparison will have to change
            if ( ([_visibleCells count] != [newVisibleIndices count]) ||
                ([newVisibleIndices countOfIndexesInRange: _visibleIndices] != _visibleIndices.length) )
            {
                // something has changed. Compute intersections and remove/add cells as required
                NSIndexSet * currentVisibleIndices = [NSIndexSet indexSetWithIndexesInRange: _visibleIndices];
                
                // index sets for removed and inserted items
                NSMutableIndexSet * removedIndices = nil, * insertedIndices = nil;
                
                // handle the simple case first
                // TODO: if we replace _visibleIndices with an index set, this comparison will have to change
                if ( [currentVisibleIndices intersectsIndexesInRange: _visibleIndices] == NO )
                {
                    removedIndices = [currentVisibleIndices mutableCopy];
                    insertedIndices = [newVisibleIndices mutableCopy];
                }
                else	// more complicated -- compute negative intersections
                {
                    removedIndices = [[currentVisibleIndices aq_indexesOutsideIndexSet: newVisibleIndices] mutableCopy];
                    insertedIndices = [[newVisibleIndices aq_indexesOutsideIndexSet: currentVisibleIndices] mutableCopy];
                }
                
                if ( [removedIndices count] != 0 )
                {
                    NSMutableIndexSet * shifted = [removedIndices mutableCopy];
                    
                    // get an index set for everything being removed relative to items' locations within the visible cell list
                    [shifted shiftIndexesStartingAtIndex: [removedIndices firstIndex] by: 0 - (NSInteger)_visibleIndices.location];
                    //NSLog( @"Removed indices relative to visible cell list: %@", shifted );
                    
                    NSUInteger index=[shifted firstIndex];
                    while(index != NSNotFound){
                        //NSLog(@"%i >= %i ?", index, [_visibleCells count]);
                        if (index >= [_visibleCells count]) {
                            [shifted removeIndex:index];
                        }
                        index=[shifted indexGreaterThanIndex: index];
                    }
                    
                    // pull out the cells for manipulation
                    NSMutableArray * removedCells = [[_visibleCells objectsAtIndexes: shifted] mutableCopy];
                    
                    // remove them from the visible list
                    [_visibleCells removeObjectsInArray: removedCells];
                    //NSLog( @"After removals, visible cells count = %lu", (unsigned long)[_visibleCells count] );
                    
                    // don't need this any more
                     shifted = nil;
                    
                    // remove cells from the view hierarchy -- but only if they're not being animated by something else
                    NSArray * animating = [[self.animatingCells valueForKey: @"animatingView"] allObjects];
                    if ( animating != nil )
                        [removedCells removeObjectsInArray: animating];
                    
                    // these are not being displayed or animated offscreen-- take them off the screen immediately
                    [removedCells makeObjectsPerformSelector: @selector(removeFromSuperview)];
                    
                    // put them into the cell reuse queue
                    [self enqueueReusableCells: removedCells];
                    
                }
                
                if ( [insertedIndices count] != 0 )
                {
                    // some items are going in -- put them at the end and the sort function will move them to the right index during layout
                    // if any of these new indices correspond to animating cells (NOT UIImageViews) then copy them into the visible cell list
                    NSMutableIndexSet * animatingInserted = [insertedIndices mutableCopy];
                    
                    // compute the intersection of insertedIndices and _animatingIndices
                    NSUInteger idx = [insertedIndices firstIndex];
                    while ( idx != NSNotFound )
                    {
                        if ( [_animatingIndices containsIndex: idx] == NO )
                            [animatingInserted removeIndex: idx];
                        
                        idx = [insertedIndices indexGreaterThanIndex: idx];
                    }
                    
                    if ( [animatingInserted count] != 0 )
                    {
                        for ( AQGridViewAnimatorItem * item in _animatingCells )
                        {
                            if ( [newVisibleIndices containsIndex: item.index] == NO )
                                continue;
                            
                            if ( [item.animatingView isKindOfClass: [AQGridViewCell class]] )
                            {
                                // ensure this is in the visible cell list
                                if ( [_visibleCells containsObject: item.animatingView] == NO )
                                    //[_visibleCells addObject: item.animatingView];
                                    [self doAddVisibleCell: item.animatingView];
                            }
                            else
                            {
                                // it's an image that's being moved, likely because it *was* going offscreen before
                                // the user scrolled. Create a real cell, but hide it until the animation is complete.
                                AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
                                //[_visibleCells addObject: cell];
                                [self doAddVisibleCell: cell];
                                
                                // we don't tell the delegate yet, we just hide it
                                cell.hiddenForAnimation = YES;
                            }
                        }
                        
                        // remove these from the set of indices for which we will generate new cells
                        [insertedIndices removeIndexes: animatingInserted];
                    }
                    
                    
                    // insert cells for these indices
                    idx = [insertedIndices firstIndex];
                    while ( idx != NSNotFound )
                    {
                        AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
                        //[_visibleCells addObject: cell];
                        [self doAddVisibleCell: cell];
                        
                        // tell the delegate
                        [self delegateWillDisplayCell: cell atIndex: idx];
                        
                        idx = [insertedIndices indexGreaterThanIndex: idx];
                    }
                }
                
                if ( [_visibleCells count] > [newVisibleIndices count] )
                {
                    //NSLog( @"Have to prune visible cell list, I've still got extra cells in there!" );
                    NSMutableIndexSet * animatingDestinationIndices = [[NSMutableIndexSet alloc] init];
                    for ( AQGridViewAnimatorItem * item in _animatingCells )
                    {
                        [animatingDestinationIndices addIndex: item.index];
                    }
                    
                    NSMutableIndexSet * toRemove = [[NSMutableIndexSet alloc] init];
                    NSMutableIndexSet * seen = [[NSMutableIndexSet alloc] init];
                    NSUInteger i, count = [_visibleCells count];
                    for ( i = 0; i < count; i++ )
                    {
                        AQGridViewCell * cell = [_visibleCells objectAtIndex: i];
                        if ( [newVisibleIndices containsIndex: cell.displayIndex] == NO &&
                            [animatingDestinationIndices containsIndex: cell.displayIndex] == NO )
                        {
                            NSLog( @"Cell for index %lu is still in visible list, removing...", (unsigned long)cell.displayIndex );
                            [cell removeFromSuperview];
                            [toRemove addIndex: i];
                        }
                        else if ( [seen containsIndex: cell.displayIndex] )
                        {
                            NSLog( @"Multiple cells with index %lu found-- removing duplicate...", (unsigned long)cell.displayIndex );
                            [cell removeFromSuperview];
                            [toRemove addIndex: i];
                        }
                        
                        [seen addIndex: cell.displayIndex];
                    }
                    
                    // all removed from superview, just need to remove from the list now
                    [_visibleCells removeObjectsAtIndexes: toRemove];
                }
                
                if ( [_visibleCells count] < [newVisibleIndices count] )
                {
                    NSLog( @"Visible cell list is missing some items!" );
                    
                    NSMutableIndexSet * visibleSet = [[NSMutableIndexSet alloc] init];
                    for ( AQGridViewCell * cell in _visibleCells )
                    {
                        [visibleSet addIndex: cell.displayIndex];
                    }
                    
                    NSMutableIndexSet * missingSet = [newVisibleIndices mutableCopy];
                    [missingSet removeIndexes: visibleSet];
                    
                    NSLog( @"Got %lu missing indices", (unsigned long)[missingSet count] );
                    
                    NSUInteger idx = [missingSet firstIndex];
                    while ( idx != NSNotFound )
                    {
                        AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
                        //[_visibleCells addObject: cell];
                        [self doAddVisibleCell: cell];
                        
                        // tell the delegate
                        [self delegateWillDisplayCell: cell atIndex: idx];
                        
                        idx = [missingSet indexGreaterThanIndex: idx];
                    }
                    
                }
                
                // everything should match up now, so update the visible range
                _visibleIndices.location = [newVisibleIndices firstIndex];
                _visibleIndices.length   = [newVisibleIndices count];
                
                // layout these cells -- this will also sort the visible cell list
                [self layoutAllCells];
            }
        }
        @catch (id exception)
        {
        }
        @finally
        {
            [UIView setAnimationsEnabled: enableAnim];
            _reloadingSuspendedCount--;
        }
        
    }
}
/*
- (void) updateVisibleGridCellsNow
{
	if ( _reloadingSuspendedCount > 0 )
		return;

	_reloadingSuspendedCount++;


	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSIndexSet * newVisibleIndices = [_gridData indicesOfCellsInRect: [self gridViewVisibleBounds]];

	BOOL enableAnim = [UIView areAnimationsEnabled];
	[UIView setAnimationsEnabled: NO];

	@try
	{
		// a couple of simple tests
		// TODO: if we replace _visibleIndices with an index set, this comparison will have to change
		if ( ([_visibleCells count] != [newVisibleIndices count]) ||
			([newVisibleIndices countOfIndexesInRange: _visibleIndices] != _visibleIndices.length) )
		{
			//NSLog( @"\n\n----------BEGIN CELL UPDATES----------" );

			// something has changed. Compute intersections and remove/add cells as required
			NSIndexSet * currentVisibleIndices = [NSIndexSet indexSetWithIndexesInRange: _visibleIndices];

			// index sets for removed and inserted items
			NSIndexSet * removedIndices = nil, * insertedIndices = nil;

			// handle the simple case first
			// TODO: if we replace _visibleIndices with an index set, this comparison will have to change
			if ( [currentVisibleIndices intersectsIndexesInRange: _visibleIndices] == NO )
			{
				removedIndices = currentVisibleIndices;
				insertedIndices = newVisibleIndices;
			}
			else	// more complicated -- compute negative intersections
			{
				removedIndices = [currentVisibleIndices aq_indexesOutsideIndexSet: newVisibleIndices];
				insertedIndices = [newVisibleIndices aq_indexesOutsideIndexSet: currentVisibleIndices];
			}

			//NSLog( @"Removing indices: %@, inserting indices: %@", removedIndices, insertedIndices );
			//NSLog( @"Visible cells count = %lu", (unsigned long)[_visibleCells count] );

			if ( ([removedIndices count] != 0) || ([insertedIndices count] != 0) )
			{
				if ( [removedIndices count] != 0 )
				{
					NSMutableIndexSet * shifted = [removedIndices mutableCopy];

					// get an index set for everything being removed relative to items' locations within the visible cell list
					[shifted shiftIndexesStartingAtIndex: [removedIndices firstIndex] by: 0 - (NSInteger)_visibleIndices.location];
					//NSLog( @"Removed indices relative to visible cell list: %@", shifted );

					// pull out the cells for manipulation
					NSArray * removedCells = [_visibleCells objectsAtIndexes: shifted];

					// remove them from the visible list
					[_visibleCells removeObjectsAtIndexes: shifted];
					//NSLog( @"After removals, visible cells count = %lu", (unsigned long)[_visibleCells count] );

					// don't need this any more
					[shifted release]; shifted = nil;

					// remove cells from the view hierarchy -- but only if they're not being animated by something else
					NSArray * animating = [[self.animatingCells valueForKey: @"animatingView"] allObjects];
					if ( [animating firstObjectCommonWithArray: removedCells] != nil )
					{
						NSMutableArray * mutable = [removedCells mutableCopy];
						[mutable removeObjectsInArray: animating];
						removedCells = [[mutable copy] autorelease];
						[mutable release];
					}

					//NSLog( @"Removing %lu cells from screen", (unsigned long)[removedCells count] );
					[removedCells makeObjectsPerformSelector: @selector(removeFromSuperview)];

					// put them into the cell reuse queue
					[self enqueueReusableCells: removedCells];
				}

				if ( [insertedIndices count] != 0 )
				{
					// now we do the insertions
					NSUInteger first = [newVisibleIndices firstIndex];
					NSLog( @"New starting index = %lu", (unsigned long)first );
					NSLog( @"Visible cell count = %lu", (unsigned long)[_visibleCells count] );

					// if there are cells being animated into place, skip them-- the animation has them already
					if ( self.animatingCells.count != 0 )
					{
						NSMutableIndexSet * tmp = [insertedIndices mutableCopy];
						for ( AQGridViewAnimatorItem * item in self.animatingCells )
						{
							// remove each animating item from the list we'll insert
							[tmp removeIndex: item.index];
						}

						insertedIndices = [[tmp copy] autorelease];
						[tmp release];
					}

					NSUInteger idx = [insertedIndices firstIndex];
					while ( idx != NSNotFound )
					{
						AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
						[self delegateWillDisplayCell: cell atIndex: idx];

						NSLog( @"Inserting cell for index %lu at visible list index %lu", (unsigned long)idx, (unsigned long)(idx - first) );
						[_visibleCells insertObject: cell atIndex: idx-first];

						idx = [insertedIndices indexGreaterThanIndex: idx];
					}
				}

				// sort the visible cell list so everything is in global-index order
				[self sortVisibleCellList];

				// remove anything from _visibleCells whose index isn't in the visible range
				NSMutableIndexSet * removeVisibleCells = [[NSMutableIndexSet alloc] init];
				NSUInteger i = 0;
				for ( AQGridViewCell * cell in _visibleCells )
				{
					if ( [newVisibleIndices containsIndex: cell.displayIndex] == NO )
						[removeVisibleCells addIndex: i];
					i++;
				}

				if ( [removeVisibleCells count] != 0 )
				{
					NSLog( @"Missed some cells which need to be pruned from the visible cell list: %@", removeVisibleCells );
					[_visibleCells removeObjectsAtIndexes: removeVisibleCells];
				}

				[removeVisibleCells release];

				if ( [_visibleCells count] < [newVisibleIndices count] )
				{
					// some items are missing-- stick them in

				}

				// this assertion is only valid if we have ownership of everything on screen
				// if an animation is going on, then some cells are owned by the animation list instead
				 if ( self.animatingCells.count == 0 )
					 NSAssert( [_visibleCells count] == [newVisibleIndices count], @"Cell count doesn't match what was expected" );

				// need to have _visibleIndices setup before calling layout
				_visibleIndices.location = [newVisibleIndices firstIndex];
				_visibleIndices.length = [newVisibleIndices count];

				// layout the new cells
				[self layoutCellsInVisibleCellRange: NSMakeRange(0, [_visibleCells count])];
			}

			//NSLog( @"----------END CELL UPDATES----------\n\n" );
		}
	}
	@finally
	{
		[UIView setAnimationsEnabled: enableAnim];
		[pool drain];
		_reloadingSuspendedCount--;
	}
}
*/
/*
- (void) updateVisibleGridCellsNow
{
	if ( _reloadingSuspendedCount > 0 )
		return;

	if ( _flags.isAnimatingUpdates || _flags.updating )
		return;

	_reloadingSuspendedCount++;
	NSIndexSet * newVisibleIndices = [_gridData indicesOfCellsInRect: [self gridViewVisibleBounds]];

	NSUInteger beforeTest = (_visibleIndices.location == 0 ? NSNotFound : _visibleIndices.location - 1);
	NSUInteger afterTest = MIN(_visibleIndices.location+_visibleIndices.length, _gridData.numberOfItems);

	//NSLog( @"New Visible Indices = %@, _visibleIndices = %@", newVisibleIndices, NSStringFromRange(_visibleIndices) );

	// do we need to remove anything?
	if ( [newVisibleIndices countOfIndexesInRange: _visibleIndices] < _visibleIndices.length )
	{
        NSMutableIndexSet * indicesToRemove = [[NSMutableIndexSet alloc] initWithIndexesInRange: _visibleIndices];
        [indicesToRemove removeIndexes: newVisibleIndices];
        if ( [indicesToRemove aq_isSetContiguous] )
        {
            // nice simple optimized version
            // front or back?
            BOOL removeFromFront = NO;
            if ( [indicesToRemove containsIndex: _visibleIndices.location] )
                removeFromFront = YES;

            NSUInteger numToRemove = [indicesToRemove count];
            NSRange arrayRange = {0, 0};
			if ( removeFromFront )
				arrayRange = NSMakeRange(0, numToRemove);
			else
				arrayRange = NSMakeRange([_visibleCells count] - numToRemove, numToRemove);

			//NSLog( @"Removing cells in visible range: %@", NSStringFromRange(arrayRange) );

			// grab the removed cells (retains them)
			NSMutableArray * removedCells = [[[_visibleCells subarrayWithRange: arrayRange] mutableCopy] autorelease];

			// don't remove cells which are animating right now
			if ( self.animatingCells.count != 0 )
			{
				[removedCells removeObjectsInArray: self.animatingCells];
				numToRemove = [removedCells count];
				arrayRange.length = numToRemove;
			}

			// remove from the visible list
			[_visibleCells removeObjectsInRange: arrayRange];

			// trim the visible cell index range
			_visibleIndices.length -= numToRemove;

			if ( removeFromFront )
				_visibleIndices.location += numToRemove;

			// remove cells from superview
			[removedCells makeObjectsPerformSelector: @selector(removeFromSuperview)];

			// put them into the recycled cell list
			[self enqueueReusableCells: removedCells];

			// done removing cells
        }
		else
		{
			// we need to be much more thorough-- a large number of items have been removed from all over
			NSMutableArray * removedCells = [[_visibleCells mutableCopy] autorelease];
			if ( self.animatingCells.count != 0 )
				[removedCells removeObjectsInArray: self.animatingCells];

			// remove any cells which aren't animating to new positions
			[_visibleCells removeObjectsInArray: removedCells];
			[removedCells makeObjectsPerformSelector: @selector(removeFromSuperview)];
			[self enqueueReusableCells: removedCells];

			// update visible indices as appropriate-- brute force this time
			_visibleIndices.location = [newVisibleIndices firstIndex];
			_visibleIndices.length = [newVisibleIndices count];

			// load the new cells
			NSUInteger idx = [newVisibleIndices firstIndex];
			while ( idx != NSNotFound )
			{
				AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
				[self delegateWillDisplayCell: cell atIndex: idx];
				[_visibleCells addObject: cell];
				idx = [newVisibleIndices indexGreaterThanIndex: idx];
			}

			[self layoutCellsInVisibleCellRange: NSMakeRange(0, [_visibleCells count])];

			// all done
		}

		[indicesToRemove release];
	}

	// no animations on automatic cell layout
	[UIView setAnimationsEnabled: NO];

	if ( (beforeTest != NSNotFound) && ([newVisibleIndices containsIndex: beforeTest]) )
	{
		// moving backwards
		NSMutableIndexSet * newIndices = [[newVisibleIndices mutableCopy] autorelease];

		// prune the ones we know about already, so we have a list of only the new ones
		[newIndices removeIndexesInRange: _visibleIndices];
		[newIndices removeIndexesInRange: _revealingIndices];

		// insert any new cells, in reverse order (so we always insert at index zero)
		NSUInteger idx = [newIndices lastIndex];
		while ( idx != NSNotFound )
		{
			AQGridViewCell * cell = [self createPreparedCellForIndex: idx];
			[self delegateWillDisplayCell: cell atIndex: idx];
			[_visibleCells insertObject: cell atIndex: _revealingIndices.length];

			idx = [newIndices indexLessThanIndex: idx];
		}

		// update the visibleCell index range
		_visibleIndices.length += [newIndices count];
		_visibleIndices.location = [newVisibleIndices firstIndex];

		// get the range of the new items
		NSRange newCellRange = NSMakeRange([newIndices firstIndex], [newIndices lastIndex] - [newIndices firstIndex] + 1);

		// map this range onto the current visible cell array
		newCellRange.location = MIN(newCellRange.location - _visibleIndices.location, 0);

		// now update their locations
		[self layoutCellsInVisibleCellRange: newCellRange];
	}
	else if ( (NSLocationInRange(afterTest, _visibleIndices) == NO) && ([newVisibleIndices containsIndex: afterTest]) )
	{
		[self updateForwardCellsForVisibleIndices: newVisibleIndices];
	}

	[UIView setAnimationsEnabled: YES];
	_reloadingSuspendedCount--;
}
*/

- (void) layoutCellsInVisibleCellRange: (NSRange) range
{
	NSParameterAssert(range.location + range.length <= [_visibleCells count]);

	@autoreleasepool {

		NSArray * layoutList = [_visibleCells subarrayWithRange: range];
		for ( AQGridViewCell * cell in layoutList )
		{
			if ( [_animatingIndices containsIndex: cell.displayIndex] )
				continue;		// don't adjust layout of something that is animating around

			CGRect gridRect = [_gridData cellRectAtIndex: cell.displayIndex];
			CGRect cellFrame = cell.frame;

			cell.frame = [self fixCellFrame: cellFrame forGridRect: gridRect];
			cell.selected = (cell.displayIndex == _selectedIndex);
		}

	}
}

- (void) layoutAllCells
{
	[self sortVisibleCellList];

	NSRange range = NSMakeRange(0, _visibleIndices.length);
	[self layoutCellsInVisibleCellRange: range];
}

- (CGRect) fixCellFrame: (CGRect) cellFrame forGridRect: (CGRect) gridRect
{
	if ( _flags.resizesCellWidths == 1 )
	{
		cellFrame = gridRect;
	}
	else
	{
		if ( cellFrame.size.width > gridRect.size.width )
			cellFrame.size.width = gridRect.size.width;
		if ( cellFrame.size.height > gridRect.size.height )
			cellFrame.size.height = gridRect.size.height;
		cellFrame.origin.x = gridRect.origin.x + floorf( (gridRect.size.width - cellFrame.size.width) * 0.5 );
		cellFrame.origin.y = gridRect.origin.y + floorf( (gridRect.size.height - cellFrame.size.height) * 0.5 );
	}

	// let the delegate update it if appropriate
	if ( _flags.delegateAdjustGridCellFrame )
		cellFrame = [self.delegate gridView: self adjustCellFrame: cellFrame withinGridCellFrame: gridRect];

	return ( cellFrame );
}

- (AQGridViewCell *) createPreparedCellForIndex: (NSUInteger) index usingGridData: (AQGridViewData *) gridData
{
	[UIView setAnimationsEnabled: NO];
	AQGridViewCell * cell = [_dataSource gridView: self cellForItemAtIndex: index];
	cell.separatorStyle = _flags.separatorStyle;
	cell.editing = self.editing;
	cell.displayIndex = index;

	cell.frame = [self fixCellFrame: cell.frame forGridRect: [gridData cellRectAtIndex: index]];
	if ( _backgroundView.superview == self )
		[self insertSubview: cell aboveSubview: _backgroundView];
	else
		[self insertSubview: cell atIndex: 0];
    [UIView setAnimationsEnabled: YES];

	return ( cell );
}

- (AQGridViewCell *) createPreparedCellForIndex: (NSUInteger) index
{
    return ( [self createPreparedCellForIndex: index usingGridData: _gridData] );
}

- (void) insertVisibleCell: (AQGridViewCell *) cell atIndex: (NSUInteger) visibleCellListIndex
{
	if ( visibleCellListIndex >= [_visibleCells count] )
		return;

	[_visibleCells insertObject: cell atIndex: visibleCellListIndex];
}

- (void) deleteVisibleCell: (AQGridViewCell *) cell atIndex: (NSUInteger) visibleCellListIndex appendingNewCell: (AQGridViewCell *) newCell
{
	if ( visibleCellListIndex >= [_visibleCells count] )
		return;

	[_visibleCells removeObjectAtIndex: visibleCellListIndex];
	//[_visibleCells addObject: newCell];
	[self doAddVisibleCell: newCell];
}

- (void) ensureCellInVisibleList: (AQGridViewCell *) cell
{
	if ( [_visibleCells containsObject: cell] == NO )
		//[_visibleCells addObject: cell];
		[self doAddVisibleCell: cell];
	[_visibleCells sortUsingSelector: @selector(compareOriginAgainstCell:)];
}

@end

@implementation AQGridView (AQGridViewPrivate)

- (void) viewWillRotateToInterfaceOrientation: (UIInterfaceOrientation) orientation
{
	// to avoid cell pop-in or pop-out:
	// if we're switching to landscape, don't update cells until after the transition.
	// if we're switching to portrait, update cells first.
	//if ( UIInterfaceOrientationIsLandscape(orientation) )
	//	_reloadingSuspendedCount++;
}

- (void) viewDidRotate
{
	if ( _reloadingSuspendedCount == 0 )
		return;

	if ( --_reloadingSuspendedCount == 0 )
		[self updateVisibleGridCellsNow];
}

@end

@implementation AQGridView (CellLocationDelegation)

- (void) delegateWillDisplayCell: (AQGridViewCell *) cell atIndex: (NSUInteger) index
{
	if ( cell.separatorStyle == AQGridViewCellSeparatorStyleSingleLine )
	{
		// determine which edges need a separator
		AQGridViewCellSeparatorEdge edge = 0;
		if ( (index % self.numberOfColumns) != self.numberOfColumns-1 )
		{
			edge |= AQGridViewCellSeparatorEdgeRight;
		}
		//if ( index <= (_gridData.numberOfItems - self.numberOfColumns) )
		{
			edge |= AQGridViewCellSeparatorEdgeBottom;
		}

		cell.separatorEdge = edge;
	}

    //NSLog( @"Displaying cell at index %lu", (unsigned long) index );

	if ( _flags.delegateWillDisplayCell == 0 )
		return;

	[self.delegate gridView: self willDisplayCell: cell forItemAtIndex: index];
}

@end
