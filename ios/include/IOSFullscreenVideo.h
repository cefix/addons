//
//  IOSFullscreenVideo.h
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 25.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once



#ifdef __OBJC__
@class MPMoviePlayerController;
@class MoviePlayerObserver;
@class BlockerView;
#else
class MPMoviePlayerController;
class MoviePlayerObserver;
class BlockerView;
#endif

#include <cefix/VideoMedia.h>
#include <cefix/Video.h>
#include <osg/Timer>

namespace cefix {

class IOSFullscreenVideo : public Video, public VideoMediaAdapter<IOSFullscreenVideo> {

public:
    IOSFullscreenVideo(const std::string& fileName = "");
            
    /// Destructor
    ~IOSFullscreenVideo();
        
    virtual Object* clone() const { return new IOSFullscreenVideo(); }
    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const IOSFullscreenVideo*>(obj) != NULL;
    }   
    
    virtual const char* className() const { return "IOSFullscreenVideo"; }

    /// Start or continue stream.
    virtual void play();
    
    /** @return true, if a movie is playing */
    
    bool isPlaying() const;
    
    /// sets the movierate
    void setMovieRate(double rate);
    
    /// gets the movierate
    double getMovieRate() const;
    
    /// Pause stream at current position.
    virtual void pause();

    
    /// stop playing 
    virtual void quit();

    /// Get total length in seconds.
    virtual double getLength() const { return _videoDuration; }
    
    /// jumps to a specific position 
    virtual void jumpTo(double pos);        
    /// returns the current playing position
    
    
    /// loads a movie from fileName
    void load(const std::string& fileName, bool full_init);
    
    virtual void load(const std::string& filename) {
        load(filename, false);
    }
    
    /** @return the current volume as float */
    virtual float getVolume() const;
    
    /** sets the volume of this quicktime to v*/
    virtual void setVolume(float v);
    
    /** @return the current balance-setting (0 = neutral, -1 = left, 1 = right */
    virtual float getAudioBalance();	
    /** sets the current balance-setting (0 = neutral, -1 = left, 1 = right */
    virtual void setAudioBalance(float b);
            
    virtual void deactivate();
    virtual void activate();
    virtual bool isActive() const;
    
    void idle();
    
    virtual bool valid() const;
    
    virtual float getVideoWidth () { return _videoWidth > 0 ? _videoWidth : s(); }
    virtual float getVideoHeight () {return _videoHeight > 0 ? _videoHeight : t(); }
    
    void createSnapshot(double time = -1);
    
private:
    void clear();
    
    float _videoDuration;
    MPMoviePlayerController* _impl;
    bool _isActive, _isPlaying, _wasPlaying;
	osg::Timer _t;
	osg::Timer_t _lastTimeStamp;
	bool _fullInited;
    
    MoviePlayerObserver* _observer;
    BlockerView*     _blocker;
    
    float _videoWidth, _videoHeight;
 
};

}
