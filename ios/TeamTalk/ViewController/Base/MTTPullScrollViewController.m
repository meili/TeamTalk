//
//  MTTPullScrollViewController.m
//  TeamTalk
//
//  Created by 宪法 on 15/6/19.
//  Copyright (c) 2015年 MoguIM. All rights reserved.
//

#import "MTTPullScrollViewController.h"

@interface MTTPullScrollViewController ()<UIScrollViewDelegate>

@property (nonatomic,strong) NSDate *beginDragTime;

@end

@implementation MTTPullScrollViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
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


#pragma mark - UIScrollViewDelegate

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView{
    
    _beginDragTime =[NSDate date];
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate{

    /*
    NSDate *currentDate =[NSDate date];
    if ([currentDate timeIntervalSinceDate:_beginDragTime] <0.1) {
        
        CGFloat startOffsetY =[[scrollView valueForKey:@"_startOffsetY"] floatValue];
        
        CGFloat contentOffsetY =scrollView.contentOffset.y;
        
        if (ABS(startOffsetY -contentOffsetY) >160) {
            
            if (self.navigationController.navigationBar) {
                
                CGAffineTransform transform =self.tabBarController.navigationController.navigationBar.transform;
                if (CGAffineTransformEqualToTransform(transform, CGAffineTransformIdentity)) {
                    transform =CGAffineTransformMakeTranslation(0, -100);
                    scrollView.contentInset =UIEdgeInsetsZero;
                }
                else{
                    transform =CGAffineTransformIdentity;
                    scrollView.contentInset =UIEdgeInsetsMake(64, 0, scrollView.contentInset.bottom,0);
                }
                [UIView animateWithDuration:0.2 animations:^{
                    self.navigationController.navigationBar.transform =transform;
                }];
                
            }
            
            if (self.tabBarController.navigationController.navigationBar) {
                
                CGAffineTransform transform =self.tabBarController.navigationController.navigationBar.transform;
                if (CGAffineTransformEqualToTransform(transform, CGAffineTransformIdentity)) {
                    transform =CGAffineTransformMakeTranslation(0, -100);
                    scrollView.contentInset =UIEdgeInsetsZero;
                }
                else{
                    transform =CGAffineTransformIdentity;
                    scrollView.contentInset =UIEdgeInsetsMake(64, 0, scrollView.contentInset.bottom,0);
                }
                [UIView animateWithDuration:0.2 animations:^{
                    self.tabBarController.navigationController.navigationBar.transform =transform;
                }];
            }
            
            if (self.tabBarController.tabBar) {
                
                CGRect frame =self.tabBarController.tabBar.bounds;
                if (frame.origin.y ==0) {
                    frame.origin.y =-100;
                    scrollView.contentInset =UIEdgeInsetsZero;
                }
                else{
                    frame.origin.y =0;
                    scrollView.contentInset =UIEdgeInsetsMake(scrollView.contentInset.top, 0, frame.size.height,0);
                }
                [UIView animateWithDuration:0.2 animations:^{
                    self.tabBarController.tabBar.bounds =frame;
                }];
            }
            
        }
        
    }
    */
}

@end
