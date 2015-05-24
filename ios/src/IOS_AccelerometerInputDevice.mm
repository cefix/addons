/*
 *  IOS_AcceleratorInputDevice.cpp
 *  cefix-for-iphone
 *
 *  Created by Stephan Huber on 18.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */
 

#include "IOS_AccelerometerInputDevice.h"
#include <cefix/MathUtils.h>
#include <cefix/InputDeviceManager.h>
#import <UIKit/UIKit.h>

@interface IOS_AccelerometerInputDeviceImpl : NSObject <UIAccelerometerDelegate>
{
	cefix::IOS_AccelerometerInputDevice* _device;
}

- (id) initWith: (cefix::IOS_AccelerometerInputDevice*)device;
- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration;

@end

@implementation IOS_AccelerometerInputDeviceImpl

- (id) initWith: (cefix::IOS_AccelerometerInputDevice*)device 
{
	id result = [super init];
	if (result) {
		_device = device;
		
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / 30.0)];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];

	}
	
	return result;
}

- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
	_device->newValue( osg::Vec3d(acceleration.x,  acceleration.y,  acceleration.z) );
}

@end


namespace cefix {


IOS_AccelerometerInputDevice::IOS_AccelerometerInputDevice(const std::string& identifier)
:	InputDevice(identifier),
	_value(0,0,0),
	_demping(0.1)
{
	setNeedsPolling(false);
	registerVec3Value(0, "acceleration", &_value);
}


bool IOS_AccelerometerInputDevice::available() {
	return true;
}


void IOS_AccelerometerInputDevice::newValue(const osg::Vec3d& value)
{
	_value = cefix::interpolate(_value, value, (1-_demping));
	if (getEventQueue())
		getEventQueue()->addEvent(new InputDeviceEventAdapter(this));
}


bool IOS_AccelerometerInputDevice::openImplementation(cefix::PropertyList* pl) 
{
	_impl = [[IOS_AccelerometerInputDeviceImpl alloc] initWith: this];
	return true;
}


void IOS_AccelerometerInputDevice::closeImplementation()
{
	[_impl release];
	_impl = NULL;
}

}

