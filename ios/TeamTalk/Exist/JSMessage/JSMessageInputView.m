//
//  JSMessageInputView.m
//
//  Created by Jesse Squires on 2/12/13.
//  Copyright (c) 2013 Hexed Bits. All rights reserved.
//
//  http://www.hexedbits.com
//
//
//  Largely based on work by Sam Soffes
//  https://github.com/soffes
//
//  SSMessagesViewController
//  https://github.com/soffes/ssmessagesviewcontroller
//
//
//  The MIT License
//  Copyright (c) 2013 Jesse Squires
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
//  associated documentation files (the "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
//  following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
//  LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
//  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
//  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#import "JSMessageInputView.h"
#import "NSString+JSMessagesView.h"
#import "UIImage+JSMessagesView.h"
#import "UIColor+JSMessagesView.h"
#import "UIView+Addition.h"

#define SEND_BUTTON_WIDTH 78.0f


@interface JSMessageInputView ()

- (void)setup;
- (void)setupTextView;
- (void)setupRecordButton;
@end



@implementation JSMessageInputView

@synthesize sendButton;

#pragma mark - Initialization
- (id)initWithFrame:(CGRect)frame delegate:(id<JSMessageInputViewDelegate>)delegate
{
    self = [super initWithFrame:frame];
    if(self) {
        [self setup];
        _delegate = delegate;
        [self setAutoresizesSubviews:NO];
      
        
    }
    return self;
}

- (void)dealloc
{
    self.textView = nil;
    self.sendButton = nil;
}

- (BOOL)resignFirstResponder
{
    [self.textView resignFirstResponder];
    return [super resignFirstResponder];
}

+ (JSInputBarStyle)inputBarStyle
{
    return JSInputBarStyleDefault;
}

#pragma mark - Setup
- (void)setup
{
    self.image = [UIImage inputBar];
    self.backgroundColor = [UIColor whiteColor];
    
    self.opaque = YES;
    self.userInteractionEnabled = YES;
    UIImageView *line = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, self.frame.size.width, 0.5)];
    line.backgroundColor=RGB(188, 188, 188);
    [self addSubview:line];
    self.emotionbutton  = [UIButton buttonWithType:UIButtonTypeCustom];
    [self.emotionbutton setBackgroundImage:[UIImage imageNamed:@"dd_emotion"] forState:UIControlStateNormal];
    self.emotionbutton.frame=CGRectMake(FULL_WIDTH-36-38, 9.0f, 28.0f, 28.0f);
    [self setSendButton:self.emotionbutton];
    self.showUtilitysbutton  = [UIButton  buttonWithType:UIButtonTypeCustom];
    [self.showUtilitysbutton setBackgroundImage:[UIImage imageNamed:@"dd_utility"] forState:UIControlStateNormal];
    self.showUtilitysbutton.frame=CGRectMake(FULL_WIDTH-38, 9.0f, 28.0f, 28.0f);
    [self addSubview:self.showUtilitysbutton];
    
    self.voiceButton = [UIButton buttonWithType:UIButtonTypeCustom];
    [self.voiceButton setBackgroundImage:[UIImage imageNamed:@"dd_record_normal"] forState:UIControlStateNormal];
    self.voiceButton.tag = 0;
     self.voiceButton.frame = CGRectMake(10, 9.0, 28.0f, 28.0f);
    [self addSubview:self.voiceButton];
    [self setupRecordButton];
    [self setupTextView];
}

- (void)setupTextView
{
    //    CGFloat width = self.frame.size.width - SEND_BUTTON_WIDTH;
    CGFloat height = [JSMessageInputView textViewLineHeight];
    
    self.textView = [[HPGrowingTextView  alloc] initWithFrame:CGRectMake(46.0f, 7.0f, self.emotionbutton.frame.origin.x-self.emotionbutton.frame.size.width-30, height)];
    //    self.textView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    self.textView.backgroundColor = [UIColor whiteColor];
    self.textView.font = [UIFont systemFontOfSize:15];
    self.textView.minHeight = 31;
    self.textView.maxNumberOfLines = 5;
    self.textView.animateHeightChange = YES;
    self.textView.animationDuration = 0.25;
    self.textView.delegate = self;
    
    [self.textView.layer setBorderWidth:0.5];
    [self.textView.layer setBorderColor:RGB(188, 188, 188).CGColor];
    [self.textView.layer setCornerRadius:2];
    self.textView.returnKeyType = UIReturnKeySend;
    [self addSubview:self.textView];
}

