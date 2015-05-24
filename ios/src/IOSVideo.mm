/*
 *  IOSMovie.cpp
 *  cefix
 *
 *  Created by Stephan Huber on 02.05.06.
 *  Copyright 2006 digitalmind.de // Stephan Maximilian Huber. All rights reserved.
 *
 */
#include "IOSVideo.h" 
#import "AVFoundationVideoPlayer.h"
#include <osgDB/FileUtils>
#include <cefix/Pixel.h>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>

namespace cefix {

IOSVideo::IOSVideo(const std::string& fileName)
:   Video(),
    VideoMediaAdapter<IOSVideo>("ios_video"),
    _impl(NULL),
    _isActive(true),
    _isPlaying(false)
{
    setPixelBufferObject(NULL);
    setInternalTextureFormat(GL_RGBA);
    if (!fileName.empty()) {
        load(fileName);
    }
}

IOSVideo::~IOSVideo() 
{
    std::cout << "~IOSVideo" << std::endl;
    
    if (_impl) {
        [_impl release];
        _impl = NULL; 
    }
}

void IOSVideo::play()
{
    if (_impl) {
		_lastTimeStamp = _t.tick();
        [_impl play];
        _isPlaying = true;
		setRunning(true);
    }
}

void IOSVideo::pause()
{
    if (_impl)
        [_impl pause];
    
    _isPlaying = false;
	setRunning(false);
}

void IOSVideo::quit()
{
    pause();
}


void IOSVideo::jumpTo(double t)
{
	if (_impl) [_impl setVideoTime: t];
}


void IOSVideo::setMovieRate(double r)
{

}


double IOSVideo::getMovieRate() const
{
	return 1.0;
}


bool IOSVideo::isPlaying() const
{
    return _isPlaying;
}

void IOSVideo::idle() 
{
	if (_impl) {
		if(_videoDuration == 0.0) {
			_videoDuration = [_impl getDuration];
			if (_videoDuration > 0 && !_isPlaying) {
				setRunning(false);
			}
		}
		
		if(_isPlaying) {
			osg::Timer_t t = _t.tick();
			[_impl updateWithElapsedTime: _t.delta_s(_lastTimeStamp, t)];
			setCurrentTime( [_impl getVideoTime]);
			_lastTimeStamp = t;
            if ([_impl hasNewFrame] && copyCurrentFrame()) {
                traverseCallbacks(this);
            }
			
		} 
	}
}

    
bool IOSVideo::valid() const 
{ 
    return true; //(_impl != NULL); 
}
    
    
void IOSVideo::load(const std::string& file_name)
{
    std::string file = osgDB::findDataFile(file_name);
    setFileName(file);
    if (!_impl) {
        _impl = [[AVFoundationVideoPlayer alloc] initWithPath: [NSString stringWithUTF8String: file.c_str()]];
		_videoDuration = [_impl getDuration];
    }
	setRunning(true);
    
}


    
void IOSVideo::deactivate()
{
    _wasPlaying = _isPlaying;
    pause();
    _isActive = false;
}


void IOSVideo::activate()
{
    _isActive = true;
    if (_wasPlaying) play();
}


bool IOSVideo::isActive() const
{
    return _isActive;
}


    
void IOSVideo::setVolume (float f)
{
	if (_impl) [_impl setVolume: f];
}


float IOSVideo::getVolume() const
{
    float vol =  1.0f; 
	if (_impl) vol = [_impl getVolume];
	return vol;
}


    
float IOSVideo::getAudioBalance()
{
    float pan =  0.0f; 
	if (_impl) pan = [_impl getPan];
	return pan;
}


    
void IOSVideo::setAudioBalance(float b)
{
    if (_impl) [_impl setPan: b];
}

bool IOSVideo::copyCurrentFrame() 
{
    bool success = false;
    CVImageBufferRef imageBuffer = [_impl getCurrentFrame]; 
    CVPixelBufferLockBaseAddress(imageBuffer,0); 
    
    uint8_t *bufferPixels = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer); 
    unsigned int w = _videoWidth = CVPixelBufferGetWidth(imageBuffer);
    unsigned int h = _videoHeight= CVPixelBufferGetHeight(imageBuffer);
	unsigned int bytes_per_row = CVPixelBufferGetBytesPerRow(imageBuffer);
	unsigned int real_w = bytes_per_row / 4;
    
    //std::cout << w << " x " << h << " bpr: " << CVPixelBufferGetBytesPerRow(imageBuffer) << std::endl;
    
    if ((real_w != s()) || (h != t()) || !data()) 
    {
        allocateImage(real_w, h, 1, GL_BGRA, GL_UNSIGNED_BYTE);
    }
    if (bufferPixels && data()) {
        size_t s = getTotalSizeInBytes();
        memcpy(data(), bufferPixels, s);
        dirty();
        success = true;
    }
    CVPixelBufferUnlockBaseAddress(imageBuffer,0);
    
    return success;
}

}


