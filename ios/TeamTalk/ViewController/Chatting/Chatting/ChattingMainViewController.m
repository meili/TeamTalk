//
//  DDChattingMainViewController.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-26.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "ChattingMainViewController.h"
#import "ChatUtilityViewController.h"
#import "MTTPhotosCache.h"
#import "DDGroupModule.h"
#import "DDMessageSendManager.h"
#import "MsgReadACKAPI.h"
#import "MTTDatabaseUtil.h"
#import "DDChatTextCell.h"
#import "DDChatVoiceCell.h"
#import "DDChatImageCell.h"
#import "DDChattingEditViewController.h"
#import "DDPromptCell.h"
#import "UIView+Addition.h"
#import "DDMessageModule.h"
#import "RecordingView.h"
#import "TouchDownGestureRecognizer.h"
#import "DDSendPhotoMessageAPI.h"
#import "NSDictionary+JSON.h"
#import "EmotionsModule.h"
#import "RuntimeStatus.h"
#import "DDEmotionCell.h"
#import "RecentUsersViewController.h"
#import "PublicProfileViewControll.h"
#import "UnAckMessageManager.h"
#import "GetMessageQueueAPI.h"
#import "GetLatestMsgId.h"
#import "MTTPhotosCache.h"
#import "UIScrollView+PullToLoadMore.h"
#import "UIImageView+WebCache.h"
#import "MTTWebViewController.h"
#import <SVWebViewController.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import "MTTPhotosCache.h"
#import "ContactsViewController.h"
#import "DDUserModule.h"
#import "LCActionSheet.h"
#import "MTTUtil.h"
#import "UIImageView+WebCache.h"
#import "MTTUsersStatAPI.h"

typedef NS_ENUM(NSUInteger, DDBottomShowComponent)
{
    DDInputViewUp                       = 1,
    DDShowKeyboard                      = 1 << 1,
    DDShowEmotion                       = 1 << 2,
    DDShowUtility                       = 1 << 3
};

typedef NS_ENUM(NSUInteger, DDBottomHiddComponent)
{
    DDInputViewDown                     = 14,
    DDHideKeyboard                      = 13,
    DDHideEmotion                       = 11,
    DDHideUtility                       = 7
};
//

typedef NS_ENUM(NSUInteger, DDInputType)
{
    DDVoiceInput,
    DDTextInput
};

typedef NS_ENUM(NSUInteger, PanelStatus)
{
    VoiceStatus,
    TextInputStatus,
    EmotionStatus,
    ImageStatus
};

#define DDINPUT_MIN_HEIGHT          44.0f
#define DDINPUT_HEIGHT              self.chatInputView.size.height
#define DDINPUT_BOTTOM_FRAME        CGRectMake(0, CONTENT_HEIGHT - self.chatInputView.height + NAVBAR_HEIGHT,FULL_WIDTH,self.chatInputView.height)
#define DDINPUT_TOP_FRAME           CGRectMake(0, CONTENT_HEIGHT - self.chatInputView.height + NAVBAR_HEIGHT - 216, FULL_WIDTH, self.chatInputView.height)
#define DDUTILITY_FRAME             CGRectMake(0, CONTENT_HEIGHT + NAVBAR_HEIGHT -216, FULL_WIDTH, 216)
#define DDEMOTION_FRAME             CGRectMake(0, CONTENT_HEIGHT + NAVBAR_HEIGHT-216, FULL_WIDTH, 216)
#define DDCOMPONENT_BOTTOM          CGRectMake(0, CONTENT_HEIGHT + NAVBAR_HEIGHT, FULL_WIDTH, 216)

@interface ChattingMainViewController ()<UIGestureRecognizerDelegate>
@property(nonatomic,assign)CGPoint inputViewCenter;
@property(nonatomic,assign)BOOL ifScrollBottom;
@property(assign)PanelStatus panelStatus;
@property(strong)NSString *chatObjectID;
@property(strong) UIButton *titleBtn ;

- (UITableViewCell*)p_textCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message;
- (UITableViewCell*)p_voiceCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message;
- (UITableViewCell*)p_promptCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(DDPromptEntity*)prompt;

- (void)n_receiveMessage:(NSNotification*)notification;
- (void)p_clickThRecordButton:(UIButton*)button;
- (void)p_record:(UIButton*)button;
- (void)p_willCancelRecord:(UIButton*)button;
- (void)p_cancelRecord:(UIButton*)button;
- (void)p_sendRecord:(UIButton*)button;
- (void)p_endCancelRecord:(UIButton*)button;

- (void)p_tapOnTableView:(UIGestureRecognizer*)sender;
- (void)p_hideBottomComponent;

- (void)p_enableChatFunction;
- (void)p_unableChatFunction;

@end

@implementation ChattingMainViewController
{
    TouchDownGestureRecognizer* _touchDownGestureRecognizer;
    NSString* _currentInputContent;
    UIButton *_recordButton;
    DDBottomShowComponent _bottomShowComponent;
    float _inputViewY;
    int _type;
}
+(instancetype )shareInstance
{
    static dispatch_once_t onceToken;
    static ChattingMainViewController *_sharedManager = nil;
    dispatch_once(&onceToken, ^{
        _sharedManager = [ChattingMainViewController new];
    });
    return _sharedManager;
}

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    CGPoint location = [gestureRecognizer locationInView:self.view];
    if (CGRectContainsPoint(DDINPUT_BOTTOM_FRAME, location))
    {
        return NO;
    }
    return YES;
}

-(void)sendImageMessage:(MTTPhotoEnity *)photo Image:(UIImage *)image
{
    NSDictionary* messageContentDic = @{DD_IMAGE_LOCAL_KEY:photo.localPath};
    NSString* messageContent = [messageContentDic jsonString];
    
    MTTMessageEntity *message = [MTTMessageEntity makeMessage:messageContent Module:self.module MsgType:DDMessageTypeImage];
    [self.tableView reloadData];
    [self scrollToBottomAnimated:YES];
    NSData *photoData = UIImagePNGRepresentation(image);
    [[MTTPhotosCache sharedPhotoCache] storePhoto:photoData forKey:photo.localPath toDisk:YES];
    //[self.chatInputView.textView setText:@""];
    [[MTTDatabaseUtil instance] insertMessages:@[message] success:^{
        DDLog(@"消息插入DB成功");
        
    } failure:^(NSString *errorDescripe) {
        DDLog(@"消息插入DB失败");
    }];
    photo=nil;
    [[DDSendPhotoMessageAPI sharedPhotoCache] uploadImage:messageContentDic[DD_IMAGE_LOCAL_KEY] success:^(NSString *imageURL) {
        [self scrollToBottomAnimated:YES];
        message.state=DDMessageSending;
        NSDictionary* tempMessageContent = [NSDictionary initWithJsonString:message.msgContent];
        NSMutableDictionary* mutalMessageContent = [[NSMutableDictionary alloc] initWithDictionary:tempMessageContent];
        [mutalMessageContent setValue:imageURL forKey:DD_IMAGE_URL_KEY];
        NSString* messageContent = [mutalMessageContent jsonString];
        message.msgContent = messageContent;
        [self sendMessage:imageURL messageEntity:message];
        [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
        }];
        
    } failure:^(id error) {
        message.state = DDMessageSendFailure;
        //刷新DB
        [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
            if (result)
            {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [_tableView reloadData];
                });
            }
        }];
        
    }];
}

