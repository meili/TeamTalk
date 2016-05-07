//
//  ViewController.h
//  CashBank
//
//  Created by Michael Scofield on 2014-10-23.
//  Copyright (c) 2014 Michael. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import "LCActionSheet.h"
@interface ScanQRCodePage : UIViewController<AVCaptureMetadataOutputObjectsDelegate,LCActionSheetDelegate>
{
    int num;
    BOOL upOrdown;
    NSTimer * timer;
}
@property (strong,nonatomic)AVCaptureDevice * device;
@property (strong,nonatomic)AVCaptureDeviceInput * input;
@property (strong,nonatomic)AVCaptureMetadataOutput * output;
@property (strong,nonatomic)AVCaptureSession * session;
@property (strong,nonatomic)AVCaptureVideoPreviewLayer * preview;
@property (nonatomic, retain) UIImageView * line;
@property (nonatomic, retain) NSString *scanResult;
@property (weak) IBOutlet UIView *scanView;
-(IBAction)scanAgain:(id)sender;
@end

