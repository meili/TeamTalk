//
//  MTTLogoAnimateView.m
//  CustomAnimate
//
//  Created by 丁丁 on 15/3/17.
//  Copyright (c) 2015年 丁丁. All rights reserved.
//

#import "MTTLogoAnimateView.h"

@interface MTTLogoAnimateLayer : CALayer

@property (nonatomic,assign) float process;


@end

@implementation MTTLogoAnimateLayer

//@dynamic process;

-(instancetype)init{

    if (self =[super init]) {
        [self setup];
    }
    return self;
}

- (instancetype)initWithLayer:(id)layer{

    if (self =[super initWithLayer:layer]) {
        [self setup];
    }
    return self;
}

-(void)setup{

    _process =0;
}

//-(void)display{
//
//    float process = [self.presentationLayer process];
//    
//    CGFloat width =CGRectGetWidth(self.bounds);
//    CGFloat height =CGRectGetHeight(self.bounds);
//    
//    CGFloat pHeight =process *height;
//    CGFloat cHeight =pHeight +height/4 > height ?height : pHeight +height/4;
//    
//    UIBezierPath *path =[UIBezierPath bezierPath];
//    [path moveToPoint:CGPointMake(0, height -pHeight)];
//    [path addCurveToPoint:CGPointMake(width, height -pHeight) controlPoint1:CGPointMake(width/2, height -cHeight) controlPoint2:CGPointMake(width/2, height -cHeight)];
//    [path addLineToPoint:CGPointMake(width, height)];
//    [path addLineToPoint:CGPointMake(0, height)];
//    [path addLineToPoint:CGPointMake(0, height -pHeight)];
//    
//    CAShapeLayer *shapeLayer =[CAShapeLayer layer];
//    shapeLayer.path =path.CGPath;
//    
//    self.mask =shapeLayer;
//}
//
//+ (BOOL)needsDisplayForKey:(NSString *)key
//{
//    if ([@"process" isEqualToString:key])
//    {
//        return YES;
//    }
//    return [super needsDisplayForKey:key];
//}
//
//- (id<CAAction>)actionForKey:(NSString *)key
//{
//    if ([key isEqualToString:@"process"])
//    {
//        CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:key];
//        animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
//        animation.fromValue = @([[self presentationLayer] process]);
//        return animation;
//    }
//    return [super actionForKey:key];
//}


-(void)setProcess:(float)process{

    _process =process;
    
    CGFloat width =CGRectGetWidth(self.bounds);
    CGFloat height =CGRectGetHeight(self.bounds);
    
    CGFloat pHeight =process *height;
    CGFloat cHeight =pHeight +height/4 > height ?height : pHeight +height/4;
    
    UIBezierPath *path =[UIBezierPath bezierPath];
    [path moveToPoint:CGPointMake(0, height -pHeight)];
    [path addCurveToPoint:CGPointMake(width, height -pHeight) controlPoint1:CGPointMake(width/2, height -cHeight) controlPoint2:CGPointMake(width/2, height -cHeight)];
    [path addLineToPoint:CGPointMake(width, height)];
    [path addLineToPoint:CGPointMake(0, height)];
    [path addLineToPoint:CGPointMake(0, height -pHeight)];
    
    CAShapeLayer *shapeLayer =[CAShapeLayer layer];
    shapeLayer.path =path.CGPath;
    
    [CATransaction setDisableActions:YES];
    self.mask =shapeLayer;
    [CATransaction setDisableActions:NO];
}


@end

@interface MTTLogoAnimateView()

@property (nonatomic,strong) CALayer *emptyImageLayer;

@property (nonatomic,strong) MTTLogoAnimateLayer *imageLayer;

@end

@implementation MTTLogoAnimateView

-(id)initWithFrame:(CGRect)frame{

    if (self =[super initWithFrame:frame]) {
        [self setup];
    }
    return self;
}

