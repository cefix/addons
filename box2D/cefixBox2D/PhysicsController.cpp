/*
 *  PhysicsController.cpp
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#include "PhysicsController.h"

namespace cefixb2d {

PhysicsController::PhysicsController() 
:	osg::Referenced(),
	_timeStep(1/30.0f),
	_velocityIterations(10),
	_positionIterations(10),
    _scale(1.0f)
{
	_world = new b2World(b2Vec2(0,-10.0f), false);
}


PhysicsController::~PhysicsController() 
{
	delete _world;
}


class PhysicsUpdateCallback : public osg::NodeCallback {
public:
    PhysicsUpdateCallback(PhysicsController* ctrl) : osg::NodeCallback(), _ctrl(ctrl) {}
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        _ctrl->update();
        traverse(node, nv);
    }
private:
    osg::ref_ptr<PhysicsController> _ctrl;
};


class PhysicsAnimation: public cefix::AnimationBase {
public:
    PhysicsAnimation(PhysicsController* ctrl) : cefix::AnimationBase(0), _ctrl(ctrl) {}
    virtual void animate(float elapsed) {
        _ctrl->update();
    }
private:
    osg::ref_ptr<PhysicsController> _ctrl;
};
	

osg::NodeCallback* PhysicsController::createUpdateCallback() {
    return new PhysicsUpdateCallback(this);
}


cefix::AnimationBase* PhysicsController::createAnimation() {
    return new PhysicsAnimation(this);
}

}