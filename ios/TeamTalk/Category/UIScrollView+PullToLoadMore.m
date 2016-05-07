

#import <QuartzCore/QuartzCore.h>

#import "UIScrollView+PullToLoadMore.h"

#import <objc/runtime.h>

#define ContentOffsetAnimateTime 0.35f

typedef NS_ENUM(NSUInteger, DDPullToRefreshState) {
    DDPullToRefreshStateHidden = 1,
    DDPullToRefreshStateTriggered,
    DDPullToRefreshStateLoading,
    DDPullToRefreshStateLoaded,
    DDPullToRefreshStateNomore,
};

//scrollview 是否在动画中
//static BOOL __isScrollViewAnimation =NO;

@interface MTTRefreshView ()

@property (nonatomic, copy) void (^pullToRefreshHandler)(void);

@property (nonatomic, copy) void (^pullToLoadMoreHandler)(void);

@property (nonatomic, assign) DDPullToRefreshState state;

@property (nonatomic, assign) UIScrollView *scrollView;

@property (nonatomic, assign) UIEdgeInsets originalScrollViewContentInset;

@property (nonatomic,strong) NSDate *lastUpdataDate;

@property (nonatomic,strong) UIImage *imageLogo;

@property (nonatomic,assign) BOOL isNeedDelayHideLogo;

//加载中标题，默认为： "加载中..."
@property (nonatomic, copy) NSString * loadingText;

//加载中标题，默认为： "加载完成..."
@property (nonatomic, copy) NSString * loadedText;

@end



@implementation MTTRefreshView

-(void)removeKOV{
    [self.scrollView removeObserver:self forKeyPath:@"contentOffset"];
    [self.scrollView removeObserver:self forKeyPath:@"contentSize"];
}

- (id)initWithScrollView:(UIScrollView *)scrollView {

    if (self =[super initWithFrame:CGRectZero]) {
        self.scrollView = scrollView;
        self.frame = CGRectMake(0, 0, _scrollView.bounds.size.width, MTTRefreshViewHeight);
        [_scrollView addSubview:self];
        
        self.titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 200, 20)];
        _titleLabel.font = [UIFont boldSystemFontOfSize:14];
        _titleLabel.backgroundColor = [UIColor clearColor];
        _titleLabel.textColor =[UIColor lightGrayColor];
        _titleLabel.textAlignment =NSTextAlignmentCenter;
        [self addSubview:_titleLabel];
        
        _activityIndicatorView =[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
        [self addSubview:_activityIndicatorView];
        
        [scrollView addObserver:self forKeyPath:@"contentOffset" options:NSKeyValueObservingOptionNew context:nil];
        [scrollView addObserver:self forKeyPath:@"contentSize" options:NSKeyValueObservingOptionNew context:nil];
        
        self.originalScrollViewContentInset = scrollView.contentInset;
        
        _state = DDPullToRefreshStateHidden;
        _activityIndicatorView.hidden =YES;
        
        self.isNeedDelayHideLogo =NO;
        
        self.loadedText = @"加载完成...";
        self.loadingText = @"加载中...";
        
        self.backgroundColor =[UIColor clearColor];
    }

    return self;
}

- (void)layoutSubviews {

    self.originalScrollViewContentInset = self.scrollView.contentInset;
    
    if (self.pullToRefreshHandler) {

        _titleLabel.center = CGPointMake(self.frame.size.width/2, self.frame.size.height/2);
        
        _activityIndicatorView.center =CGPointMake(self.frame.size.width/2, self.frame.size.height/2);
    }
}

#pragma mark - KVO

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    
    if (self.state != DDPullToRefreshStateLoading && !self.scrollView.noMore) {
     
        if([keyPath isEqualToString:@"contentOffset"]){
            [self scrollViewDidScroll:[[change valueForKey:NSKeyValueChangeNewKey] CGPointValue]];
        }
        else if([keyPath isEqualToString:@"contentSize"]){
            CGSize contentSize=[[change valueForKey:NSKeyValueChangeNewKey] CGSizeValue];
            [self scrollViewDidAdjustContentSize:contentSize];
        }
    }
}

- (void)scrollViewDidAdjustContentSize:(CGSize)contentSize{

    CGSize frameSize =self.scrollView.frame.size;
    
    if (contentSize.height <= frameSize.height) {
        
        self.frame = CGRectMake(0, -MTTRefreshViewHeight, _scrollView.bounds.size.width, MTTRefreshViewHeight);
        
        if ([self.scrollView isKindOfClass:[UITableView class]]) {
            UIView *head =[(UITableView *)self.scrollView tableHeaderView];
            if (head && CGRectGetHeight(head.frame) != 10) {
                UIView *headView1 =[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 10)];
                [(UITableView *)self.scrollView setTableHeaderView:headView1];
            }
        }
    }
    else{
        self.frame = CGRectMake(0, 0, _scrollView.bounds.size.width, MTTRefreshViewHeight);
        
        if ([self.scrollView isKindOfClass:[UITableView class]]) {
            UIView *head =[(UITableView *)self.scrollView tableHeaderView];
            if (head && CGRectGetHeight(head.frame) != MTTRefreshViewHeight) {
                UIView *headView1 =[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, MTTRefreshViewHeight)];
                [(UITableView *)self.scrollView setTableHeaderView:headView1];
            }
        }
    }
}