- (void)textViewChanged
{
    NSRange range = self.chatInputView.textView.selectedRange;
    NSInteger location = range.location;
    __block NSString* text = [self.chatInputView.textView text];
    if(location){
        __block NSRange range = NSMakeRange(location-1, 1);
        NSString* lastText =  [text substringWithRange:range];
        if([lastText isEqualToString:@"@"]){
            self.isGotoAt = YES;
            ContactsViewController *contact = [ContactsViewController new];
            contact.isFromAt=YES;
            contact.selectUser =^(MTTUserEntity *user){
                NSString *atName = [NSString stringWithFormat:@"@%@ ",user.nick];
                text = [text stringByReplacingCharactersInRange:range withString:atName];
                [self.chatInputView.textView setText:text];
            };
            [self.navigationController pushViewController:contact animated:YES];
        }
    }
}

- (void)textViewEnterSend
{
    //发送消息
    NSString* text = [self.chatInputView.textView text];
    
    NSString* parten = @"\\s";
    NSRegularExpression* reg = [NSRegularExpression regularExpressionWithPattern:parten options:NSRegularExpressionCaseInsensitive error:nil];
    NSString* checkoutText = [reg stringByReplacingMatchesInString:text options:NSMatchingReportProgress range:NSMakeRange(0, [text length]) withTemplate:@""];
    if ([checkoutText length] == 0)
    {
        return;
    }
    DDMessageContentType msgContentType = DDMessageTypeText;
    MTTMessageEntity *message = [MTTMessageEntity makeMessage:text Module:self.module MsgType:msgContentType];
    [self.tableView reloadData];
    [self.chatInputView.textView setText:nil];
    [[MTTDatabaseUtil instance] insertMessages:@[message] success:^{
        DDLog(@"消息插入DB成功");
    } failure:^(NSString *errorDescripe) {
        DDLog(@"消息插入DB失败");
    }];
    [self sendMessage:text messageEntity:message];
}


- (void)sendPrompt:(NSString*)prompt
{
    [self.module addPrompt:prompt];
    [self.tableView reloadData];
    [self scrollToBottomAnimated:YES];
}

-(void)sendMessage:(NSString *)msg messageEntity:(MTTMessageEntity *)message
{
    BOOL isGroup = [self.module.MTTSessionEntity isGroup];
    [[DDMessageSendManager instance] sendMessage:message isGroup:isGroup Session:self.module.MTTSessionEntity  completion:^(MTTMessageEntity* theMessage,NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            message.state= theMessage.state;
            [self.tableView reloadData];
            [self scrollToBottomAnimated:YES];
        });
    } Error:^(NSError *error) {
        [self.tableView reloadData];
    }];
}
//--------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark RecordingDelegate
- (void)recordingFinishedWithFileName:(NSString *)filePath time:(NSTimeInterval)interval
{
    NSMutableData* muData = [[NSMutableData alloc] init];
    NSData* data = [NSData dataWithContentsOfFile:filePath];
    int length = [RecorderManager sharedManager].recordedTimeInterval;
    if (length < 1 )
    {
        DDLog(@"录音时间太短");
        dispatch_async(dispatch_get_main_queue(), ^{
            [_recordingView setHidden:NO];
            [_recordingView setRecordingState:DDShowRecordTimeTooShort];
        });
        return;
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [_recordingView setHidden:YES];
        });
    }
    int8_t ch[4];
    for(int32_t i = 0;i<4;i++){
        ch[i] = ((length >> ((3 - i)*8)) & 0x0ff);
    }
    [muData appendBytes:ch length:4];
    [muData appendData:data];
    DDMessageContentType msgContentType = DDMessageTypeVoice;
    MTTMessageEntity* message = [MTTMessageEntity makeMessage:filePath Module:self.module MsgType:msgContentType];
    [self.tableView reloadData];
    [self scrollToBottomAnimated:YES];
    BOOL isGroup = [self.module.MTTSessionEntity isGroup];
    if (isGroup) {
        message.msgType=MsgTypeMsgTypeGroupAudio;
    }else
    {
        message.msgType = MsgTypeMsgTypeSingleAudio;
    }
    [message.info setObject:@(length) forKey:VOICE_LENGTH];
    [message.info setObject:@(1) forKey:DDVOICE_PLAYED];
    dispatch_async(dispatch_get_main_queue(), ^{
        [self scrollToBottomAnimated:YES];
        [[MTTDatabaseUtil instance] insertMessages:@[message] success:^{
            NSLog(@"消息插入DB成功");
        } failure:^(NSString *errorDescripe) {
            NSLog(@"消息插入DB失败");
        }];
        
    });
    
    [[DDMessageSendManager instance] sendVoiceMessage:muData filePath:filePath forSessionID:self.module.MTTSessionEntity.sessionID isGroup:isGroup Message:message Session:self.module.MTTSessionEntity completion:^(MTTMessageEntity *theMessage, NSError *error) {
        if (!error)
        {
            DDLog(@"发送语音消息成功");
            [[PlayerManager sharedManager] playAudioWithFileName:@"msg.caf" playerType:DDSpeaker delegate:self];
            message.state = DDmessageSendSuccess;
            [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    [_tableView reloadData];
                });
                
                
            }];
        }
        else
        {
            DDLog(@"发送语音消息失败");
            message.state = DDMessageSendFailure;
            [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    [_tableView reloadData];
                });
                
            }];
            
        }
    }];
}

- (void)playingStoped
{
    
}
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        self.titleBtn = [UIButton buttonWithType:UIButtonTypeCustom];
        self.titleBtn.frame=CGRectMake(0, 0, 150, 40);
        [self.titleBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [self.titleBtn setTitleColor:[UIColor blackColor] forState:UIControlStateHighlighted];
        [self.titleBtn addTarget:self action:@selector(titleTap:) forControlEvents:UIControlEventTouchUpInside];
        [self.titleBtn.titleLabel setTextAlignment:NSTextAlignmentLeft];
    }
    return self;
}
-(void)notificationCenter
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(n_receiveMessage:)
                                                 name:DDNotificationReceiveMessage
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleWillShowKeyboard:)
                                                 name:UIKeyboardWillShowNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleWillHideKeyboard:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reloginSuccess)
                                                 name:@"ReloginSuccess"
                                               object:nil];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self notificationCenter];
    [self initialInput];
    //    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self
    //                                                                          action:@selector(p_tapOnTableView:)];
    //    [self.tableView addGestureRecognizer:tap];
    
    UIPanGestureRecognizer* pan = [[UIPanGestureRecognizer alloc] initWithTarget:self
                                                                          action:@selector(p_tapOnTableView:)];
    pan.delegate = self;
    [self.tableView addGestureRecognizer:pan];
    self.tableView.delegate=self;
    self.tableView.dataSource=self;
    //    [self.tableView setBackgroundColor:TTBG];
    [self.tableView setBackgroundColor:[UIColor clearColor]];
    UIView* headView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, FULL_WIDTH, MTTRefreshViewHeight)];
    [headView setBackgroundColor:[UIColor clearColor]];
    [self.tableView setTableHeaderView:headView];
    
    [self scrollToBottomAnimated:NO];
    
    [self initScrollView];
    
    UIBarButtonItem* item = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"myprofile"]
                                                             style:UIBarButtonItemStylePlain
                                                            target:self
                                                            action:@selector(Edit:)];
    self.navigationItem.rightBarButtonItem=item;
    [self.module addObserver:self
                  forKeyPath:@"showingMessages"
                     options:NSKeyValueObservingOptionOld|NSKeyValueObservingOptionNew
                     context:NULL];
    [self.module addObserver:self
                  forKeyPath:@"MTTSessionEntity.sessionID"
                     options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                     context:NULL];
    [self.navigationItem.titleView setUserInteractionEnabled:YES];
    self.view.backgroundColor=TTBG;
    
    if([TheRuntime.user.nick isEqualToString:@"蝎紫"]){
        UIImageView *chatBgView = [[UIImageView alloc]initWithFrame:CGRectMake(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)];
        [chatBgView setImage:[UIImage imageNamed:@"chatBg"]];
        [self.view insertSubview:chatBgView atIndex:0];
        self.tableView.backgroundView =chatBgView;
    }
    
    self.navigationController.interactivePopGestureRecognizer.enabled = YES;
    self.isGotoAt = NO;
    self.ifScrollBottom = YES;
}

