/*
 * AQGridViewController.m
 * AQGridView
 * 
 * Created by Jim Dovey on 24/2/2010.
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

#import "AQGridViewController.h"

@interface AQGridView (AQGridViewPrivate)
- (void) viewWillRotateToInterfaceOrientation: (UIInterfaceOrientation) orientation;
- (void) viewDidRotate;
@end

@implementation AQGridViewController

@synthesize clearsSelectionOnViewWillAppear=_clearsSelectionOnViewWillAppear;

- (void) _sharedGridViewDefaultSetup
{
	self.gridView.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
	self.gridView.autoresizesSubviews = YES;
	self.gridView.delegate = self;
	self.gridView.dataSource = self;
}

- (void) loadView
{
	AQGridView * aView = [[AQGridView alloc] initWithFrame: CGRectZero];
	self.gridView = aView;
    
    [self _sharedGridViewDefaultSetup];
}

- (void) awakeFromNib
{
    [self _sharedGridViewDefaultSetup];
}

- (void) viewDidLoad
{
	[super viewDidLoad];
	[self.gridView reloadData];
	
	_popoverShowing = NO;
}

- (AQGridView *) gridView
{
	return ( (AQGridView *) self.view );
}

- (void) setGridView: (AQGridView *) value
{
	if ( [value isKindOfClass: [AQGridView class]] == NO )
	{
		[NSException raise: NSInvalidArgumentException format: @"-setGridView: called with non-AQGridView argument '%@'", NSStringFromClass([value class])];
	}
	
	self.view = value;
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

- (void) viewWillAppear: (BOOL) animated
{
	if ( (_clearsSelectionOnViewWillAppear) && ([self.gridView indexOfSelectedItem] != NSNotFound) )
	{
		[self.gridView deselectItemAtIndex: [self.gridView indexOfSelectedItem] animated: NO];
	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Overriden to allow any orientation.
    return YES;
}

- (void) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation
								 duration: (NSTimeInterval) duration
{
	[self.gridView viewWillRotateToInterfaceOrientation: toInterfaceOrientation];
}

- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation
{
	[self.gridView viewDidRotate];
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}



#pragma mark -
#pragma mark Grid View Data Source

- (NSUInteger) numberOfItemsInGridView: (AQGridView *) gridView
{
	return ( 0 );
}


- (AQGridViewCell *) gridView: (AQGridView *) gridView cellForItemAtIndex: (NSUInteger) index
{
	return ( nil );
}

#pragma mark -
#pragma mark UIPopoverControllerDelegate methods

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
	_popoverShowing = NO;
}

@end
