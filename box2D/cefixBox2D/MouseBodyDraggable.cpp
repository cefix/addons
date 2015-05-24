/*
 *  MouseBodyDraggable.cpp
 *  edp_physics_test_1
 *
 *  Created by Stephan Huber on 17.05.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#include "MouseBodyDraggable.h"
#include <cefix/MouseLocationProvider.h>

namespace cefixb2d {

void MouseBodyDraggable::down()
{
	osg::Vec3 wp;
	wp[0] =  cefix::MouseLocationProvider::instance()->getNativeX();
	wp[1] =  cefix::MouseLocationProvider::instance()->getNativeY();
    wp = _physics->screenToPhysics(wp);
	
    b2Vec2 p;
	p.x = wp[0];
	p.y = wp[1];
	
	b2BodyDef body_def;
	
	_otherBody = _physics->createBody(&body_def);
		
	b2MouseJointDef md;
	md.bodyA = _otherBody;
	md.bodyB = _body;
	md.target = p;
	md.maxForce = 1000.0f * _body->GetMass();
	_joint = (b2MouseJoint*)_physics->createJoint(&md);
	_body->SetAwake(true);	

}
	
void MouseBodyDraggable::handle()
{
	if (_joint) 
	{
		osg::Vec3 wp;
		wp[0] =  cefix::MouseLocationProvider::instance()->getNativeX();
		wp[1] =  cefix::MouseLocationProvider::instance()->getNativeY();
		wp = _physics->screenToPhysics(wp);
        
        b2Vec2 p;
		p.x = wp[0];
		p.y = wp[1];
        
		_joint->SetTarget(p);
	}
}
	
void MouseBodyDraggable::up(bool outside)
{
	if (_joint) 
	{
		_physics->destroyJoint(_joint);
		_joint = NULL;
	}
	
	if (_otherBody) {
		_physics->destroyBody(_otherBody);
	}
}

}