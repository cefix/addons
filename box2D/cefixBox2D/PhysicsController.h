/*
 *  PhysicsController.h
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */
 
#ifndef CEFIXB2D_PHYSICS_CONTROLLER_HEADER
#define CEFIXB2D_PHYSICS_CONTROLLER_HEADER

#include <osg/NodeCallback>
#include <cefix/AnimationBase.h>
#include <osg/MatrixTransform>

#include <Box2D/Box2D.h>

#include "DebugDrawer.h"
#include "Util.h"

namespace cefixb2d {

class MotionState : public osg::MatrixTransform {
public:
    MotionState(b2Body* body) : osg::MatrixTransform(), _body(body), _scale(1.0f) {}
    void setScale(float f) { _scale = f; }
    inline void update() 
    {
        const b2Vec2& pos = _body->GetPosition();
        float angle = _body->GetAngle();
        
        setMatrix(osg::Matrix::rotate(angle, osg::Z_AXIS) * osg::Matrix::translate(pos.x*_scale,pos.y*_scale, 0));
    }
    
    b2Body* getBody() { return _body; }
	float getAngle() { return _body->GetAngle(); }
	osg::Vec2 getPosition() const { return asVec2( _body->GetPosition()) * _scale; }
    void setPosition(const osg::Vec2& p) { _body->SetTransform(asB2Vec2(p / _scale), _body->GetAngle()); }
private:
    b2Body* _body;
    float _scale;
};


class PhysicsController : public osg::Referenced {
public:
    typedef std::vector<osg::ref_ptr<MotionState> > MotionStates;
	PhysicsController();
	
    void setScale(float f) { _scale = f; }
    float getScale() const { return _scale; }
    
    void setTimeStep(float ts) { _timeStep = ts; }
    float getTimeStep() const { return _timeStep; }
    
    void setVelocityIterations(unsigned int i) { _velocityIterations = i; }
    void setPositionIterations(unsigned int i) { _positionIterations = i; }
    void setIterations(unsigned int velocity, unsigned int position) { setVelocityIterations(velocity); setPositionIterations(position); }
    
	void update() 
	{
		_world->Step(_timeStep, _velocityIterations, _positionIterations);
		if (_debugDrawer.valid()) {
			_debugDrawer->clear();
			_world->DrawDebugData();
			_debugDrawer->finish();
		}
        
        for(MotionStates::iterator i = _motionStates.begin(); i != _motionStates.end(); ++i) {
            (*i)->update();
        }
	}
	
	b2World* getWorld() { return _world; }
	
	b2Body* createBody(const b2BodyDef* def) { return _world->CreateBody(def); }
    b2Joint* createJoint(const b2JointDef* def) { return _world->CreateJoint(def); }
    
    void destroyBody(b2Body* body) { _world->DestroyBody(body); }
    void destroyJoint(b2Joint* joint) { _world->DestroyJoint(joint); }
	
    MotionState* createMotionState(const b2BodyDef* def) { 
        MotionState* ms = new MotionState(_world->CreateBody(def));
        ms->setScale(getScale());
        addMotionState(ms);
        return ms;
    }
    
    
    
	osg::NodeCallback* createUpdateCallback();
	cefix::AnimationBase* createAnimation();
	
	osg::Node* getDebugDrawNode(unsigned int flags =  b2DebugDraw::e_shapeBit |  b2DebugDraw::e_jointBit | b2DebugDraw::e_aabbBit | b2DebugDraw::e_pairBit | b2DebugDraw::e_centerOfMassBit)
	{
		if (_debugDrawer.valid() == false) {
			_debugDrawer = new DebugDrawer();
			_debugDrawer->SetFlags(flags);
			_debugDrawer->setScale(1 / getScale());
			_world->SetDebugDraw(_debugDrawer);
		}
		
		return _debugDrawer->getNode();		
	}
	
	
	
	void setGravity(const osg::Vec3& gravity) { _world->SetGravity(asB2Vec2(gravity)); }
	void setGravity(const osg::Vec2& gravity) { _world->SetGravity(asB2Vec2(gravity)); }
    
    osg::Vec2 getGravity() const { b2Vec2 g = _world->GetGravity(); return asVec2(g); }
	
    void addMotionState(MotionState* ms) { 
        _motionStates.push_back(ms); 
        ms->setScale(1/ getScale());
    }
    
    void removeMotionState(MotionState* ms) { 
        MotionStates::iterator i = std::find(_motionStates.begin(), _motionStates.end(), ms);
        if (i != _motionStates.end())
            _motionStates.erase(i);
    }
    
    template <typename T> T screenToPhysics(const T& t) const { return t * getScale(); }
    template <typename T> T physicsToScreen(const T& t) const { return t / getScale(); }
    
protected:
	~PhysicsController();
private:
	b2World* _world;
	float _timeStep;
	unsigned int _velocityIterations, _positionIterations;
	osg::ref_ptr<DebugDrawer> _debugDrawer;
    MotionStates _motionStates;
    float _scale;

};

}

#endif