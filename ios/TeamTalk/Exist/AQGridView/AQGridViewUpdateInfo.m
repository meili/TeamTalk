/*
 * AQGridViewUpdateInfo.m
 * AQGridView
 * 
 * Created by Jim Dovey on 3/3/2010.
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

#import "AQGridViewUpdateInfo.h"
#import "AQGridViewData.h"
#import "AQGridView+CellLayout.h"
#import "AQGridView+CellLocationDelegation.h"
#import "AQGridViewCell+AQGridViewCellPrivate.h"
#import "AQGridViewAnimatorItem.h"
#import <UIKit/UIView.h>
#import <QuartzCore/CALayer.h>

@implementation AQGridViewUpdateInfo

- (id) initWithOldGridData: (AQGridViewData *) oldGridData forGridView: (AQGridView *) gridView
{
	self = [super init];
	if ( self == nil )
		return ( nil );
	
	_insertItems = [[NSMutableArray alloc] init];
	_deleteItems = [[NSMutableArray alloc] init];
	_moveItems   = [[NSMutableArray alloc] init];
	_reloadItems = [[NSMutableArray alloc] init];
	
	_oldGridData = [oldGridData copy];
	_newGridData = [oldGridData copy];
	
	_gridView = gridView;
	
	return ( self );
}

- (void) dealloc
{
	if ( _oldToNewIndexMap != NULL )
		free( _oldToNewIndexMap );
	if ( _newToOldIndexMap != NULL )
		free( _newToOldIndexMap );
}

- (NSMutableArray *) updateItemArrayForAction: (AQGridViewUpdateAction) action
{
	switch ( action )
	{
		case AQGridViewUpdateActionInsert:
			return ( _insertItems );
			
		case AQGridViewUpdateActionDelete:
			return ( _deleteItems );
			
		case AQGridViewUpdateActionMove:
			return ( _moveItems );
			
		case AQGridViewUpdateActionReload:
			return ( _reloadItems );
			
		default:
			break;
	}
	
	return ( nil );
}

- (void) updateItemsAtIndices: (NSIndexSet *) indices
				 updateAction: (AQGridViewUpdateAction) action
				withAnimation: (AQGridViewItemAnimation) animation
{
	NSMutableArray * array = [self updateItemArrayForAction: action];
	NSUInteger i = [indices firstIndex];
	while ( i != NSNotFound )
	{
		AQGridViewUpdateItem * item = [[AQGridViewUpdateItem alloc] initWithIndex: i
																		   action: action
																		animation: animation];
		[array addObject: item];
		
		i = [indices indexGreaterThanIndex: i];
	}
}

- (void) moveItemAtIndex: (NSUInteger) index
				 toIndex: (NSUInteger) newIndex
		   withAnimation: (AQGridViewItemAnimation) animation
{
	NSMutableArray * array = [self updateItemArrayForAction: AQGridViewUpdateActionMove];
	AQGridViewUpdateItem * item = [[AQGridViewUpdateItem alloc] initWithIndex: index
																	   action: AQGridViewUpdateActionMove
																	animation: animation];
	item.newIndex = newIndex;
	[array addObject: item];
}

- (NSUInteger) numberOfUpdates
{
	return ( [_insertItems count] + [_deleteItems count] + [_moveItems count] + [_reloadItems count] );
}

- (void) updateNewGridDataAndCreateMappingTables
{
#define GUARD_ITEMS 1
#if GUARD_ITEMS
# define TEST_GUARD(array,count)                                                    \
    for ( int j = 0; j < 8; j++ )                                                   \
    {                                                                               \
        NSAssert((array)[(count)+j] == 0x55555555, @"Overwrote the guard area!" );  \
    }                                                                               \
    do {} while (0)
#else
# define TEST_GUARD(array,count)
#endif
    
	NSUInteger numberOfItems = _oldGridData.numberOfItems;
	numberOfItems += [_insertItems count];
	numberOfItems -= [_deleteItems count];
	
	_newGridData.numberOfItems = numberOfItems;
	
	NSArray * sortedInserts = [_insertItems sortedArrayUsingSelector: @selector(compare:)];
	NSArray * sortedDeletes = [_deleteItems sortedArrayUsingSelector: @selector(compare:)];
	
	NSMutableIndexSet * oldToNewIndices = [[NSMutableIndexSet alloc] initWithIndexesInRange: NSMakeRange(0, _oldGridData.numberOfItems)];
	NSMutableIndexSet * newToOldIndices = [[NSMutableIndexSet alloc] initWithIndexesInRange: NSMakeRange(0, _newGridData.numberOfItems)];
	
	// Shift indices based on insertions/deletions
	for ( AQGridViewUpdateItem * item in sortedInserts )
	{
		[oldToNewIndices shiftIndexesStartingAtIndex: item.originalIndex by: 1];
		[newToOldIndices shiftIndexesStartingAtIndex: item.originalIndex by: -1];
	}
	
	for ( AQGridViewUpdateItem * item in sortedDeletes )
	{
		[newToOldIndices shiftIndexesStartingAtIndex: item.originalIndex by: 1];
	}
	
	NSUInteger stamp = NSNotFound;
	
	if ( _oldGridData.numberOfItems > 0 )
	{
#if GUARD_ITEMS
		NSUInteger count = _oldGridData.numberOfItems + 8;
#else
		NSUInteger count = _oldGridData.numberOfItems;
#endif
		_oldToNewIndexMap = malloc( count * sizeof(NSUInteger) );
#if GUARD_ITEMS
		memset(_oldToNewIndexMap, 0x55, count * sizeof(NSUInteger));
#endif
		memset_pattern4( _oldToNewIndexMap, &stamp, _oldGridData.numberOfItems * sizeof(NSUInteger) );
#if GUARD_ITEMS
		NSAssert(_oldToNewIndexMap[_oldGridData.numberOfItems] == 0x55555555, @"Eeek! Scribbling on guards didn't work!");
#endif
	}
	else
	{
		_oldToNewIndexMap = NULL;		// won't be used, no old indices
	}
	
	if ( _newGridData.numberOfItems > 0 )
	{
#if GUARD_ITEMS
		NSUInteger count = _newGridData.numberOfItems + 8;
#else
		NSUInteger count = _newGridData.numberOfItems;
#endif
		_newToOldIndexMap = malloc( count * sizeof(NSUInteger) );
#if GUARD_ITEMS
		memset(_newToOldIndexMap, 0x55, count * sizeof(NSUInteger));
#endif
		memset_pattern4( _newToOldIndexMap, &stamp, _newGridData.numberOfItems * sizeof(NSUInteger) );
#if GUARD_ITEMS
		NSAssert(_newToOldIndexMap[_newGridData.numberOfItems] == 0x55555555, @"Eeek! Scribbling on guards didn't work!");
#endif
	}
	else
	{
		_newToOldIndexMap = NULL;
	}
	
	// create map contents from our indices
	if ( _oldToNewIndexMap != NULL )
	{
		// set mappings
		NSUInteger idx = [oldToNewIndices firstIndex];
		for ( NSUInteger i = 0; i < _oldGridData.numberOfItems && idx != NSNotFound; i++ )
		{
			if ( [newToOldIndices containsIndex: i] == NO )
			{
				_oldToNewIndexMap[i] = NSNotFound;
                TEST_GUARD(_oldToNewIndexMap, _oldGridData.numberOfItems);
				continue;
			}
			
			_oldToNewIndexMap[i] = idx;
			idx = [oldToNewIndices indexGreaterThanIndex: idx];
		}
		
		for ( AQGridViewUpdateItem * item in _moveItems )
		{
			_oldToNewIndexMap[item.index] = item.newIndex;
            TEST_GUARD(_oldToNewIndexMap, _oldGridData.numberOfItems);
            
			if ( _moveItems.count == 1 )
			{
				if ( item.index < item.newIndex )
				{
					// moving forwards-- shuffle middle items down one place
					for ( NSInteger i = item.index+1; i <= item.newIndex && i < _oldGridData.numberOfItems; i++ )
					{
						if ( _oldToNewIndexMap[i] != NSNotFound )
                        {
                            if ( i < _oldGridData.numberOfItems-1 )
                            {
                                _oldToNewIndexMap[i] = _oldToNewIndexMap[i]-1;
                                TEST_GUARD(_oldToNewIndexMap, _oldGridData.numberOfItems);
                            }
                        }
                        else
                        {
                            break;      // stop when we reach a gap
                        }
					}
				}
				else if ( item.index > item.newIndex )
				{
					// moving backwards-- shuffle middle items up one place
					for ( NSInteger i = MIN(item.index-1, (_oldGridData.numberOfItems-1)); i >= item.newIndex; i-- )
					{
						if ( _oldToNewIndexMap[i] != NSNotFound )
                        {
                            if ( i >= 0 )
                            {
                                _oldToNewIndexMap[i] = _oldToNewIndexMap[i]+1;
                                TEST_GUARD(_oldToNewIndexMap, _oldGridData.numberOfItems);
                            }
                        }
                        else
                        {
                            break;      // stop when we reach a gap
                        }
					}
				}
			}
		}
	}
	
	if ( _newToOldIndexMap != NULL )
	{
		NSUInteger idx = [newToOldIndices firstIndex];
		for ( NSUInteger i = 0; i < _newGridData.numberOfItems && idx != NSNotFound; i++ )
		{
			if ( [oldToNewIndices containsIndex: i] == NO )
			{
				_newToOldIndexMap[i] = NSNotFound;
                TEST_GUARD(_newToOldIndexMap, _newGridData.numberOfItems);
				continue;
			}
			
			_newToOldIndexMap[i] = idx;
			idx = [newToOldIndices indexGreaterThanIndex: idx];
		}
		
		for ( AQGridViewUpdateItem * item in _moveItems )
		{
			_newToOldIndexMap[item.newIndex] = item.index;
            TEST_GUARD(_newToOldIndexMap, _newGridData.numberOfItems);
            
			if ( _moveItems.count == 1 )
			{
				if ( item.index < item.newIndex )
				{
					// moving forwards-- shuffle middle items down one place
					for ( NSInteger i = item.index; i <= item.newIndex && i < _newGridData.numberOfItems; i++ )
					{
						if ( _newToOldIndexMap[i] != NSNotFound )
                        {
                            if ( i < _newGridData.numberOfItems-1 )
                            {
                                _newToOldIndexMap[i] = _newToOldIndexMap[i]+1;
                                TEST_GUARD(_newToOldIndexMap, _newGridData.numberOfItems);
                            }
                        }
                        else
                        {
                            break;      // stop when we reach a gap
                        }
					}
				}
				else
				{
					// moving backwards-- shuffle middle items up one place
					for ( NSInteger i = MIN(item.newIndex, (_newGridData.numberOfItems-1)); (i < item.index && i < _newGridData.numberOfItems); i++ )
					{
						if ( _newToOldIndexMap[i] != NSNotFound )
                        {
                            if ( i >= 0 )
                            {
                                _newToOldIndexMap[i] = _newToOldIndexMap[i]-1;
                                TEST_GUARD(_newToOldIndexMap, _newGridData.numberOfItems);
                            }
                        }
                        else
                        {
                            break;      // stop when we reach a gap
                        }
					}
				}
			}
		}
	}
	
}

- (void) cleanupUpdateItems
{
	// sort the lists in ascending order
	[_insertItems sortUsingSelector: @selector(inverseCompare:)];
	[_deleteItems sortUsingSelector: @selector(inverseCompare:)];
	[_moveItems sortUsingSelector: @selector(inverseCompare:)];
	[_reloadItems sortUsingSelector: @selector(inverseCompare:)];
	
	// _deleteItems will be processed first, in reverse order, so we don't need to modify that particular array
	// we do however need to modify the others based on the contents of the delete list
	
	// step one: get a list of all indices to be deleted
	_insertedIndices = [[NSMutableIndexSet alloc] init];
	_reloadedIndices = [[NSMutableIndexSet alloc] init];
	_deletedIndices  = [[NSMutableIndexSet alloc] init];
	_oldMovedIndices = [[NSMutableIndexSet alloc] init];
	_newMovedIndices = [[NSMutableIndexSet alloc] init];
	
	for ( AQGridViewUpdateItem * item in _deleteItems )
	{
		[_deletedIndices addIndex: item.index];
	}
	
	for ( AQGridViewUpdateItem * item in _moveItems )
	{
		[_oldMovedIndices addIndex: item.index];
		[_newMovedIndices addIndex: item.newIndex];
	}
	
	// create a range to query the delete indices
	
	// now update insertItems by appropriate offsets
	NSRange range = NSMakeRange(0, 0);
	for ( AQGridViewUpdateItem * item in _insertItems )
	{
		[_insertedIndices addIndex: item.index];
		
		range.length = item.index;
		// decrement index by number of deleted items prior to this insertion
		NSUInteger numIndices = [_deletedIndices countOfIndexesInRange: range];
		if ( numIndices != 0 )
		{
			// set the item's index offset
			item.offset = -((NSInteger)numIndices);
		}
	}
	
	// now update reloadItems by delete offsets
	for ( AQGridViewUpdateItem * item in _reloadItems )
	{
		[_reloadedIndices addIndex: item.index];
		
		range.length = item.index;
		NSUInteger numIndices = [_deletedIndices countOfIndexesInRange: range];
		if ( numIndices != 0 )
		{
			item.offset = -((NSInteger)numIndices);
		}
	}
	
	// now update reloadItems by insert offsets
	for ( AQGridViewUpdateItem * item in _reloadItems )
	{
		range.length = item.index;		// modified index
		NSUInteger numIndices = [_insertedIndices countOfIndexesInRange: range];
		if ( numIndices == 0 )
			break;		// none left
		
		item.offset = item.offset + numIndices;
		
		[_reloadedIndices addIndex: item.index];
	}
	
	// all indices are now consistent for the assumed implementation order of delete, insert, reload
	// update the new grid data to match the insertions/deletions now
	[self updateNewGridDataAndCreateMappingTables];
}

- (NSUInteger) newIndexForOldIndex: (NSUInteger) oldIndex
{
	if ( _oldToNewIndexMap == NULL )
		return ( oldIndex );
	
	return ( _oldToNewIndexMap[oldIndex] );
}

- (NSArray *) sortedInsertItems
{
	return ( [_insertItems copy] );
}

- (NSArray *) sortedDeleteItems
{
	return ( [_deleteItems copy] );
}

- (NSArray *) sortedMoveItems
{
	return ( [_moveItems copy] );
}

- (NSArray *) sortedReloadItems
{
	return ( [_reloadItems copy] );
}

- (AQGridViewData *) newGridViewData
{
	return ( _newGridData );
}

- (NSUInteger) numberOfItemsAfterUpdates
{
	return ( _newGridData.numberOfItems + [_insertItems count] - [_deleteItems count] );
}

- (UIImageView *) _imageViewForView: (UIView *) view
{
    UIGraphicsBeginImageContextWithOptions(view.bounds.size, NO, [[UIScreen mainScreen] scale]);
	[view.layer renderInContext: UIGraphicsGetCurrentContext()];
	
	UIImageView * result = [[UIImageView alloc] initWithImage: UIGraphicsGetImageFromCurrentImageContext()];
	
	UIGraphicsEndImageContext();
	
	return ( result );
}

- (UIImageView *) animateDeletionForCell: (AQGridViewCell *) cell withAnimation: (AQGridViewItemAnimation) animation
{
	if ( animation == AQGridViewItemAnimationNone )
	{
		[cell removeFromSuperview];
		return ( nil );
	}
	
	[UIView setAnimationsEnabled: NO];
	
	UIImageView * imageView = [self _imageViewForView: cell];
	imageView.frame = cell.frame;
	CGSize cellSize = cell.frame.size;
	[_animatingCells addObject: imageView];
	
	// swap 'em around
	// image view goes underneath all real cells
	if ( _gridView.backgroundView != nil )
		[_gridView insertSubview: imageView aboveSubview: _gridView.backgroundView];
	else
		[_gridView insertSubview: imageView atIndex: 0];
	[cell removeFromSuperview];
	
	[UIView setAnimationsEnabled: YES];
    
    // fade is implicit
    imageView.alpha = 0.0;
	
	// this is what we'll animate
	switch ( animation )
	{
		case AQGridViewItemAnimationFade:
			// nothing else left for the fade animation
			break;
			
		case AQGridViewItemAnimationRight:
		{
			CGPoint center = imageView.center;
			center.x += cellSize.width;
			imageView.center = center;
			break;
		}
			
		case AQGridViewItemAnimationLeft:
		{
			CGPoint center = imageView.center;
			center.x -= cellSize.width;
			imageView.center = center;
			break;
		}
			
		case AQGridViewItemAnimationTop:
		{
			CGPoint center = imageView.center;
			center.y -= cellSize.height;
			imageView.center = center;
			break;
		}
			
		case AQGridViewItemAnimationBottom:
		{
			CGPoint center = imageView.center;
			center.y += cellSize.height;
			imageView.center = center;
			break;
		}
			
		default:
			break;
	}
	
	return ( imageView );
}

- (void) animateInsertionForCell: (AQGridViewCell *) cell withAnimation: (AQGridViewItemAnimation) animation
{
	[UIView setAnimationsEnabled: NO];
	[_gridView addSubview: cell];
	[UIView setAnimationsEnabled: YES];
	
	if ( animation == AQGridViewItemAnimationNone )
		return;
	
	// this is what we'll animate
	NSMutableDictionary * itemsToSetBeforeAnimation = [[NSMutableDictionary alloc] init];
	NSMutableDictionary * itemsToAnimate = [[NSMutableDictionary alloc] init];
	
	CGSize cellSize = cell.frame.size;
    
    [itemsToSetBeforeAnimation setObject: [NSNumber numberWithFloat: 0.0] forKey: @"alpha"];
    [itemsToAnimate setObject: [NSNumber numberWithFloat: 1.0] forKey: @"alpha"];
	
	switch ( animation )
	{
		case AQGridViewItemAnimationFade:
		{
            // nothing left to do-- fade is implicit
			break;
		}
			
		case AQGridViewItemAnimationRight:
		{
			CGPoint center = cell.center;
			[itemsToAnimate setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			center.x += cellSize.width;
			[itemsToSetBeforeAnimation setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			break;
		}
			
		case AQGridViewItemAnimationLeft:
		{
			CGPoint center = cell.center;
			[itemsToAnimate setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			center.x -= cellSize.width;
			[itemsToSetBeforeAnimation setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			break;
		}
			
		case AQGridViewItemAnimationTop:
		{
			CGPoint center = cell.center;
			[itemsToAnimate setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			center.y -= cellSize.height;
			[itemsToSetBeforeAnimation setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			break;
		}
			
		case AQGridViewItemAnimationBottom:
		{
			CGPoint center = cell.center;
			[itemsToAnimate setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			center.y += cellSize.height;
			[itemsToSetBeforeAnimation setObject: [NSValue valueWithCGPoint: center] forKey: @"center"];
			break;
		}
			
		default:
			break;
	}
	
	[UIView setAnimationsEnabled: NO];
	for ( NSString * keyPath in itemsToSetBeforeAnimation )
	{
		[cell setValue: [itemsToSetBeforeAnimation objectForKey: keyPath] forKey: keyPath];
	}
	[UIView setAnimationsEnabled: YES];
	
	for ( NSString * keyPath in itemsToAnimate )
	{
		[cell setValue: [itemsToAnimate objectForKey: keyPath] forKey: keyPath];
	}
    
}

- (void) animateReloadForCell: (AQGridViewCell *) newCell originalCell: (AQGridViewCell *) originalCell withAnimation: (AQGridViewItemAnimation) animation
{
	if ( animation == AQGridViewItemAnimationNone )
	{
		// just remove the original cell
		[originalCell removeFromSuperview];
		return;
	}
	
	[UIView setAnimationsEnabled: NO];
	
	// get an image of the original cell to animate out
	UIImageView * imageView = [self _imageViewForView: originalCell];
	imageView.frame = originalCell.frame;
	CGSize cellSize = originalCell.frame.size;
	[_animatingCells addObject: imageView];
	
	// swap 'em aroundAQGrid
	// image view goes underneath all real cells
	if ( _gridView.backgroundView != nil )
		[_gridView insertSubview: imageView aboveSubview: _gridView.backgroundView];
	else
		[_gridView insertSubview: imageView atIndex: 0];
	[originalCell removeFromSuperview];
	
	CGRect cellStartFrame = imageView.frame;
	CGRect cellEndFrame = imageView.frame;
	CGRect imageEndFrame = imageView.frame;
	
	newCell.alpha = 0.0;
	
	switch ( animation )
	{
		case AQGridViewItemAnimationFade:
		default:
			break;		// fade always happens
			
		case AQGridViewItemAnimationTop:
			imageEndFrame.origin.y += cellSize.height;
			cellStartFrame.origin.y -= cellSize.height;
			break;
			
		case AQGridViewItemAnimationBottom:
			imageEndFrame.origin.y -= cellSize.height;
			cellStartFrame.origin.y += cellSize.height;
			break;
			
		case AQGridViewItemAnimationLeft:
			imageEndFrame.origin.x += cellSize.width;
			cellStartFrame.origin.x -= cellSize.width;
			break;
			
		case AQGridViewItemAnimationRight:
			imageEndFrame.origin.x -= cellSize.width;
			cellStartFrame.origin.x += cellSize.width;
			break;
	}
	
	// set starting frames outside the animation
	newCell.frame = cellStartFrame;
	
	// re-enable animations
	[UIView setAnimationsEnabled: YES];
	
	// animate fade
	imageView.alpha = 0.0;
	newCell.alpha = 1.0;
	
	// animate end location
	imageView.frame = imageEndFrame;
	newCell.frame = cellEndFrame;
}

- (NSSet *) animateCellUpdatesUsingVisibleContentRect: (CGRect) contentRect
{
	// we might need to change the new visible indices and content rect, if we're looking at the last row and it's going to disappear
	CGSize gridSize = [_newGridData sizeForEntireGrid];
	CGFloat maxX = CGRectGetMaxX(contentRect);
	CGFloat maxY = CGRectGetMaxY(contentRect);
	BOOL isVertical = (_newGridData.layoutDirection == AQGridViewLayoutDirectionVertical);
	
	// indices of items visible from old grid
	NSIndexSet * oldVisibleIndices = [_oldGridData indicesOfCellsInRect: contentRect];
	
    // The line below is commented because it produces too many logs
	// NSLog( @"Updating from original content rect %@", NSStringFromCGRect(contentRect) );
	
	if ( (isVertical) && (maxY > gridSize.height) )
	{
		CGFloat diff = maxY - gridSize.height;
		
		// grow its height so both incoming and outgoing items get animated
		contentRect.origin.y = MAX(0.0, contentRect.origin.y - diff);
		contentRect.size.height += diff;
		
		// this will set the bounds for us, and it'll animate thanks to our animation block
		_gridView.contentSize = CGSizeMake(contentRect.size.width, gridSize.height);
	}
	else if ( (!isVertical) && (maxX > gridSize.width) )
	{
		CGFloat diff = maxX - gridSize.width;
		
		// grow its width so both incoming and outgoing items get animated
		contentRect.origin.x = MAX(0.0, contentRect.origin.x - diff);
		contentRect.size.width += diff;
		
		// this will set the bounds for us, and it'll animate thanks to our animation block
		_gridView.contentSize = CGSizeMake(gridSize.width, contentRect.size.height);
	}
	else
	{
		[_gridView updateGridViewBoundsForNewGridData: _newGridData];
	}
	
    // The line below is fixed because it produces too many logs
	//NSLog( @"Updated content rect: %@", NSStringFromCGRect(contentRect) );
	NSIndexSet * newVisibleIndices = [_newGridData indicesOfCellsInRect: contentRect];
	
	NSMutableSet * newVisibleCells = [[NSMutableSet alloc] initWithSet: _gridView.animatingCells];
	
	// make a lookup table for all currently-animating cells, indexed by their new location's index
	// we use CF because our keys are integers
	CFMutableDictionaryRef animatingCellTable = CFDictionaryCreateMutable( kCFAllocatorDefault, (CFIndex)_gridView.animatingCells.count, NULL, &kCFTypeDictionaryValueCallBacks );
	for ( AQGridViewAnimatorItem * item in newVisibleCells )
	{
		// only store real cells here
		if ( [item.animatingView isKindOfClass: [AQGridViewCell class]] )
			CFDictionaryAddValue( animatingCellTable, (void *)item.index, objc_unretainedPointer(item) );
	}
	
	// a set of the indices (in old grid data) for all currently-known cells which are now or will become visible
	NSMutableIndexSet * oldIndicesOfAllVisibleCells = [oldVisibleIndices mutableCopy];
	for ( NSUInteger idx = [newVisibleIndices firstIndex]; idx != NSNotFound; idx = [newVisibleIndices indexGreaterThanIndex: idx] )
	{
		NSUInteger oldIndex = _newToOldIndexMap[idx];
		if ( oldIndex != NSNotFound )
			[oldIndicesOfAllVisibleCells addIndex: oldIndex];
	}
    
    NSMutableIndexSet * movingSet = [[NSMutableIndexSet alloc] initWithIndexSet: oldVisibleIndices];
    [movingSet addIndexes: oldIndicesOfAllVisibleCells];
	
	// most items were just moved from one location to another
	for ( NSUInteger oldIndex = [movingSet firstIndex]; oldIndex != NSNotFound; oldIndex = [movingSet indexGreaterThanIndex: oldIndex] )
	{
		NSUInteger newIndex = _oldToNewIndexMap[oldIndex];
		AQGridViewAnimatorItem * animatingItem = (AQGridViewAnimatorItem *)objc_unretainedObject(CFDictionaryGetValue( animatingCellTable, (void *)oldIndex ));
		
		AQGridViewCell * cell = (AQGridViewCell *)animatingItem.animatingView;
		if ( cell == nil )
			cell = [_gridView cellForItemAtIndex: oldIndex];
		
		// don't do this -- we could be revealing things which weren't previously on screen
		/*
        if ( newIndex == oldIndex )
		{
			if ( cell != nil )
				[newVisibleCells addObject: [AQGridViewAnimatorItem itemWithView: cell index: newIndex]];
            continue;
		}
		*/
		if ( newIndex == NSNotFound )
        {
			continue;
        }
		
		if ( cell == nil )
		{
			// create a new cell
			cell = [_gridView createPreparedCellForIndex: newIndex];
			// in its old location
			[UIView setAnimationsEnabled: NO];
			cell.frame = [_gridView fixCellFrame: cell.frame forGridRect: [_oldGridData cellRectAtIndex: oldIndex]];
			[UIView setAnimationsEnabled: YES];
		}
		else
		{
			cell.displayIndex = newIndex;
		}
		
		// keep the cell in our internal list
		if ( animatingItem != nil )
			animatingItem.index = newIndex;		// just update the index on the existing item
		else
			[newVisibleCells addObject: [AQGridViewAnimatorItem itemWithView: cell index: newIndex]];
		
		// animate it into its new location
		CGRect frame = [_gridView fixCellFrame: cell.frame forGridRect: [_newGridData cellRectAtIndex: newIndex]];
		//if ( CGRectEqualToRect(frame, cell.frame) == NO )
		//	NSLog( @"Moving frame from %@ to %@", NSStringFromCGRect(cell.frame), NSStringFromCGRect(frame) );
		cell.frame = frame;
		
		// tell the grid view's delegate about it
		[_gridView delegateWillDisplayCell: cell atIndex: newIndex];
	}
	
	
	// delete old items first
	if ( _deleteItems.count != 0 )
	{
		// animate deletion of currently-visible items
		for ( AQGridViewUpdateItem * item in _deleteItems )
		{
			if ( [oldVisibleIndices containsIndex: item.originalIndex] )
			{
				AQGridViewAnimatorItem * animatingItem = (AQGridViewAnimatorItem *)objc_unretainedObject(CFDictionaryGetValue( animatingCellTable, (void *)item.originalIndex ));
				
				AQGridViewCell * deletingCell = (AQGridViewCell *)animatingItem.animatingView;
				if ( deletingCell == nil )
					deletingCell = [_gridView cellForItemAtIndex: item.originalIndex];
				
				UIImageView * imageView = [self animateDeletionForCell: deletingCell withAnimation: item.animation];
				if ( imageView != nil )
				{
					if ( animatingItem != nil )
					{
						animatingItem.animatingView = imageView;
						animatingItem.index = NSNotFound;
						CFDictionaryRemoveValue( animatingCellTable, (void *)item.originalIndex );
					}
					else
					{
						[newVisibleCells addObject: [AQGridViewAnimatorItem itemWithView: imageView index: NSNotFound]];
					}
				}
			}
		}
	}
	
	// now insert new items -- no need to take already-animating cells into account here
	for ( AQGridViewUpdateItem * item in _insertItems )
	{
		if ( [newVisibleIndices containsIndex: item.index] )
		{
			AQGridViewCell * cell = [_gridView createPreparedCellForIndex: item.index usingGridData: _newGridData];
			if ( cell != nil )
			{
				[self animateInsertionForCell: cell withAnimation: item.animation];
				[_gridView delegateWillDisplayCell: cell atIndex: item.index];
				[newVisibleCells addObject: [AQGridViewAnimatorItem itemWithView: cell index: item.index]];
			}
		}
	}
	
	// now reload items
	for ( AQGridViewUpdateItem * item in _reloadItems )
	{
		if ( [newVisibleIndices containsIndex: item.index] == NO )
			continue;
		
		AQGridViewAnimatorItem * animatingItem = (AQGridViewAnimatorItem *)objc_unretainedObject(CFDictionaryGetValue( animatingCellTable, (void *)item.originalIndex ));
		
		AQGridViewCell * origCell = (AQGridViewCell *)animatingItem.animatingView;
		if ( origCell == nil )
			origCell = [_gridView cellForItemAtIndex: item.originalIndex];
		
		// create a new cell with the latest data
		AQGridViewCell * newCell = [_gridView createPreparedCellForIndex: item.index];
		[_gridView delegateWillDisplayCell: newCell atIndex: item.index];
		[self animateReloadForCell: newCell originalCell: origCell withAnimation: item.animation];
		
		if ( animatingItem != nil )
		{
			animatingItem.animatingView = newCell;
			animatingItem.index = item.originalIndex;
		}
		else
		{
			AQGridViewAnimatorItem * tmp = [AQGridViewAnimatorItem itemWithView: newCell index: item.index];
			
			// newVisibleCells is a set, meaning that it will probably not actually insert the new item
			// if there is something matching this index, let's just update that value
			animatingItem = [newVisibleCells member: tmp];
			if ( animatingItem == nil )
				[newVisibleCells addObject: tmp];
			else
				animatingItem.animatingView = newCell;
		}
	}
	
	CFRelease( animatingCellTable );
	
	return ( newVisibleCells );
}

@end
