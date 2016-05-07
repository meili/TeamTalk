//
//  SpeexCodec.h
//  OggSpeex
//
//  Created by Jiang Chuncheng on 11/26/12.
//  Copyright (c) 2012 Sense Force. All rights reserved.
//

/**----Guide----
 * Encode: open -> encode:length -> close
 * Decode: open -> encode:length:output -> close
 */

#import <Foundation/Foundation.h>
#import "SpeexAllHeaders.h"

@interface SpeexCodec : NSObject {
    int codecOpenedTimes;         //the times the codec is opened
    
    int encodeFrameSize;
    int decodeFrameSize;
    SpeexBits encodeSpeexBits;
    SpeexBits decodeSpeexBits;
    void *encodeState;
    void *decodeState;
}

- (void)open:(int)quality;
- (NSData *)encode:(short *)pcmBuffer length:(int)lengthOfShorts;
- (int)decode:(Byte *)encodedBytes length:(int)lengthOfBytes output:(short *)decoded;
- (void)close;

@end