-(void)queryUserStat
{
    MTTUsersStatAPI *request = [MTTUsersStatAPI new];
    NSMutableArray *array = [NSMutableArray new];
    NSString *sessionId = self.module.MTTSessionEntity.sessionID;
    UInt32 uid = [MTTUserEntity localIDTopb:sessionId];
    [array addObject:@(uid)];
    [request requestWithObject:array Completion:^(NSArray *response, NSError *error) {
//        if(response){
//            NSMutableArray *narray = [NSMutableArray arrayWithArray:response];
//            if([narray[0][1] intValue]== UserStatTypeUserStatusOnline){
//            }
//        }
    }];
}

-(void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    if(!self.isGotoAt){
        [self.chatInputView.textView setText:nil];
    }
    self.isGotoAt = NO;
    
    self.navigationItem.titleView=self.titleBtn;
    
    self.tableView.noMore =NO;
}

-(void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    [self.chatInputView.textView setEditable:YES];
    
    [self.navigationController.navigationBar setHidden:NO];
    if (self.ddUtility != nil)
    {
        NSString *sessionId = self.module.MTTSessionEntity.sessionID;
        self.ddUtility.userId = [MTTUserEntity localIDTopb:sessionId];
    }
}
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [self.module.ids removeAllObjects];
    
    //    [[PlayerManager sharedManager] stopPlaying];
}

- (void)viewDidDisappear:(BOOL)animated
{
    if(!self.isGotoAt){
        [super viewDidDisappear:animated];
        [self.chatInputView.textView setEditable:NO];
    }
    
}


-(void)initScrollView{
    
    __weak ChattingMainViewController *tmpSelf =self;
    
    //下拉加载历史数据
    [self.tableView setRefreshHandler:^{
        
        [tmpSelf loadHistoryRecords];
    }];
    
}

-(void)loadHistoryRecords{
    
    __weak ChattingMainViewController *tmpSelf =self;
    self.hadLoadHistory=YES;
//    NSInteger preCount = [self.module.showingMessages count];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        
        CGFloat contentSizeHeightOld =tmpSelf.tableView.contentSize.height;
        CGFloat contentOffsetYOld =tmpSelf.tableView.contentOffset.y;
        
        
        [tmpSelf.module loadMoreHistoryCompletion:^(NSUInteger addCount,NSError *error) {
        
            [tmpSelf.tableView reloadData];
            
            CGFloat contentSizeHeightNew =tmpSelf.tableView.contentSize.height;
            CGFloat contentOffsetYNew =contentSizeHeightNew -contentSizeHeightOld +contentOffsetYOld;
            
            if (addCount == 0){
                tmpSelf.tableView.noMore =YES;
            }
            else{
                [tmpSelf.tableView setContentOffset:CGPointMake(0, contentOffsetYNew)];
            }
            
            [tmpSelf.tableView refreshFinished];
        }];
    });
    
}

-(void)setThisViewTitle:(NSString *)title
{
    [self.titleBtn setTitle:title forState:UIControlStateNormal];
    [self queryUserStat];
}
-(IBAction)Edit:(id)sender
{
    DDChattingEditViewController *chattingedit = [DDChattingEditViewController new];
    chattingedit.session=self.module.MTTSessionEntity;
    self.title=@"";
    [self pushViewController:chattingedit animated:YES];
}



- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void)scrollToBottomAnimated:(BOOL)animated
{
    NSInteger rows = [self.tableView numberOfRowsInSection:0];
    if(rows > 0) {
        [self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:rows - 1 inSection:0]
                              atScrollPosition:UITableViewScrollPositionBottom
                                      animated:animated];
        
    }
    
}

- (ChattingModule*)module
{
    if (!_module)
    {
        _module = [[ChattingModule alloc] init];
    }
    return _module;
}

#pragma mark -
#pragma mark ActionMethods  发送sendAction 音频 voiceChange  显示表情 disFaceKeyboard
-(IBAction)sendAction:(id)sender{
    if (self.chatInputView.textView.text.length>0) {
        NSLog(@"点击发送");
        [self.chatInputView.textView setText:@""];
    }
}

#pragma mark -
#pragma mark UIGesture Delegate
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
    if ([gestureRecognizer.view isEqual:_tableView])
    {
        return YES;
    }
    return NO;
}

