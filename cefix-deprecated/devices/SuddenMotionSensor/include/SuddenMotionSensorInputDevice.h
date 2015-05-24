/*
 *  SuddenMotionSensorInputDevice.h
 *  SuddenMotionSensor
 *
 *  Created by Stephan Huber on 30.09.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef SUDDEN_MOTION_SENSOR_INPUT_DEVICE
#define SUDDEN_MOTION_SENSOR_INPUT_DEVICE

#include <cefix/InputDevice.h>
#include <osgGA/GUIEventAdapter>
#include <osg/Vec3>

namespace cefix {


class SuddenMotionSensorInputDevice : public cefix::InputDevice {
public:
	SuddenMotionSensorInputDevice(const std::string& identifier);
	
	static const std::string getIdentifier() { return "SuddenMotionSensor"; }
	
	virtual bool available();
	
	virtual void poll();
	
protected:
	~SuddenMotionSensorInputDevice() { close(); }
	virtual bool openImplementation(cefix::PropertyList* pl);
	virtual void closeImplementation();
	
private:
	osg::Vec3d _value;
	int			_type;
	
};

}
#endif