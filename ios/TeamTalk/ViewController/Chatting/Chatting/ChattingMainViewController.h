//
//  DDChattingMainViewController.h
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTBaseViewController.h"
#import "JSMessageInputView.h"
#import "RecorderManager.h"
#import "PlayerManager.h"
#import "JSMessageTextView.h"
#import "MTTMessageEntity.h"
#import "ChattingModule.h"
#import "EmotionsViewController.h"
#import <AssetsLibrary/AssetsLibrary.h>
#import <TTTAttributedLabel.h>
#import "MTTPhotosCache.h"
#import "LCActionSheet.h"
#import "DDChatImagePreviewViewController.h"
typedef void(^TimeCellAddBlock)(bool isok);
@class ChatUtilityViewController;
@class EmotionsViewController;
@class MTTSessionEntity;
@class RecordingView;
@class Photo;
@interface ChattingMainViewController : MTTBaseViewController<UITextViewDelegate, JSMessageInputViewDelegate,UITableViewDataSource,UITableViewDelegate,RecordingDelegate,PlayingDelegate,UIScrollViewDelegate,UIGestureRecognizerDelegate,UIAlertViewDelegate,DDEmotionsViewControllerDelegate,UINavigationControllerDelegate,TTTAttributedLabelDelegate,LCActionSheetDelegate>{
    
    RecordingView* _recordingView;
}
@property(nonatomic,strong)ChattingModule* module;
@property(nonatomic,strong)ChatUtilityViewController *ddUtility;
@property(nonatomic,strong)JSMessageInputView *chatInputView;
@property (assign, nonatomic) CGFloat previousTextViewContentHeight;
@property(nonatomic,weak)IBOutlet UITableView *tableView;
@property(nonatomic,strong)EmotionsViewController *emotions;
@property (assign, nonatomic, readonly) UIEdgeInsets originalTableViewContentInset;
@property (nonatomic, strong) UIRefreshControl* refreshControl;
@property (nonatomic, strong) NSMutableArray* images;
@property (nonatomic, strong) UIImageView* preShow;
@property (nonatomic, strong) ALAsset *lastPhoto;
@property (nonatomic, strong) MTTPhotoEnity *preShowPhoto;
@property (nonatomic, strong) UIImage *preShowImage;
@property (nonatomic, strong) NSString *phoneNumber;
@property (nonatomic, strong) NSString *email;
@property (assign)BOOL hadLoadHistory;
// 去@页面
@property (assign)BOOL isGotoAt;
+(instancetype )shareInstance;

- (void)sendImageMessage:(MTTPhotoEnity *)photo Image:(UIImage *)image;
- (void)sendPrompt:(NSString*)prompt;
-(void)removeImage;
/**
 *  任意页面跳转到聊天界面并开始一个会话
 *
 *  @param session 传入一个会话实体
 */
- (void)loadChattingContentFromSearch:(MTTSessionEntity*)session message:(MTTMessageEntity*)message;
- (void)showChattingContentForSession:(MTTSessionEntity*)session;
- (void)insertEmojiFace:(NSString *)string;
- (void)deleteEmojiFace;
- (void)p_popViewController;
@end


@interface ChattingMainViewController(ChattingInput)
- (void)initialInput;
@end
