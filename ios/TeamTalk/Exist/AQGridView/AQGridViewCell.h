/*
 * AQGridViewCell.h
 * AQGridView
 * 
 * Created by Jim Dovey on 25/2/2010.
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

#import <UIKit/UIKit.h>

typedef enum {
	AQGridViewCellSeparatorStyleNone,
	AQGridViewCellSeparatorStyleEmptySpace,
	AQGridViewCellSeparatorStyleSingleLine
} AQGridViewCellSeparatorStyle;

typedef enum {
	AQGridViewCellSelectionStyleNone,
	AQGridViewCellSelectionStyleBlue,
	AQGridViewCellSelectionStyleGray,
	AQGridViewCellSelectionStyleBlueGray,
	AQGridViewCellSelectionStyleGreen,
	AQGridViewCellSelectionStyleRed,
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_3_2
	AQGridViewCellSelectionStyleGlow		// see also 'selectionGlowColor' property
#endif
} AQGridViewCellSelectionStyle;

@interface AQGridViewCell : UIView
{
	NSString *				_reuseIdentifier;
	UIView *				_contentView;
	UIView *				_backgroundView;
	UIView *				_selectedBackgroundView;
	UIView *				_selectedOverlayView;
	CGFloat					_selectionFadeDuration;
	UIColor *				_backgroundColor;
	UIColor *				_separatorColor;
	UIColor *				_selectionGlowColor;
	CGFloat					_selectionGlowShadowRadius;
	UIView *				_bottomSeparatorView;
	UIView *				_rightSeparatorView;
	NSTimer *				_fadeTimer;
	CFMutableDictionaryRef	_selectionColorInfo;
	NSUInteger				_displayIndex;			// le sigh...
	struct {
		unsigned int separatorStyle:3;
		unsigned int selectionStyle:3;
		unsigned int separatorEdge:2;
		unsigned int animatingSelection:1;
		unsigned int backgroundColorSet:1;
		unsigned int selectionGlowColorSet:1;
		unsigned int usingDefaultSelectedBackgroundView:1;
		unsigned int selected:1;
		unsigned int highlighted:1;
		unsigned int becomingHighlighted:1;
        unsigned int setShadowPath:1;
        unsigned int editing:1;
		unsigned int hiddenForAnimation:1;
		unsigned int __RESERVED__:14;
	} _cellFlags;
}

- (id) initWithFrame: (CGRect) frame reuseIdentifier: (NSString *) reuseIdentifier;

// If you want to customize cells by simply adding additional views, you should add them to the content view so they will be positioned appropriately as the cell transitions into and out of editing mode.
@property (nonatomic, readonly, retain) UIView * contentView;

// default is nil. The background view will be added as a subview behind all other views
@property (nonatomic, retain) UIView * backgroundView;

// The 'selectedBackgroundView' will be added as a subview directly above the backgroundView if not nil, or behind all other views. It is added as a subview only when the cell is selected. Calling -setSelected:animated: will cause the 'selectedBackgroundView' to animate in and out with an alpha fade.
@property (nonatomic, retain) UIView * selectedBackgroundView;

@property (nonatomic, readonly, copy) NSString * reuseIdentifier;
- (void) prepareForReuse;		// if the cell is reusable (has a reuse identifier), this is called just before the cell is returned from the grid view method dequeueReusableCellWithIdentifier:.  If you override, you MUST call super.

@property (nonatomic) AQGridViewCellSelectionStyle selectionStyle;		// default is AQGridViewCellSelectionStyleGlow
@property (nonatomic, getter=isSelected) BOOL selected;					// default is NO
@property (nonatomic, getter=isHighlighted) BOOL highlighted;			// default is NO
@property (nonatomic, retain) UIColor * selectionGlowColor;				// default is dark grey, ignored if selectionStyle != AQGridViewCellSelectionStyleGlow
@property (nonatomic) CGFloat selectionGlowShadowRadius;				// default is 12.0, ignored if selectionStyle != AQGridViewCellSelectionStyleGlow

// this can be overridden by subclasses to return a subview's layer to which to add the glow
// the default implementation returns the contentView's layer
@property (nonatomic, readonly) CALayer * glowSelectionLayer;

- (void) setSelected: (BOOL) selected animated: (BOOL) animated;
- (void) setHighlighted: (BOOL) highlighted animated: (BOOL) animated;

// Editing

@property(nonatomic,getter=isEditing) BOOL          editing;                    // show appropriate edit controls (+/- & reorder). By default -setEditing: calls setEditing:animated: with NO for animated.
- (void)setEditing:(BOOL)editing animated:(BOOL)animated;

// Sorting
- (NSComparisonResult) compareOriginAgainstCell: (AQGridViewCell *) otherCell;

@end
