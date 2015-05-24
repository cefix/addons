//
//  SuddenMotionSensorInputDevice.h
//  cefixSketch
//
//  Created by Stephan Huber on 03.03.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <osgGA/Device>

class SuddenMotionInputDevice : public osgGA::Device {
public:
    SuddenMotionInputDevice();
    
    virtual const char* className() { return "SuddenMotionSensorInputDevice"; }
    
    virtual void checkEvents();
    
    const osg::Vec3d& getValue() const { return _value; }

private:
    osg::Vec3d _value;
	int			_type;
    unsigned int _numTries;
};