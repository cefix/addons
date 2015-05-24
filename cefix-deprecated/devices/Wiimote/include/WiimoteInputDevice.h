/*
 *  SuddenMotionSensorInputDevice.h
 *  SuddenMotionSensor
 *
 *  Created by Stephan Huber on 30.09.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef WIIMOTE_INPUT_DEVICE
#define WIIMOTE_INPUT_DEVICE

#include <cefix/InputDevice.h>
#include <osgGA/GUIEventAdapter>
#include <osg/Vec3>
#include <iostream>

#ifdef __OBJC__
@class WiiRemoteDiscoveryDelegate;
@class WiiRemote;
@class WiiRemoteDiscovery;
#else
class WiiRemoteDiscoveryDelegate;
class WiiRemote;
class WiiRemoteDiscovery;
#endif

namespace cefix {


class WiimoteRegistry : public osg::Referenced {
public:
	WiimoteRegistry();
	
	void addWiimote(WiiRemote* wii);	
	WiiRemote* getWiimote(unsigned int i);
	
	static WiimoteRegistry* instance();
	
	void deviceDiscoveryErrorOccured(int code);

	void restartRecovery();
private:
	std::vector<WiiRemote*> _wiis;
	WiiRemoteDiscoveryDelegate* _discoveryDelegate;
	WiiRemoteDiscovery* _discovery;

};

class WiimoteInputDevice : public cefix::InputDevice {
public:
	WiimoteInputDevice(const std::string& identifier);
	
	static const std::string getIdentifier() { return "Wiimote"; }
	
	virtual bool available();
	
	virtual void poll();
	
	void setLEDs(bool l1, bool l2, bool l3, bool l4);
	
	inline void setAcceleration(unsigned char accX, unsigned char accY, unsigned char accZ)
	{
		_acceleration.set( (accX-128) / 128.0, (accY-128) / 128.0, (accZ-128) / 128.0 );
	}
	
	inline void setAButton(bool down) { _aButton = down; }
	inline void setBButton(bool down) { _bButton = down; }
	inline void setMinusButton(bool down) { _minusButton = down; }
	inline void setPlusButton(bool down) { _plusButton = down; }
	inline void setHomeButton(bool down) { _homeButton = down; }
	inline void setOneButton(bool down) { _oneButton = down; }
	inline void setTwoButton(bool down) { _twoButton = down; }
	inline void setLeftButton(bool down) { _leftButton = down; }
	inline void setRightButton(bool down) { _rightButton = down; }
	inline void setUpButton(bool down) { _upButton = down; }
	inline void setDownButton(bool down) { _downButton = down; }
	
	void setIr(int ndx, int x, int y, int size) 
	{
		if (x != 0x3ff) 
			_ir[ndx].set(x / static_cast<double>(0x3ff), y / static_cast<double>(0x3ff), size / static_cast<double>(0xf));
		else
			_ir[ndx].set(-1, -1, 0);
	}
	
protected:
	void setWiiRemote(WiiRemote* wiimote); 


	~WiimoteInputDevice();
	virtual bool openImplementation(cefix::PropertyList* pl);
	virtual void closeImplementation();
	
private:
	osg::Vec3d _acceleration;
	WiiRemote*	_wiimote;
	unsigned int _deviceNdx;
	int _aButton, _bButton, _minusButton, _plusButton, _homeButton, _oneButton, _twoButton, _leftButton, _rightButton, _upButton, _downButton;
	osg::Vec3d _ir[4];
};

}
#endif