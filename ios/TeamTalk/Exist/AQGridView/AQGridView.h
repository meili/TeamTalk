/*
 * AQGridView.h
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

#import <UIKit/UIKit.h>
#import "AQGridViewCell.h"

typedef enum {
	AQGridViewScrollPositionNone,
	AQGridViewScrollPositionTop,
	AQGridViewScrollPositionMiddle,
	AQGridViewScrollPositionBottom
} AQGridViewScrollPosition;

typedef enum {
	AQGridViewItemAnimationFade,
	AQGridViewItemAnimationRight,
	AQGridViewItemAnimationLeft,
	AQGridViewItemAnimationTop,
	AQGridViewItemAnimationBottom,
	AQGridViewItemAnimationNone
} AQGridViewItemAnimation;

typedef enum {
	AQGridViewLayoutDirectionVertical,
	AQGridViewLayoutDirectionHorizontal
} AQGridViewLayoutDirection;

@protocol AQGridViewDataSource;
@class AQGridView, AQGridViewData, AQGridViewUpdateInfo;

@protocol AQGridViewDelegate <NSObject, UIScrollViewDelegate>

@optional

// Display customization

- (void) gridView: (AQGridView *) gridView willDisplayCell: (AQGridViewCell *) cell forItemAtIndex: (NSUInteger) index;

// Selection

// Called before selection occurs. Return a new index, or NSNotFound, to change the proposed selection.
- (NSUInteger) gridView: (AQGridView *) gridView willSelectItemAtIndex: (NSUInteger) index;
- (NSUInteger) gridView: (AQGridView *) gridView willSelectItemAtIndex: (NSUInteger) index numFingersTouch:(NSUInteger) numFingers;
- (NSUInteger) gridView: (AQGridView *) gridView willDeselectItemAtIndex: (NSUInteger) index;
// Called after the user changes the selection
- (void) gridView: (AQGridView *) gridView didSelectItemAtIndex: (NSUInteger) index;
- (void) gridView: (AQGridView *) gridView didSelectItemAtIndex: (NSUInteger) index numFingersTouch:(NSUInteger)numFingers;
- (void) gridView: (AQGridView *) gridView didDeselectItemAtIndex: (NSUInteger) index;

// Called after animated updates finished
- (void) gridViewDidEndUpdateAnimation:(AQGridView *) gridView;

// NOT YET IMPLEMENTED
- (void) gridView: (AQGridView *) gridView gestureRecognizer: (UIGestureRecognizer *) recognizer activatedForItemAtIndex: (NSUInteger) index;

- (CGRect) gridView: (AQGridView *) gridView adjustCellFrame: (CGRect) cellFrame withinGridCellFrame: (CGRect) gridCellFrame;

// Editing
- (void)gridView:(AQGridView *)aGridView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndex:(NSUInteger)index;

@end

extern NSString * const AQGridViewSelectionDidChangeNotification;

@interface AQGridView : UIScrollView
{
	id<AQGridViewDataSource>		__unsafe_unretained _dataSource;

	AQGridViewData *				_gridData;
	NSMutableArray *				_updateInfoStack;
	NSInteger						_animationCount;

	CGRect							_visibleBounds;
	NSRange							_visibleIndices;
	NSMutableArray *				_visibleCells;
	NSMutableDictionary *			_reusableGridCells;

	NSSet *							_animatingCells;
	NSIndexSet *					_animatingIndices;

	NSMutableIndexSet *				_highlightedIndices;
	UIView *						__unsafe_unretained _touchedContentView;		// weak reference

	UIView *						_backgroundView;
	UIColor *						_separatorColor;

	NSInteger						_reloadingSuspendedCount;
	NSInteger						_displaySuspendedCount;

	NSInteger						_updateCount;

	NSUInteger						_selectedIndex;
	NSUInteger						_pendingSelectionIndex;

	CGPoint							_touchBeganPosition;

	UIView *						_headerView;
	UIView *						_footerView;

	struct
	{
		unsigned	resizesCellWidths:1;
		unsigned	numColumns:6;
		unsigned	separatorStyle:3;
		unsigned	allowsSelection:1;
		unsigned	backgroundViewExtendsUp:1;
		unsigned	backgroundViewExtendsDown:1;
		unsigned	usesPagedHorizontalScrolling:1;
		unsigned	updating:1;				// unused
		unsigned	ignoreTouchSelect:1;
		unsigned	needsReload:1;
		unsigned	allCellsNeedLayout:1;
		unsigned	isRotating:1;
		unsigned	clipsContentWidthToBounds:1;
		unsigned	isAnimatingUpdates:1;	// unused, see _animationCount instead
		unsigned	requiresSelection:1;
		unsigned	contentSizeFillsBounds:1;

		unsigned	delegateWillDisplayCell:1;
		unsigned	delegateWillSelectItem:1;
        unsigned    delegateWillSelectItemMultiTouch:1;
		unsigned	delegateWillDeselectItem:1;
		unsigned	delegateDidSelectItem:1;
        unsigned    delegateDidSelectItemMultiTouch:1;
		unsigned	delegateDidDeselectItem:1;
		unsigned	delegateGestureRecognizerActivated:1;
		unsigned	delegateAdjustGridCellFrame:1;
		unsigned    delegateDidEndUpdateAnimation:1;

		unsigned	dataSourceGridCellSize:1;

        unsigned int isEditing:1;

		unsigned	__RESERVED__:1;
	} _flags;
}

@property (nonatomic, unsafe_unretained) IBOutlet id<AQGridViewDataSource> dataSource;
@property (nonatomic, unsafe_unretained) IBOutlet id<AQGridViewDelegate> delegate;

@property (nonatomic, assign) AQGridViewLayoutDirection layoutDirection;

// Data

- (void) reloadData;

// Info

@property (nonatomic, readonly) NSUInteger numberOfItems;
@property (nonatomic, readonly) NSUInteger numberOfColumns;
@property (nonatomic, readonly) NSUInteger numberOfRows;

@property (nonatomic, readonly) CGSize gridCellSize;

- (void)doAddVisibleCell: (UIView *)cell;

- (CGRect) rectForItemAtIndex: (NSUInteger) index;
- (CGRect) gridViewVisibleBounds;
- (AQGridViewCell *) cellForItemAtIndex: (NSUInteger) index;
- (NSUInteger) indexForItemAtPoint: (CGPoint) point;
- (NSUInteger) indexForCell: (AQGridViewCell *) cell;
- (AQGridViewCell *) cellForItemAtPoint: (CGPoint) point;

- (NSArray *) visibleCells;
- (NSIndexSet *) visibleCellIndices;

- (void) scrollToItemAtIndex: (NSUInteger) index atScrollPosition: (AQGridViewScrollPosition) scrollPosition animated: (BOOL) animated;

// Insertion/deletion/reloading

- (void) beginUpdates;		// allow multiple insert/delete of items to be animated simultaneously. Nestable.
- (void) endUpdates;		// only call insert/delete/reload calls inside an update block.

- (void) insertItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation;
- (void) deleteItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation;
- (void) reloadItemsAtIndices: (NSIndexSet *) indices withAnimation: (AQGridViewItemAnimation) animation;

- (void) moveItemAtIndex: (NSUInteger) index toIndex: (NSUInteger) newIndex withAnimation: (AQGridViewItemAnimation) animation;

// Selection

@property (nonatomic) BOOL allowsSelection;	// default is YES
@property (nonatomic) BOOL requiresSelection;	// if YES, tapping on a selected cell will not de-select it

- (NSUInteger) indexOfSelectedItem;		// returns NSNotFound if no item is selected
- (void) selectItemAtIndex: (NSUInteger) index animated: (BOOL) animated scrollPosition: (AQGridViewScrollPosition) scrollPosition;
- (void) deselectItemAtIndex: (NSUInteger) index animated: (BOOL) animated;

// Appearance

@property (nonatomic, assign) BOOL resizesCellWidthToFit;	// default is NO. Set to YES if the view should resize cells to fill all available space in their grid square. Ignored if separatorStyle == AQGridViewCellSeparatorStyleEmptySpace.

// this property is now officially deprecated -- it will instead set the layout direction to horizontal if
//  this property is set to YES, or to vertical otherwise.
@property (nonatomic, assign) BOOL clipsContentWidthToBounds __attribute__((deprecated));	// default is YES. If you want to enable horizontal scrolling, set this to NO.

@property (nonatomic, retain) UIView * backgroundView;		// specifies a view to place behind the cells
@property (nonatomic) BOOL backgroundViewExtendsUp;			// default is NO. If YES, the background view extends upward and is visible during a bounce.
@property (nonatomic) BOOL backgroundViewExtendsDown;		// default is NO. If YES, the background view extends downward and is visible during a bounce.
@property (nonatomic) BOOL usesPagedHorizontalScrolling;	// default is NO, and scrolls verticalls only. Set to YES to have horizontal-only scrolling by page.

@property (nonatomic) AQGridViewCellSeparatorStyle separatorStyle;	// default is AQGridViewCellSeparatorStyleEmptySpace
@property (nonatomic, retain) UIColor * separatorColor;		// ignored unless separatorStyle == AQGridViewCellSeparatorStyleSingleLine. Default is standard separator gray.

- (AQGridViewCell *) dequeueReusableCellWithIdentifier: (NSString *) reuseIdentifier;

// Headers and Footers

@property (nonatomic, retain) UIView * gridHeaderView;
@property (nonatomic, retain) UIView * gridFooterView;

@property (nonatomic, assign) CGFloat leftContentInset;
@property (nonatomic, assign) CGFloat rightContentInset;

@property (nonatomic, assign) BOOL contentSizeGrowsToFillBounds;	// default is YES. Prior to iPhone OS 3.2, pattern colors tile from the bottom-left, necessitating that this be set to NO to avoid specially-constructed background patterns falling 'out of sync' with the cells displayed on top of it.

@property (nonatomic, readonly) BOOL isAnimatingUpdates;

// Editing

@property(nonatomic,getter=isEditing) BOOL editing;                             // default is NO. setting is not animated.
- (void)setEditing:(BOOL)editing animated:(BOOL)animated;

@end

@protocol AQGridViewDataSource <NSObject>

@required

- (NSUInteger) numberOfItemsInGridView: (AQGridView *) gridView;
- (AQGridViewCell *) gridView: (AQGridView *) gridView cellForItemAtIndex: (NSUInteger) index;

@optional

// all cells are placed in a logical 'grid cell', all of which are the same size. The default size is 96x128 (portrait).
// The width/height values returned by this function will be rounded UP to the nearest denominator of the screen width.
- (CGSize) portraitGridCellSizeForGridView: (AQGridView *) gridView;

@end
