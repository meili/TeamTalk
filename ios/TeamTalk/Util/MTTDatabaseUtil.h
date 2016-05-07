//
//  MTTDatabaseUtil.h
//  Duoduo
//
//  Created by zuoye on 14-3-21.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FMDB.h"
@class MTTDepartment;
@class MTTMessageEntity;
@class MTTGroupEntity;
@class MTTSessionEntity;
@class MessageEntity,MTTUserEntity;

@interface MTTDatabaseUtil : NSObject
@property(strong)NSString *recentsession;
//在数据库上的操作
@property (nonatomic,readonly)dispatch_queue_t databaseMessageQueue;


+ (instancetype)instance;

- (void)openCurrentUserDB;

@end

typedef void(^LoadMessageInSessionCompletion)(NSArray* messages,NSError* error);
typedef void(^MessageCountCompletion)(NSInteger count);
typedef void(^DeleteSessionCompletion)(BOOL success);
typedef void(^DDDBGetLastestMessageCompletion)(MTTMessageEntity* message,NSError* error);
typedef void(^DDUpdateMessageCompletion)(BOOL result);
typedef void(^DDGetLastestCommodityMessageCompletion)(MTTMessageEntity* message);

@interface MTTDatabaseUtil(Message)

/**
 *  在|databaseMessageQueue|执行查询操作，分页获取聊天记录
 *
 *  @param sessionID  会话ID
 *  @param pagecount  每页消息数
 *  @param page       页数
 *  @param completion 完成获取
 */
- (void)loadMessageForSessionID:(NSString*)sessionID pageCount:(int)pagecount index:(NSInteger)index completion:(LoadMessageInSessionCompletion)completion;

- (void)loadMessageForSessionID:(NSString*)sessionID afterMessage:(MTTMessageEntity*)message completion:(LoadMessageInSessionCompletion)completion;
- (void)searchHistory:(NSString*)key completion:(LoadMessageInSessionCompletion)completion;
- (void)searchHistoryBySessionId:(NSString*)key sessionId:(NSString *)sessionId completion:(LoadMessageInSessionCompletion)completion;
/**
 *  获取对应的Session的最新的自己发送的商品气泡
 *
 *  @param sessionID  会话ID
 *  @param completion 完成获取
 */
- (void)getLasetCommodityTypeImageForSession:(NSString*)sessionID completion:(DDGetLastestCommodityMessageCompletion)completion;

/**
 *  在|databaseMessageQueue|执行查询操作，获取DB中
 *
 *  @param sessionID  sessionID
 *  @param completion 完成获取最新的消息
 */
- (void)getLastestMessageForSessionID:(NSString*)sessionID completion:(DDDBGetLastestMessageCompletion)completion;

/**
 *  在|databaseMessageQueue|执行查询操作，分页获取聊天记录
 *
 *  @param sessionID  会话ID
 *  @param completion 完成block
 */
- (void)getMessagesCountForSessionID:(NSString*)sessionID completion:(MessageCountCompletion)completion;

/**
 *  批量插入message，需要用户必须在线，避免插入离线时阅读的消息
 *
 *  @param messages message集合
 *  @param success 插入成功
 *  @param failure 插入失败
 */
- (void)insertMessages:(NSArray*)messages
               success:(void(^)())success
               failure:(void(^)(NSString* errorDescripe))failure;

/**
 *  删除相应会话的所有消息
 *
 *  @param sessionID  会话
 *  @param completion 完成删除
 */
- (void)deleteMesagesForSession:(NSString*)sessionID completion:(DeleteSessionCompletion)completion;

/**
 *  更新数据库中的某条消息
 *
 *  @param message    更新后的消息
 *  @param completion 完成更新
 */
- (void)updateMessageForMessage:(MTTMessageEntity*)message completion:(DDUpdateMessageCompletion)completion;
@end

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

typedef void(^LoadRecentContactsComplection)(NSArray* contacts,NSError* error);
typedef void(^LoadAllContactsComplection)(NSArray* contacts,NSError* error);
typedef void(^LoadAllSessionsComplection)(NSArray* session,NSError* error);
typedef void(^UpdateRecentContactsComplection)(NSError* error);
typedef void(^InsertsRecentContactsCOmplection)(NSError* error);

@interface MTTDatabaseUtil(Users)

/**
 *  加载本地数据库的最近联系人列表
 *
 *  @param completion 完成加载
 */
- (void)loadContactsCompletion:(LoadRecentContactsComplection)completion;

/**
 *  更新本地数据库的最近联系人信息
 *
 *  @param completion 完成更新本地数据库
 */
- (void)updateContacts:(NSArray*)users inDBCompletion:(UpdateRecentContactsComplection)completion;

/**
 *  更新本地数据库某个用户的信息
 *
 *  @param user       某个用户
 *  @param completion 完成更新本地数据库
 */


/**
 *  插入本地数据库的最近联系人信息
 *
 *  @param users      最近联系人数组
 *  @param completion 完成插入
 */
- (void)insertUsers:(NSArray*)users completion:(InsertsRecentContactsCOmplection)completion;
/**
 *  插入组织架构信息
 *
 *  @param departments 组织架构数组
 *  @param completion  完成插入
 */
- (void)insertDepartments:(NSArray*)departments completion:(InsertsRecentContactsCOmplection)completion;

- (void)getDepartmentFromID:(NSString*)departmentID completion:(void(^)(MTTDepartment *department))completion;
- (void)insertAllUser:(NSArray*)users completion:(InsertsRecentContactsCOmplection)completion;

- (void)getAllUsers:(LoadAllContactsComplection )completion;

- (void)getUserFromID:(NSString*)userID completion:(void(^)(MTTUserEntity *user))completion;

- (void)updateRecentGroup:(MTTGroupEntity *)group completion:(InsertsRecentContactsCOmplection)completion;
- (void)updateRecentSessions:(NSArray *)sessions completion:(InsertsRecentContactsCOmplection)completion;
- (void)updateRecentSession:(MTTSessionEntity *)session completion:(InsertsRecentContactsCOmplection)completion;
- (void)loadGroupsCompletion:(LoadRecentContactsComplection)completion;
- (void)loadSessionsCompletion:(LoadAllSessionsComplection)completion;
-(void)removeSession:(NSString *)sessionID;
- (void)deleteMesages:(MTTMessageEntity * )message completion:(DeleteSessionCompletion)completion;
- (void)loadGroupByIDCompletion:(NSString *)groupID Block:(LoadRecentContactsComplection)completion;
@end