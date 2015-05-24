//
//  IOSAVFoundationVideo.h
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 25.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once



#ifdef __OBJC__

#else

#endif

#include <cefix/VideoMedia.h>
#include <cefix/Video.h>
#include <osg/Timer>

namespace cefix {

class IOSAVFoundationVideo : public Video, public VideoMediaAdapter<IOSAVFoundationVideo> {

public:
    IOSAVFoundationVideo(const std::string& fileName = "");
            
    /// Destructor
    ~IOSAVFoundationVideo();
        
    virtual Object* clone() const { return new IOSAVFoundationVideo(); }
    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const IOSAVFoundationVideo*>(obj) != NULL;
    }
    
    virtual const char* className() const { return "IOSAVFoundationVideo"; }

    /// Start or continue stream.
    virtual void play();
    
    /** @return true, if a movie is playing */
    
    bool isPlaying() const { return _isPlaying; }
    
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
    double 	getCurrentTime ();
    
    
    virtual void load(const std::string& filename);
    
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
    virtual bool isActive() const { return _isActive; }
    
    void idle();
    
    virtual bool valid() const { return _isValid; }
    
    virtual float getVideoWidth () { return _videoWidth > 0 ? _videoWidth : s(); }
    virtual float getVideoHeight () {return _videoHeight > 0 ? _videoHeight : t(); }
    
    void createSnapshot(double time = -1);
    
    virtual void applyLoopingMode();
private:
    class Data;
    
    void clear();
    
    float _videoDuration;
    bool _isValid, _isActive, _isPlaying, _wasPlaying;
	osg::Timer _t;
	osg::Timer_t _lastTimeStamp;
    
    unsigned int _videoWidth, _videoHeight;
    double _volume;
    
    Data* _data;
 
};

}
