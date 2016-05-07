//
//  Encapsulator.m
//  OggSpeex
//
//  Created by Jiang Chuncheng on 6/25/13.
//  Copyright (c) 2013 Sense Force. All rights reserved.
//

#import "Encapsulator.h"

#define NOTIFICATION_ENCAPSULTING_OVER @"EncapsulatingOver"

@implementation Encapsulator

@synthesize moreDataInputing,isCanceled;
@synthesize speexHeader;
@synthesize mode, sampleRate, channels, nframes, vbr, streamSeraialNmber;
@synthesize mFileName;
@synthesize delegete;

void writeInt(unsigned char *dest, int offset, int value) {
    for(int i = 0;i < 4;i++) {
        dest[offset + i]=(unsigned char)(0xff & ((unsigned int)value)>>(i*8));
    }
}

void writeString(unsigned char *dest, int offset, unsigned char *value, int length) {
    unsigned char *tempPointr = dest + offset;
    memcpy(tempPointr, value, length);
}

+ (NSString *)defaultFileName {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *voiceDirectory = [documentsDirectory stringByAppendingPathComponent:@"voice"];
    if ( ! [[NSFileManager defaultManager] fileExistsAtPath:voiceDirectory]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:voiceDirectory withIntermediateDirectories:YES attributes:nil error:NULL];
    }
    return [voiceDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"%f.spx", [[NSDate date] timeIntervalSince1970]]];
    
}

- (id)initWithFileName:(NSString *)filename {
    if (self = [super init]) {
        mFileName = [NSString stringWithString:filename];
        NSFileManager *fileManager = [NSFileManager defaultManager];
        if ([fileManager fileExistsAtPath:filename]) {
            [fileManager removeItemAtPath:filename error:nil];
        }
        bufferData = [NSMutableData data];
        tempData = [NSMutableData data];
        pcmDatas = [NSMutableArray array];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(encapsulatingOver:) name:NOTIFICATION_ENCAPSULTING_OVER object:nil];
        
        [self setMode:0 sampleRate:8000 channels:1 frames:1 vbr:YES];
        
        NSLog(@"size of version(%s)",speexHeader.speex_version);
        speex_init_header(&speexHeader, sampleRate, channels, &speex_nb_mode);
        
        NSLog(@"size of version(%s)",speexHeader.speex_version);

        operationQueue = [[NSOperationQueue alloc] init];
    }
    return self;
}

- (void)resetWithFileName:(NSString *)filename {
    for(NSOperation *operation in [operationQueue operations]) {
        [operation cancel];
    }
    mFileName = [NSString stringWithString:filename];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:filename]) {
        [fileManager removeItemAtPath:filename error:nil];
    }

    [bufferData setLength:0];
    [tempData setLength:0];
    [pcmDatas removeAllObjects];
}

- (NSMutableData *)getBufferData {
    return bufferData;
}

- (NSMutableArray *)getPCMDatas {
    @synchronized(pcmDatas) {
        return pcmDatas;
    }
}


- (void)setMode:(int)_mode sampleRate:(int)_sampleRate channels:(int)_channels frames:(int)_nframes vbr:(BOOL)_vbr {
    self.mode = _mode;
    self.sampleRate = _sampleRate;
    self.channels = _channels;
    self.nframes = _nframes;
    self.vbr = _vbr;
    
}

- (void)prepareForEncapsulating {
        
    self.moreDataInputing = YES;
    self.isCanceled = NO;
    encapsulationOperation = [[EncapsulatingOperation alloc] initWithParent:self];
    if (operationQueue) {
        [operationQueue addOperation:encapsulationOperation];
    }
    
    //写入一些数据之前的头
    [encapsulationOperation writeHeaderWithComment:@"Encoded with:test by jcccn "];
    
}

- (void)inputPCMDataFromBuffer:(Byte *)buffer size:(UInt32)dataSize {

    if ( ! self.moreDataInputing) {
        return;
    }
    int packetSize = FRAME_SIZE * 2;
    @synchronized(pcmDatas) {
        [tempData appendBytes:buffer length:dataSize];
        while ([tempData length] >= packetSize) {
            @autoreleasepool {
                NSData *pcmData = [NSData dataWithBytes:[tempData bytes] length:packetSize];
                [pcmDatas addObject:pcmData];
                
                Byte *dataPtr = (Byte *)[tempData bytes];
                dataPtr += packetSize;
                tempData = [NSMutableData dataWithBytesNoCopy:dataPtr length:[tempData length] - packetSize freeWhenDone:NO];

            }
        }
    }
}

