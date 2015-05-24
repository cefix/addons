/*
 *  MouseBodyDraggable.h
 *  edp_physics_test_1
 *
 *  Created by Stephan Huber on 17.05.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#ifndef MOUSE_BODY_DRAGGABLE_HEADER
#define MOUSE_BODY_DRAGGABLE_HEADER

#include <cefix/Pickable.h>
#include <cefix/Timer.h>
#include "PhysicsController.h"

namespace cefixb2d {


class MouseBodyDraggable : public cefix::Pickable, public osg::Referenced {
public:
	MouseBodyDraggable(b2Body* body, cefixb2d::PhysicsController* ctrl) 
	:	cefix::Pickable(), 
		osg::Referenced(),
		_body(body), 
		_otherBody(NULL),
		_physics(ctrl),
		_joint(NULL) 
	{
	}
	
	
	virtual void down();
	virtual void up(bool outside);
	virtual void handle();
	
    
	b2Body *_body, *_otherBody;
	b2MouseJoint* _joint;
	cefixb2d::PhysicsController* _physics;
	osg::ref_ptr<cefix::Timer> _timer;
};

}

#endif