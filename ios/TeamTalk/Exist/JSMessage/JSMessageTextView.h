//
//  JSMessageTextView.h
//  PoGo
//
//  Created by Yongchao on 20/11/13.
//  Copyright (c) 2013 Yongchao. All rights reserved.
//

#import "JSDismissiveTextView.h"

@interface JSMessageTextView : JSDismissiveTextView 

/**
 *  The text to be displayed when the text view is empty. The default value is `nil`.
 */
@property (copy, nonatomic) NSString *placeHolder;

/**
 *  The color of the place holder text. The default value is `[UIColor lightGrayColor]`.
 */
@property (strong, nonatomic) UIColor *placeHolderTextColor;

/**
 *  Returns an unsigned integer describing the number of lines of text contained in the text view.
 *
 *  @return The number of lines of text in the text view.
 */
- (NSUInteger)numberOfLinesOfText;

/**
 *  Returns a constant describing the maximum number of characters that can fit on a single line of the text view.
 *
 *  @return The maximum number of characters per line in the text view.
 */
+ (NSUInteger)maxCharactersPerLine;

/**
 *  Returns an unsigned integer describing the number of lines necessary to display the given text in the text view.
 *
 *  @param text The text to be displayed in the text view.
 *
 *  @return The number of lines needed to display the given text.
 */
+ (NSUInteger)numberOfLinesForMessage:(NSString *)text;

@end
