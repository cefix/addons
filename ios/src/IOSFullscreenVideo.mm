//
//  IOSNativeVideo.cpp
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 25.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "IOSFullscreenVideo.h"
#include <osgDB/FileUtils>
#include <osg/ImageUtils>
#include <cefix/Log.h>
#include "IOSUtils.h"
#include <cefix/DisplayCollection.h>
#include <cefix/ApplicationWindow.h>
#include <cefix/Pixel.h>

#import <MediaPlayer/MediaPlayer.h>
#import "BlockerView.h"


@interface MoviePlayerObserver : NSObject
{
	cefix::IOSFullscreenVideo* _video;
}

@property (readwrite,assign) cefix::IOSFullscreenVideo* video;

- (void)handleExitFullscreen:(NSNotification*)notification;
- (void)handlePlaybackStateDidChange:(NSNotification*)notification;
- (void)dealloc;
@end

@implementation MoviePlayerObserver
@synthesize video=_video;

- (void)handleExitFullscreen:(NSNotification*)notification {
    
    MPMoviePlayerController *player = [notification object];
    [[NSNotificationCenter defaultCenter] 
      removeObserver:self
      name:MPMoviePlayerPlaybackDidFinishNotification
      object:player];

    _video->pause();
}

- (void)handlePlaybackStateDidChange:(NSNotification*)notification
{
    MPMoviePlayerController *player = [notification object];
    if (player.playbackState != MPMoviePlaybackStatePlaying)
        _video->createSnapshot();
}

- (void)dealloc
{
    [super dealloc];
}
@end

