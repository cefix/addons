//
//  Text2Speech.cpp
//  cefix-text-to-speech
//
//  Created by Stephan Huber on 30.01.12.
//  Copyright (c) 2012 Digital Mind. All rights reserved.
//

#include "Text2Speech.h"
#include <cefix/Log.h>


#pragma mark -

class DummyText2SpeechImpl : public cefix::Text2Speech::Impl {
public:
    DummyText2SpeechImpl(cefix::Text2Speech* parent) : cefix::Text2Speech::Impl(parent) {}
    virtual ~DummyText2SpeechImpl() {}
    
    virtual void setVolume(float f) {};
    virtual float getVolume() const { return 0; }
    
    virtual const std::string& getVoice() { return _voice; }
    virtual void setVoice(const std::string& voice_name) { _voice = voice_name; }
    virtual void setRate(float rate) {}
    virtual float getRate() const { return 0;}
    virtual void speak(const std::string& text, cefix::Text2Speech::Callback* cb) {}
    virtual void cancel() {}
    virtual bool isSpeaking() const { return false; }
    
    unsigned int getNumVoices() const { return 0; }
    std::string getVoiceAt(unsigned int ndx) const { return _voice; }
    
private:
    std::string _voice;
};

#pragma mark -

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>

static std::string toString(NSString* str) 
{
    return str ? std::string([str UTF8String]) : "";
}

static NSString* toNSString(const std::string& str) {
    return [NSString stringWithUTF8String: str.c_str()];
}


class CocoaText2SpeechImpl;
@interface CocoaText2SpeechImplDelegate : NSObject<NSSpeechSynthesizerDelegate> {
    
    CocoaText2SpeechImpl* _parent;
    cefix::Text2Speech::Callback* _cb;
}

- (id) initWithParent: (CocoaText2SpeechImpl*) parent;
- (void) setCallback: (cefix::Text2Speech::Callback*)cb;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)wordToSpeak ofString:(NSString *)text;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)success;
@end


class CocoaText2SpeechImpl : public cefix::Text2Speech::Impl {
public:
    CocoaText2SpeechImpl(cefix::Text2Speech* parent) : cefix::Text2Speech::Impl(parent)
    {
        _synth = [[NSSpeechSynthesizer alloc] initWithVoice: NULL];
        _delegate = [[CocoaText2SpeechImplDelegate alloc] initWithParent: this];
        
        [_synth setDelegate: _delegate];
    }
    
    
    ~CocoaText2SpeechImpl() 
    {
        [_synth release];
        [_delegate release];
    }
    
    virtual void setVolume(float f) { [_synth setVolume: f];}
    virtual float getVolume() const { return [_synth volume]; }
    
    virtual const std::string& getVoice() const
    { 
        _voice = toString([_synth voice]);
        return _voice; 
    }
    
    virtual void setVoice(const std::string& voice_name) 
    { 
        _voice = voice_name; 
        if (!voice_name.empty())
            [_synth setVoice: toNSString(_voice)];
    }
    
    virtual void setRate(float rate) 
    {
        [_synth setRate: rate];
    }
    
    virtual float getRate() const 
    { 
        return [_synth rate]; 
    }
    
    virtual void speak(const std::string& text, cefix::Text2Speech::Callback* cb) 
    {
        _cb = cb;
        [_delegate setCallback: cb];
        [_synth startSpeakingString: toNSString(text)];
    }
    
    virtual bool isSpeaking() const 
    {
        return [_synth isSpeaking];
    }
    
    virtual void cancel() {
    
    }
    
    unsigned int getNumVoices() const 
    {
        return [[NSSpeechSynthesizer availableVoices] count];
    }
    
    
    std::string getVoiceAt(unsigned int ndx) const
    {
        NSString* voice_name = [[NSSpeechSynthesizer availableVoices] objectAtIndex: ndx];
        return (voice_name) ? toString(voice_name) : "";
    }

    
private:
    mutable std::string _voice;
    NSSpeechSynthesizer* _synth;
    CocoaText2SpeechImplDelegate* _delegate;
    osg::ref_ptr<cefix::Text2Speech::Callback> _cb;
};


@implementation CocoaText2SpeechImplDelegate

-(id) initWithParent: (CocoaText2SpeechImpl*) parent
{
    self = [super init];
    if (self) {
        _parent = parent;
    }
    
    return self;
}


- (void) setCallback: (cefix::Text2Speech::Callback*)cb
{
    _cb = cb;
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)wordToSpeak ofString:(NSString *)text
{
    @try {
        if (_cb) _cb->willSpeakWord(toString([text substringWithRange:wordToSpeak]));
    }
    @catch (NSException* e) {
        cefix::log::error("Text2Speech") << "exception while handling willSpeakWord: " << toString([e reason]) << std::endl;
    }
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)success
{
    if(_cb) _cb->speakEnded(success);
}


@end

#endif

#pragma mark - 

namespace cefix {


Text2Speech::Text2Speech(const std::string& voice)
{
    #ifdef __APPLE__
        _impl = new CocoaText2SpeechImpl(this);
    #else
        _impl = DummyText2SpeechImpl(this);
    #endif
    
    _impl->setVoice(voice);
 }
 
Text2Speech::~Text2Speech()
{
    delete _impl;
}


}