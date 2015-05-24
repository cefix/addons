

#include "IOSAVFoundationVideo.h"
#include <cefix/Log.h>
#include <osgdB/FileNameUtils>
#include "IOSUtils.h"

#include <iostream>

#import <AVFoundation/AVFoundation.h>

@interface AVPlayer (MOAdditions)
- (NSURL *)currentURL;
- (void)setVolume:(CGFloat)volume;
@end;

@implementation AVPlayer (MOAdditions)

- (NSURL *)currentURL {
    AVAsset *asset = self.currentItem.asset;
    if ([asset isMemberOfClass:[AVURLAsset class]])
        return ((AVURLAsset *)asset).URL;
    return nil;
}

- (void)setVolume:(CGFloat)volume {
    NSArray *audioTracks = [self.currentItem.asset tracksWithMediaType:AVMediaTypeAudio];
    NSMutableArray *allAudioParams = [NSMutableArray array];
    for (AVAssetTrack *track in audioTracks) {
        AVMutableAudioMixInputParameters *audioInputParams = [AVMutableAudioMixInputParameters audioMixInputParameters];
        [audioInputParams setVolume:volume atTime:kCMTimeZero];
        [audioInputParams setTrackID:[track trackID]];
        [allAudioParams addObject:audioInputParams];
    }
    AVMutableAudioMix *audioMix = [AVMutableAudioMix audioMix];
    [audioMix setInputParameters:allAudioParams];
    [self.currentItem setAudioMix:audioMix];
}

@end

@interface IOSAVFoundationVideoDelegate : NSObject {
    cefix::IOSAVFoundationVideo* video;
}
@property (readwrite,assign) cefix::IOSAVFoundationVideo* video;

- (void) playerItemDidReachEnd:(NSNotification*)the_notification;

@end;

@implementation IOSAVFoundationVideoDelegate

@synthesize video;

- (void) playerItemDidReachEnd:(NSNotification*)the_notification
{
    if (video->getLoopingMode() == osg::ImageStream::LOOPING) {
        video->jumpTo(0);
    }
    else {
        video->pause();
    }
}

@end


namespace cefix {

class IOSAVFoundationVideo::Data {
public:
    AVPlayer* avplayer;
    AVPlayerItem* avplayeritem;
    AVPlayerItemVideoOutput* output;
    IOSAVFoundationVideoDelegate* delegate;
    CVPixelBufferRef lastFrame;
    Data() : avplayer(NULL), avplayeritem(NULL), output(NULL), delegate(NULL), lastFrame(NULL) {}
    ~Data() {
        [output release];
        [avplayeritem release];
        [avplayer release];
        
        [delegate release];
        
        if (lastFrame) {
            CVBufferRelease(lastFrame);
        }
        
        output = NULL;
        avplayer = NULL;
        avplayeritem = NULL;
        delegate = NULL;
        lastFrame = NULL;
    }
};

IOSAVFoundationVideo::IOSAVFoundationVideo(const std::string& fileName)
:   cefix::Video(),
    cefix::VideoMediaAdapter<IOSAVFoundationVideo>("av_foundation_video"),
    _isValid(false),
    _isActive(true),
    _isPlaying(false),
    _wasPlaying(false),
    _videoWidth(0),
    _videoHeight(0),
    _volume(1.0)
{
    _data = new Data();
    if (!fileName.empty())
        load(fileName);
    
    setOrigin(TOP_LEFT);
}


IOSAVFoundationVideo::~IOSAVFoundationVideo()
{
    if (_data)
        delete _data;
}
        
void IOSAVFoundationVideo::play()
{
    if (_data->avplayer) {
        [_data->avplayer play];
        setRunning(true);
    }
}


void IOSAVFoundationVideo::setMovieRate(double rate)
{
    if (_data->avplayer)
        _data->avplayer.rate = rate;
}

double IOSAVFoundationVideo::getMovieRate() const
{
    return _data->avplayer ? _data->avplayer.rate : 0.0f;
}
    

void IOSAVFoundationVideo::pause()
{
    if (_data->avplayer) {
        [_data->avplayer pause];
        setRunning(false);
        _isPlaying = false;
    }
}


void IOSAVFoundationVideo::clear()
{
    [_data->output release];
    [_data->avplayeritem release];
    [_data->avplayer release];
    
    if (_data->delegate) {
        [[NSNotificationCenter defaultCenter] removeObserver: _data->delegate
            name:AVPlayerItemDidPlayToEndTimeNotification
            object:[_data->avplayer currentItem]
        ];
    }
    
    [_data->delegate release];
    
    _data->output = NULL;
    _data->avplayer = NULL;
    _data->avplayeritem = NULL;
    _data->delegate = NULL;
}


void IOSAVFoundationVideo::quit()
{
    pause();
}


void IOSAVFoundationVideo::jumpTo(double pos)
{
    if(_data->avplayer)
        [_data->avplayer seekToTime: CMTimeMakeWithSeconds(pos, 600)];
}
    
double IOSAVFoundationVideo::getCurrentTime ()
{
    return _data->avplayer ? CMTimeGetSeconds([_data->avplayer currentTime]) : 0;
}

void IOSAVFoundationVideo::load(const std::string& filename)
{
    clear();
    
    _data->delegate = [[IOSAVFoundationVideoDelegate alloc] init];
    _data->delegate.video = this;
    
    NSURL* url(NULL);
    if (osgDB::containsServerAddress(filename)) {
        url = [NSURL URLWithString: cefix::IOSUtils::toNSString(filename)];
    } else {
        url = [NSURL fileURLWithPath: cefix::IOSUtils::toNSString(filename)];
    }
    
    _data->output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey, nil]];
    if (_data->output) {
        _data->output.suppressesPlayerRendering = YES;
    }
    
    _data->avplayeritem = [[AVPlayerItem alloc] initWithURL: url];
    _data->avplayer = [AVPlayer playerWithPlayerItem: _data->avplayeritem];
    _data->avplayer.actionAtItemEnd = AVPlayerActionAtItemEndNone;
    
    [[_data->avplayer currentItem] addOutput:_data->output];
    
    [[NSNotificationCenter defaultCenter] addObserver: _data->delegate
        selector:@selector(playerItemDidReachEnd:)
        name:AVPlayerItemDidPlayToEndTimeNotification
        object:[_data->avplayer currentItem]];
    
    _videoDuration = CMTimeGetSeconds([[_data->avplayer currentItem] duration]);
    
    _isValid = true;
}
    
