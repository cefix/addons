//
//  Text2Speech.h
//  cefix-text-to-speech
//
//  Created by Stephan Huber on 30.01.12.
//  Copyright (c) 2012 Digital Mind. All rights reserved.
//

#pragma once

#include <osg/Referenced>
#include <string>

namespace cefix {

class Text2Speech : public osg::Referenced {

public:
    class Callback : public osg::Referenced {
    public:
        virtual void speakEnded(bool success)  = 0;
        virtual void willSpeakWord(const std::string& word) = 0;
    };
    
    class Impl {
    public:
        Impl(Text2Speech* parent) : _parent(parent) {}
        virtual ~Impl() {}
        
        virtual void setVolume(float f) = 0;
        virtual float getVolume() const = 0;
        
        virtual const std::string& getVoice() const = 0;
        virtual void setVoice(const std::string& voice_name) = 0;
        virtual void speak(const std::string& text, Callback* cb) = 0;
        virtual void setRate(float rate) = 0;
        virtual float getRate() const = 0;
        virtual void cancel() = 0;
        virtual bool isSpeaking() const = 0;
        
        virtual unsigned int getNumVoices() const = 0;
        virtual std::string getVoiceAt(unsigned int ndx) const = 0;
    
    private:
        Text2Speech* _parent;
    };
        
    Text2Speech(const std::string& voice = "");
    ~Text2Speech();
    
    void setVolume(float volume) { _impl->setVolume(volume); }
    float getVolume() const { return _impl->getVolume(); }
    
    void setRate(float rate) { _impl->setRate(rate); }
    float getRate() const { return _impl->getRate(); }
    
    void speak(const std::string& text, Callback* cb = NULL)
    {
        _impl->speak(text, cb ? cb : _defaultCb.get() );
    }
    
    bool isSpeaking() const { return _impl->isSpeaking(); }
    
    void cancel() { if (isSpeaking()) _impl->cancel(); }
    
    void setVoice(const std::string voice_name) { _impl->setVoice(voice_name); }
    const std::string& getVoice() const { return _impl->getVoice(); }
    
    unsigned int getNumVoices() const { return _impl->getNumVoices(); }
    std::string getVoiceAt(unsigned int ndx) const { return _impl->getVoiceAt(ndx); }
    
    void setCallback(Callback* cb) { _defaultCb = cb; }
private:
    Impl* _impl;
    osg::ref_ptr<Callback> _defaultCb;
};

}
