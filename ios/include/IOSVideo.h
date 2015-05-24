/*
 *
 *      (\(\
 *     (='.')           cefix --
 *    o(_")")           a multipurpose library on top of OpenSceneGraph
 *  
 *
 *                      Copyright 2001-2011, stephanmaximilianhuber.com 
 *                      Stephan Maximilian Huber. 
 *
 *                      All rights reserved.
 *
 */
 
#ifndef IOS_VIDEO_HEADER
#define IOS_VIDEO_HEADER


#ifdef __OBJC__
@class AVFoundationVideoPlayer;
#else
class AVFoundationVideoPlayer;
#endif

#include <cefix/VideoMedia.h>
#include <cefix/Video.h>
#include <osg/Timer>

namespace cefix {

class IOSVideo : public Video, public VideoMediaAdapter<IOSVideo> {

public:
    IOSVideo(const std::string& fileName = "");
            
    /// Destructor
    ~IOSVideo();
        
    virtual Object* clone() const { return new IOSVideo(); }
    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const IOSVideo*>(obj) != NULL;
    }
    
    virtual const char* className() const { return "IOSVideo"; }

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
    virtual void load(const std::string& fileName);
    
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
private:
    bool copyCurrentFrame();
    
    float _videoDuration;
    AVFoundationVideoPlayer* _impl;
    bool _isActive, _isPlaying, _wasPlaying;
	osg::Timer _t;
	osg::Timer_t _lastTimeStamp;
	unsigned int _videoWidth, _videoHeight;
};

}


#endif