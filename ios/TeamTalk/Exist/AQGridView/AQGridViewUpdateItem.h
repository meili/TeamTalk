/*
 * AQGridViewUpdateItem.h
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
#import "AQGridView.h"

typedef enum {
	AQGridViewUpdateActionInsert,
	AQGridViewUpdateActionDelete,
	AQGridViewUpdateActionMove,
	AQGridViewUpdateActionReload
} AQGridViewUpdateAction;

@interface AQGridViewUpdateItem : NSObject
{
	NSUInteger				_index;
	NSUInteger				_newIndex;
	AQGridViewUpdateAction	_action;
	AQGridViewItemAnimation	_animation;
	NSInteger				_offset;
}

- (id) initWithIndex: (NSUInteger) index action: (AQGridViewUpdateAction) action animation: (AQGridViewItemAnimation) animation;

- (NSComparisonResult) compare: (AQGridViewUpdateItem *) other;
- (NSComparisonResult) inverseCompare: (AQGridViewUpdateItem *) other;

@property (nonatomic, readonly) NSUInteger index;
@property (nonatomic) NSUInteger newIndex;		// only valid for AQGridViewUpdateActionMove
@property (nonatomic, readonly) AQGridViewUpdateAction action;
@property (nonatomic, readonly) AQGridViewItemAnimation animation;

// this is an offset to apply to the index, due to other changes in the list which occurred since this index was chosen
@property (nonatomic) NSInteger offset;
@property (nonatomic, readonly) NSUInteger originalIndex;	// returns index without offset

@end
