//
//  MTTEditSignViewController.m
//  TeamTalk
//
//  Created by scorpio on 15/6/19.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTEditSignViewController.h"
#import "DDUserModule.h"
#import "MTTSignatureAPI.h"
#import <Masonry/Masonry.h>

@interface MTTEditSignViewController ()

@end

@implementation MTTEditSignViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    MTT_WEAKSELF(ws);
    
    [self.view setBackgroundColor:TTBG];
    self.automaticallyAdjustsScrollViewInsets = NO;
    
    _signText = [UITextView new];
    [self.view addSubview:_signText];

    [_signText mas_makeConstraints:^(MASConstraintMaker *make) {
        make.leading.equalTo(ws.view);
        make.top.mas_equalTo(80);
        make.trailing.equalTo(ws.view);
        make.height.mas_equalTo(100);
    }];
    
    UILabel *desc = [UILabel new];
    [desc setText:@"*限30个字."];
    [desc setTextColor:TTGRAY];
    [desc setFont:systemFont(12)];
    [self.view addSubview:desc];
    [desc mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.mas_equalTo(15);
        make.top.equalTo(_signText.mas_bottom).offset(10);
        make.height.mas_equalTo(15);
        make.right.mas_equalTo(15);
    }];

    _signText.returnKeyType=UIReturnKeyDone;
    _signText.layer.borderColor = RGB(229, 229, 229).CGColor;
    _signText.layer.borderWidth= 1;
    _signText.delegate = self;
    _signText.font = systemFont(16);
    _signText.textContainerInset = UIEdgeInsetsMake(10, 10, 10, 10);
    _signText.delegate = self;
    [_signText becomeFirstResponder];
    
    [[DDUserModule shareInstance] getUserForUserID:[RuntimeStatus instance].user.objID Block:^(MTTUserEntity *user) {
        self.user=user;
        _signText.text = user.signature;
    }];
    
    UIBarButtonItem* item = [[UIBarButtonItem alloc] initWithTitle:@"保存" style:UIBarButtonItemStylePlain target:self action:@selector(Edit)];
    self.navigationItem.rightBarButtonItem=item;
    
    [self setTitle:@"个性签名"];
}

-(void)Edit
{
    NSString *sign = _signText.text;
    MTTSignatureAPI *request = [MTTSignatureAPI new];
    NSMutableArray *array = [NSMutableArray new];
    [array addObject:sign];
    [request requestWithObject:array Completion:^(id response, NSError *error) {
    }];
    [RuntimeStatus instance].user.signature = sign;
    [[DDUserModule shareInstance] getUserForUserID:[RuntimeStatus instance].user.objID Block:^(MTTUserEntity *user) {
        user.signature = sign;
    }];
    [self.navigationController popViewControllerAnimated:YES];
    
}

-(BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text{
    NSString *temp = [textView.text stringByReplacingCharactersInRange:range withString:text];
    if (temp.length > 30) {
        textView.text = [temp substringToIndex:30];
        return NO;
    }
    if ([text isEqualToString:@"\n"]) {
        [self Edit];
        return NO;
    }
    
    return YES;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
