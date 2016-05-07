/*
 * AQGridViewUpdateItem.m
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

#import "AQGridViewUpdateItem.h"

@implementation AQGridViewUpdateItem

@synthesize originalIndex=_index, newIndex=_newIndex, action=_action, animation=_animation, offset=_offset;

- (id) initWithIndex: (NSUInteger) index action: (AQGridViewUpdateAction) action animation: (AQGridViewItemAnimation) animation
{
	self = [super init];
	if ( self == nil )
		return ( nil );
	
	_index = index;
	_action = action;
	_animation = animation;
	
	return ( self );
}

- (void) setNewIndex: (NSUInteger) value
{
	NSAssert(self.action == AQGridViewUpdateActionMove, @"newIndex set on a non-move update item");
	_newIndex = value;
}

- (NSString *) description
{
	NSString * actionDesc = @"<Unknown>";
	switch ( _action )
	{
		case AQGridViewUpdateActionInsert:
			actionDesc = @"Insert";
			break;
		case AQGridViewUpdateActionDelete:
			actionDesc = @"Delete";
			break;
		case AQGridViewUpdateActionMove:
			actionDesc = @"Move";
			break;
		case AQGridViewUpdateActionReload:
			actionDesc = @"Reload";
			break;
		default:
			break;
	}
	
	NSString * animationDesc = @"<Unknown>";
	switch ( (UITableViewRowAnimation)_animation )
	{
		case UITableViewRowAnimationFade:
			animationDesc = @"Fade";
			break;
		case UITableViewRowAnimationRight:
			animationDesc = @"Right";
			break;
		case UITableViewRowAnimationLeft:
			animationDesc = @"Left";
			break;
		case UITableViewRowAnimationTop:
			animationDesc = @"Top";
			break;
		case UITableViewRowAnimationBottom:
			animationDesc = @"Bottom";
			break;
		case UITableViewRowAnimationNone:
			animationDesc = @"None";
			break;
		case UITableViewRowAnimationMiddle:
			animationDesc = @"Middle";
			break;
		default:
			break;
	}
	
	return ( [NSString stringWithFormat: @"%@{index=%u, action=%@, animation=%@, offset=%.02f}", [super description], (unsigned)_index, actionDesc, animationDesc, (double)_offset] );
}

- (NSComparisonResult) compare: (AQGridViewUpdateItem *) other
{
	if ( _index > other->_index )
		return ( NSOrderedDescending );
	else if ( _index < other->_index )
		return ( NSOrderedAscending );
	return ( NSOrderedSame );
}

- (NSComparisonResult) inverseCompare: (AQGridViewUpdateItem *) other
{
	return ( [other compare: self] );
}

- (NSUInteger) index
{
	// handle case where offset is negative and would cause index to wrap
	if ( (_offset < 0) && (abs(_offset) > _index) )
		return ( 0 );
	
	return ( _index + _offset );
}

@end
