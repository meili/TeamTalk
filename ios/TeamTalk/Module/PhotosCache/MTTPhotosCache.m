//
//  MTTPhotosCache.m
//  IOSDuoduo
//
//  Created by 东邪 on 14-5-29.
//  Copyright (c) 2014年 dujia. All rights reserved.
//

#import "MTTPhotosCache.h"
#import "SDImageCache.h"

#import "NSString+Additions.h"
#import <CommonCrypto/CommonDigest.h>
#import "MTTSundriesCenter.h"


@implementation MTTPhotoEnity
@end


@interface MTTPhotosCache()
@property (readonly, nonatomic) dispatch_queue_t ioQueue;
@property(nonatomic,retain) NSFileManager *fileManager;
@property (retain, nonatomic) NSCache *memCache;
@end
@implementation MTTPhotosCache


+(void)calculatePhotoSizeWithCompletionBlock:(void (^)(NSUInteger fileCount, NSUInteger totalSize))completionBlock
{
    NSURL *diskCacheURL = [NSURL fileURLWithPath:PhotosMessageDir isDirectory:YES];
    [ [MTTSundriesCenter instance] pushTaskToSerialQueue:^{
        NSUInteger fileCount = 0;
        NSUInteger totalSize = 0;
        NSFileManager *fileManager = [[NSFileManager alloc] init];
        NSDirectoryEnumerator *fileEnumerator = [fileManager enumeratorAtURL:diskCacheURL
                                                  includingPropertiesForKeys:@[NSFileSize]
                                                                     options:NSDirectoryEnumerationSkipsHiddenFiles
                                                                errorHandler:NULL];
        
        for (NSURL *fileURL in fileEnumerator) {
            NSNumber *fileSize;
            [fileURL getResourceValue:&fileSize forKey:NSURLFileSizeKey error:NULL];
            totalSize += [fileSize unsignedIntegerValue];
            fileCount += 1;
        }
        
        if (completionBlock) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completionBlock(fileCount, totalSize);
            });
        }
    }];
    
}
-(NSData *)photoFromDiskCacheForKey:(NSString *)key
{
    NSData *photoData = [self photoFromMemoryCacheForKey:key];
    if (photoData) {
        return photoData;
    }
    // Second check the disk cache...
    NSData *diskphotoData = [self diskPhotoForKey:key];
    if (diskphotoData) {
        [self.memCache setObject:diskphotoData forKey:key ];
    }
    return diskphotoData;
}
+ (MTTPhotosCache *)sharedPhotoCache
{
    static dispatch_once_t once;
    static id instance;
    dispatch_once(&once, ^{
        instance = [self new];
     
    });
    return instance;
}
- (instancetype)init
{
    self = [super init];
    if (self) {
        _ioQueue = dispatch_queue_create("com.mogujie.DDPhotosCache", DISPATCH_QUEUE_SERIAL);
        _memCache = [NSCache new];
        
        dispatch_sync(_ioQueue, ^{
            _fileManager = [NSFileManager new];
        });
        
    }
    return self;
}
- (void)storePhoto:(NSData *)data forKey:(NSString *)key
{
    [self storePhoto:data forKey:key toDisk:YES];
}

- (void)removePhotoForKey:(NSString *)key
{
    [self.memCache removeObjectForKey:key];
    
    dispatch_async(self.ioQueue, ^{
        [_fileManager removeItemAtPath:[self defaultCachePathForKey:key] error:nil];
    });
    
    
}
- (void)removePhotoFromNSCacheForKey:(NSString *)key
{
    [self.memCache removeObjectForKey:key];
    
}
- (NSUInteger)getSize {
    __block NSUInteger size = 0;
    dispatch_sync(self.ioQueue, ^{
        NSDirectoryEnumerator *fileEnumerator = [_fileManager enumeratorAtPath:PhotosMessageDir];
        for (NSString *fileName in fileEnumerator) {
            NSString *filePath = [PhotosMessageDir stringByAppendingPathComponent:fileName];
            NSDictionary *attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:nil];
            size += [attrs fileSize];
        }
    });
    return size;
}
- (int)getDiskCount {
    __block int count = 0;
    dispatch_sync(self.ioQueue, ^{
        NSDirectoryEnumerator *fileEnumerator = [_fileManager enumeratorAtPath:PhotosMessageDir];
        for (__unused NSString *fileName in fileEnumerator) {
            count += 1;
        }
    });
    return count;
}
- (void)storePhoto:(NSData *)photo forKey:(NSString *)key toDisk:(BOOL)toDisk {
    if (!photo || !key) {
        return;
    }
    [self.memCache setObject:photo forKey:key ];
    
    if (toDisk) {
        dispatch_async(self.ioQueue, ^{
            if (photo) {
                if (![_fileManager fileExistsAtPath:PhotosMessageDir]) {
                    [_fileManager createDirectoryAtPath:PhotosMessageDir withIntermediateDirectories:YES attributes:nil error:NULL];
                }
                
                [_fileManager createFileAtPath:[self defaultCachePathForKey:key] contents:photo attributes:nil];
            }
        });
    }
}
- (NSString *)cachePathForKey:(NSString *)key inPath:(NSString *)path {
    NSString *filename = [self cachedFileNameForKey:key];
    return [path stringByAppendingPathComponent:filename];
}

