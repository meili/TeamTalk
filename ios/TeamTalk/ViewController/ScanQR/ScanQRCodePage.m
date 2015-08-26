//
//  ViewController.m
//  CashBank
//
//  Created by Michael Scofield on 2014-10-23.
//  Copyright (c) 2014 Michael. All rights reserved.
//

#import "ScanQRCodePage.h"
#import "LCActionSheet.h"
#import <SVWebViewController.h>

@interface ScanQRCodePage ()

@end

@implementation ScanQRCodePage

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title=@"扫一扫";

    CGFloat width = 0.8*SCREEN_WIDTH;
    CGFloat height = width;
    
    CGFloat x = (SCREEN_WIDTH - width) / 2;
    CGFloat y = (SCREEN_HEIGHT - height) / 2;
    
    upOrdown = NO;
    num =0;
    _line = [[UIImageView alloc] initWithFrame:CGRectMake(x, y, width, 2)];
    _line.image = [UIImage imageNamed:@"QRCodeScanLine"];
    [self.view addSubview:_line];
    
    timer = [NSTimer scheduledTimerWithTimeInterval:.02 target:self selector:@selector(animation1) userInfo:nil repeats:YES];
     [self setupCamera];
    
    // 添加背景view
    CGFloat topSize = (SCREEN_HEIGHT - height)/2;
    CGFloat leftSize = (SCREEN_WIDTH - width)/2;
    
    UIView *upView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, SCREEN_WIDTH, topSize)];
    upView.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0.5];
    [self.view addSubview:upView];
    
    UIView *leftView = [[UIView alloc] initWithFrame:CGRectMake(0, topSize, leftSize, height)];
    leftView.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0.5];
    [self.view addSubview:leftView];
    
    UIView *rightView = [[UIView alloc] initWithFrame:CGRectMake(SCREEN_WIDTH-leftSize, topSize, leftSize, height)];
    rightView.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0.5];
    [self.view addSubview:rightView];
    
    UIView *bottomView = [[UIView alloc] initWithFrame:CGRectMake(0,SCREEN_HEIGHT-topSize, SCREEN_WIDTH, topSize)];
    bottomView.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0.5];
    [self.view addSubview:bottomView];
    
    UIImageView *leftTopBorder = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"ScanQR1"]];
    leftTopBorder.frame = CGRectMake(leftSize, topSize, 16, 16);
    [self.view addSubview:leftTopBorder];
    
    UIImageView *rightTopBorder = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"ScanQR2"]];
    rightTopBorder.frame = CGRectMake(SCREEN_WIDTH-leftSize-16, topSize, 16, 16);
    [self.view addSubview:rightTopBorder];
    
    UIImageView *leftBottomBorder = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"ScanQR3"]];
    leftBottomBorder.frame = CGRectMake(leftSize, topSize+height-16, 16, 16);
    [self.view addSubview:leftBottomBorder];
    
    UIImageView *rightBottomBorder = [[UIImageView alloc]initWithImage:[UIImage imageNamed:@"ScanQR4"]];
    rightBottomBorder.frame = CGRectMake(SCREEN_WIDTH-leftSize-16, topSize+height-16, 16, 16);
    [self.view addSubview:rightBottomBorder];
    
    UILabel *label = [[UILabel alloc]initWithFrame:CGRectMake(0, topSize+height+20, SCREEN_WIDTH, 14)];
    [self.view addSubview:label];
    [label setText:@"将二维码放入框内,即可自动扫描"];
    [label setFont:systemFont(14)];
    [label setTextColor:[UIColor whiteColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
}

-(void)animation1
{
    CGFloat width = 0.8*SCREEN_WIDTH;
    CGFloat height = width;
    
    CGFloat x = (SCREEN_WIDTH - width) / 2;
    CGFloat y = (SCREEN_HEIGHT - height) / 2;
    if (upOrdown == NO) {
        num ++;
        _line.frame = CGRectMake(x, y+2*num, width, 2);
        if (2*num >= height) {
            upOrdown = YES;
        }
    }
    else {
        num --;
        _line.frame = CGRectMake(x, y+2*num, width, 2);
        if (num == 0) {
            upOrdown = NO;
        }
    }
    
}
-(void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [timer setFireDate:[NSDate distantFuture]];
}
-(void)viewWillAppear:(BOOL)animated
{
    [timer setFireDate:[NSDate distantPast]];
    [_session startRunning];    
}
- (void)setupCamera
{
    // Device
    _device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    
    // Input
    _input = [AVCaptureDeviceInput deviceInputWithDevice:self.device error:nil];
    
    // Output
    _output = [[AVCaptureMetadataOutput alloc]init];
    
    CGFloat width = 300;
    CGFloat height = 300;
    
    CGFloat x = (SCREEN_WIDTH - width) / 2;
    CGFloat y = (SCREEN_HEIGHT - height) / 2;
    
    [_output setRectOfInterest:CGRectMake(y/SCREEN_HEIGHT,x/SCREEN_WIDTH,width/SCREEN_HEIGHT,height/SCREEN_WIDTH)];
    [_output setMetadataObjectsDelegate:self queue:dispatch_get_main_queue()];
    
    // Session
    _session = [[AVCaptureSession alloc]init];
    [_session setSessionPreset:AVCaptureSessionPresetHigh];
    if ([_session canAddInput:self.input])
    {
        [_session addInput:self.input];
    }
    
    if ([_session canAddOutput:self.output])
    {
        [_session addOutput:self.output];
    }
    
    // 条码类型 AVMetadataObjectTypeQRCode
    _output.metadataObjectTypes =@[AVMetadataObjectTypeQRCode];
    _preview =[AVCaptureVideoPreviewLayer layerWithSession:self.session];
    _preview.videoGravity = AVLayerVideoGravityResizeAspectFill;
    _preview.frame =CGRectMake(0, 0, FULL_WIDTH, FULL_HEIGHT);
    [self.view.layer insertSublayer:_preview atIndex:0];
    // Start
    [timer setFireDate:[NSDate distantPast]];
    [_session startRunning];
}
#pragma mark AVCaptureMetadataOutputObjectsDelegate
- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputMetadataObjects:(NSArray *)metadataObjects fromConnection:(AVCaptureConnection *)connection
{
    
    NSString *stringValue;
    
    if ([metadataObjects count] >0)
    {
        AVMetadataMachineReadableCodeObject * metadataObject = [metadataObjects objectAtIndex:0];
        stringValue = metadataObject.stringValue;
        
        [_session stopRunning];
        
        [timer setFireDate:[NSDate distantFuture]];
        [self showResult:stringValue];
    }
    
}
-(void)showScanResult:(NSString *)scanResult
{
    self.scanResult = scanResult;
    // 判断结果是不是url
    NSURL *url = [NSURL URLWithString:scanResult];
    NSString *title = [NSString stringWithFormat:@"%@%@",@"扫描结果是:",scanResult];
    LCActionSheet *sheet;
    if(url){
        sheet = [[LCActionSheet alloc] initWithTitle:title
                                                       buttonTitles:@[@"复制",@"打开",@"再来一发"]
                                                     redButtonIndex:-1
                                                           delegate:self];
        sheet.tag = 10000;
    }else{
        sheet = [[LCActionSheet alloc] initWithTitle:title
                                        buttonTitles:@[@"复制",@"再来一发"]
                                      redButtonIndex:-1
                                            delegate:self];
        sheet.tag = 10001;
    }
    [sheet show];
}
#pragma mark - LCActionSheetDelegate
-(void)actionSheet:(LCActionSheet *)actionSheet didClickedBlankPlace:(NSInteger)buttonIndex
{
    [timer setFireDate:[NSDate distantPast]];
    [_session startRunning];
}

- (void)actionSheet:(LCActionSheet *)actionSheet didClickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(buttonIndex == 0){
        UIPasteboard *pboard = [UIPasteboard generalPasteboard];
        if(self.scanResult.length){
            pboard.string = self.scanResult;
        }
    }
    if(buttonIndex == 1){
        if(actionSheet.tag == 10000){
            SVWebViewController *webViewController = [[SVWebViewController alloc] initWithURL:[NSURL URLWithString:self.scanResult]];
            [self.navigationController pushViewController:webViewController animated:YES];
        }
    }
    [timer setFireDate:[NSDate distantPast]];
    [_session startRunning];
}
-(NSString *)showResult:(NSString *)codeSources
{
    [self showScanResult:codeSources];
    return nil;
    
}
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
@end
