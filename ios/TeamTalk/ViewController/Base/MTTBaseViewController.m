//
//  RootViewController.m
//  TeamTalk
//
//  Created by Michael Scofield on 2015-01-28.
//  Copyright (c) 2015 Michael Hu. All rights reserved.
//

#import "MTTBaseViewController.h"

@interface MTTBaseViewController ()

@end

@implementation MTTBaseViewController

-(id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil{

    if (self =[super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {

        _viewLifecycleState =MTTViewLifecycleInit;
    }
    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder{

    if (self =[super initWithCoder:aDecoder]) {
        
        _viewLifecycleState =MTTViewLifecycleInit;
    }
    return self;
}

-(void)setup{

}

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    _viewLifecycleState =MTTViewLifecycleDidLoad;
    
    UIButton *back = [UIButton buttonWithType:UIButtonTypeCustom];
    back.frame=CGRectMake(0, 0, 60, 40);
    UIImage* image = [UIImage imageNamed:@"top_back"];
    [back setImage:image forState:UIControlStateNormal];
    [back setImageEdgeInsets:UIEdgeInsetsMake(0, 0, 0, 10)];
    [back setTitle:@"返回" forState:UIControlStateNormal];
    [back setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [back addTarget:self action:@selector(p_popViewController) forControlEvents:UIControlEventTouchUpInside];
     UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithCustomView:back];
    self.navigationItem.backBarButtonItem = backButton;
}

-(void)viewWillAppear:(BOOL)animated{

    [super viewWillAppear:animated];
    
    _viewLifecycleState =MTTViewLifecycleWillAppear;

    if (self.tabBarController ==nil) {
        self.navigationItem.hidesBackButton =NO;
    }
}

-(void)viewDidAppear:(BOOL)animated{

    [super viewDidAppear:animated];
    
    _viewLifecycleState =MTTViewLifecycleDidAppear;
    
//    if (self.tabBarController.navigationController.navigationBar) {
//        UIView *navigationBar =self.tabBarController.navigationController.navigationBar;
//        [[navigationBar superview] bringSubviewToFront:navigationBar];
//        NSLog(@"");
//    }
}

-(void)viewWillDisappear:(BOOL)animated{

    [super viewWillDisappear:animated];
    
    _viewLifecycleState =MTTViewLifecycleWillDisAppear;
}

-(void)viewDidDisappear:(BOOL)animated{
    
    [super viewDidDisappear:animated];
    
    _viewLifecycleState =MTTViewLifecycleDidDisAppear;
}

-(void)setTitle:(NSString *)title{

    [super setTitle:title];
    
    self.navigationItem.title =title;

    self.navigationItem.titleView =nil;
}

-(UINavigationItem*)navigationItem{

    if (self.tabBarController) {
        return [self.tabBarController navigationItem];
    }
    return [super navigationItem];
}

-(void)p_popViewController
{
    [self.navigationController popViewControllerAnimated:YES];
    
}


- (void)pushViewController:(UIViewController *)viewController animated:(BOOL)animated{

    if (self.tabBarController ==nil) {
        [self.navigationController pushViewController:viewController animated:animated];
    }
    else{
        [self.tabBarController.navigationController pushViewController:viewController animated:animated];
    }
}

- (UIViewController *)popViewControllerAnimated:(BOOL)animated{

    if (self.tabBarController ==nil) {
        return [self.navigationController popViewControllerAnimated:animated];
    }
    else{
        return [self.tabBarController.navigationController popViewControllerAnimated:animated];
    }
}

- (NSArray *)popToViewController:(UIViewController *)viewController animated:(BOOL)animated{

    if (self.tabBarController ==nil) {
        return [self.navigationController popToViewController:viewController animated:animated];
    }
    else{
        return [self.tabBarController.navigationController popToViewController:viewController animated:animated];
    }
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
