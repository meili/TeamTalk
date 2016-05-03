/*

    File: AQRecorder.mm
Abstract: n/a
 Version: 2.5

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may
be used to endorse or promote products derived from the Apple Software
without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or
implied, are granted by Apple herein, including but not limited to any
patent rights that may be infringed by your derivative works or by other
works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2012 Apple Inc. All Rights Reserved.


*/

#include "AQRecorder.h"
#include "CAStreamBasicDescription.h"
#include "CAXException.h"

// ____________________________________________________________________________________
// Determine the size, in bytes, of a buffer necessary to represent the supplied number
// of seconds of audio data.
int AQRecorder::ComputeRecordBufferSize(const AudioStreamBasicDescription *format, float seconds)
{
	int packets, frames, bytes = 0;
	frames = (int)ceil(seconds * format->mSampleRate);
    
    if (format->mBytesPerFrame > 0)
        bytes = frames * format->mBytesPerFrame;
    else {
        UInt32 maxPacketSize;
        if (format->mBytesPerPacket > 0)
            maxPacketSize = format->mBytesPerPacket;	// constant packet size
        else {
            UInt32 propertySize = sizeof(maxPacketSize);
            errorStatus = AudioQueueGetProperty(mQueue, kAudioQueueProperty_MaximumOutputPacketSize, &maxPacketSize,
                                                &propertySize);
            if (errorStatus) {
                NSLog(@"ComputeRecordBufferSize error:%ld", errorStatus);
                return 0;
            }
        }
        if (format->mFramesPerPacket > 0)
            packets = frames / format->mFramesPerPacket;
        else
            packets = frames;	// worst-case scenario: 1 frame in a packet
        if (packets == 0)		// sanity check
            packets = 1;
        bytes = packets * maxPacketSize;
    }
	return bytes;
}

// ____________________________________________________________________________________
// AudioQueue callback function, called when an input buffers has been filled.
void AQRecorder::MyInputBufferHandler(	void *								inUserData,
										AudioQueueRef						inAQ,
										AudioQueueBufferRef					inBuffer,
										const AudioTimeStamp *				inStartTime,
										UInt32								inNumPackets,
										const AudioStreamPacketDescription*	inPacketDesc)
{
	AQRecorder *aqr = (AQRecorder *)inUserData;
	
    if (inNumPackets > 0) {
        
        if (aqr->mEncapsulator) {
            [aqr->mEncapsulator inputPCMDataFromBuffer:(unsigned char *)inBuffer->mAudioData size:inBuffer->mAudioDataByteSize];
        }
        
        aqr->mRecordPacket += inNumPackets;
    }
    
    // if we're not stopping, re-enqueue the buffe so that it gets filled again
    if (aqr->IsRunning()) {
        OSStatus errorStatus = AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
        if (errorStatus) {
            NSLog(@"MyInputBufferHandler error:%ld", errorStatus);
            return;
        }
    }
}

AQRecorder::AQRecorder()
{
	mIsRunning = false;
	mRecordPacket = 0;
}

AQRecorder::~AQRecorder()
{
	AudioQueueDispose(mQueue, TRUE);
//	AudioFileClose(mRecordFile);
//	if (mFileName) CFRelease(mFileName);
    mEncapsulator = nil;
}

void AQRecorder::SetupAudioFormat(UInt32 inFormatID)
{
	memset(&mRecordFormat, 0, sizeof(mRecordFormat));
			
	mRecordFormat.mFormatID = inFormatID;
	if (inFormatID == kAudioFormatLinearPCM)
	{
		// if we want pcm, default to signed 16-bit little-endian
		mRecordFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
		mRecordFormat.mBitsPerChannel = 16;
        mRecordFormat.mChannelsPerFrame = 1;
		mRecordFormat.mBytesPerFrame = (mRecordFormat.mBitsPerChannel / 8) * mRecordFormat.mChannelsPerFrame;
        mRecordFormat.mFramesPerPacket = 1;
        mRecordFormat.mBytesPerPacket = mRecordFormat.mBytesPerFrame * mRecordFormat.mFramesPerPacket;
        
        mRecordFormat.mSampleRate = 8000;   //8k frames per second
	}
}

Boolean AQRecorder::StartRecord(Encapsulator *encapsulator) {
    int i, bufferByteSize;
	UInt32 size;
	
	mEncapsulator = encapsulator;
    [mEncapsulator prepareForEncapsulating];
    
    // specify the recording format
    SetupAudioFormat(kAudioFormatLinearPCM);
    
    // create the queue
    errorStatus = AudioQueueNewInput(
                                     &mRecordFormat,
                                     MyInputBufferHandler,
                                     this /* userData */,
                                     NULL /* run loop */, NULL /* run loop mode */,
                                     0 /* flags */, &mQueue);
    if (errorStatus) {
        NSLog(@"StartRecord error:%ld when AudioQueueNewInput ", errorStatus);
        return false;
    }
    
    // get the record format back from the queue's audio converter --
    // the file may require a more specific stream description than was necessary to create the encoder.
    mRecordPacket = 0;
    
    size = sizeof(mRecordFormat);
    errorStatus = AudioQueueGetProperty(mQueue, kAudioQueueProperty_StreamDescription,
                                        &mRecordFormat, &size);
    if (errorStatus) {
        NSLog(@"StartRecord error:%ld when AudioQueueGetProperty StreamDescription", errorStatus);
    }
    
    UInt32 val = 1;
    errorStatus = AudioQueueSetProperty(mQueue, kAudioQueueProperty_EnableLevelMetering, &val, sizeof(UInt32));
    if (errorStatus) {
        NSLog(@"StartRecord error:%ld when AudioQueueGetProperty LevelMetering", errorStatus);
    }
    
    // allocate and enqueue buffers
    bufferByteSize = ComputeRecordBufferSize(&mRecordFormat, kBufferDurationSeconds);	// enough bytes for half a second
    for (i = 0; i < kNumberRecordBuffers; ++i) {
        errorStatus = AudioQueueAllocateBuffer(mQueue, bufferByteSize, &mBuffers[i]);
        errorStatus = AudioQueueEnqueueBuffer(mQueue, mBuffers[i], 0, NULL);
        if (errorStatus) {
            NSLog(@"StartRecord error:%ld alloc and enqueue buffer", errorStatus);
        }
    }
    // start the queue
    mIsRunning = true;
    errorStatus = AudioQueueStart(mQueue, NULL);
    if (errorStatus) {
        NSLog(@"StartRecord error:%ld", errorStatus);
        return false;
    }
    return true;
}

void AQRecorder::StopRecord()
{
	// end recording
	mIsRunning = false;
	errorStatus = AudioQueueStop(mQueue, true);
    if (errorStatus) {
        NSLog(@"StopRecord error:%ld", errorStatus);
    }
	AudioQueueDispose(mQueue, true);
    
    if (mEncapsulator) {
        [mEncapsulator stopEncapsulating:NO];
    }
    mEncapsulator = nil;

}