#pragma mark - EmojiFace Funcation
-(void)insertEmojiFace:(NSString *)string
{
    DDMessageContentType msgContentType = DDMEssageEmotion;
    MTTMessageEntity *message = [MTTMessageEntity makeMessage:string Module:self.module MsgType:msgContentType];
    [self.tableView reloadData];
    //[self.chatInputView.textView setText:nil];
    [[MTTDatabaseUtil instance] insertMessages:@[message] success:^{
        DDLog(@"消息插入DB成功");
    } failure:^(NSString *errorDescripe) {
        DDLog(@"消息插入DB失败");
    }];
    [self sendMessage:string messageEntity:message];
    
}
-(void)deleteEmojiFace
{
    EmotionsModule* emotionModule = [EmotionsModule shareInstance];
    NSString* toDeleteString = nil;
    if (self.chatInputView.textView.text.length == 0)
    {
        return;
    }
    if (self.chatInputView.textView.text.length == 1)
    {
        self.chatInputView.textView.text = @"";
    }
    else
    {
        toDeleteString = [self.chatInputView.textView.text substringFromIndex:self.chatInputView.textView.text.length - 1];
        int length = [emotionModule.emotionLength[toDeleteString] intValue];
        if (length == 0)
        {
            toDeleteString = [self.chatInputView.textView.text substringFromIndex:self.chatInputView.textView.text.length - 2];
            length = [emotionModule.emotionLength[toDeleteString] intValue];
        }
        length = length == 0 ? 1 : length;
        self.chatInputView.textView.text = [self.chatInputView.textView.text substringToIndex:self.chatInputView.textView.text.length - length];
    }
    
}
#pragma mark - UITableView DataSource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.module.showingMessages count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    float height = 0;
    id object = self.module.showingMessages[indexPath.row];
    if ([object isKindOfClass:[MTTMessageEntity class]])
    {
        MTTMessageEntity* message = object;
        height = [self.module messageHeight:message];
    }
    else if([object isKindOfClass:[DDPromptEntity class]])
    {
        height = 30;
    }
    return height+10;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    id object = self.module.showingMessages[indexPath.row];
    UITableViewCell* cell = nil;
    if ([object isKindOfClass:[MTTMessageEntity class]])
    {
        MTTMessageEntity* message = (MTTMessageEntity*)object;
        if (message.msgContentType == DDMessageTypeText ) {
            cell = [self p_textCell_tableView:tableView cellForRowAtIndexPath:indexPath message:message];
        }else if (message.msgContentType == DDMessageTypeVoice)
        {
            cell = [self p_voiceCell_tableView:tableView cellForRowAtIndexPath:indexPath message:message];
        }
        else if(message.msgContentType == DDMessageTypeImage)
        {
            cell = [self p_imageCell_tableView:tableView cellForRowAtIndexPath:indexPath message:message];
        }else if (message.msgContentType == DDMEssageEmotion)
        {
            cell = [self p_emotionCell_tableView:tableView cellForRowAtIndexPath:indexPath message:message];
        }
        else
        {
            cell = [self p_textCell_tableView:tableView cellForRowAtIndexPath:indexPath message:message];
        }
        
    }
    else if ([object isKindOfClass:[DDPromptEntity class]])
    {
        DDPromptEntity* prompt = (DDPromptEntity*)object;
        cell = [self p_promptCell_tableView:tableView cellForRowAtIndexPath:indexPath message:prompt];
    }
    
    return cell;
}


