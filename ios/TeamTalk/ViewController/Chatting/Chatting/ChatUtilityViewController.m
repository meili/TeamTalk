//
//  DDDDChatUtilityViewController.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-23.
//  Copyright (c) 2014年 dujia. All rights reserved.
//
#import "ChatUtilityViewController.h"
#import "ChattingMainViewController.h"
#import "AlbumViewController.h"
#import "DDSendPhotoMessageAPI.h"
#import "ChattingMainViewController.h"
#import "DDMessageSendManager.h"
#import "MTTPhotosCache.h"
#import "MTTShakeAPI.h"
#import <Masonry.h>

@interface ChatUtilityViewController ()
@property(nonatomic,strong)NSArray *itemsArray;
@property(nonatomic,strong)UIView *rightView;
@end

@implementation ChatUtilityViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}

- (void)viewDidLoad
{
    MTT_WEAKSELF(ws);
    [super viewDidLoad];
    
    self.view.backgroundColor=RGB(244, 244, 246);
    
    UIView *topLine = [UIView new];
    [topLine setBackgroundColor:[UIColor lightGrayColor]];
    [self.view addSubview:topLine];
    [topLine mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.mas_equalTo(0);
        make.top.mas_equalTo(0);
        make.right.mas_equalTo(0);
        make.height.mas_equalTo(0.5);
    }];
    
    _imagePicker = [[UIImagePickerController alloc] init];
    _imagePicker.delegate = self;
    
    UIView *leftView = [UIView new];
    [self.view addSubview:leftView];
    [leftView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(ws.view);
        make.height.equalTo(ws.view);
        make.top.equalTo(ws.view);
        make.width.equalTo(ws.view).multipliedBy(0.25);
    }];
    
    UIButton *takePhotoBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    [self.view addSubview:takePhotoBtn];
    [takePhotoBtn setBackgroundImage:[UIImage imageNamed:@"chat_take_photo"] forState:UIControlStateNormal];
    [takePhotoBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(leftView);
        make.top.mas_equalTo(15);
        make.size.mas_equalTo(CGSizeMake(65, 65));
    }];
    [takePhotoBtn setClipsToBounds:YES];
    [takePhotoBtn.layer setCornerRadius:5];
    [takePhotoBtn.layer setBorderWidth:0.5];
    [takePhotoBtn.layer setBorderColor:RGB(174, 177, 181).CGColor];
    
    [takePhotoBtn addTarget:self action:@selector(takePicture:) forControlEvents:UIControlEventTouchUpInside];
    
    UILabel *takePhotoLabel = [UILabel new];
    [takePhotoLabel setText:@"拍照"];
    [takePhotoLabel setTextAlignment:NSTextAlignmentCenter];
    [takePhotoLabel setFont:systemFont(13)];
    [takePhotoLabel setTextColor:RGB(174, 177, 181)];
    [self.view addSubview:takePhotoLabel];
    [takePhotoLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(leftView);
        make.top.mas_equalTo(takePhotoBtn.mas_bottom).offset(15);
        make.width.equalTo(leftView);
        make.height.mas_equalTo(13);
    }];
    
    UIView *middleView = [UIView new];
    [self.view addSubview:middleView];
    [middleView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(leftView.mas_right);
        make.height.equalTo(ws.view);
        make.top.equalTo(ws.view);
        make.width.equalTo(ws.view).multipliedBy(0.25);
    }];
    
    UIButton *choosePhotoBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    [self.view addSubview:choosePhotoBtn];
    [choosePhotoBtn setBackgroundImage:[UIImage imageNamed:@"chat_pick_photo"] forState:UIControlStateNormal];
    [choosePhotoBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(middleView);
        make.top.mas_equalTo(15);
        make.size.mas_equalTo(CGSizeMake(65, 65));
    }];
    [choosePhotoBtn setClipsToBounds:YES];
    [choosePhotoBtn.layer setCornerRadius:5];
    [choosePhotoBtn.layer setBorderWidth:0.5];
    [choosePhotoBtn.layer setBorderColor:RGB(174, 177, 181).CGColor];
    [choosePhotoBtn addTarget:self action:@selector(choosePicture:) forControlEvents:UIControlEventTouchUpInside];
    
    UILabel *choosePhotoLabel = [UILabel new];
    [choosePhotoLabel setText:@"相册"];
    [choosePhotoLabel setTextAlignment:NSTextAlignmentCenter];
    [choosePhotoLabel setFont:systemFont(13)];
    [choosePhotoLabel setTextColor:RGB(174, 177, 181)];
    [self.view addSubview:choosePhotoLabel];
    [choosePhotoLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(middleView);
        make.top.mas_equalTo(choosePhotoBtn.mas_bottom).offset(15);
        make.width.equalTo(middleView);
        make.height.mas_equalTo(13);
    }];
    
    _rightView = [UIView new];
    [self.view addSubview:_rightView];
    [_rightView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(middleView.mas_right);
        make.height.equalTo(ws.view);
        make.top.equalTo(ws.view);
        make.width.equalTo(ws.view).multipliedBy(0.25);
    }];
    
    UIButton *shakeBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    [_rightView addSubview:shakeBtn];
    [shakeBtn setBackgroundImage:[UIImage imageNamed:@"chat_shake_pc"] forState:UIControlStateNormal];
    [shakeBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(_rightView);
        make.top.mas_equalTo(15);
        make.size.mas_equalTo(CGSizeMake(65, 65));
    }];
    [shakeBtn setClipsToBounds:YES];
    [shakeBtn.layer setCornerRadius:5];
    [shakeBtn.layer setBorderWidth:0.5];
    [shakeBtn.layer setBorderColor:RGB(174, 177, 181).CGColor];
    [shakeBtn addTarget:self action:@selector(shakePC:) forControlEvents:UIControlEventTouchUpInside];
    
    UILabel *shakeLabel = [UILabel new];
    [shakeLabel setText:@"抖动"];
    [shakeLabel setTextAlignment:NSTextAlignmentCenter];
    [shakeLabel setFont:systemFont(13)];
    [shakeLabel setTextColor:RGB(174, 177, 181)];
    [_rightView addSubview:shakeLabel];
    [shakeLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(shakeBtn);
        make.top.mas_equalTo(shakeBtn.mas_bottom).offset(15);
        make.width.equalTo(_rightView);
        make.height.mas_equalTo(13);
    }];
}
-(void)setShakeHidden
{
    if(self.userId){
        [_rightView setHidden:NO];
    }else{
        [_rightView setHidden:YES];
    }
}
-(void)shakePC:(id)sender
{
    if([MTTUtil ifCanShake])
    {
        NSDate *date = [NSDate date];
        [MTTUtil setLastShakeTime:date];
        MTTShakeAPI *request = [MTTShakeAPI new];
        NSMutableArray *array = [NSMutableArray new];
        [array addObject:@(self.userId)];
        [request requestWithObject:array Completion:^(id response, NSError *error) {
        }];
        [[ChattingMainViewController shareInstance] sendPrompt:@"你向对方发送了一个抖动~"];
        NSString* nick = [RuntimeStatus instance].user.nick;
        NSDictionary *dict = @{@"nick":nick};
    }else{
        [[ChattingMainViewController shareInstance] sendPrompt:@"留条活路吧...别太频繁了"];
    }
}
-(void)choosePicture:(id)sender
{
    self.imagePicker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    self.imagePicker.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
    [self pushViewController:[AlbumViewController new] animated:YES];
}
-(void)takePicture:(id)sender
{
        dispatch_async(dispatch_get_main_queue(), ^{
            if ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera])
            {
                self.imagePicker.sourceType = UIImagePickerControllerSourceTypeCamera;
            }
            self.imagePicker.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
            if (self.imagePicker ) {
                 [[ChattingMainViewController shareInstance].navigationController presentViewController:self.imagePicker animated:NO completion:nil];
            }else{
                self.imagePicker = [[UIImagePickerController alloc] init];
                self.imagePicker.delegate = self;
                self.imagePicker.sourceType = UIImagePickerControllerSourceTypeCamera;
                [[ChattingMainViewController shareInstance].navigationController presentViewController:self.imagePicker animated:NO completion:nil];
            }
           
        });
}
- (void) viewDidUnload
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
   
}

