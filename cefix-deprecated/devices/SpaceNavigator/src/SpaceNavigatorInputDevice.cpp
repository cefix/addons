/*
 *  SpaceNavigatorInputDevice.cpp
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 13.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SpaceNavigatorInputDevice.h"
#include "3DconnexionClient/ConnexionClientAPI.h"
#include <cefix/InputDeviceManager.h>
#include <cefix/PropertyList.h>
#include <Carbon/Carbon.h>
#include "SpaceNavigatorManipulator.h"

static osg::ref_ptr<cefix::SpaceNavigatorInputDevice> s_space_navigator(NULL);

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


namespace cefix {


bool SpaceNavigatorInputDevice::available() 
{
	return (InstallConnexionHandlers != NULL);
}


bool SpaceNavigatorInputDevice::openImplementation(cefix::PropertyList* pl) 
{
	
    if (pl && pl->hasKey("eventMask")) setEventMask(pl->get("eventmask")->asInt());
    
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
	_clientid = RegisterConnexionClient(0, pname,  kConnexionClientModeTakeOver, kConnexionMaskAll);
	
	CFRelease(frontAppName);
	
	setEventMask(ALL_EVENTS);
	
	return true;
}

osgGA::CameraManipulator* SpaceNavigatorInputDevice::getOrCreateCameraManipulator() 
{
    if (!_cameraManipulator.valid()) _cameraManipulator = new SpaceNavigatorManipulator(this);
    return _cameraManipulator.get();
}

void SpaceNavigatorInputDevice::closeImplementation() 
{
	UnregisterConnexionClient(_clientid);
	CleanupConnexionHandlers();
	s_space_navigator = NULL;
}


void SpaceNavigatorInputDevice::setEventMask(unsigned int eventMask) {
	_eventMask = eventMask;
	/*
	if (available()) {
		SetConnexionClientMask(_clientid, _eventMask);
	}
	*/
}

unsigned int SpaceNavigatorInputDevice::getEventMask() {
	return _eventMask; 
}


cefix::InputDeviceRegistrar<SpaceNavigatorInputDevice> snid;


}