- (void)stopEncapsulating:(BOOL)forceCancel {
    self.moreDataInputing = NO;
    if ( ! self.isCanceled) {
        self.isCanceled = forceCancel;
    }
}

- (void)encapsulatingOver:(NSNotification *)notification {
    NSLog(@"encapsulatingOver by %@", [self description]);
    if (self.delegete) {
        [self.delegete encapsulatingOver];
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end

@implementation EncapsulatingOperation

@synthesize mParent;

//不停从bufferData中获取数据构建paket并且修改相关计数器
- (void)main {
    SpeexCodec *codec = [[SpeexCodec alloc] init];
    [codec open:4];     //压缩率为4
    while ( ! self.mParent.isCanceled) {
        if ([[self.mParent getPCMDatas] count] > 0) {
            NSData *pcmData = [[self.mParent getPCMDatas] objectAtIndex:0];
            
            NSData *speexData = [codec encode:(short *)[pcmData bytes] length:[pcmData length]/sizeof(short)];
            
            [self inputOggPacketFromSpeexData:speexData];
            
            if ([[self.mParent getPCMDatas] count] > 0)
            {
                [[self.mParent getPCMDatas] removeObjectAtIndex:0];
            }
        }
        else {
            [NSThread sleepForTimeInterval:0.02];
            
            if ( ! [self.mParent moreDataInputing]) {
                break;
            }
        }

    }
    [codec close];
    codec = nil;
    if ( ! [self.mParent isCanceled]) {
        [self outputAPage:NO endOfSteam:YES];
        [[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_ENCAPSULTING_OVER object:self userInfo:nil];
    }
}

//初始化NSOperation
- (id)initWithParent:(Encapsulator *)parent {
    if (self = [super init]) {
        self.mParent = parent;
        
        isFirstInput = 1;
        mPacketCount = 0;
        mGranulepos = 0;
        
        oggPeckets = [NSMutableArray array];
        
        ogg_stream_init(&oggStreamState, arc4random()%8888);
    }
    return self;
}


//写入ogg的头以及comment
- (void)writeHeaderWithComment:(NSString *)comment {
    
    mPacketCount = 0;
    mGranulepos = 0;
    
    //first, write the ogg header page
    unsigned char speexHeader[80];
    
    int offset = 0;
    writeString(speexHeader, offset+0, (unsigned char *)"Speex   ", 8);    //  0 -  7: speex_string
    int versionSize = sizeof(self.mParent.speexHeader.speex_version);
    NSLog(@"size of version(%s) chars array:%d",self.mParent.speexHeader.speex_version, versionSize);
    writeString(speexHeader, offset+8, (unsigned char *)self.mParent.speexHeader.speex_version, versionSize);  //8 - 27: speex_version
    writeInt(speexHeader, offset+28, 1);           // 28 - 31: speex_version_id
    writeInt(speexHeader, offset+32, 80);          // 32 - 35: header_size
    writeInt(speexHeader, offset+36, 8000);  // 36 - 39: rate
    writeInt(speexHeader, offset+40, 0);        // 40 - 43: mode (0=NB, 1=WB, 2=UWB)
    writeInt(speexHeader, offset+44, 4);           // 44 - 47: mode_bitstream_version
    writeInt(speexHeader, offset+48, 1);    // 48 - 51: nb_channels
    writeInt(speexHeader, offset+52, -1);          // 52 - 55: bitrate
    writeInt(speexHeader, offset+56, 160 << 0); // 56 - 59: frame_size (NB=160, WB=320, UWB=640)
    writeInt(speexHeader, offset+60, 1);     // 60 - 63: vbr
    writeInt(speexHeader, offset+64, 1);     // 64 - 67: frames_per_packet
    writeInt(speexHeader, offset+68, 0);           // 68 - 71: extra_headers
    writeInt(speexHeader, offset+72, 0);           // 72 - 75: reserved1
    writeInt(speexHeader, offset+76, 0);           // 76 - 79: reserved2
    
    ogg_packet speexHeaderPacket;
    speexHeaderPacket.packet = (unsigned char *)speexHeader;
    speexHeaderPacket.bytes = 80;
    speexHeaderPacket.b_o_s = 1;
    speexHeaderPacket.e_o_s = 0;
    speexHeaderPacket.granulepos = 0;
    speexHeaderPacket.packetno = mPacketCount++;
    
    ogg_stream_packetin(&oggStreamState, &speexHeaderPacket);
    [self outputAPage:YES endOfSteam:NO];
    NSLog(@"ogg header writed\n");
    
    
    
    //second. write the ogg comment page
    offset = 0;
    const char *commentChars = [comment cStringUsingEncoding:NSUTF8StringEncoding];
    int length = [comment lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    unsigned char speexCommentHeader[length + 8];
    writeInt(speexCommentHeader, offset, length);       // vendor comment size
    writeString(speexCommentHeader, offset+4, (unsigned char *)commentChars, length); // vendor comment
    writeInt(speexCommentHeader, offset+length+4, 0);   // user comment list length
    
    ogg_packet speexCommentPacket;
    speexCommentPacket.packet = (unsigned char *)speexCommentHeader;
    speexCommentPacket.bytes = length + 8;
    speexCommentPacket.b_o_s = 0;
    speexCommentPacket.e_o_s = 0;
    speexCommentPacket.granulepos = 0;
    speexCommentPacket.packetno = mPacketCount++;
    
    ogg_stream_packetin(&oggStreamState, &speexCommentPacket);
    [self outputAPage:YES endOfSteam:NO];
    NSLog(@"ogg comment writed\n");
}

- (void)inputOggPacketFromSpeexData:(NSData *)data {
    ogg_packet packet;
    packet.packet = (unsigned char *)[data bytes];
    packet.bytes = (long)([data length]);
    packet.b_o_s = 0;
    packet.e_o_s = 0;
    mGranulepos += FRAME_SIZE;
    packet.granulepos = mGranulepos;
    packet.packetno = mPacketCount++;
    ogg_stream_packetin(&oggStreamState, &packet);
    
    [self checkPageSufficient];
}

//检查packet是否足够生成一个page
- (void)checkPageSufficient {
    [self outputAPage:NO endOfSteam:NO];
}

//将页保存至文件并重置一些计数器。是否关闭文件。
- (void)outputAPage:(BOOL)isHeaderOrComment endOfSteam:(BOOL)endOfStream {
    if (isHeaderOrComment || endOfStream) {
        ogg_stream_flush(&oggStreamState, &oggPage);
        [[self.mParent getBufferData] appendBytes:oggPage.header length:oggPage.header_len];
        [[self.mParent getBufferData] appendBytes:oggPage.body length:oggPage.body_len];
        [self writeDataToFile:[self.mParent getBufferData]];
        [[self.mParent getBufferData] setLength:0];
        
        if (endOfStream) {
            NSLog(@"end of stream");
//            self.mParent.moreDataInputing = NO;
        }
    }
    else {
        if (ogg_stream_pageout(&oggStreamState, &oggPage)) {
            NSLog(@"page out");
            [[self.mParent getBufferData] appendBytes:oggPage.header length:oggPage.header_len];
            [[self.mParent getBufferData] appendBytes:oggPage.body length:oggPage.body_len];
            [self writeDataToFile:[self.mParent getBufferData]];
            
            [[self.mParent getBufferData] setLength:0];
        }
    }
    
}

- (void)writeDataToFile:(NSData *)newData {
    NSString *filename = (NSString *)self.mParent.mFileName;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ( ! [fileManager fileExistsAtPath:filename]) {
        [fileManager createFileAtPath:filename contents:nil attributes:nil];
    }
//    NSLog(@"write data of %d bytes to file %@", [newData length], filename);
    NSFileHandle *file = [NSFileHandle fileHandleForUpdatingAtPath:filename];
    [file seekToEndOfFile];
    [file writeData:newData];
    [file closeFile];
}


@end