#pragma mark -scrollViewDidScroll

- (void)scrollViewDidScroll:(CGPoint)contentOffset {
    
    BOOL isDragging =self.scrollView.isDragging;
    BOOL isDecelerating =self.scrollView.decelerating;
    
    if (self.pullToRefreshHandler) {
        
        CGFloat offsetY =self.scrollView.contentOffset.y;
        
        CGFloat selfHeight =CGRectGetHeight(self.frame);
        
        BOOL isScrollInFrame =YES;
        if (self.scrollView.contentSize.height >= CGRectGetHeight(self.scrollView.frame)) {
            offsetY -=selfHeight;
            isScrollInFrame =NO;
        }
        
//        NSLog(@"offsetY:%f,%d,%d,%d",offsetY,isDragging,isDecelerating,isTracking);
        
        if (self.state ==DDPullToRefreshStateNomore){
            if (![self.scrollView noMore]) {
                self.state =DDPullToRefreshStateHidden;
            }
        }
        else if (self.state ==DDPullToRefreshStateLoaded){
            self.state =DDPullToRefreshStateHidden;
        }
        else if (self.state ==DDPullToRefreshStateLoading ) {
        }
        else if(((!isScrollInFrame && !(isDragging && isDecelerating)) || (isScrollInFrame && !isDragging)) && self.state == DDPullToRefreshStateTriggered){
//            NSLog(@"DDPullToRefreshStateLoading");
            
            if (isScrollInFrame && offsetY<-MTTRefreshViewHeight) {
                [self.scrollView setContentOffset:CGPointMake(0, -MTTRefreshViewHeight -self.scrollView.contentInset.top) animated:YES];
            }
            
            self.state = DDPullToRefreshStateLoading;
        }
        else if(isDragging && offsetY < 0 && self.state != DDPullToRefreshStateLoading){
//            NSLog(@"DDPullToRefreshStateTriggered");
            //条件触发
            self.state = DDPullToRefreshStateTriggered;
        }
//        else if(convertY <= -MTTRefreshViewHeight && self.state != DDPullToRefreshStateHidden){
//            
//            self.state = DDPullToRefreshStateHidden;
//        }
    }
}

#pragma mark - setter


- (void)setState:(DDPullToRefreshState)theState {
    
    if (_state == theState) {
        return;
    }
    _state = theState;
    
    if (self.pullToRefreshHandler) {
        switch (theState) {
                
            case DDPullToRefreshStateNomore:{
                _titleLabel.text = @"没有更多记录了...";
                _activityIndicatorView.hidden =YES;
                
                CGFloat offsetY =self.scrollView.contentOffset.y;
                CGFloat selfHeight =CGRectGetHeight(self.frame);
                BOOL isScrollInFrame =YES;
                if (self.scrollView.contentSize.height >= CGRectGetHeight(self.scrollView.frame)) {
                    offsetY -=selfHeight;
                    isScrollInFrame =NO;
                }
                if (isScrollInFrame && offsetY<-MTTRefreshViewHeight) {
                    [self.scrollView setContentOffset:CGPointMake(0, -self.scrollView.contentInset.top) animated:YES];
                }
            }
                break;
            case DDPullToRefreshStateHidden:{
                _titleLabel.text =@"";
                _activityIndicatorView.hidden =YES;
                [_activityIndicatorView stopAnimating];
            }
                break;
                
            case DDPullToRefreshStateTriggered:{
                
                if (!self.scrollView.noMore) {
                    _titleLabel.text =@"";
                }
                _activityIndicatorView.hidden =NO;
                [_activityIndicatorView startAnimating];
            }
                break;
                
            case DDPullToRefreshStateLoading:{
                
                _scrollView.isRefresh =YES;
                _scrollView.isLoadMore =NO;
                _pullToRefreshHandler();
                _isNeedDelayHideLogo =YES;
                
            }
                break;
                
            case DDPullToRefreshStateLoaded:{
                
                [self setLoadedComplete];

            }
                break;
                default:
                break;
        }
        
    }
}

-(void)setLoadedComplete{
    
    if (self.pullToRefreshHandler) {

        self.lastUpdataDate =[NSDate date];
        
        [self.activityIndicatorView stopAnimating];
        self.activityIndicatorView.hidden =YES;
        
        if (self.scrollView.noMore) {
            self.state =DDPullToRefreshStateNomore;
        }
    }
}


#pragma mark - refreshFinished

- (void)refreshFinished {
    
//    [self setNeedsLayout];
    
    self.state =DDPullToRefreshStateLoaded;
}

@end


#pragma mark - UIScrollView (Refresh)

static char *UIScrollViewRefreshView ="UIScrollViewRefreshView";
static char *UIScrollViewLoadMoreView ="UIScrollViewLoadMoreView";

