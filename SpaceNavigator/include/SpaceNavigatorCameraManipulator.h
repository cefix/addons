/*
 *  SpaceNavigatorManipulator.h
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 14.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SPACE_NAVIGATOR_MANIPULATOR_HEADER
#define SPACE_NAVIGATOR_MANIPULATOR_HEADER

#include <osgGA/TrackballManipulator>

class SpaceNavigatorCameraManipulator : public osgGA::TrackballManipulator {

	public:
		enum {
			ROTATE_TRANSLATE = 1,
            ROTATE_DISTANCE = 2
		} Mode;
		
		SpaceNavigatorCameraManipulator();
		
		virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us);
		
		void setRotationalDemping(float f) { _rotationalDemping = f; }
		void setTranslationalDemping(float f) { _translationalDemping = f; }
				
	private:
		void update();

		float _rotationalDemping, _translationalDemping;
		unsigned int	_mode;
		
};




#endif
