/*
 *  DebugDrawer.h
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#ifndef CEFIX_B2_DEBUG_DRAWER_HEADER
#define CEFIX_B2_DEBUG_DRAWER_HEADER

#include <Box2D/Box2D.h>
#include <cefix/DebugGeometryFactory.h>

namespace cefixb2d {

class DebugDrawer: public b2DebugDraw, public osg::Referenced {
public:
	DebugDrawer();
	virtual void DrawPolygon (const b2Vec2 *vertices, int32 vertexCount, const b2Color &color);
 	
	virtual void DrawSolidPolygon (const b2Vec2 *vertices, int32 vertexCount, const b2Color &color);
		
	virtual void DrawCircle (const b2Vec2 &center, float32 radius, const b2Color &color);
		 
	virtual void DrawSolidCircle (const b2Vec2 &center, float32 radius, const b2Vec2 &axis, const b2Color &color);
		
	virtual void DrawSegment (const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color);
		
	virtual void DrawTransform (const b2Transform &xf);
	
	osg::Node* getNode() { return _geo->getOrCreateNode(); }
	
	void clear() { _geo->clear(); }
	void finish() { _geo->finish(); }
    
    void setScale(float scale) { _scale = scale; }
	
private:
	osg::ref_ptr<cefix::DebugOutputBase> _geo;
    float _scale;
};

}

#endif