float IOSAVFoundationVideo::getVolume() const
{
    return _volume;
}


void IOSAVFoundationVideo::setVolume(float v)
{
    _volume = v;
    if (_data->avplayer)
        [_data->avplayer setVolume: v];
}


float IOSAVFoundationVideo::getAudioBalance()
{
    return 0.0f;
}


void IOSAVFoundationVideo::setAudioBalance(float b)
{
    cefix::log::info("IOSAVFoundationVideo") << "setAudioBalance not supported!" << std::endl;
}

            
void IOSAVFoundationVideo::deactivate()
{
    _isActive = false;
}


void IOSAVFoundationVideo::activate()
{
    _isActive = true;
}


    
void IOSAVFoundationVideo::idle()
{
    _isValid = (_data && (_data->avplayer.status != AVPlayerStatusFailed));
    if (!_isValid) {
        pause();
        cefix::log::error("IOSAVFoundationVideo") << "error: " << cefix::IOSUtils::toString([_data->avplayer.error localizedFailureReason]) << std::endl;
    }
    
    _isPlaying = _isValid && (getMovieRate() != 0);
    
    
    CMTime outputItemTime = [_data->output itemTimeForHostTime:CACurrentMediaTime()];
	if ([_data->output hasNewPixelBufferForItemTime:outputItemTime]) {
        if (_data->lastFrame) {
            CVBufferRelease(_data->lastFrame);
        }
		
        _data->lastFrame = [_data->output copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
        
        CVPixelBufferLockBaseAddress(_data->lastFrame,0);

        // Get the number of bytes per row for the pixel buffer
        // size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer); 
        // Get the pixel buffer width and height
        size_t width = CVPixelBufferGetWidth(_data->lastFrame);
        size_t height = CVPixelBufferGetHeight(_data->lastFrame);

        // Get the base address of the pixel buffer
        void *baseAddress = CVPixelBufferGetBaseAddress(_data->lastFrame);
        bool callback_needed = (width != s()) || (height != t());
        setImage(width, height, 1, GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE, (unsigned char*)baseAddress, NO_DELETE);
        
        CVPixelBufferUnlockBaseAddress(_data->lastFrame, 0);
        if (callback_needed)
            handleDimensionChangedCallbacks();
        
    }
}


void IOSAVFoundationVideo::applyLoopingMode()
{
}

}
    