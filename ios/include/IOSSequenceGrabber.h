/*
 *  IOSSequenceGrabber.h
 *  DiePhysiker
 *
 *  Created by Stephan Huber on 14.10.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#ifndef IOS_SEQUENCE_GRABBER_HEADER
#define IOS_SEQUENCE_GRABBER_HEADER

#include <cefix/SequenceGrabber.h>

namespace cefix {
class IOSSequenceGrabber : public cefix::SequenceGrabber {
	struct Data;
public:
	IOSSequenceGrabber(const std::string &name, unsigned int w, unsigned int h, unsigned int rate);
	
	virtual void idle () {  }
	virtual void start ();
	virtual void stop ();
	virtual void showSettings ();
	
    static const char *getGrabberId() { return "ios"; }
    static void populateDeviceInfoList(SequenceGrabberDeviceInfoList &deviceList);

    bool isValid() { return _valid; }
    
protected:
	void init(const std::string& params, unsigned int w, unsigned int h, unsigned int rate);
	
	~IOSSequenceGrabber();
private:
	Data* _data;
    bool _valid;
};


}


#endif