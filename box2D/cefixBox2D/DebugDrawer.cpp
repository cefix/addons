/*
 *  DebugDrawerer.cpp
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#include "DebugDrawer.h"
#include <cefix/ColorUtils.h>
#include "Util.h"

namespace cefixb2d {

DebugDrawer::DebugDrawer()
:	b2DebugDraw(),
	osg::Referenced(),
    _scale(1.0f)
{
	_geo = new cefix::DebugGeometry();
}
void DebugDrawer::DrawPolygon (const b2Vec2 *vertices, int32 vertexCount, const b2Color &color)
{
	for(int i = 1; i < vertexCount; ++i) {
		_geo->addLine(asVec2(vertices[i-1]) * _scale, asVec2(vertices[i]) * _scale, asVec4(color));
	}
	_geo->addLine(asVec2(vertices[vertexCount-1]) * _scale, asVec2(vertices[0]) * _scale, asVec4(color));
}

void DebugDrawer::DrawSolidPolygon (const b2Vec2 *vertices, int32 vertexCount, const b2Color &color)
{
	DrawPolygon(vertices, vertexCount, color);
}

	
void DebugDrawer::DrawCircle (const b2Vec2 &center, float32 radius, const b2Color &color)
{
	_geo->addCircle(asVec2(center) * _scale, radius * _scale, asVec4(color));
}

	 
void DebugDrawer::DrawSolidCircle (const b2Vec2 &center, float32 radius, const b2Vec2 &axis, const b2Color &color)
{
	DrawCircle(center, radius, color);
	_geo->addNormal(asVec2(center) * _scale, asVec2(axis) * _scale, osg::Vec4(0,1,1,1));
}

	
void DebugDrawer::DrawSegment (const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color)
{
	_geo->addLine(asVec2(p1) * _scale, asVec2(p2) * _scale, asVec4(color));
}

	
void DebugDrawer::DrawTransform (const b2Transform &xf)
{
	
	b2Vec2 p1 = xf.position, p2;
	
	p2 = p1 + 0.4 * xf.R.col1;
	_geo->addLine(asVec2(p1) * _scale, asVec2(p2) * _scale, osg::Vec4(1,0,0,0.7));

	p2 = p1 + 0.4 * xf.R.col2;
	_geo->addLine(asVec2(p1) * _scale, asVec2(p2) * _scale, osg::Vec4(0,1,0,0.7));
}




}