namespace cefix {

IOSFullscreenVideo::IOSFullscreenVideo(const std::string& fileName)
:   Video(),
    VideoMediaAdapter<IOSFullscreenVideo>("ios_fullscreen_video"),
    _impl(NULL),
    _isActive(true),
    _isPlaying(false),
    _observer(NULL),
    _fullInited(false),
    _videoWidth(0),
    _videoHeight(0)
{
    setPixelBufferObject(NULL);
    setInternalTextureFormat(GL_RGBA);
    if (!fileName.empty()) {
        load(fileName);
    }
    allocateImage(10,10,1,GL_RGBA, GL_UNSIGNED_BYTE);
    osg::clearImageToColor(this, osg::Vec4(0,0,0,1));
}

IOSFullscreenVideo::~IOSFullscreenVideo()
{
    clear();
}

void IOSFullscreenVideo::clear()
{
    if (_observer) {
        [[NSNotificationCenter defaultCenter] 
            removeObserver: _observer
            name:MPMoviePlayerPlaybackStateDidChangeNotification
            object: _impl];
        [_observer release];
        _observer = NULL;
    }
    
    if (_impl) {
        [_impl release];
        _impl = NULL; 
    }
    
    _fullInited = false;
    
}

void IOSFullscreenVideo::play()
{
    // std::cout << "IOSNativeVideo::play" << std::endl;
    if (!_fullInited) {
        load(getFileName(), true);
        _fullInited = true;
    }
    if (_impl) {
		_lastTimeStamp = _t.tick();
        UIView* parent_win = cefix::DisplayCollection::instance()->getDisplay("main")->getWindow()->getNativeWindow();
        UIView* parent_view = cefix::DisplayCollection::instance()->getDisplay("main")->getWindow()->getNativeView();
        
        NSLog(@"win  frame: %@ bounds: %@", NSStringFromCGRect(parent_win.frame), NSStringFromCGRect(parent_win.bounds));
        NSLog(@"view frame: %@ bounds: %@", NSStringFromCGRect(parent_view.frame), NSStringFromCGRect(parent_view.bounds));
        
        
        _blocker = [[BlockerView alloc] initWithFrame: parent_view.bounds];
        _blocker.backgroundColor = [UIColor blackColor];
        [parent_view addSubview: _blocker];


        // CGRect r = {100, 100, 400, 300};
        _impl.view.frame = parent_view.bounds;
                
        [parent_view addSubview: _impl.view];
        
        [_impl setFullscreen: YES animated: NO];
        [_impl play];
        
        _isPlaying = true;
		setRunning(true);
        
    }
}

void IOSFullscreenVideo::pause()
{
    if (_impl && _isPlaying)
    {
        std::cout << "IOSNativeVideo::pause" << std::endl;
    
        [_impl pause];
        createSnapshot();
        
        UIView* parent_view = cefix::DisplayCollection::instance()->getDisplay("main")->getWindow()->getNativeView();
        parent_view.userInteractionEnabled = YES;
        
        [_impl.view removeFromSuperview];
        
        [_blocker removeFromSuperview];
        [_blocker release];
        
        clear();
    }
    
    _isPlaying = false;
	setRunning(false);
}

void IOSFullscreenVideo::quit()
{
    std::cout << "IOSNativeVideo::quit" << std::endl;
    pause();
}


void IOSFullscreenVideo::jumpTo(double t)
{
    std::cout << "IOSNativeVideo::jumpTo" << std::endl;
    //if (_impl) [_impl setVideoTime: t];
}


void IOSFullscreenVideo::setMovieRate(double r)
{
    std::cout << "IOSNativeVideo::setMovieRate" << std::endl;

}


double IOSFullscreenVideo::getMovieRate() const
{
	std::cout << "IOSNativeVideo::getMovieRate" << std::endl;
    return 1.0;
}


bool IOSFullscreenVideo::isPlaying() const
{
    return _isPlaying;
}

void IOSFullscreenVideo::idle() 
{
    if(_impl) {
        _currentTime = _impl.currentPlaybackTime;
        _videoWidth =_impl.naturalSize.width;
        _videoHeight = _impl.naturalSize.height;
    }
}

    
bool IOSFullscreenVideo::valid() const 
{ 
    return true; //(_impl != NULL); 
}
    
    
void IOSFullscreenVideo::load(const std::string& file_name, bool full_init)
{
    std::cout << "IOSNativeVideo::load " << file_name << std::endl;
    std::string file = osgDB::findDataFile(file_name);
    setFileName(file);
    
    if (full_init) {
        clear();
        
        _observer = [[MoviePlayerObserver alloc] init];
        _observer.video = this;


        NSURL* url = [NSURL fileURLWithPath: cefix::IOSUtils::toNSString(file)];
        
        _impl= [[MPMoviePlayerController alloc] initWithContentURL: url];
        
        [[NSNotificationCenter defaultCenter] addObserver:_observer
                       selector:@selector(handleExitFullscreen:)
                       name: MPMoviePlayerDidExitFullscreenNotification
                       object:_impl];
        [[NSNotificationCenter defaultCenter] addObserver:_observer
                       selector:@selector(handlePlaybackStateDidChange:)
                       name: MPMoviePlayerPlaybackStateDidChangeNotification
                       object:_impl];
        
        
        [_impl prepareToPlay];
        
        _impl.controlStyle = MPMovieControlStyleDefault;
        _impl.shouldAutoplay = YES;
        _impl.allowsAirPlay = YES;
        
        }
}



void IOSFullscreenVideo::createSnapshot(double time)
{
    double t = (time >= 0) ? time : _currentTime;
    if (_impl)
    {
        UIImage* img = [_impl thumbnailImageAtTime: t timeOption: MPMovieTimeOptionExact];
        cefix:IOSUtils::updateOsgImage(this, img);
        
        std::cout << "snapshot created: " << this->s() << " x " << this->t() << std::endl;
    }
}


    
void IOSFullscreenVideo::deactivate()
{
    std::cout << "IOSNativeVideo::deactivate" << std::endl;
    _wasPlaying = _isPlaying;
    pause();
    _isActive = false;
}


void IOSFullscreenVideo::activate()
{
    std::cout << "IOSNativeVideo::activate" << std::endl;
    _isActive = true;
    if (_wasPlaying) play();
}


bool IOSFullscreenVideo::isActive() const
{
    return _isActive;
}


    
void IOSFullscreenVideo::setVolume (float f)
{
}


float IOSFullscreenVideo::getVolume() const
{
    return 1.0f;
}


    
float IOSFullscreenVideo::getAudioBalance()
{
    return 0.0f;
}


    
void IOSFullscreenVideo::setAudioBalance(float b)
{
}

}

