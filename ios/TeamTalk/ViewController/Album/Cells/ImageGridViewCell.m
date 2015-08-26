/*
 * ImageGridViewCell.m
 * Classes
 *
 * Created by Jim Dovey on 16/8/2010.
 *
 * Copyright (c) 2010 Jim Dovey
 * All rights reserved.
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

#import "ImageGridViewCell.h"
@implementation ImageGridViewCell

- (id) initWithFrame: (CGRect) frame reuseIdentifier: (NSString *) aReuseIdentifier
{
	self = [super initWithFrame: frame reuseIdentifier: aReuseIdentifier];
	if ( self == nil )
		return  nil ;
	
	_imageView = [[UIImageView alloc] initWithFrame: CGRectZero];
	[self.contentView addSubview: _imageView];
	self.selectImage = [[UIImageView alloc] initWithFrame:CGRectMake(50, 5, 20, 20)];
    [self.selectImage setImage:[UIImage imageNamed:@"unselected"]];
    [self.selectImage setHighlightedImage:[UIImage imageNamed:@"select"]];
    [self.contentView addSubview:self.selectImage];
    self.title = [[UILabel alloc] initWithFrame:CGRectMake(0,self.frame.size.height+10, self.frame.size.width, 15)];
    [self.title setFont:[UIFont systemFontOfSize:14.0]];
    [self.title setTextAlignment:NSTextAlignmentCenter];
    [self.contentView addSubview:self.title];

	return self ;
}
-(void)setCellIsToHighlight:(BOOL)isHighlight
{
    [self.selectImage setHighlighted:isHighlight];
}


- (CALayer *) glowSelectionLayer
{
    return _imageView.layer;
}

- (UIImage *) image
{
    return _imageView.image ;
}

- (void) setImage: (UIImage *) anImage
{
    _imageView.image = anImage;
    [self setNeedsLayout];
}

- (void) layoutSubviews
{
    [super layoutSubviews];
    if (!self.isShowSelect) {
        [self.selectImage setHidden:YES];
    }else
    {
        [self.selectImage setHidden:NO];
    }
    CGSize imageSize = _imageView.image.size;
    CGRect frame = _imageView.frame;
    CGRect bounds = self.contentView.bounds;
    
    if ( (imageSize.width <= bounds.size.width) &&
		(imageSize.height <= bounds.size.height) )
    {
        return;
    }
    
    // scale it down to fit
    CGFloat hRatio = bounds.size.width / imageSize.width;
    CGFloat vRatio = bounds.size.height / imageSize.height;
    CGFloat ratio = MAX(hRatio, vRatio);
    
    frame.size.width = floorf(imageSize.width * ratio);
    frame.size.height = floorf(imageSize.height * ratio);
    frame.origin.x = floorf((bounds.size.width - frame.size.width) * 0.5);
    frame.origin.y = floorf((bounds.size.height - frame.size.height) * 0.5);
    _imageView.frame = frame;
    self.title.frame=CGRectMake(0,self.frame.size.height+10, self.frame.size.width, 15);
}

@end
