//
//  DDChattingModule.h
//  IOSDuoduo
//
//  Created by 独嘉 on 14-5-28.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTTSessionEntity.h"
#import "MTTUserEntity.h"
#define DD_PAGE_ITEM_COUNT                  20

typedef void(^DDReuestServiceCompletion)(MTTUserEntity* user);
typedef void(^DDRequestGoodDetailCompletion)(NSDictionary* detail,NSError* error);
@class DDCommodity;
@class MTTMessageEntity;
typedef void(^DDChatLoadMoreHistoryCompletion)(NSUInteger addcount, NSError* error);

@interface ChattingModule : NSObject
@property (strong)MTTSessionEntity* MTTSessionEntity;
@property(strong)NSMutableArray *ids ;
@property (strong)NSMutableArray* showingMessages;
@property (assign) NSInteger isGroup;
/**
 *  加载历史消息接口，这里会适时插入时间
 *
 *  @param completion 加载完成
 */
- (void)loadMoreHistoryCompletion:(DDChatLoadMoreHistoryCompletion)completion;
- (void)loadAllHistoryCompletion:(MTTMessageEntity*)message Completion:(DDChatLoadMoreHistoryCompletion)completion;

- (void)loadHostoryUntilCommodity:(MTTMessageEntity*)message completion:(DDChatLoadMoreHistoryCompletion)completion;

- (float)messageHeight:(MTTMessageEntity*)message;

- (void)addPrompt:(NSString*)prompt;
- (void)addShowMessage:(MTTMessageEntity*)message;
- (void)addShowMessages:(NSArray*)messages;
-(void)scanDBAndFixIDList:(void(^)(bool isok))block;
- (void)updateSessionUpdateTime:(NSUInteger)time;
- (void)clearChatData;
-(void)m_clearScanRecord;
- (void)showMessagesAddCommodity:(MTTMessageEntity*)message;
-(void)getCurrentUser:(void(^)(MTTUserEntity *))block;
-(void)loadHisToryMessageFromServer:(NSUInteger)FromMsgID loadCount:(NSUInteger)count Completion:(DDChatLoadMoreHistoryCompletion)completion;
-(void)loadHostoryMessageFromServer:(NSUInteger)FromMsgID Completion:(DDChatLoadMoreHistoryCompletion)completion;
+ (NSArray*)p_spliteMessage:(MTTMessageEntity*)message;
-(void)getNewMsg:(DDChatLoadMoreHistoryCompletion)completion;
@end


@interface DDPromptEntity : NSObject
@property(nonatomic,retain)NSString* message;

@end