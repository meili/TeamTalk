/*
 * AQGridViewUpdateInfo.h
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

#import <Foundation/Foundation.h>
#import "AQGridViewUpdateItem.h"
#import "AQGridView.h"

// used internally by AQGridView and AQGridViewUpdateInfo
@interface AQGridView ()
@property (nonatomic, copy) NSSet * animatingCells;
@end

@interface AQGridViewUpdateInfo : NSObject
{
	// NB: These are never cleared, only sorted & modified.
	// It is assumed that a deferred update info object will be created in -beginUpdates
	//  and released in -endUpdates, and will not need to last across multiple suspended update sequences
	NSMutableArray *	_insertItems;
	NSMutableArray *	_deleteItems;
	NSMutableArray *	_moveItems;
	NSMutableArray *	_reloadItems;
	
	// index sets, cached for handiness
	NSMutableIndexSet * _insertedIndices;
	NSMutableIndexSet *	_deletedIndices;
	NSMutableIndexSet * _oldMovedIndices;
	NSMutableIndexSet * _newMovedIndices;
	NSMutableIndexSet * _reloadedIndices;
	
	// old and new grid data -- for bounds calculations
	AQGridViewData *	_oldGridData;
	AQGridViewData *	_newGridData;
	
	// mapping tables, used to map from old indices to new ones
	NSUInteger *		_oldToNewIndexMap;
	NSUInteger *		_newToOldIndexMap;
	
	// indices of all items which were simply shuffled around as a result of other operations
	NSMutableIndexSet *	_onlyMovedIndices;
	
	// needs to ask the grid view for cells
	AQGridView *		__unsafe_unretained _gridView;		// weak reference
	
	NSMutableSet *		_animatingCells;
}

- (id) initWithOldGridData: (AQGridViewData *) oldGridData forGridView: (AQGridView *) gridView;

- (void) updateItemsAtIndices: (NSIndexSet *) indices
				 updateAction: (AQGridViewUpdateAction) action
				withAnimation: (AQGridViewItemAnimation) animation;
- (void) moveItemAtIndex: (NSUInteger) index
				 toIndex: (NSUInteger) index
		   withAnimation: (AQGridViewItemAnimation) animation;

@property (nonatomic, readonly) NSUInteger numberOfUpdates;

// This function assumed a certain ordering in which items will be inserted/deleted etc.
// Specifically, it will assume deletions happen FIRST, then insertions SECOND, and reloads LAST.
// The indices provided are all assumed to refer to the content index set as it existed prior
//  to ANY inserts/deletes occurring.
// Needless to say: this is therefore quite private, since AQGridView must conform to and rely
//  on this behaviour
- (void) cleanupUpdateItems;

// the returned values are not guaranteed to be correct prior to invocation of -cleanupUpdateItems above
- (NSArray *) sortedInsertItems;
- (NSArray *) sortedDeleteItems;
- (NSArray *) sortedMoveItems;
- (NSArray *) sortedReloadItems;

- (AQGridViewData *) newGridViewData;
- (NSUInteger) numberOfItemsAfterUpdates;

- (NSUInteger) newIndexForOldIndex: (NSUInteger) oldIndex;

// returns a list of all the views being animated
- (NSSet *) animateCellUpdatesUsingVisibleContentRect: (CGRect) contentRect;

@end
