
#import <UIKit/UIKit.h>

#define MTTRefreshViewHeight 40.0f

#define MTTRefreshOffsetHeight 0

@interface MTTRefreshView : UIView

@property (nonatomic, strong) UILabel *titleLabel;

@property (nonatomic,strong) UIActivityIndicatorView *activityIndicatorView;

- (void)refreshFinished;

-(void)removeKOV;

@end

@protocol UIScrollViewPullResfreshDelegate <NSObject>

-(void)scrollViewRefresh:(UIScrollView *)scrollView;

-(void)scrollViewLoadMore:(UIScrollView *)scrollView;

@end

@protocol UIScrollPullToRefreshViewDelegate;

@interface UIScrollView (PullToLoadMore)

@property (nonatomic, strong) MTTRefreshView *refreshView;

@property (nonatomic, strong) MTTRefreshView *loadMoreView;

@property (nonatomic,assign) BOOL noMore;

@property (nonatomic,assign,setter = setRefresh:) BOOL isRefresh;

@property (nonatomic,assign,setter = setLoadMore:) BOOL isLoadMore;

@property (nonatomic,assign) NSUInteger pageNumber;

-(void)setRefreshLogo:(UIImage *)logo;

- (void)setRefreshHandler:(void (^)(void))actionHandler;

- (void)setLoadMoreHandler:(void (^)(void))actionHandler;

//设置 下拉刷新标题
-(void)setRefreshTitle:(NSString *)title;

//设置 加载更多标题
-(void)setLoadMoreTitle:(NSString *)title;

- (void)refreshFinished;

-(void)loadMoreFinished;

-(CGPoint)theBeiginContentOffset;

@end

@protocol UIScrollPullToRefreshViewDelegate <UIScrollViewDelegate,NSObject>

-(CGPoint)scrollViewBeginContentOffset:(UIScrollView *)scrollView;

@end