-(void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:YES];
    
    
}
- (void) imagePickerControllerDidCancel: (UIImagePickerController *) picker
{
    
    [picker dismissViewControllerAnimated:YES completion:nil];
      self.imagePicker=nil;
}
-(void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info{
    NSString *mediaType = [info objectForKey:UIImagePickerControllerMediaType];
    if ([mediaType isEqualToString:( NSString *)kUTTypeImage]){
  
        __block UIImage *theImage = nil;
        if ([picker allowsEditing]){
            theImage = [info objectForKey:UIImagePickerControllerEditedImage];
        } else {
            theImage = [info objectForKey:UIImagePickerControllerOriginalImage];
            
        }
        UIImage *image = [self scaleImage:theImage toScale:0.3];
        NSData *imageData = UIImageJPEGRepresentation(image, (CGFloat)1.0);
        UIImage * m_selectImage = [UIImage imageWithData:imageData];
            __block MTTPhotoEnity *photo = [MTTPhotoEnity new];
            NSString *keyName = [[MTTPhotosCache sharedPhotoCache] getKeyName];
            photo.localPath=keyName;
        [picker dismissViewControllerAnimated:NO completion:nil];
        self.imagePicker=nil;
            [[ChattingMainViewController shareInstance] sendImageMessage:photo Image:m_selectImage];
    }

}
#pragma mark -
#pragma mark 等比縮放image
- (UIImage *)scaleImage:(UIImage *)image toScale:(float)scaleSize
{
    UIGraphicsBeginImageContext(CGSizeMake(image.size.width*scaleSize, image.size.height*scaleSize));
    [image drawInRect:CGRectMake(0, 0, image.size.width * scaleSize, image.size.height * scaleSize)];
    UIImage *scaledImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return scaledImage;
}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
