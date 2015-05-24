/*
 *  SpaceNavigatorInputDevice.h
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 13.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SPACE_NAVIGATOR_INPUT_DEVICE
#define SPACE_NAVIGATOR_INPUT_DEVICE


#include <cefix/InputDevice.h>
#include "SpaceNavigatorManipulator.h"
#include <osg/Vec3>

namespace cefix {


class SpaceNavigatorInputDevice : public InputDevice {

	public:
		
		enum Events { 
				BUTTON_1 = 0x0001,
				BUTTON_2 = 0x0002, 
				BUTTON_3 = 0x0004,
				BUTTON_4 = 0x0008,
				BUTTON_5 = 0x0010,
				BUTTON_6 = 0x0020,
				BUTTON_7 = 0x0040,
				BUTTON_8 = 0x0080,

				AXIS_TX = 0x0100,
				AXIS_TY = 0x0200,
				AXIS_TZ = 0x0400,
				AXIS_RX = 0x0800,
				AXIS_RY = 0x1000,
				AXIS_RZ = 0x2000,
				
				ALL_BUTTONS = 0x00FF, 
				
				AXIS_TRANSLATION = AXIS_TX | AXIS_TY | AXIS_TZ,
				AXIS_ROTATION = AXIS_RX | AXIS_RY | AXIS_RZ,
				ALL_AXIS = AXIS_TRANSLATION | AXIS_ROTATION,
				
				ALL_EVENTS = 0x3FFF
		};


		SpaceNavigatorInputDevice(const std::string& identifier) 
        :   InputDevice(identifier),
            _newEventAvailable(false)
        { 
            registerVec3Value(0, "translation", &_translation);
            registerVec3Value(1, "rotation", &_rotation);
            
            registerIntValue(10, "buttons", &_buttons);
            
            registerIntValue(11, "button_1", &_button1);
            registerIntValue(12, "button_2", &_button2);
            registerIntValue(13, "button_3", &_button3);
            registerIntValue(14, "button_4", &_button4);
            
            setNeedsPolling(true); 
            
            setUsedProperties("eventmask");
            
        }
		
		virtual bool available();
		
		unsigned int getClientID() { return _clientid; }
		
		inline void adaptAxis(const osg::Vec3& trans, const osg::Vec3& rot) { 
			_translation[0] = (_eventMask & AXIS_TX) ? trans[0] : 0;
			_translation[1] = (_eventMask & AXIS_TY) ? trans[1] : 0;
			_translation[2] = (_eventMask & AXIS_TZ) ? trans[2] : 0; 
			
			_rotation[0] = (_eventMask & AXIS_RX) ? rot[0] : 0;
			_rotation[1] = (_eventMask & AXIS_RY) ? rot[1] : 0;
			_rotation[2] = (_eventMask & AXIS_RZ) ? rot[2] : 0; 
			
			_newEventAvailable = true;
				
		}
		
		inline void adaptButtons(unsigned int buttons) { 
			_buttons = buttons;
			_button1 = _buttons & (1);
            _button2 = _buttons & (2);
            _button3 = _buttons & (4);
            _button4 = _buttons & (8);
            
            if (getEventQueue())
				getEventQueue()->addEvent(new InputDeviceEventAdapter(this));
			
			if (_lastButtons != _buttons)
				_lastButtons = _buttons;
		}
        
        virtual void poll() 
        {
            if (_newEventAvailable) {
                if (getEventQueue())
                    getEventQueue()->addEvent(new InputDeviceEventAdapter(this));
            }
            _newEventAvailable = false;
        }
            
        
		
		static const std::string getIdentifier() { return "SpaceNavigator"; }
		
		inline const osg::Vec3d& getTranslation() const { return _translation; }
		inline const osg::Vec3d& getRotation() const { return _rotation; }
		
		void setEventMask(unsigned int events);
		unsigned int getEventMask();
        
        virtual osgGA::CameraManipulator* getOrCreateCameraManipulator();
		
	protected:
	
		virtual bool openImplementation(cefix::PropertyList* pl);
		
		virtual void closeImplementation();
	
	private:
		unsigned int _clientid;
		osg::Vec3d _translation, _rotation;
		int		_buttons, _lastButtons;
        int        _button1, _button2, _button3, _button4;
		unsigned int		_eventMask;
        osg::ref_ptr<osgGA::CameraManipulator> _cameraManipulator;
        bool    _newEventAvailable;

};

}


#endif 