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

namespace cefix {
class SpaceNavigatorInputDevice;
class SpaceNavigatorManipulator : public osgGA::TrackballManipulator {

	public:
		enum {
			ROTATE_TRANSLATE = 1,
            ROTATE_DISTANCE = 2
		} Mode;
		
		SpaceNavigatorManipulator(SpaceNavigatorInputDevice* device);
		
		virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us);
		
		void setRotationalDemping(float f) { _rotationalDemping = f; }
		void setTranslationalDemping(float f) { _translationalDemping = f; }
		



		
	private:
		void update();

		float _rotationalDemping, _translationalDemping;
		osg::ref_ptr<SpaceNavigatorInputDevice>	_device;
		unsigned int	_mode;
		
};


} // end of namespace



#endif