- (void)setupRecordButton
{
    //    CGFloat width = self.frame.size.width - SEND_BUTTON_WIDTH;
    CGFloat height = [JSMessageInputView textViewLineHeight];
    
    self.recordButton = [[UIImageView alloc] initWithFrame:CGRectMake(46, 7.0f, self.emotionbutton.frame.origin.x-self.emotionbutton.frame.size.width-30, height)];
    [self.recordButton setClipsToBounds:YES];
    [self.recordButton.layer setBorderWidth:1.0];
    [self.recordButton.layer setBorderColor:RGB(192, 192, 192).CGColor];
    [self.recordButton.layer setCornerRadius:3.0];
    [self.recordButton setUserInteractionEnabled:YES];
    
    [self.recordButton setImage:[self createImageWithColor:RGB(248, 248, 248)]];
    [self.recordButton setHighlightedImage:[self createImageWithColor:RGB(230, 230, 230)]];
    self.buttonTitle = [[UILabel alloc] initWithFrame:CGRectMake(0, 7, self.emotionbutton.frame.origin.x-self.emotionbutton.frame.size.width-30, 18)];
    self.buttonTitle.text=@"按住说话";
    [self.buttonTitle setTextColor:RGB(86, 86, 86)];
    [self.buttonTitle setTextAlignment:NSTextAlignmentCenter];
    [self.recordButton addSubview:self.buttonTitle];
    [self.recordButton setOpaque:YES];
    [self.recordButton setHidden:YES];
    [self addSubview:self.recordButton];
      [self.recordButton addObserver:self forKeyPath:@"image" options:0 context:NULL];
}

#pragma mark - HPTextViewDelegate
//- (BOOL)growingTextViewShouldBeginEditing:(HPGrowingTextView *)growingTextView;
//- (BOOL)growingTextViewShouldEndEditing:(HPGrowingTextView *)growingTextView;

//- (void)growingTextViewDidBeginEditing:(HPGrowingTextView *)growingTextView;
//- (void)growingTextViewDidEndEditing:(HPGrowingTextView *)growingTextView;

- (BOOL)growingTextView:(HPGrowingTextView *)growingTextView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    if ([text isEqual:@"\n"])
    {
        [self.delegate textViewEnterSend];
        return NO;
    }
    return YES;
}
- (void)growingTextViewDidChange:(HPGrowingTextView *)growingTextView
{
    [self.delegate textViewChanged];
}

- (void)growingTextView:(HPGrowingTextView *)growingTextView willChangeHeight:(float)height
{
    float bottom = self.bottom;
    if ([growingTextView.text length] == 0)
    {
        [self setHeight:height + 13];
    }
    else
    {
        [self setHeight:height + 10];
    }
    [self setBottom:bottom];
//    [growingTextView setContentInset:UIEdgeInsetsZero];
    //    [UIView animateKeyframesWithDuration:0.25 delay:0 options:0 animations:^{
    //
    //    } completion:^(BOOL finished) {
    //
    //    }];
    [self.delegate viewheightChanged:height];
}

//- (void)growingTextView:(HPGrowingTextView *)growingTextView didChangeHeight:(float)height
//{
//}

//- (void)growingTextViewDidChangeSelection:(HPGrowingTextView *)growingTextView;
- (BOOL)growingTextViewShouldReturn:(HPGrowingTextView *)growingTextView
{
    return YES;
}


#pragma mark - Setters
- (void)setSendButton:(UIButton *)btn
{
    if(sendButton)
        [sendButton removeFromSuperview];
    
    sendButton = btn;
    [self addSubview:self.sendButton];
}

#pragma mark - Message input view

+ (CGFloat)textViewLineHeight
{
    return 32.0f; // for fontSize 16.0f
}

+ (CGFloat)maxLines
{
    return 5.0f;
}

+ (CGFloat)maxHeight
{
    return ([JSMessageInputView maxLines] + 1.0f) * [JSMessageInputView textViewLineHeight];
}

- (void)willBeginRecord
{
    [self.textView setHidden:YES];
    [self.recordButton setHidden:NO];
}

- (void)willBeginInput
{
    [self.textView setHidden:NO];
    [self.recordButton setHidden:YES];
}
-(void)setDefaultHeight
{
    
}

- (UIImage *) createImageWithColor: (UIColor *) color
{
    CGRect rect=CGRectMake(0.0f, 0.0f, 1.0f, 1.0f);
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);
    
    UIImage *theImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return theImage;
}
@end
