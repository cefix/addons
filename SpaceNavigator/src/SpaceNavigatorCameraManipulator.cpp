/*
 *  SpaceNavigatorManipulator.cpp
 *  cefixTemplate
 *
 *  Created by Stephan Huber on 14.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SpaceNavigatorCameraManipulator.h"
#include <osg/io_utils>

#include <osg/ValueObject>



SpaceNavigatorCameraManipulator::SpaceNavigatorCameraManipulator() 
:	osgGA::TrackballManipulator(), 
	_rotationalDemping(0.0001f), 
	_translationalDemping(0.0025f),
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

void SpaceNavigatorCameraManipulator::update() {
	setByMatrix( osg::Matrix::translate(-_homeEye) * osg::Matrix::rotate(_rotation) *  getMatrix() );
	
}


bool SpaceNavigatorCameraManipulator::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us) {

	if ((ea.getEventType() == osgGA::GUIEventAdapter::USER) && ea.getName() == "spacenavigator") {
        osg::Vec3d rot, trans;
        ea.getUserValue("rotation", rot);
        ea.getUserValue("translation", trans);
        
        rot *= _rotationalDemping;
		trans *= _translationalDemping;
        
        

        trans[1] *= -1;
        if ((ea.getModKeyMask() & osgGA::GUIEventAdapter:: MODKEY_SHIFT) || (_mode == ROTATE_TRANSLATE)) {
            _center  += trans;
        } else {
            _distance += trans[2];
        }
        
        _rotation *= osg::Quat(rot[2], osg::X_AXIS, -rot[0], osg::Y_AXIS , rot[1], osg::Z_AXIS);
        
	} 
	
    return osgGA::TrackballManipulator::handle(ea, us);
}



