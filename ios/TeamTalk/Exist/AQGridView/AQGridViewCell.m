/*
 * AQGridViewCell.m
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

#import "AQGridViewCell.h"
#import "AQGridViewCell+AQGridViewCellPrivate.h"
#import "UIColor+AQGridView.h"
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>

#ifdef BUILTIN_IMAGES
#import "AQGridViewCell_png.h"
#endif

@interface AQGridViewCell ()
@property (nonatomic, retain) UIView * contentView;
@property (nonatomic, copy) NSString * reuseIdentifier;
- (void) flipHighlightTimerFired: (NSTimer *) timer;
@end

@implementation AQGridViewCell

@synthesize contentView=_contentView, backgroundView=_backgroundView, selectedBackgroundView=_selectedBackgroundView;
@synthesize reuseIdentifier=_reuseIdentifier, selectionGlowColor=_selectionGlowColor;
@synthesize selectionGlowShadowRadius=_selectionGlowShadowRadius;

- (id) initWithFrame: (CGRect) frame reuseIdentifier: (NSString *) reuseIdentifier
{
	self = [super initWithFrame: frame];
	if ( self == nil )
		return ( nil );
	
	self.reuseIdentifier = reuseIdentifier;
	_cellFlags.usingDefaultSelectedBackgroundView = 1;
	_cellFlags.separatorStyle = AQGridViewCellSeparatorStyleEmptySpace;
	
	if ( [CALayer instancesRespondToSelector: @selector(shadowPath)] )
		_cellFlags.selectionStyle = AQGridViewCellSelectionStyleGlow;
	else
		_cellFlags.selectionStyle = AQGridViewCellSelectionStyleGray;
    _cellFlags.setShadowPath = 0;
	_selectionColorInfo = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,  &kCFTypeDictionaryValueCallBacks );
	self.backgroundColor = [UIColor whiteColor];
	
	_selectionGlowShadowRadius = 12.0f;
	
	return ( self );
}

- (void) awakeFromNib
{
    _cellFlags.usingDefaultSelectedBackgroundView = 1;
	_cellFlags.separatorStyle = AQGridViewCellSeparatorStyleEmptySpace;
	
	if ( [CALayer instancesRespondToSelector: @selector(shadowPath)] )
		_cellFlags.selectionStyle = AQGridViewCellSelectionStyleGlow;
	else
		_cellFlags.selectionStyle = AQGridViewCellSelectionStyleGray;
	_selectionColorInfo = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,  &kCFTypeDictionaryValueCallBacks );
	self.backgroundColor = [UIColor whiteColor];
    
    [super awakeFromNib];
}

- (void) dealloc
{
	if ( _selectionColorInfo != NULL )
		CFRelease( _selectionColorInfo );
}

- (NSComparisonResult) compareOriginAgainstCell: (AQGridViewCell *) otherCell
{
	CGPoint myOrigin = self.frame.origin;
	CGPoint theirOrigin = otherCell.frame.origin;
	
	if ( myOrigin.y > theirOrigin.y )
		return ( NSOrderedDescending );
	else if ( myOrigin.y < theirOrigin.y )
		return ( NSOrderedAscending );
	
	if ( myOrigin.x > theirOrigin.x )
		return ( NSOrderedDescending );
	else if ( myOrigin.x < theirOrigin.x )
		return ( NSOrderedAscending );
	
	return ( NSOrderedSame );
}

- (UIView *) contentView
{
	if ( _contentView == nil )
    {
		_contentView = [[UIView alloc] initWithFrame: self.bounds];
        _contentView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
        _contentView.autoresizesSubviews = YES;
        self.autoresizesSubviews = YES;
        _contentView.backgroundColor = [UIColor whiteColor];
		[_contentView.layer setValue: [NSNumber numberWithBool: YES] forKey: @"KoboHackInterestingLayer"];
        [self addSubview: _contentView];
    }
	return ( _contentView );
}

- (CALayer *) glowSelectionLayer
{
	return ( _contentView.layer );
}

- (AQGridViewCellSelectionStyle) selectionStyle
{
	return ( _cellFlags.selectionStyle );
}

- (void) setSelectionStyle: (AQGridViewCellSelectionStyle) style
{
	if ( (style == AQGridViewCellSelectionStyleGlow) && ([CALayer instancesRespondToSelector: @selector(shadowPath)] == NO) )
		style = AQGridViewCellSelectionStyleGray;
	_cellFlags.selectionStyle = style;
}

- (AQGridViewCellSeparatorEdge) separatorEdge
{
	return ( _cellFlags.separatorEdge );
}

- (void) setSeparatorEdge: (AQGridViewCellSeparatorEdge) value
{
	if ( _cellFlags.separatorEdge == value )
		return;
	
	_cellFlags.separatorEdge = value;
	[self setNeedsLayout];
}

- (BOOL) isSelected
{
	return ( _cellFlags.selected );
}

- (void) setSelected: (BOOL) value
{
	[self setSelected: value animated: NO];
}

- (void) setSelected: (BOOL) value animated: (BOOL) animated
{
	_cellFlags.selected = (value ? 1 : 0);
	[self setHighlighted: value animated: animated];
}

- (BOOL) isHighlighted
{
	return ( _cellFlags.highlighted );
}

- (void) setHighlighted: (BOOL) value
{
	[self setHighlighted: value animated: NO];
}

- (void) makeSubviewsOfView: (UIView *) aView nonOpaqueWithBackgroundColor: (UIColor *) color
{
	for ( UIView * view in aView.subviews )
	{
		if ( view.opaque )
		{
			NSMutableDictionary * info = (NSMutableDictionary *) objc_unretainedObject(CFDictionaryGetValue( _selectionColorInfo, objc_unretainedPointer(view) ));
			if ( info == nil )
			{
				info = [NSMutableDictionary dictionaryWithCapacity: 2];
				CFDictionarySetValue( _selectionColorInfo, objc_unretainedPointer(view), objc_unretainedPointer(info) );
			}
			
			id value = view.backgroundColor;
			if ( value == nil )
				value = [NSNull null];
			[info setObject: value forKey: @"backgroundColor"];
			
			view.opaque = NO;
			view.backgroundColor = color;
		}
		
		[self makeSubviewsOfView: view nonOpaqueWithBackgroundColor: color];
	}
}

- (void) makeSubviewsOfViewOpaqueAgain: (UIView *) aView
{
	for ( UIView * view in aView.subviews )
	{
		NSMutableDictionary * info = (NSMutableDictionary *) objc_unretainedObject(CFDictionaryGetValue( _selectionColorInfo, objc_unretainedPointer(view) ));
		if ( info != nil )
		{
			id value = [info objectForKey: @"backgroundColor"];
			if ( value == nil )
				continue;
			
			if ( value == [NSNull null] )
				value = nil;
			
			view.opaque = YES;
			view.backgroundColor = value;
		}
		
		[self makeSubviewsOfViewOpaqueAgain: view];
	}
}

//- (void) setTextColor: (UIColor *) color forSubviewsOfView: (UIView *) aView
- (void) highlightSubviewsOfView: (UIView *) aView
{
	for ( UIView * view in aView.subviews )
	{
		if ( [view respondsToSelector: @selector(setHighlighted:)] )
		{
			NSMutableDictionary * info = (NSMutableDictionary *) objc_unretainedObject(CFDictionaryGetValue( _selectionColorInfo, objc_unretainedPointer(view) ));
			if ( info == nil )
			{
				info = [NSMutableDictionary dictionaryWithCapacity: 2];
				CFDictionarySetValue( _selectionColorInfo, objc_unretainedPointer(view), objc_unretainedPointer(info) );
			}
			
			// don't overwrite any prior cache of a view's original highlighted state.
			// this is because 'highlighted' will be set, then 'selected', which can perform 'highlight' again before the animation completes
			if ( [info objectForKey: @"highlighted"] == nil )
			{
				id value = [view valueForKey: @"highlighted"];
				if ( value == nil )
					value = [NSNumber numberWithBool: NO];
				[info setObject: value forKey: @"highlighted"];
			}
			
			[view setValue: [NSNumber numberWithBool: YES]
					forKey: @"highlighted"];
		}
		
		[self highlightSubviewsOfView: view];
	}
}

- (void) resetHighlightForSubviewsOfView: (UIView *) aView
{
	for ( UIView * view in aView.subviews )
	{
		if ([view respondsToSelector:@selector(setHighlighted:)]) {
			NSMutableDictionary * info = (NSMutableDictionary *) objc_unretainedObject(CFDictionaryGetValue( _selectionColorInfo, objc_unretainedPointer(view) ));
			if ( info != nil )
			{
				id value = [info objectForKey: @"highlighted"];
				[view setValue: value forKey: @"highlighted"];
			}
		}
		[self resetHighlightForSubviewsOfView: view];
	}
}

- (void) _beginBackgroundHighlight: (BOOL) highlightOn animated: (BOOL) animated
{
	if ( (_cellFlags.usingDefaultSelectedBackgroundView == 1) && (_selectedBackgroundView == nil) )
	{
        NSString *imageName = nil;
#ifdef BUILTIN_IMAGES
		unsigned char * pngBytes = AQGridSelection_png;
		NSUInteger pngLength = AQGridSelection_png_len;

		switch ( _cellFlags.selectionStyle )
		{
			case AQGridViewCellSelectionStyleBlue:
			default:
				break;
				
			case AQGridViewCellSelectionStyleGray:
				imageName = @"AQGridSelectionGray.png";
				break;
				
			case AQGridViewCellSelectionStyleBlueGray:
				imageName = @"AQGridSelectionGrayBlue.png";
				break;
				
			case AQGridViewCellSelectionStyleGreen:
				imageName = @"AQGridSelectionGreen.png";
				break;
				
			case AQGridViewCellSelectionStyleRed:
				imageName = @"AQGridSelectionRed.png";
				break;
		}
		
		NSData *pngData = [NSData dataWithBytesNoCopy: pngBytes length: pngLength freeWhenDone: NO];
		_selectedBackgroundView = [[UIImageView alloc] initWithImage: [UIImage imageWithData: pngData]];
#else
	        imageName = @"AQGridSelection.png";
		switch ( _cellFlags.selectionStyle )
		{
			case AQGridViewCellSelectionStyleBlue:
			default:
				break;
				
			case AQGridViewCellSelectionStyleGray:
				imageName = @"AQGridSelectionGray.png";
				break;
				
			case AQGridViewCellSelectionStyleBlueGray:
				imageName = @"AQGridSelectionGrayBlue.png";
				break;
				
			case AQGridViewCellSelectionStyleGreen:
				imageName = @"AQGridSelectionGreen.png";
				break;
				
			case AQGridViewCellSelectionStyleRed:
				imageName = @"AQGridSelectionRed.png";
				break;
		}
		
		_selectedBackgroundView = [[UIImageView alloc] initWithImage: [UIImage imageNamed: imageName]];
#endif
		_selectedBackgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
		_selectedBackgroundView.contentMode = UIViewContentModeScaleToFill;
	}
	
	// we'll set the text color to something here
	if ( highlightOn )
	{
		// start it invisible
		[UIView setAnimationsEnabled: NO];
		_selectedBackgroundView.alpha = 0.0;
		
		// find all opaque subviews and make non-opaque with clear backgrounds
		[self makeSubviewsOfView: self nonOpaqueWithBackgroundColor: [UIColor clearColor]];
		
		if ( _backgroundView != nil )
			[self insertSubview: _selectedBackgroundView aboveSubview: _backgroundView];
		else
			[self insertSubview: _selectedBackgroundView atIndex: 0];
		_selectedBackgroundView.frame = self.bounds;
		
		[UIView setAnimationsEnabled: YES];
		
		// now the animating bit -- make the selection fade in
		_selectedBackgroundView.alpha = 1.0;
	}
	else
	{
		_selectedBackgroundView.alpha = 0.0;
	}
	
	if ( animated )
	{
		if ( _fadeTimer != nil )
		{
			[_fadeTimer invalidate];
		}
		
		_fadeTimer = [[NSTimer alloc] initWithFireDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]
											  interval: 0.1
												target: self
											  selector: @selector(flipHighlightTimerFired:)
											  userInfo: [NSNumber numberWithBool: highlightOn]
											   repeats: NO];
		[[NSRunLoop currentRunLoop] addTimer: _fadeTimer forMode: NSDefaultRunLoopMode];
	}
	else
	{
		if ( highlightOn )
			[self highlightSubviewsOfView: self];
		else
			[self resetHighlightForSubviewsOfView: self];
	}
}

- (void) flipHighlightTimerFired: (NSTimer *) timer
{
	if ( [[timer userInfo] boolValue] )
		[self highlightSubviewsOfView: self];
	else
		[self resetHighlightForSubviewsOfView: self];
}

- (void) highlightAnimationStopped: (NSString * __unused) animationID context: (void * __unused) context
{
	BOOL isHighlighting = (_cellFlags.becomingHighlighted ? YES : NO);
	
	if ( isHighlighting )
	{
		_cellFlags.highlighted = 1;
	}
	else
	{
		[UIView setAnimationsEnabled: NO];
		// find all non-opaque subviews and make opaque again, with original background colors
		[self makeSubviewsOfViewOpaqueAgain: self];
		[UIView setAnimationsEnabled: YES];
		
		_cellFlags.highlighted = 0;
		[_selectedBackgroundView removeFromSuperview];
		CFDictionaryRemoveAllValues( _selectionColorInfo );
	}
	
	_cellFlags.animatingSelection = 0;
}

- (void) setHighlighted: (BOOL) value animated: (BOOL) animated
{
	if ( _cellFlags.selectionStyle == AQGridViewCellSelectionStyleNone )
	{
		_cellFlags.highlighted = (value ? 1 : 0);
		return;
	}
	
	_cellFlags.becomingHighlighted = (value ? 1 : 0);
	
	if ( (animated) && ([UIView areAnimationsEnabled]) )
	{
		[UIView beginAnimations: @"AQGridCellViewHighlightAnimation" context: NULL];
		[UIView setAnimationCurve: UIViewAnimationCurveLinear];
		[UIView setAnimationBeginsFromCurrentState: YES];
		[UIView setAnimationDidStopSelector: @selector(highlightAnimationStopped:context:)];
		_cellFlags.animatingSelection = 1;
	}
	
	switch ( _cellFlags.selectionStyle )
	{
		case AQGridViewCellSelectionStyleNone:
		default:
			break;
			
		case AQGridViewCellSelectionStyleBlue:
		case AQGridViewCellSelectionStyleGray:
		case AQGridViewCellSelectionStyleBlueGray:
		case AQGridViewCellSelectionStyleGreen:
		case AQGridViewCellSelectionStyleRed:
		{
			[self _beginBackgroundHighlight: value animated: animated];
			break;
		}
			
		case AQGridViewCellSelectionStyleGlow:
		{
			CALayer * theLayer = self.glowSelectionLayer;
			
			if ([theLayer respondsToSelector: @selector(setShadowPath:)] && [theLayer respondsToSelector: @selector(shadowPath)])
			{
				if ( _cellFlags.setShadowPath == 0 )
				{
					CGMutablePathRef path = CGPathCreateMutable();
					CGPathAddRect( path, NULL, theLayer.bounds );
					theLayer.shadowPath = path;
                    CGPathRelease( path );
                    _cellFlags.setShadowPath = 1;
				}
			
				theLayer.shadowOffset = CGSizeZero;
				
				if ( _cellFlags.selectionGlowColorSet == 1 )
					theLayer.shadowColor = self.selectionGlowColor.CGColor;
				else
					theLayer.shadowColor = [[UIColor darkGrayColor] CGColor];
				
				theLayer.shadowRadius = self.selectionGlowShadowRadius;
				
				// add or remove the 'shadow' as appropriate
				if ( value )
					theLayer.shadowOpacity = 1.0;
				else
					theLayer.shadowOpacity = 0.0;
			}
			
			break;
		}
	}
	
	if ( (animated) && ([UIView areAnimationsEnabled]) )
		[UIView commitAnimations];
	else
		[self highlightAnimationStopped: @"" context: NULL];
}

- (void) setBackgroundView: (UIView *) aView
{
	if ( aView == _backgroundView )
		return;
	
	if ( _backgroundView.superview == self )
		[_backgroundView removeFromSuperview];
	
	_backgroundView = aView;
	
	_backgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
	
	CGRect bgFrame = self.bounds;
	if ( _cellFlags.separatorStyle == AQGridViewCellSeparatorStyleSingleLine )
	{
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeBottom )
			bgFrame.size.height -= 1.0;
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeRight )
			bgFrame.size.width -= 1.0;
	}
	
	_backgroundView.frame = bgFrame;
	[self insertSubview: _backgroundView atIndex: 0];
}

- (void) layoutSubviews
{
	[super layoutSubviews];
    
    _cellFlags.setShadowPath = 0;
	
	CGRect cFrame = self.bounds;
	if ( _cellFlags.separatorStyle == AQGridViewCellSeparatorStyleSingleLine )
	{
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeBottom )
		{
			if ( _bottomSeparatorView == nil )
			{
				_bottomSeparatorView = [[UIView alloc] initWithFrame: CGRectMake(0.0, self.bounds.size.height - 1.0, self.bounds.size.width, 1.0)];
				_bottomSeparatorView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleTopMargin;
				[self insertSubview: _bottomSeparatorView atIndex: 0];
			}
			
			_bottomSeparatorView.backgroundColor = (_separatorColor == nil ? [UIColor AQDefaultGridCellSeparatorColor] : _separatorColor);
			
			cFrame.size.height -= 1.0;
		}
		else if ( _bottomSeparatorView != nil )
		{
			[_bottomSeparatorView removeFromSuperview];
			_bottomSeparatorView = nil;
		}
		
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeRight )
		{
			if ( _rightSeparatorView == nil )
			{
				_rightSeparatorView = [[UIView alloc] initWithFrame: CGRectMake(self.bounds.size.width - 1.0, 0.0, 1.0, self.bounds.size.height)];
				_rightSeparatorView.autoresizingMask = UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleLeftMargin;
				[self insertSubview: _rightSeparatorView atIndex: 0];
			}
			
			_rightSeparatorView.backgroundColor = (_separatorColor == nil ? [UIColor AQDefaultGridCellSeparatorColor] : _separatorColor);
			
			cFrame.size.width -= 1.0;
		}
		else if ( _rightSeparatorView != nil )
		{
			[_rightSeparatorView removeFromSuperview];
			_rightSeparatorView = nil;
		}
	}
	
	self.contentView.frame = cFrame;
	self.backgroundView.frame = cFrame;
	self.selectedBackgroundView.frame = cFrame;
}

- (void) setSelectionGlowColor: (UIColor *) aColor
{
	_selectionGlowColor = aColor;
	
	_cellFlags.selectionGlowColorSet = (aColor == nil ? 0 : 1);
}

- (void) setSelectedBackgroundView: (UIView *) aView
{
	if ( aView == _selectedBackgroundView )
		return;
	
	if ( _selectedBackgroundView.superview == self )
		[_selectedBackgroundView removeFromSuperview];
	
	_selectedBackgroundView = aView;
	
	_selectedBackgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
	
	CGRect bgFrame = self.bounds;
	if ( _cellFlags.separatorStyle == AQGridViewCellSeparatorStyleSingleLine )
	{
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeBottom )
			bgFrame.size.height -= 1.0;
		if ( _cellFlags.separatorEdge & AQGridViewCellSeparatorEdgeRight )
			bgFrame.size.width -= 1.0;
	}
	
	_selectedBackgroundView.frame = bgFrame;
	[self insertSubview: _selectedBackgroundView atIndex: 0];
	
	_cellFlags.usingDefaultSelectedBackgroundView = (aView == nil ? 1 : 0);
}

- (void) prepareForReuse
{
    _cellFlags.setShadowPath = 0;
}

- (BOOL) isEditing
{
	return ( _cellFlags.editing == 1 );
}

- (void) setEditing: (BOOL) value
{
	[self setEditing:value animated:NO];
}

- (void)setEditing:(BOOL)editing animated:(BOOL)animated
{
	_cellFlags.editing = (editing ? 1 : 0);
}

@end

@implementation AQGridViewCell (AQGridViewCellPrivate)

- (UIColor *) separatorColor
{
	return ( _separatorColor );
}

- (void) setSeparatorColor: (UIColor *) color
{
	if ( _separatorColor == color )
		return;
	
	_separatorColor = color;
	
	_bottomSeparatorView.backgroundColor = _separatorColor;
	_rightSeparatorView.backgroundColor = _separatorColor;
}

- (AQGridViewCellSeparatorStyle) separatorStyle
{
	return ( _cellFlags.separatorStyle );
}

- (void) setSeparatorStyle: (AQGridViewCellSeparatorStyle) style
{
	if ( style == _cellFlags.separatorStyle )
		return;
	
	_cellFlags.separatorStyle = style;
	[self setNeedsLayout];
}

- (NSUInteger) displayIndex
{
	return ( _displayIndex );
}

- (void) setDisplayIndex: (NSUInteger) index
{
	_displayIndex = index;
}

- (BOOL) hiddenForAnimation
{
	return ( _cellFlags.hiddenForAnimation == 1 );
}

- (void) setHiddenForAnimation: (BOOL) value
{
	if ( value )
	{
		self.hidden = YES;
		_cellFlags.hiddenForAnimation = 1;
	}
	else
	{
		// don't make visible here-- might still be hidden by something else. Caller should un-hide if appropriate
		_cellFlags.hiddenForAnimation = 0;
	}
}

@end
