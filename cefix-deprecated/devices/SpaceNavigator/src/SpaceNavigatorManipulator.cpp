/*
 *  SpaceNavigatorManipulator.cpp
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 14.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SpaceNavigatorManipulator.h"
#include "SpaceNavigatorInputDevice.h"
#include <osg/io_utils>

namespace cefix {


SpaceNavigatorManipulator::SpaceNavigatorManipulator(SpaceNavigatorInputDevice* device) 
:	osgGA::TrackballManipulator(), 
	_rotationalDemping(0.0001f), 
	_translationalDemping(0.0025f),
    _device(device),
	_mode(ROTATE_DISTANCE) 
{
	setAutoComputeHomePosition(true);
}



/*
osg::Matrixd SpaceNavigatorManipulator::getMatrix() const {
   return _matrix;
}

osg::Matrixd SpaceNavigatorManipulator::getInverseMatrix() const {
    osg::Matrix  m = osg::Matrix::inverse(_matrix);
    return m;
}

void SpaceNavigatorManipulator::setByMatrix(const osg::Matrixd& mat) {
	
	_homeEye = mat.getTrans();
	_rotation = mat.getRotate();
	_homeEye = _rotation.inverse() * _homeEye;
	//_rotation.inverse();
   _matrix = mat;
    // osg::notify(osg::ALWAYS) << "saved: rot: " << _rotation <<"  trans: " <<  _translation << std::endl;
}

*/

void SpaceNavigatorManipulator::update() {
	setByMatrix( osg::Matrix::translate(-_homeEye) * osg::Matrix::rotate(_rotation) *  getMatrix() );
	
}


bool SpaceNavigatorManipulator::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) {

	if (_device.valid() && (ea.getEventType() == osgGA::GUIEventAdapter::USER)) {
		osg::Vec3 rot = _device->getRotation() * _rotationalDemping;
		osg::Vec3 trans = _device->getTranslation() * _translationalDemping;
        
        

        trans[1] *= -1;
        if ((ea.getModKeyMask() & osgGA::GUIEventAdapter:: MODKEY_SHIFT) || (_mode == ROTATE_TRANSLATE)) {
            _center  += trans;
        } else {
            _distance += trans[2];
        }
        
        _rotation *= osg::Quat(rot[2], osg::X_AXIS, -rot[0], osg::Y_AXIS , rot[1], osg::Z_AXIS);
        
        //update();		// osg::notify(osg::ALWAYS) << "Xrot: " << _matrix.getRotate() <<"  trans: " <<  _matrix.getTrans() << std::endl;
        
        static int btn_1_id = _device->getKey("button_1");
        static int btn_2_id = _device->getKey("button_2");
        if (_device->getIntValue(btn_1_id)) {
            flushMouseEventStack();
            _thrown = false;
            home(ea,us);
        }
        if (_device->getIntValue(btn_2_id)) {
            if (_mode == ROTATE_TRANSLATE)
                _mode = ROTATE_DISTANCE;
            else
                _mode = ROTATE_TRANSLATE;
        }
	} 
	
    return osgGA::TrackballManipulator::handle(ea, us);
}



}
	
