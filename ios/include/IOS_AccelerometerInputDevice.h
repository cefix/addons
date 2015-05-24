/*
 *  IOS_AcceleratorInputDevice.h
 *  cefix-for-iphone
 *
 *  Created by Stephan Huber on 18.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#ifndef IOS_ACCELERATORINPUT_DEVICE_HEADER
#define IOS_ACCELERATORINPUT_DEVICE_HEADER


#include <cefix/InputDevice.h>

#ifdef __OBJC__
@class IOS_AccelerometerInputDeviceImpl;
#else
class IOS_AccelerometerInputDeviceImpl;
#endif

namespace cefix {




class IOS_AccelerometerInputDevice : public cefix::InputDevice {

public:
	IOS_AccelerometerInputDevice(const std::string& identifier);
	
	static const std::string getIdentifier() { return "IOS_Accelerometer"; }
	
	virtual bool available();
	
	void newValue(const osg::Vec3d& vec);
	
protected:
	~IOS_AccelerometerInputDevice() { close(); }
	virtual bool openImplementation(cefix::PropertyList* pl);
	virtual void closeImplementation();
	
private: 
	osg::Vec3d _value;
	IOS_AccelerometerInputDeviceImpl* _impl;
	float _demping;

};


}


#endif