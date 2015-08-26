//
//  DDEmotionsViewController.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <UIKit/UIKit.h>
@protocol DDEmotionsViewControllerDelegate<NSObject>

- (void)emotionViewClickSendButton;

@end
@interface EmotionsViewController : UIViewController<UIScrollViewDelegate>
@property(nonatomic,strong)UIScrollView *scrollView;
@property(nonatomic,strong)UIPageControl *pageControl;
@property(strong)NSArray *emotions;
@property(assign)BOOL isOpen;
@property(nonatomic,assign)id<DDEmotionsViewControllerDelegate>delegate;
@end