- (void)scrollViewDidScroll:(UIScrollView *)scrollView{
    
    if(scrollView.contentOffset.y + self.tableView.height >= self.tableView.contentSize.height - 100){
        self.ifScrollBottom = YES;
    }else{
        self.ifScrollBottom = NO;
    }
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate{
    // 上拉弹出键盘
    //    if (scrollView.contentOffset.y < scrollView.contentSize.height -scrollView.frame.size.height +scrollView.contentInset.bottom){
    //        [self p_hideBottomComponent];
    //    }
    //    else if (scrollView.contentOffset.y > scrollView.contentSize.height -scrollView.frame.size.height +scrollView.contentInset.bottom +80) {
    //            [self.chatInputView.textView becomeFirstResponder];
    //    }
}

#pragma mark PublicAPI
- (void)loadChattingContentFromSearch:(MTTSessionEntity*)session message:(MTTMessageEntity*)message
{
    [self.module.showingMessages removeAllObjects];
    [self.tableView reloadData];
    self.module.MTTSessionEntity = nil;
    self.hadLoadHistory=YES;
    [self.module.showingMessages removeAllObjects];
    self.module.MTTSessionEntity = session;
    [self setThisViewTitle:session.name];
    [self.module loadAllHistoryCompletion:message Completion:^(NSUInteger addcount, NSError *error) {
        [self.tableView reloadData];
    }];
}
- (void)showChattingContentForSession:(MTTSessionEntity*)session
{
    self.module.MTTSessionEntity = nil;
    self.hadLoadHistory=NO;
    [self p_unableChatFunction];
    [self p_enableChatFunction];
    [self.module.showingMessages removeAllObjects];
    [self.tableView reloadData];
    self.module.MTTSessionEntity = session;
    [self setThisViewTitle:session.name];
    //    self.module.MTTSessionEntity.unReadMsgCount = 0;
    [self.module loadMoreHistoryCompletion:^(NSUInteger addcount, NSError *error) {
        [self.tableView reloadData];
        if (self.hadLoadHistory == NO) {
            [self scrollToBottomAnimated:NO];
        }
        if (session.unReadMsgCount !=0 ) {
            
            MsgReadACKAPI* readACK = [[MsgReadACKAPI alloc] init];
            if(self.module.MTTSessionEntity.sessionID){
                [readACK requestWithObject:@[self.module.MTTSessionEntity.sessionID,@(self.module.MTTSessionEntity.lastMsgID),@(self.module.MTTSessionEntity.sessionType)] Completion:nil];
                self.module.MTTSessionEntity.unReadMsgCount=0;
                [[MTTDatabaseUtil instance] updateRecentSession:self.module.MTTSessionEntity completion:^(NSError *error) {
                    
                }];
            }
            
        }
        
    }];
}
#pragma mark - Text view delegatef

- (void)viewheightChanged:(float)height
{
    [self setValue:@(self.chatInputView.origin.y) forKeyPath:@"_inputViewY"];
}

#pragma mark PrivateAPI


- (UITableViewCell*)p_textCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message
{
    static NSString* identifier = @"DDChatTextCellIdentifier";
    DDChatBaseCell* cell = (DDChatBaseCell*)[tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell)
    {
        cell = [[DDChatTextCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
        cell.contentLabel.delegate = self;
    }
    cell.session =self.module.MTTSessionEntity;
    NSString* myUserID = [RuntimeStatus instance].user.objID;
    if ([message.senderId isEqualToString:myUserID])
    {
        [cell setLocation:DDBubbleRight];
    }
    else
    {
        [cell setLocation:DDBubbleLeft];
    }
    
    if (![[UnAckMessageManager instance] isInUnAckQueue:message] && message.state == DDMessageSending && [message isSendBySelf]) {
        message.state=DDMessageSendFailure;
    }
    [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
        
    }];
    
    [cell setContent:message];
    __weak DDChatTextCell* weakCell = (DDChatTextCell*)cell;
    cell.sendAgain = ^{
        [weakCell showSending];
        [weakCell sendTextAgain:message];
    };
    
    return cell;
}

- (UITableViewCell*)p_voiceCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message
{
    static NSString* identifier = @"DDVoiceCellIdentifier";
    DDChatBaseCell* cell = (DDChatBaseCell*)[tableView dequeueReusableCellWithIdentifier:identifier];
    
    if (!cell)
    {
        cell = [[DDChatVoiceCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
    }
    cell.session =self.module.MTTSessionEntity;
    NSString* myUserID = [RuntimeStatus instance].user.objID;
    if ([message.senderId isEqualToString:myUserID])
    {
        [cell setLocation:DDBubbleRight];
    }
    else
    {
        [cell setLocation:DDBubbleLeft];
    }
    [cell setContent:message];
    __weak DDChatVoiceCell* weakCell = (DDChatVoiceCell*)cell;
    [(DDChatVoiceCell*)cell setTapInBubble:^{
        //播放语音
        if ([[PlayerManager sharedManager] playingFileName:message.msgContent]) {
            [[PlayerManager sharedManager] stopPlaying];
        }else{
            NSString* fileName = message.msgContent;
            [[PlayerManager sharedManager] playAudioWithFileName:fileName delegate:self];
            [message.info setObject:@(1) forKey:DDVOICE_PLAYED];
            [weakCell showVoicePlayed];
            [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
            }];
            
        }
        
    }];
    
    [(DDChatVoiceCell*)cell setEarphonePlay:^{
        //听筒播放
        NSString* fileName = message.msgContent;
        [[PlayerManager sharedManager] playAudioWithFileName:fileName playerType:DDEarPhone delegate:self];
        [message.info setObject:@(1) forKey:DDVOICE_PLAYED];
        [weakCell showVoicePlayed];
        
        [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
            
        }];
        
    }];
    
    [(DDChatVoiceCell*)cell setSpeakerPlay:^{
        //扬声器播放
        NSString* fileName = message.msgContent;
        [[PlayerManager sharedManager] playAudioWithFileName:fileName playerType:DDSpeaker delegate:self];
        [message.info setObject:@(1) forKey:DDVOICE_PLAYED];
        [weakCell showVoicePlayed];
        [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
            
        }];
        
    }];
    [(DDChatVoiceCell *)cell setSendAgain:^{
        //重发
        [weakCell showSending];
        [weakCell sendVoiceAgain:message];
    }];
    return cell;
}

- (UITableViewCell*)p_promptCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(DDPromptEntity*)prompt
{
    static NSString* identifier = @"DDPromptCellIdentifier";
    DDPromptCell* cell = (DDPromptCell*)[tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell)
    {
        cell = [[DDPromptCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
    }
    NSString* promptMessage = prompt.message;
    [cell setprompt:promptMessage];
    return cell;
}

- (UITableViewCell*)p_emotionCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message
{
    static NSString* identifier = @"DDEmotionCellIdentifier";
    DDEmotionCell* cell = (DDEmotionCell*)[tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell)
    {
        cell = [[DDEmotionCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
    }
    cell.session =self.module.MTTSessionEntity;
    NSString* myUserID =[RuntimeStatus instance].user.objID;
    if ([message.senderId isEqualToString:myUserID])
    {
        [cell setLocation:DDBubbleRight];
    }
    else
    {
        [cell setLocation:DDBubbleLeft];
    }
    
    [cell setContent:message];
    __weak DDEmotionCell* weakCell = cell;
    
    [cell setSendAgain:^{
        [weakCell sendTextAgain:message];
        
    }];
    
    [cell setTapInBubble:^{
        
    }];
    return cell;
}


- (UITableViewCell*)p_imageCell_tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath message:(MTTMessageEntity*)message
{
    static NSString* identifier = @"DDImageCellIdentifier";
    DDChatImageCell* cell = (DDChatImageCell*)[tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell)
    {
        cell = [[DDChatImageCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
    }
    cell.session =self.module.MTTSessionEntity;
    NSString* myUserID =[RuntimeStatus instance].user.objID;
    if ([message.senderId isEqualToString:myUserID])
    {
        [cell setLocation:DDBubbleRight];
    }
    else
    {
        [cell setLocation:DDBubbleLeft];
    }
    
    [[MTTDatabaseUtil instance] updateMessageForMessage:message completion:^(BOOL result) {
        
    }];
    [cell setContent:message];
    __weak DDChatImageCell* weakCell = cell;
    
    [cell setSendAgain:^{
        [weakCell sendImageAgain:message];
        
    }];
    
    [cell setTapInBubble:^{
        NSString *originUrl =  message.msgContent;
        originUrl = [originUrl stringByReplacingOccurrencesOfString:DD_MESSAGE_IMAGE_PREFIX withString:@""];
        originUrl = [originUrl stringByReplacingOccurrencesOfString:DD_MESSAGE_IMAGE_SUFFIX withString:@""];
        NSURL* url = [NSURL URLWithString:originUrl];
        NSMutableArray *photos = [[NSMutableArray alloc]init];
        [self.module.showingMessages enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            if ([obj isKindOfClass:[MTTMessageEntity class]])
            {
                MTTMessageEntity* message = (MTTMessageEntity*)obj;
                NSURL* url;
                if(message.msgContentType == DDMessageTypeImage){
                    NSString* urlString = message.msgContent;
                    urlString = [urlString stringByReplacingOccurrencesOfString:DD_MESSAGE_IMAGE_PREFIX withString:@""];
                    urlString = [urlString stringByReplacingOccurrencesOfString:DD_MESSAGE_IMAGE_SUFFIX withString:@""];
                    if([urlString rangeOfString:@"\"local\" : "].length >0){
                        NSData* data = [urlString dataUsingEncoding:NSUTF8StringEncoding];
                        NSDictionary* dic = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
                        url = [NSURL URLWithString:dic[@"url"]];
                    }else{
                        url = [NSURL URLWithString:urlString];
                    }
                    if(url){
                        [photos addObject:url];
                    }
                }
            }
        }];
        DDChatImagePreviewViewController *preViewControll = [DDChatImagePreviewViewController new];
        NSMutableArray *array = [NSMutableArray array];
        [photos enumerateObjectsUsingBlock:^(NSURL *obj, NSUInteger idx, BOOL *stop) {
            [array addObject:[MWPhoto photoWithURL:obj]];
        }];
        preViewControll.photos=array;
        preViewControll.index=[photos indexOfObject:url];
//        [preViewControll addChildViewController:preViewControll];
        
        [self presentViewController:preViewControll animated:YES completion:NULL];
    }];
    
    [cell setPreview:cell.tapInBubble];
    
    return cell;
}

- (void)p_clickThRecordButton:(UIButton*)button
{
    switch (button.tag) {
        case DDVoiceInput:
            //开始录音
            [self p_hideBottomComponent];
            [button setImage:[UIImage imageNamed:@"dd_input_normal"] forState:UIControlStateNormal];
            button.tag = DDTextInput;
            [self.chatInputView willBeginRecord];
            [self.chatInputView.textView resignFirstResponder];
            _currentInputContent = self.chatInputView.textView.text;
            if ([_currentInputContent length] > 0)
            {
                [self.chatInputView.textView setText:nil];
            }
            break;
        case DDTextInput:
            //开始输入文字
            [button setImage:[UIImage imageNamed:@"dd_record_normal"] forState:UIControlStateNormal];
            button.tag = DDVoiceInput;
            [self.chatInputView willBeginInput];
            if ([_currentInputContent length] > 0)
            {
                [self.chatInputView.textView setText:_currentInputContent];
            }
            [self.chatInputView.textView becomeFirstResponder];
            break;
    }
}

- (void)p_record:(UIButton*)button
{
    [self.chatInputView.recordButton setHighlighted:YES];
    [self.chatInputView.buttonTitle setText:@"松开发送"];
    if (![[self.view subviews] containsObject:_recordingView])
    {
        [self.view addSubview:_recordingView];
    }
    [_recordingView setHidden:NO];
    [_recordingView setRecordingState:DDShowVolumnState];
    [[RecorderManager sharedManager] setDelegate:self];
    [[RecorderManager sharedManager] startRecording];
    DDLog(@"record");
}

- (void)p_willCancelRecord:(UIButton*)button
{
    [_recordingView setHidden:NO];
    [_recordingView setRecordingState:DDShowCancelSendState];
    DDLog(@"will cancel record");
}

- (void)p_cancelRecord:(UIButton*)button
{
    [self.chatInputView.recordButton setHighlighted:NO];
    [self.chatInputView.buttonTitle setText:@"按住说话"];
    [_recordingView setHidden:YES];
    [[RecorderManager sharedManager] cancelRecording];
    DDLog(@"cancel record");
}

- (void)p_sendRecord:(UIButton*)button
{
    [self.chatInputView.recordButton setHighlighted:NO];
    [self.chatInputView.buttonTitle setText:@"按住说话"];
    [[RecorderManager sharedManager] stopRecording];
    DDLog(@"send record");
}


- (void)p_endCancelRecord:(UIButton*)button
{
    [_recordingView setHidden:NO];
    [_recordingView setRecordingState:DDShowVolumnState];
}

- (void)tapOnPreShow:(UIGestureRecognizer*)sender
{
    NSString* nick = [RuntimeStatus instance].user.nick;
    NSDictionary *dict = @{@"nick":nick};
    [self removeImage];
    [self sendImageMessage:_preShowPhoto Image:_preShowImage];
}

- (void)p_tapOnTableView:(UIGestureRecognizer*)sender
{
    [self removeImage];
    if (_bottomShowComponent)
    {
        [self p_hideBottomComponent];
    }
}

- (void)p_hideBottomComponent
{
    _bottomShowComponent = _bottomShowComponent & 0;
    //隐藏所有
    
    [self.chatInputView.textView resignFirstResponder];
    [UIView animateWithDuration:0.25 animations:^{
        [self.ddUtility.view setFrame:DDCOMPONENT_BOTTOM];
        [self.emotions.view setFrame:DDCOMPONENT_BOTTOM];
        [self.chatInputView setFrame:DDINPUT_BOTTOM_FRAME];
    }];
    
    [self setValue:@(self.chatInputView.origin.y) forKeyPath:@"_inputViewY"];
    [self.view endEditing:YES];
    
}

- (void)p_enableChatFunction
{
    [self.chatInputView setUserInteractionEnabled:YES];
}

- (void)p_unableChatFunction
{
    [self.chatInputView setUserInteractionEnabled:NO];
}

- (void)p_popViewController
{
    
    [self p_hideBottomComponent];
    //[self.navigationController popViewControllerAnimated:YES];
    self.title=@"";
    [self setThisViewTitle:@""];
}

#pragma mark -
#pragma mark DDEmotionViewCOntroller Delegate
- (void)emotionViewClickSendButton
{
    [self textViewEnterSend];
}

- (void)levelMeterChanged:(float)levelMeter
{
    [_recordingView setVolume:levelMeter];
}
#pragma mark -
#pragma mark - KVO

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    
    if ([keyPath isEqualToString:@"MTTSessionEntity.sessionID"]) {
        if ([change objectForKey:@"new"] !=nil) {
            [self setThisViewTitle:self.module.MTTSessionEntity.name];
        }
    }
//    if ([keyPath isEqualToString:@"showingMessages"]) {
//        
//        dispatch_async(dispatch_get_main_queue(), ^{
////            [self.tableView reloadData];
//            if (self.hadLoadHistory == NO) {
//                [self scrollToBottomAnimated:NO];
//            }
//        });
//    }
    if ([keyPath isEqualToString:@"_inputViewY"])
    {
        float maxY = FULL_HEIGHT - DDINPUT_MIN_HEIGHT;
        float gap = maxY - _inputViewY;
        [UIView animateWithDuration:0.25 animations:^{
            _tableView.contentInset = UIEdgeInsetsMake(_tableView.contentInset.top, 0, gap+DDINPUT_MIN_HEIGHT, 0);
            
            if (_bottomShowComponent & DDShowEmotion)
            {
                [self.emotions.view setTop:self.chatInputView.bottom];
            }
            if (_bottomShowComponent & DDShowUtility)
            {
                [self.ddUtility.view setTop:self.chatInputView.bottom];
            }
            
        } completion:^(BOOL finished) {
            
        }];
        if (gap != 0)
        {
            [self scrollToBottomAnimated:NO];
        }
    }
    
}
@end

@implementation ChattingMainViewController(ChattingInput)

- (void)initialInput
{
    CGRect inputFrame = CGRectMake(0, CONTENT_HEIGHT - DDINPUT_MIN_HEIGHT + NAVBAR_HEIGHT,FULL_WIDTH,DDINPUT_MIN_HEIGHT);
    self.chatInputView = [[JSMessageInputView alloc] initWithFrame:inputFrame delegate:self];
    [self.chatInputView setBackgroundColor:RGBA(249, 249, 249, 0.9)];
    [self.view addSubview:self.chatInputView];
    [self.chatInputView.emotionbutton addTarget:self
                                         action:@selector(showEmotions:)
                               forControlEvents:UIControlEventTouchUpInside];
    
    [self.chatInputView.showUtilitysbutton addTarget:self
                                              action:@selector(showUtilitys:)
                                    forControlEvents:UIControlEventTouchDown];
    
    [self.chatInputView.voiceButton addTarget:self
                                       action:@selector(p_clickThRecordButton:)
                             forControlEvents:UIControlEventTouchUpInside];
    
    
    _touchDownGestureRecognizer = [[TouchDownGestureRecognizer alloc] initWithTarget:self action:nil];
    __weak ChattingMainViewController* weakSelf = self;
    _touchDownGestureRecognizer.touchDown = ^{
        [weakSelf p_record:nil];
    };
    
    _touchDownGestureRecognizer.moveInside = ^{
        [weakSelf p_endCancelRecord:nil];
    };
    
    _touchDownGestureRecognizer.moveOutside = ^{
        [weakSelf p_willCancelRecord:nil];
    };
    
    _touchDownGestureRecognizer.touchEnd = ^(BOOL inside){
        if (inside)
        {
            [weakSelf p_sendRecord:nil];
        }
        else
        {
            [weakSelf p_cancelRecord:nil];
        }
    };
    [self.chatInputView.recordButton addGestureRecognizer:_touchDownGestureRecognizer];
    _recordingView = [[RecordingView alloc] initWithState:DDShowVolumnState];
    [_recordingView setHidden:YES];
    [_recordingView setCenter:CGPointMake(FULL_WIDTH/2, self.view.centerY)];
    [self addObserver:self forKeyPath:@"_inputViewY" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
}

-(IBAction)showUtilitys:(id)sender
{
    [_recordButton setImage:[UIImage imageNamed:@"dd_record_normal"] forState:UIControlStateNormal];
    _recordButton.tag = DDVoiceInput;
    [self.chatInputView willBeginInput];
    if ([_currentInputContent length] > 0)
    {
        [self.chatInputView.textView setText:_currentInputContent];
    }
    
    if (self.ddUtility == nil)
    {
        self.ddUtility = [ChatUtilityViewController new];
        NSString *sessionId = self.module.MTTSessionEntity.sessionID;
        if(self.module.isGroup){
            self.ddUtility.userId = 0;
        }else{
            self.ddUtility.userId = [MTTUserEntity localIDTopb:sessionId];
        }
        [self addChildViewController:self.ddUtility];
        self.ddUtility.view.frame=CGRectMake(0, self.view.size.height,FULL_WIDTH , 280);
        [self.view addSubview:self.ddUtility.view];
    }
    [self.ddUtility setShakeHidden];
    
    if (_bottomShowComponent & DDShowKeyboard)
    {
        //显示的是键盘,这是需要隐藏键盘，显示插件，不需要动画
        _bottomShowComponent = (_bottomShowComponent & 0) | DDShowUtility;
        [self.chatInputView.textView resignFirstResponder];
        [self.ddUtility.view setFrame:DDUTILITY_FRAME];
        [self.emotions.view setFrame:DDCOMPONENT_BOTTOM];
    }
    else if (_bottomShowComponent & DDShowUtility)
    {
        //插件面板本来就是显示的,这时需要隐藏所有底部界面
        //        [self p_hideBottomComponent];
        [self.chatInputView.textView becomeFirstResponder];
        _bottomShowComponent = _bottomShowComponent & DDHideUtility;
    }
    else if (_bottomShowComponent & DDShowEmotion)
    {
        //显示的是表情，这时需要隐藏表情，显示插件
        [self.emotions.view setFrame:DDCOMPONENT_BOTTOM];
        [self.ddUtility.view setFrame:DDUTILITY_FRAME];
        _bottomShowComponent = (_bottomShowComponent & DDHideEmotion) | DDShowUtility;
    }
    else
    {
        //这是什么都没有显示，需用动画显示插件
        _bottomShowComponent = _bottomShowComponent | DDShowUtility;
        [UIView animateWithDuration:0.25 animations:^{
            [self.ddUtility.view setFrame:DDUTILITY_FRAME];
            [self.chatInputView setFrame:DDINPUT_TOP_FRAME];
        }];
        [self setValue:@(DDINPUT_TOP_FRAME.origin.y) forKeyPath:@"_inputViewY"];
    }
    
    // 判断最后一张照片是不是90s内
    
    ALAssetsLibrary *assetsLibrary;
    assetsLibrary = [[ALAssetsLibrary alloc] init];
    __block NSDate *lastDate = [[NSDate alloc] initWithTimeInterval:-90 sinceDate:[NSDate date]];
    [assetsLibrary enumerateGroupsWithTypes:ALAssetsGroupAll usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
        if (group) {
            [group enumerateAssetsUsingBlock:^(ALAsset *result, NSUInteger index, BOOL *stop) {
                if (result) {
                    NSDate *date= [result valueForProperty:ALAssetPropertyDate];
                    if ([date compare:lastDate] == NSOrderedDescending) {
                        lastDate = date;
                        _lastPhoto = result;
                    }
                }
            }];
        }else{
            if(_lastPhoto && ([[MTTUtil getLastPhotoTime] compare:lastDate] != NSOrderedSame)){
                [MTTUtil setLastPhotoTime:lastDate];
                //10s后隐藏界面
                [self performSelector:@selector(removeImage) withObject:nil afterDelay:10];
                _preShow = [[UIImageView alloc]initWithFrame:CGRectMake(SCREEN_WIDTH - 70, SCREEN_HEIGHT-385, 67, 110)];
                [_preShow setTag:10000];
                [_preShow setUserInteractionEnabled:YES];
                UIImage *preShowBg = [UIImage imageNamed:@"chat_bubble_pre_image"];
                preShowBg = [preShowBg stretchableImageWithLeftCapWidth:8 topCapHeight:8];
                [_preShow setImage:preShowBg];
                
                UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(tapOnPreShow:)];
                [_preShow addGestureRecognizer:tap];
                
                UILabel *label = [[UILabel alloc]initWithFrame:CGRectMake(4, 5, 60, 30)];
                [label setText:@"你可能要发送的图片:"];
                [label setFont:systemFont(12)];
                [label setNumberOfLines:0];
                
                UIImageView *imgView = [[UIImageView alloc]initWithFrame:CGRectMake(4, 40, 60, 60)];
                UIImage *image=[UIImage imageWithCGImage:_lastPhoto.aspectRatioThumbnail];
                [imgView setImage:image];
                [imgView setContentMode:UIViewContentModeScaleAspectFill];
                [imgView setClipsToBounds:YES];
                [imgView.layer setBorderColor:RGB(229, 229, 229).CGColor];
                [imgView.layer setBorderWidth:1];
                [imgView.layer setCornerRadius:5];
                [_preShow addSubview:label];
                [_preShow addSubview:imgView];
                [self.view addSubview:_preShow];
                
                _preShowPhoto = [MTTPhotoEnity new];
                ALAssetRepresentation* representation = [_lastPhoto defaultRepresentation];
                NSURL* url = [representation url];
                _preShowPhoto.localPath=url.absoluteString;
                _preShowImage = nil;
                if (representation == nil) {
                    CGImageRef thum = [_lastPhoto aspectRatioThumbnail];
                    _preShowImage = [[UIImage alloc]initWithCGImage:thum];
                }else
                {
                    _preShowImage =[[UIImage alloc]initWithCGImage:[[_lastPhoto defaultRepresentation] fullScreenImage]];
                }
                NSString *keyName = [[MTTPhotosCache sharedPhotoCache] getKeyName];
                
                _preShowPhoto.localPath=keyName;
            }
        }
    } failureBlock:^(NSError *error) {
        NSLog(@"Group not found!\n");
    }];
    
}

-(void)removeImage
{
    _lastPhoto = nil;
    [_preShow removeFromSuperview];
}

-(IBAction)showEmotions:(id)sender
{
    [_recordButton setImage:[UIImage imageNamed:@"dd_record_normal"] forState:UIControlStateNormal];
    _recordButton.tag = DDVoiceInput;
    [self.chatInputView willBeginInput];
    if ([_currentInputContent length] > 0)
    {
        [self.chatInputView.textView setText:_currentInputContent];
    }
    
    if (self.emotions == nil) {
        self.emotions = [EmotionsViewController new];
        [self.emotions.view setBackgroundColor:[UIColor whiteColor]];
        self.emotions.view.frame=DDCOMPONENT_BOTTOM;
        self.emotions.delegate = self;
        [self.view addSubview:self.emotions.view];
    }
    if (_bottomShowComponent & DDShowKeyboard)
    {
        //显示的是键盘,这是需要隐藏键盘，显示表情，不需要动画
        _bottomShowComponent = (_bottomShowComponent & 0) | DDShowEmotion;
        [self.chatInputView.textView resignFirstResponder];
        [self.emotions.view setFrame:DDEMOTION_FRAME];
        [self.ddUtility.view setFrame:DDCOMPONENT_BOTTOM];
    }
    else if (_bottomShowComponent & DDShowEmotion)
    {
        //表情面板本来就是显示的,这时需要隐藏所有底部界面
        [self.chatInputView.textView resignFirstResponder];
        _bottomShowComponent = _bottomShowComponent & DDHideEmotion;
    }
    else if (_bottomShowComponent & DDShowUtility)
    {
        //显示的是插件，这时需要隐藏插件，显示表情
        [self.ddUtility.view setFrame:DDCOMPONENT_BOTTOM];
        [self.emotions.view setFrame:DDEMOTION_FRAME];
        _bottomShowComponent = (_bottomShowComponent & DDHideUtility) | DDShowEmotion;
    }
    else
    {
        //这是什么都没有显示，需用动画显示表情
        _bottomShowComponent = _bottomShowComponent | DDShowEmotion;
        [UIView animateWithDuration:0.25 animations:^{
            [self.emotions.view setFrame:DDEMOTION_FRAME];
            [self.chatInputView setFrame:DDINPUT_TOP_FRAME];
        }];
        [self setValue:@(DDINPUT_TOP_FRAME.origin.y) forKeyPath:@"_inputViewY"];
    }
}
#pragma mark - KeyBoardNotification
- (void)handleWillShowKeyboard:(NSNotification *)notification
{
    
    CGRect keyboardRect;
    keyboardRect = [(notification.userInfo)[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    keyboardRect = [self.view convertRect:keyboardRect fromView:nil];
    _bottomShowComponent = _bottomShowComponent | DDShowKeyboard;
    [UIView animateWithDuration:0.25 animations:^{
        [self.chatInputView setFrame:CGRectMake(0, keyboardRect.origin.y - DDINPUT_HEIGHT, self.view.size.width, DDINPUT_HEIGHT)];
    }];
    [self setValue:@(keyboardRect.origin.y - DDINPUT_HEIGHT) forKeyPath:@"_inputViewY"];
    
}

- (void)handleWillHideKeyboard:(NSNotification *)notification
{
    CGRect keyboardRect;
    keyboardRect = [notification.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    keyboardRect = [self.view convertRect:keyboardRect fromView:nil];
    _bottomShowComponent = _bottomShowComponent & DDHideKeyboard;
    if (_bottomShowComponent & DDShowUtility)
    {
        //显示的是插件
        [UIView animateWithDuration:0.25 animations:^{
            [self.chatInputView setFrame:DDINPUT_TOP_FRAME];
        }];
        [self setValue:@(self.chatInputView.origin.y) forKeyPath:@"_inputViewY"];
    }
    else if (_bottomShowComponent & DDShowEmotion)
    {
        //显示的是表情
        [UIView animateWithDuration:0.25 animations:^{
            [self.chatInputView setFrame:DDINPUT_TOP_FRAME];
        }];
        [self setValue:@(self.chatInputView.origin.y) forKeyPath:@"_inputViewY"];
        
    }
    else
    {
        [self p_hideBottomComponent];
    }
}

-(IBAction)titleTap:(id)sender
{
    if ([self.module.MTTSessionEntity isGroup]) {
        return;
    }
    [self.module getCurrentUser:^(MTTUserEntity *user) {
        PublicProfileViewControll *profile = [PublicProfileViewControll new];
        profile.title=user.nick;
        profile.user=user;
        [self pushViewController:profile animated:YES];
    }];
}

- (void)n_receiveMessage:(NSNotification*)notification
{
    if (![self.navigationController.topViewController isEqual:self])
    {
        //当前不是聊天界面直接返回
        return;
    }
    
    MTTMessageEntity* message = [notification object];
    UIApplicationState state =[UIApplication sharedApplication].applicationState;
    if (state == UIApplicationStateBackground) {
        if([message.sessionId isEqualToString:self.module.MTTSessionEntity.sessionID])
        {
            [self.module addShowMessage:message];
            [self.module updateSessionUpdateTime:message.msgTime];
            dispatch_async(dispatch_get_main_queue(), ^{
                [self scrollToBottomAnimated:YES];
            });
        }
        return;
    }
    //显示消息
    
    if([message.sessionId isEqualToString:self.module.MTTSessionEntity.sessionID])
    {
        [self.module addShowMessage:message];
        [self.module updateSessionUpdateTime:message.msgTime];
        [self.tableView reloadData];
        [[DDMessageModule shareInstance] sendMsgRead:message];
        if(self.ifScrollBottom){
            [self scrollToBottomAnimated:YES];
        }
    }
    
}
- (void)recordingTimeout
{
    
}

- (void)recordingStopped  //录音机停止采集声音
{
    
}

- (void)recordingFailed:(NSString *)failureInfoString
{
    
}

- (void)levelMeterChanged:(float)levelMeter
{
    [_recordingView setVolume:levelMeter];
}
-(void)reloginSuccess
{
    [self.module getNewMsg:^(NSUInteger addcount, NSError *error) {
        [self.tableView reloadData];
    }];
}

#pragma mark - TTTAttributedLabelDelegate

- (void)attributedLabel:(__unused TTTAttributedLabel *)label
   didSelectLinkWithURL:(NSURL *)url
{
    SVWebViewController *webViewController = [[SVWebViewController alloc] initWithURL:url];
    [self.navigationController pushViewController:webViewController animated:YES];
}

- (void)attributedLabel:(TTTAttributedLabel *)label didSelectLinkWithPhoneNumber:(NSString *)phoneNumber
{
    NSArray *res = [phoneNumber componentsSeparatedByString:LINK_SPLIT];
    if([res count] == 2){
        if([res[0] isEqualToString:NICK_SPLIT]){
            MTTUserEntity *user = [[DDUserModule shareInstance] getUserByNick:res[1]];
            PublicProfileViewControll *public = [PublicProfileViewControll new];
            public.user=user;
            [self pushViewController:public animated:YES];
        }
        if([res[0] isEqualToString:PHONE_SPLIT]){
            NSString *phone = [res[1] stringByReplacingOccurrencesOfString:@" " withString:@""];
            phone = [phone stringByReplacingOccurrencesOfString:@"-" withString:@""];
            NSString *title = [NSString stringWithFormat:@"%@%@",phone,@"可能是一个电话号码,你可以"];
            self.phoneNumber = phone;
            LCActionSheet *sheet = [[LCActionSheet alloc] initWithTitle:title
                                                           buttonTitles:@[@"呼叫",@"复制"]
                                                         redButtonIndex:-1
                                                               delegate:self];
            sheet.tag = 10000;
            [sheet show];
        }
        if([res[0] isEqualToString:EMAIL_SPLIT]){
            self.email = res[1];
            NSString *title = [NSString stringWithFormat:@"%@%@%@",@"向",self.email,@"发送邮件"];
            LCActionSheet *sheet = [[LCActionSheet alloc] initWithTitle:title
                                                           buttonTitles:@[@"使用默认邮件账户",@"复制"]
                                                         redButtonIndex:-1
                                                               delegate:self];
            sheet.tag = 10001;
            [sheet show];
        }
    }
}


#pragma mark - LCActionSheetDelegate
- (void)actionSheet:(LCActionSheet *)actionSheet didClickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(actionSheet.tag == 10000){
        if(buttonIndex == 0){
            if(!self.phoneNumber.length){
                return;
            }
            NSString *phone =[NSString stringWithFormat:@"tel:%@",self.phoneNumber];
            NSURL *url = [NSURL URLWithString:phone];
            [[UIApplication sharedApplication] openURL:url];
        }
        if(buttonIndex == 1){
            UIPasteboard *pboard = [UIPasteboard generalPasteboard];
            pboard.string = self.phoneNumber;
        }
    }
    if(actionSheet.tag == 10001){
        if (!self.email.length) {
            return;
        }
        if(buttonIndex == 0){
            NSString *email =[NSString stringWithFormat:@"mailto:%@",self.email];
            NSURL *url = [NSURL URLWithString:email];
            [[UIApplication sharedApplication] openURL:url];
        }
        if(buttonIndex == 1){
            UIPasteboard *pboard = [UIPasteboard generalPasteboard];
            pboard.string = self.email;
        }
    }
}

#pragma mark - UIGestureRecognizerDelegate
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch
{
    [self p_tapOnTableView:gestureRecognizer];
    return YES;
}

@end