static char *UIScrollViewNoMore ="UIScrollViewNoMore";
static char *UIScollViewIsRefresh ="UIScollViewIsRefresh";
static char *UIScollViewIsLoadMore ="UIScollViewlIsLoadMore";

@implementation UIScrollView (PullToLoadMore)

@dynamic refreshView;

@dynamic loadMoreView;

@dynamic noMore;

@dynamic isLoadMore;

@dynamic isRefresh;

@dynamic pageNumber;



#pragma mark - setter

- (void)setRefreshHandler:(void (^)(void))actionHandler {
    
    MTTRefreshView *pullToRefreshView = [[MTTRefreshView alloc] initWithScrollView:self];
    pullToRefreshView.pullToRefreshHandler =actionHandler;
    self.refreshView = pullToRefreshView;
}

- (void)setLoadMoreHandler:(void (^)(void))actionHandler {
    
    MTTRefreshView *pullToLoadMoreView = [[MTTRefreshView alloc] initWithScrollView:self];
    pullToLoadMoreView.pullToLoadMoreHandler =actionHandler;
    self.loadMoreView = pullToLoadMoreView;
}

-(void)setRefreshLogo:(UIImage *)logo{

    self.refreshView.imageLogo =logo;
}

-(void)setRefreshTitle:(NSString *)title{

    self.refreshView.titleLabel.text =title;
}

-(void)setLoadMoreTitle:(NSString *)title{

    self.loadMoreView.titleLabel.text =title;
}

- (void)refreshFinished{
    
    [self.refreshView refreshFinished];
}

-(void)loadMoreFinished{
    
    [self.loadMoreView refreshFinished];
}

#pragma mark -getter & setter


-(CGPoint)theBeiginContentOffset{
    
    CGPoint point =CGPointZero;
    
    id  aDelegate =self.delegate;
    
    if ([aDelegate respondsToSelector:@selector(scrollViewBeginContentOffset:)]) {
        point =[aDelegate scrollViewBeginContentOffset:self];
    }
    
    return point;
}

-(NSUInteger)pageNumber{
    NSNumber *n =objc_getAssociatedObject(self, "TablePageNumber");
    return [n unsignedIntegerValue];
}

-(void)setPageNumber:(NSUInteger)number{
    NSNumber *n =[NSNumber numberWithUnsignedInteger:number];
    objc_setAssociatedObject(self, "TablePageNumber", n, OBJC_ASSOCIATION_RETAIN);
}


-(BOOL)noMore{
    
    NSNumber *n =objc_getAssociatedObject(self, &UIScrollViewNoMore);
    if (n ==nil || ![n isKindOfClass:[NSNumber class]]) {
        return NO;
    }
    
    return [n boolValue];
}

-(void)setNoMore:(BOOL)noMore{
    
    objc_setAssociatedObject(self, &UIScrollViewNoMore,
                             [NSNumber numberWithBool:noMore],
                             OBJC_ASSOCIATION_RETAIN);
}

-(void)setRefresh:(BOOL)isRefresh{
    
    objc_setAssociatedObject(self, &UIScollViewIsRefresh,
                             [NSNumber numberWithBool:isRefresh],
                             OBJC_ASSOCIATION_RETAIN);
}

-(BOOL)isRefresh{
    
    NSNumber *n =objc_getAssociatedObject(self, &UIScollViewIsRefresh);
    if (n ==nil || ![n isKindOfClass:[NSNumber class]]) {
        return NO;
    }
    return [n boolValue];
}

-(void)setLoadMore:(BOOL)isLoadMore{
    
    objc_setAssociatedObject(self, &UIScollViewIsLoadMore,
                             [NSNumber numberWithBool:isLoadMore],
                             OBJC_ASSOCIATION_RETAIN);
}

-(BOOL)isLoadMore{
    
    NSNumber *n =objc_getAssociatedObject(self, &UIScollViewIsLoadMore);
    if (n ==nil || ![n isKindOfClass:[NSNumber class]]) {
        return NO;
    }
    
    return [n boolValue];
}


- (void)setRefreshView:(MTTRefreshView *)pullToRefreshView {
    [self willChangeValueForKey:@"refreshView"];
    objc_setAssociatedObject(self, &UIScrollViewRefreshView,
                             pullToRefreshView,
                             OBJC_ASSOCIATION_RETAIN);
    [self didChangeValueForKey:@"refreshView"];
}

- (MTTRefreshView *)refreshView {
    MTTRefreshView *view =objc_getAssociatedObject(self, &UIScrollViewRefreshView);
    return view;
}

- (void)setLoadMoreView:(MTTRefreshView *)pullToLoadMoreView{
    
    [self willChangeValueForKey:@"loadMoreView"];
    
    objc_setAssociatedObject(self, &UIScrollViewLoadMoreView,
                             pullToLoadMoreView,
                             OBJC_ASSOCIATION_RETAIN);
    
    [self didChangeValueForKey:@"loadMoreView"];
}

- (MTTRefreshView *)loadMoreView {
    
    MTTRefreshView *view  =objc_getAssociatedObject(self, &UIScrollViewLoadMoreView);
    return view;
}


@end
