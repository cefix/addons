/*
 *  SpaceNavigatorInputDevice.cpp
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 13.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include <osg/ValueObject>
#include "SpaceNavigatorDevice.h"
#include "3DconnexionClient/ConnexionClientAPI.h"
#include <cefix/InputDeviceManager.h>
#include <cefix/PropertyList.h>
#include <Carbon/Carbon.h>
#include "SpaceNavigatorCameraManipulator.h"

static osg::ref_ptr<SpaceNavigatorDevice> s_space_navigator(NULL);

#ifdef __cplusplus
extern "C" {
#endif
    
/* Weak link 3Dx framework so this app can run without crashing even on 
 * systems that don't have the 3Dx driver installed. For more info on 
 * weak linking, see:
 * http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/WeakLinking.html 
 */
extern OSErr InstallConnexionHandlers(ConnexionMessageHandlerProc messageHandler, ConnexionAddedHandlerProc addedHandler, ConnexionRemovedHandlerProc removedHandler) __attribute__((weak_import));
void tdx_drv_handler (io_connect_t connection,  natural_t messageType,  void *messageArgument) {

	if (!s_space_navigator.valid()) return;
	
	ConnexionDeviceState  *state; 
  
	switch (messageType) 
	{ 
		case kConnexionMsgDeviceState: 
			state = (ConnexionDeviceState*)messageArgument; 
			if (state->client == s_space_navigator->getClientID()) 
			{ 
				// decipher what command/event is being reported by the driver 
				switch (state->command) 
				{ 
					case kConnexionCmdHandleAxis: 
						s_space_navigator->adaptAxis(osg::Vec3(state->axis[0],state->axis[1],state->axis[2]), osg::Vec3(state->axis[3],state->axis[4],state->axis[5]));
						// state->axis will contain values for the 6 axis 
						break; 

					case kConnexionCmdHandleButtons: 
						s_space_navigator->adaptButtons(state->buttons);
						// state->buttons reports the buttons that are pressed 
						break; 
				}                 
			} 
			break; 

		default: 
			// other messageTypes can happen and should be ignored 
			break; 
	} 

}

}




bool SpaceNavigatorDevice::init()
{
	
    OSStatus err;
	err = InstallConnexionHandlers(tdx_drv_handler, 0L, 0L);
	if (errorOccured(err, "InstallConnexionHandlers failed")) return false;
	
	ProcessSerialNumber psn;
	CFStringRef frontAppName;
	Str255   pname;
	err = GetCurrentProcess(&psn);
	err = CopyProcessName(&psn, &frontAppName);
	if (errorOccured(err, "CopyProcessName failed")) return false;
	
	s_space_navigator = this;
	CFStringGetPascalString(frontAppName, pname, 255, kCFStringEncodingASCII);
	_clientID = RegisterConnexionClient(0, pname,  kConnexionClientModeTakeOver, kConnexionMaskAll);
	
	CFRelease(frontAppName);
	
	
	return true;
}



void SpaceNavigatorDevice::close()
{
	UnregisterConnexionClient(_clientID);
	CleanupConnexionHandlers();
	s_space_navigator = NULL;
}


void  SpaceNavigatorDevice::addEvent() {
    osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter();
    ea->setEventType(osgGA::GUIEventAdapter::USER);
    ea->setName("spacenavigator");
    ea->setUserValue("translation", _translation);
    ea->setUserValue("rotation", _rotation);
    ea->setUserValue("button1", _button1);
    ea->setUserValue("button2", _button2);
    ea->setUserValue("button3", _button3);
    ea->setUserValue("button4", _button4);
    getEventQueue()->addEvent(ea);
}


bool SpaceNavigatorDevice::errorOccured(unsigned err_num, const std::string& msg) {
    if (err_num) {
        cefix::log::error("SpaceNavigatorDevice") << err_num << " " << msg << std::endl;
    }
    return (err_num != 0);
}
