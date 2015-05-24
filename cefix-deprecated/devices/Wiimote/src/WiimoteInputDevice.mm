/*
 *  SuddenMotionSensorInputDevice.cpp
 *  SuddenMotionSensor
 *
 *  Created by Stephan Huber on 30.09.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "WiimoteInputDevice.h"
#include <cefix/InputDevicemanager.h>
#include <cefix/PropertyList.h>
#include "WiiRemoteDiscovery.h"
#include <cefix/Timer.h>
#include <cefix/Log.h>




@interface WiiRemoteDiscoveryDelegate : NSObject
{
	cefix::WiimoteRegistry* _registry;
	WiiRemoteDiscovery* _discovery;
	NSAutoreleasePool* _pool;
}
- (id) initWithRegistry: (cefix::WiimoteRegistry*) reg;
- (void) WiiRemoteDiscovered:(WiiRemote*)wiimote;
- (void) WiiRemoteDiscoveryError:(int)code;
- (void) release;
@end

@implementation WiiRemoteDiscoveryDelegate

- (id) initWithRegistry: (cefix::WiimoteRegistry*) reg 
{
	_pool = [[NSAutoreleasePool alloc] init];
	_registry = reg;
	return [super init];
}

- (void) release
{
	[_discovery release];
	[_pool release];
}

- (void) WiiRemoteDiscovered:(WiiRemote*)wiimote 
{
	[wiimote retain];
	_registry->addWiimote(wiimote);
}


- (void) WiiRemoteDiscoveryError:(int)code
{
	_registry->deviceDiscoveryErrorOccured(code);
}


@end

@interface WiimoteDelegate : NSObject
{
	cefix::WiimoteInputDevice* _device;
}

-(id) initWithInputDevice: (cefix::WiimoteInputDevice*) device;
- (void) irPointMovedX:(float)px Y:(float)py wiiRemote:(WiiRemote*)wiiRemote;
- (void) rawIRData: (IRData[4])irData wiiRemote:(WiiRemote*)wiiRemote;
- (void) buttonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed wiiRemote:(WiiRemote*)wiiRemote;
- (void) accelerationChanged:(WiiAccelerationSensorType)type accX:(unsigned char)accX accY:(unsigned char)accY accZ:(unsigned char)accZ wiiRemote:(WiiRemote*)wiiRemote;
- (void) joyStickChanged:(WiiJoyStickType)type tiltX:(unsigned char)tiltX tiltY:(unsigned char)tiltY wiiRemote:(WiiRemote*)wiiRemote;
- (void) analogButtonChanged:(WiiButtonType)type amount:(unsigned)press wiiRemote:(WiiRemote*)wiiRemote;
- (void) wiiRemoteDisconnected:(IOBluetoothDevice*)device;


@end
@implementation WiimoteDelegate

-(id) initWithInputDevice: (cefix::WiimoteInputDevice*) device
{
	_device = device;
	
	return [super init];
}

- (void) irPointMovedX:(float)px Y:(float)py wiiRemote:(WiiRemote*)wiiRemote 
{

}


- (void) rawIRData: (IRData[4])irData wiiRemote:(WiiRemote*)wiiRemote
{
	for(unsigned int i = 0; i < 4; ++i) {
		//std::cout <<irData[i].x <<"/"<< irData[i].y <<"/"<< irData[i].s << std::endl;
		_device->setIr(i, irData[i].x, irData[i].y, irData[i].s);
	}
}


- (void) buttonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed wiiRemote:(WiiRemote*)wiiRemote
{
	switch(type) {
		case WiiRemoteAButton:
			_device->setAButton(isPressed);file://localhost/Users/stephan/Documents/cefix/privateapps/wiimote/XCode/build/Debug/cefixTemplate.app/
			break;
		case WiiRemoteBButton:
			_device->setBButton(isPressed);
			break;
		case WiiRemoteOneButton:
			_device->setOneButton(isPressed);
			break;
		case WiiRemoteTwoButton:
			_device->setTwoButton(isPressed);
			break;
		case WiiRemoteMinusButton:
			_device->setMinusButton(isPressed);
			break;
		case WiiRemoteHomeButton:
			_device->setHomeButton(isPressed);
			break;
		case WiiRemotePlusButton:
			_device->setPlusButton(isPressed);
			break;
		case WiiRemoteUpButton:
			_device->setUpButton(isPressed);
			break;
		case WiiRemoteDownButton:
			_device->setDownButton(isPressed);
			break;
		case WiiRemoteLeftButton:
			_device->setLeftButton(isPressed);
			break;
		case WiiRemoteRightButton:
			_device->setRightButton(isPressed);
			break;
			
		case WiiNunchukZButton:
		case WiiNunchukCButton:
		case WiiClassicControllerXButton:
		case WiiClassicControllerYButton:
		case WiiClassicControllerAButton:
		case WiiClassicControllerBButton:
		case WiiClassicControllerLButton:
		case WiiClassicControllerRButton:
		case WiiClassicControllerZLButton:
		case WiiClassicControllerZRButton:
		case WiiClassicControllerUpButton:
		case WiiClassicControllerDownButton:
		case WiiClassicControllerLeftButton:
		case WiiClassicControllerRightButton:
		case WiiClassicControllerMinusButton:
		case WiiClassicControllerHomeButton:
		case WiiClassicControllerPlusButton:
			osg::notify(osg::INFO) << "Button " << type << " currently not handled" << std::endl;
			break;
			
	
	}
}

- (void) accelerationChanged:(WiiAccelerationSensorType)type accX:(unsigned char)accX accY:(unsigned char)accY accZ:(unsigned char)accZ wiiRemote:(WiiRemote*)wiiRemote
{
	_device->setAcceleration(accX, accY, accZ);
}


- (void) joyStickChanged:(WiiJoyStickType)type tiltX:(unsigned char)tiltX tiltY:(unsigned char)tiltY wiiRemote:(WiiRemote*)wiiRemote
{

}


- (void) analogButtonChanged:(WiiButtonType)type amount:(unsigned)press wiiRemote:(WiiRemote*)wiiRemote
{

}

- (void) wiiRemoteDisconnected:(IOBluetoothDevice*)device 
{

}


@end




namespace cefix {


WiimoteRegistry* WiimoteRegistry::instance() 
{
	static osg::ref_ptr<WiimoteRegistry> s_ptr = new WiimoteRegistry();
	return s_ptr.get();
}


WiimoteRegistry::WiimoteRegistry() :osg::Referenced() 
{

	_discoveryDelegate = [[WiiRemoteDiscoveryDelegate alloc] initWithRegistry: this];
	_discovery = [[ WiiRemoteDiscovery alloc] init];
	[_discovery retain];
	[_discovery setDelegate: _discoveryDelegate];
	[_discovery start];
}


WiiRemote* WiimoteRegistry::getWiimote(unsigned int i) {
	if (i >= _wiis.size()) {
		return NULL;
	}
	
	WiiRemote* wii = _wiis[i];
	if ([wii available]) {
		log::info("WiimoteRegistry::getWiimote") <<"returning wiimote at ndx " << i << std::endl;
		return wii;
	}
	else {
		log::info("WiimoteRegistry::getWiimote") <<"removing wiimote at ndx " << i << std::endl;
		[wii release];
		_wiis.erase(_wiis.begin() + i);
	}
	return NULL;
}


void WiimoteRegistry::deviceDiscoveryErrorOccured(int code) 
{
	log::error("WiimoteRegistry::deviceDiscoveryErrorOccured") << "error while discovering devices: " << code << std::endl;
	[_discovery stop];
	
	cefix::Timer* t = cefix::FunctorTimer::create(100, this, &WiimoteRegistry::restartRecovery, 1);
	t->start();
}

void WiimoteRegistry::restartRecovery() 
{
	[_discovery start];
}

void WiimoteRegistry::addWiimote(WiiRemote* wii) 
{ 
	_wiis.push_back(wii); 
	log::info("WiimoteRegistry::addWiimote") <<"WiiRemotes discovered " << std::endl;
	
}




WiimoteInputDevice::WiimoteInputDevice(const std::string& identifier)
:	cefix::InputDevice(identifier),
	_wiimote(NULL),
	_deviceNdx(0),
	_aButton(0), 
	_bButton(0), 
	_minusButton(0), 
	_plusButton(0), 
	_homeButton(0),
	_oneButton(0), 
	_twoButton(0), 
	_leftButton(0), 
	_rightButton(0), 
	_upButton(0), 
	_downButton(0)
{
	setNeedsPolling(true);
	registerVec3Value(0, "acceleration", &_acceleration);
	registerIntValue(10, "AButton", &_aButton);
	registerIntValue(11, "BButton", &_bButton);
	registerIntValue(12, "OneButton", &_oneButton);
	registerIntValue(13, "TwoButton", &_twoButton);
	registerIntValue(14, "MinusButton", &_minusButton);
	registerIntValue(15, "HomeButton", &_homeButton);
	registerIntValue(16, "PlusButton", &_plusButton);
	registerIntValue(17, "UpButton", &_upButton);
	registerIntValue(18, "DownButton", &_downButton);
	registerIntValue(19, "LeftButton", &_leftButton);
	registerIntValue(20, "RightButton", &_rightButton);
	
	registerVec3Value(30, "ir_1", &(_ir[0]));
	registerVec3Value(31, "ir_2", &(_ir[1]));
	registerVec3Value(32, "ir_3", &(_ir[2]));
	registerVec3Value(33, "ir_4", &(_ir[3]));
	
	setUsedProperties("deviceIndex");
}




bool WiimoteInputDevice::available() {
	return true;
}

void WiimoteInputDevice::setWiiRemote(WiiRemote* wiimote) 
{ 
	_wiimote = wiimote; 
	if (wiimote) {
		[_wiimote setIRSensorEnabled: YES];
		[_wiimote setMotionSensorEnabled: YES];
		[_wiimote setDelegate: [[WiimoteDelegate alloc] initWithInputDevice: this]];
		
		bool l1(false), l2(false), l3(false), l4(false);
		int cnt = _deviceNdx + 1;
		l1 = (cnt & 1);
		l2 = (cnt & 2);
		l3 = (cnt & 4);
		l4 = (cnt & 8);
		
		setLEDs(l1, l2, l3, l4);
	}
}


void WiimoteInputDevice::poll()
{
	if (!_wiimote) 
	{
		setWiiRemote( WiimoteRegistry::instance()->getWiimote(_deviceNdx) );
		if (!_wiimote)
			return;
		else {
			log::info("WiimoteRegistry::poll") <<"got device with index " << _deviceNdx << std::endl;
		}
	}
	
	if ([_wiimote available] == NO) {
		log::error("WiimoteRegistry::poll") <<"lost connection to wiimote with deviceNdx " << _deviceNdx << std::endl;
		_wiimote = NULL;
	}
	
	if (getEventQueue())
		getEventQueue()->addEvent(new InputDeviceEventAdapter(this));
}


bool WiimoteInputDevice::openImplementation(cefix::PropertyList* pl) 
{
	if ((pl) && pl->hasKey("deviceIndex")) _deviceNdx = pl->get("deviceIndex")->asInt();
	return true;
}


void WiimoteInputDevice::closeImplementation()
{
	if (_wiimote)
		[_wiimote closeConnection];
}


WiimoteInputDevice::~WiimoteInputDevice() 
{ 
	close(); 
}

void WiimoteInputDevice::setLEDs(bool l1, bool l2, bool l3, bool l4)
{
	if (_wiimote) 
		[_wiimote setLEDEnabled1: l1 enabled2: l2 enabled3: l3 enabled4: l4];
}


}



class WiimoteInputDeviceRegistrar : public cefix::InputDeviceRegistrar<cefix::WiimoteInputDevice> {
public:
  WiimoteInputDeviceRegistrar() : cefix::InputDeviceRegistrar<cefix::WiimoteInputDevice>()
	{
		cefix::WiimoteRegistry::instance();
	}
};

static WiimoteInputDeviceRegistrar wii_reg;