- (NSString *)defaultCachePathForKey:(NSString *)key {
    return [self cachePathForKey:key inPath:PhotosMessageDir];
}

- (NSString *)cachedFileNameForKey:(NSString *)key {
    const char *str = [key UTF8String];
    if (str == NULL) {
        str = "";
    }
    unsigned char r[CC_MD5_DIGEST_LENGTH];
    CC_MD5(str, (CC_LONG)strlen(str), r);
    NSString *filename = [NSString stringWithFormat:@"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                          r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15]];
    
    return filename;
}
- (NSOperation *)queryDiskCacheForKey:(NSString *)key done:(void (^)(NSData *photo))doneBlock {
    NSOperation *operation = [NSOperation new];
    
    if (!doneBlock) return nil;
    
    if (!key) {
        doneBlock(nil);
        return nil;
    }
    
    // First check the in-memory cache...
    NSData *photo = [self photoFromMemoryCacheForKey:key];
    if (photo) {
        doneBlock(photo);
        return nil;
    }
    
    dispatch_async(self.ioQueue, ^{
        if (operation.isCancelled) {
            return;
        }
        
        @autoreleasepool {
            NSData *diskPhotos = [self diskPhotoForKey:key];
            if (diskPhotos) {
                
                [self.memCache setObject:diskPhotos forKey:key];
            }
            
            dispatch_async(dispatch_get_main_queue(), ^{
                doneBlock(diskPhotos);
            });
        }
    });
    
    return operation;
}
- (NSData *)photoFromMemoryCacheForKey:(NSString *)key {
    return [self.memCache objectForKey:key];
}
- (NSData *)diskPhotoForKey:(NSString *)key {
    NSData *data = [self diskPhotosDataBySearchingAllPathsForKey:key];
    if (data) {
        return data;
    }
    else {
        return nil;
    }
}
- (NSData *)diskPhotosDataBySearchingAllPathsForKey:(NSString *)key {
    NSString *defaultPath = [self defaultCachePathForKey:key];
    NSData *data = [NSData dataWithContentsOfFile:defaultPath];
    if (data) {
        return data;
    }
    return nil;
}
-(NSString *)getKeyName
{
    NSDateFormatter * formatter = [[NSDateFormatter alloc ] init];
    [formatter setDateFormat:@"YYYYMMddhhmmssSSS"];
    NSString *date =  [formatter stringFromDate:[NSDate date]];
    NSString *timeLocal = [[NSString alloc] initWithFormat:@"%@", date];
    return [NSString stringWithFormat:@"%@_send",timeLocal];
}
-(NSMutableArray *)getAllImageCache
{
    __block NSMutableArray *array = [NSMutableArray new];
    dispatch_sync(self.ioQueue, ^{
        NSDirectoryEnumerator *fileEnumerator = [_fileManager enumeratorAtPath:PhotosMessageDir];
        for (__unused NSString *fileName in fileEnumerator) {
       
                [array addObject:[NSString stringWithFormat:@"%@/%@",PhotosMessageDir,fileName]];
            
        }
    });
    return array;
}
-(void)clearAllCache:(void(^)(bool isfinish))block;
{
    [self.memCache removeAllObjects];
    NSArray *allimage = [self getAllImageCache];
    if( [allimage count] == 0 )
          block(YES);
    [allimage enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *path = (NSString *)obj;
        [_fileManager removeItemAtPath:path error:nil];
        if (idx == [allimage count]-1 ) {
            block(YES);
        }
    }];
}
@end