-(id)initWithImage:(UIImage *)image{

    if (self =[super initWithImage:image]) {
        [self setup];
    }
    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder{

    if (self =[super initWithCoder:aDecoder]) {
        
        [self setup];
    }
    return self;
}

-(void)setup{

    self.processTintColor =[UIColor whiteColor];
    
    _process =0;
    
    self.activityIndicatorView =[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
    _activityIndicatorView.hidesWhenStopped =NO;
    [self addSubview:self.activityIndicatorView];
}


-(void)setImage:(UIImage *)image{

    self.orginImage =image;
    
    if (image !=nil) {
        self.emptyImage =[self convertWithImage:image color:self.processTintColor];
    }
}


-(void)layoutSubviews{

    
    /*
    if (self.emptyImageLayer ==nil) {
        self.emptyImageLayer =[CALayer layer];
        [self.layer addSublayer:self.emptyImageLayer];
    }
    if (self.emptyImageLayer.contents ==nil) {
        self.emptyImageLayer.contents =(id)[self.emptyImage CGImage];
    }
    
    if (self.imageLayer ==nil) {
        self.imageLayer =[MTTLogoAnimateLayer layer];
        [self.layer addSublayer:self.imageLayer];
    }
    if (self.imageLayer.contents ==nil) {
        self.imageLayer.contents =(id)[self.orginImage CGImage];
    }
    
    self.emptyImageLayer.frame =CGRectMake(0, 0, CGRectGetWidth(self.frame), CGRectGetHeight(self.frame));
    
    self.imageLayer.frame =CGRectMake(0, 0, CGRectGetWidth(self.frame), CGRectGetHeight(self.frame));
    
    self.process =0;
    
    [super layoutSubviews];
     */
    
    _activityIndicatorView.center =CGPointMake(CGRectGetWidth(self.frame)/2, CGRectGetHeight(self.frame)/2);
    
    [super layoutSubviews];
}

-(void)setProcess:(float)process{

//    if (process <0) {
//        process =0;
//    }
//    if (process >1) {
//        process =1;
//    }
//    
//    _process =process;
//    
//    [self.imageLayer setProcess:_process];
    
}

-(void)showAnimation{

    [self.activityIndicatorView startAnimating];
}

-(void)hideAnimation{
    [self.activityIndicatorView stopAnimating];
}

#pragma mark -

-(UIImage *)convertWithImage:(UIImage *)image color:(UIColor *)color{
    
    CGSize size = [image size];
    
    long width = size.width;
    
    long height = size.height;
    
    // the pixels will be painted to this array
    
    uint32_t *pixels = (uint32_t *) malloc(width * height * sizeof(uint32_t));
    
    // clear the pixels so any transparency is preserved
    
    memset(pixels, 0, width * height * sizeof(uint32_t));
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    // create a context with RGBA pixels
    
    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, width * sizeof(uint32_t), colorSpace,kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedLast);
    
    // paint the bitmap to our context which will fill in the pixels array
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), [image CGImage]);
    
    uint32_t *pixelsNew = (uint32_t *) malloc(width * height * sizeof(uint32_t));
    memcpy(pixelsNew, pixels, width * height * sizeof(uint32_t));
    
    CGContextRef contextNew = CGBitmapContextCreate(pixelsNew, width, height, 8, width * sizeof(uint32_t), colorSpace,kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedLast);
    
    CGFloat pixR =1;
    CGFloat pixG =1;
    CGFloat pixB =1;
    CGFloat pixA =1;
    
    BOOL isget =[color getRed:&pixR green:&pixG blue:&pixB alpha:&pixA];
    if (!isget) {
        pixR =1;
        pixG =1;
        pixB =1;
    }
    
    uint8_t cPixR =(uint8_t)pixR *255;
    uint8_t cPixG =(uint8_t)pixG *255;
    uint8_t cPixB =(uint8_t)pixB *255;
    
    for(int y = 0; y < height; y++) {
        
        for(int x = 0; x < width; x++) {
            
            uint8_t *rgbaPixel = (uint8_t *) &pixelsNew[y * width + x];

            uint8_t alpha =rgbaPixel[0];
            
            if (alpha >120) {
                
                rgbaPixel[0] =255;
                
                rgbaPixel[3] = cPixR;
                
                rgbaPixel[2] = cPixG;
                
                rgbaPixel[1] = cPixB;
            }
            else{
                rgbaPixel[0] =0;
            }
        }
    }
    
    CGImageRef imageRef = CGBitmapContextCreateImage(contextNew);
    
    CGContextRelease(context);
    CGContextRelease(contextNew);
    CGColorSpaceRelease(colorSpace);
    free(pixels);
    free(pixelsNew);
    
    // make a new UIImage to return
    UIImage *resultUIImage = [UIImage imageWithCGImage:imageRef];
    // we're done with image now too
    CGImageRelease(imageRef);
    
    return resultUIImage;
}


@end
