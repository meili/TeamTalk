//
//  MTTURLProtocal.m
//  TeamTalk

#import "MTTURLProtocal.h"

#define kOpCenterAPI_url @"https://mogujie.org"

static NSString *cachingURLHeader = @"hadInURLProtocal";

@interface MTTURLProtocal ()

@property (nonatomic,strong) NSURLConnection *connection;

@property (nonatomic,strong)NSMutableData *proRespData;

@end

@implementation MTTURLProtocal

- (void)startLoading
{
    //NSLog(@"1.start loading");
    //可以这样子拿到原来的请求，用以拼装自己的请求
    NSMutableURLRequest *proxyRequest = [self.request mutableCopy];
    //NSLog(@"protocal url is %@",[[proxyRequest URL] absoluteString]);
    [proxyRequest setValue:@"" forHTTPHeaderField:cachingURLHeader];
    //connection = [NSURLConnection connectionWithRequest:proxyRequest delegate:self];
    _connection = [[NSURLConnection alloc] initWithRequest:proxyRequest delegate:self startImmediately:NO];
    [_connection start];
}

- (void)stopLoading
{
    [_connection cancel];
}

- (NSURLRequest *)connection:(NSURLConnection *)connection willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)response
{
    if(response!=nil)
    {
        NSMutableURLRequest *redirectableRequest = [request mutableCopy];
        [redirectableRequest setValue:nil forHTTPHeaderField:cachingURLHeader];
        [self.client URLProtocol:self wasRedirectedToRequest:redirectableRequest redirectResponse:response];
        return redirectableRequest ;
    }
    return request;
}

- (BOOL)connection:(NSURLConnection *)connection canAuthenticateAgainstProtectionSpace:(NSURLProtectionSpace *)protectionSpace {
    //NSLog(@"3.We are checking protection Space!");
    return YES;
    if([protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust])
    {
        //NSLog(@"Can Auth Secure Requestes!");
        return YES;
    }
    else if([protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodHTTPBasic])
    {
        //NSLog(@"Can Auth Basic Requestes!");
        return YES;
        //return NO;
    }
    NSLog(@"Cannot Auth!");
    return NO;
    
}

- (void)connection:(NSURLConnection *)connection didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    
    OSStatus err;
    NSURLCredential * credential;
    
    assert(challenge != nil);
    
    credential = nil;
    
    // Handle ServerTrust and Client Certificate challenges
    
    NSString *authenticationMethod = [[challenge protectionSpace] authenticationMethod];
    if ([authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        //NSLog(@"Trust Challange");
        SecTrustResultType trustResultType;
        err = SecTrustEvaluate(challenge.protectionSpace.serverTrust, &trustResultType);
        
        //NSLog(@"SecTrustResult %u %d",trustResultType, (int)err);
        
        if (trustResultType == kSecTrustResultProceed || trustResultType == kSecTrustResultUnspecified) {
            credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
            assert(credential != nil);
        }
    } else {
        credential = [NSURLCredential credentialWithIdentity:[[self class] identityWithCert] certificates:nil persistence:NSURLCredentialPersistencePermanent]; //到呢一步，certificates需要喺nil先过到
    }
    //NSLog(@"credential is %@",credential);
    [challenge.sender useCredential:credential forAuthenticationChallenge:challenge];
    //[protocol resolveAuthenticationChallenge:challenge withCredential:credential];
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    //这里是收到响应的头部信息，比如HTTP Header，可视情况做对self.client做相应操作，也可以不做处理
    //NSLog(@"5.didReceiveResponse lalala");
    //NSLog(@"response is %@",response);
    [self.client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageNotAllowed];
}

//这几个不是Protocol的方法，是自发起的URLConnection的回调
- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)sourceData
{
    //NSLog(@"sourceData is %@",sourceData);
    //此方法会收到一部分或者全部的数据，可以这样子丢给原始请求的发起者
    //NSLog(@"6.in didReceiveData method");
    if (_proRespData == nil) {
        _proRespData = [sourceData mutableCopy];
    } else {
        [_proRespData appendData:sourceData];
    }
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    //和上一个方法类似，这里作为错误通知
    //NSLog(@"7.1in didFailWithError method %@",error);
    [self.client URLProtocol:self didFailWithError:error];
    //这里可以弹出一个错误提示
    //    [toastView hideToast];
    //    NSString *msg = [@"网络错误 - " stringByAppendingString:[error description]];
    //    [toastView showToastWithOK:msg];
    return;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
   
    NSData *getData = _proRespData ;
    [self.client URLProtocol:self didLoadData:getData];
    [self.client URLProtocolDidFinishLoading:self];
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)request
{
    //根据request来决定要不要劫持
    //需要特别注意的是，如果你用NSURLConnection来发起代理请求，那么那个代理请求的request也同样会经过这里来做判决，所以一定要判断是不是代理请求，然后返回NO
    if ([request.URL.absoluteString rangeOfString:kOpCenterAPI_url].location != NSNotFound && [request valueForHTTPHeaderField:cachingURLHeader] == nil) return YES;
    return NO;
}

+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request
{
    //这里是干嘛的，我还没研究清楚，就简单的返回了原始值，有兴趣的话你可以研究一下
    return request;
}

#pragma mark -

+ (SecIdentityRef)identityWithCert{
    NSString *path = [[NSBundle mainBundle]pathForResource:@"yunwei" ofType:@"p12"];
    NSData *p12data = [NSData dataWithContentsOfFile:path];
    SecIdentityRef identity = NULL; SecCertificateRef certificate = NULL;
    
    if (!p12data) {
        NSLog(@"Can not load pkcs12 cert , plz check !");
        return nil;
    }
    
    [[self class] identity:&identity andCertificate:&certificate fromPKCS12Data:p12data];
    return identity;
}


+ (BOOL)identity:(SecIdentityRef *)outIdentity andCertificate:(SecCertificateRef*)outCert fromPKCS12Data:(NSData *)inPKCS12Data{
    
    OSStatus securityError = errSecSuccess;
    
    NSDictionary *optionsDictionary = [NSDictionary dictionaryWithObject:@"1234" forKey:(__bridge id)kSecImportExportPassphrase];
    
    CFArrayRef items = CFArrayCreate(NULL, 0, 0, NULL); securityError = SecPKCS12Import((__bridge CFDataRef)inPKCS12Data,(__bridge CFDictionaryRef)optionsDictionary,&items);
    
    if (securityError == 0) {
        CFDictionaryRef myIdentityAndTrust = CFArrayGetValueAtIndex (items, 0);
        const void *tempIdentity = NULL;
        tempIdentity = CFDictionaryGetValue (myIdentityAndTrust, kSecImportItemIdentity);
        *outIdentity = (SecIdentityRef)tempIdentity;
        const void *tempCert = NULL;
        tempCert = CFDictionaryGetValue (myIdentityAndTrust, kSecImportItemCertChain);
        *outCert = (SecCertificateRef)tempCert;
        
    } else {
        return NO;
    }
    
    return YES;
}


@end
