/*
 *  SuddenMotionSensorInputDevice.cpp
 *  SuddenMotionSensor
 *
 *  Created by Stephan Huber on 30.09.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "SuddenMotionSensorInputDevice.h"
#include "unimotion.h"
#include <cefix/InputDevicemanager.h>

namespace cefix {

SuddenMotionSensorInputDevice::SuddenMotionSensorInputDevice(const std::string& identifier)
:	cefix::InputDevice(identifier),
	_value(0,0,0)
{
	setNeedsPolling(true);
	registerVec3Value(0, "acceleration", &_value);
}

bool SuddenMotionSensorInputDevice::available() {
	return (detect_sms() != 0);
}

void SuddenMotionSensorInputDevice::poll()
{
	if (_type == unknown) {
		return;
	}
	
	read_sms_real(_type, &_value[0], &_value[1], &_value[2]);
	if (getEventQueue())
		getEventQueue()->addEvent(new InputDeviceEventAdapter(this));
}

bool SuddenMotionSensorInputDevice::openImplementation(cefix::PropertyList* pl) 
{
	_type = detect_sms();
	return (_type != unknown);
}

void SuddenMotionSensorInputDevice::closeImplementation()
{
}

}

static cefix::InputDeviceRegistrar<cefix::SuddenMotionSensorInputDevice> sms_reg;
