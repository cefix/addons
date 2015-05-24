//
//  SuddenMotionSensorInputDevice.cpp
//  cefixSketch
//
//  Created by Stephan Huber on 03.03.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SuddenMotionSensorInputDevice.h"
#include <osg/ValueObject>
extern "C" {
    #include "unimotion.h"
}

SuddenMotionInputDevice::SuddenMotionInputDevice()
    : osgGA::Device()
    , _type(unknown)
    , _value(0,0,0)
    , _numTries(0)
{
    setCapabilities(RECEIVE_EVENTS);
}

void SuddenMotionInputDevice::checkEvents()
{
    if ((_type == unknown) && (_numTries < 10)) {
        _type = detect_sms();
        _numTries++;
    }
    if (_type != unknown) {
        read_sms_real(_type, &_value[0], &_value[1], &_value[2]);
        
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter();
        event->setEventType(osgGA::GUIEventAdapter::USER);
        event->setUserValue("acceleration", _value);
        event->setName("sms");
        getEventQueue()->addEvent(event);
	}
}