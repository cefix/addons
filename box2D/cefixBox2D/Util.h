/*
 *  Util.h
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#ifndef CEFIXB2D_UTIL_HEADER
#define CEFIXB2D_UTIL_HEADER


#include <Box2D/Box2D.h>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

namespace cefix {
class Serializer;
}

namespace cefixb2d {


inline osg::Vec2 asVec2(const b2Vec2& v) { return osg::Vec2(v.x, v.y); }
inline osg::Vec3 asVec3(const b2Vec2& v, float z = 0) { return osg::Vec3(v.x, v.y, z); }
inline osg::Vec4 asVec4(const b2Color& v) { return osg::Vec4(v.r, v.g, v.b, 1.0); }

inline b2Vec2 asB2Vec2(const osg::Vec2& v) { return b2Vec2(v.x(), v.y()); }
inline b2Vec2 asB2Vec2(const osg::Vec3& v) { return b2Vec2(v.x(), v.y()); }
inline b2Color asB2Color(const osg::Vec4& color) { return b2Color(color[0], color[1], color[2]); }


inline void scaleShape(b2CircleShape& t, float scale)
{
    t.m_radius *= scale;
    t.m_p *= scale;
}

inline void scaleShape(b2PolygonShape& t, float scale) 
{
    for(int i = 0; i < t.m_vertexCount; ++i) {
        t.m_vertices[i] *= scale;
    }
}

void scaleShapes(b2Body* body, float scale);

template<class T> void scaleJoint(T* joint_def, float sf) 
{
	joint_def->localAnchorA.x *= sf;
	joint_def->localAnchorA.y *= sf;
	
	joint_def->localAnchorB.x *= sf;
	joint_def->localAnchorB.y *= sf;
}


template<> void scaleJoint(b2PrismaticJointDef* joint_def, float sf);
template<> void scaleJoint(b2LineJointDef* joint_def, float sf);
template<> void scaleJoint(b2DistanceJointDef* joint_def, float sf);


} // end of namespace


void writeToSerializer(cefix::Serializer& serializer, b2Vec2& p);
void readFromSerializer(cefix::Serializer& serializer, b2Vec2& p);

void writeToSerializer(cefix::Serializer& serializer, b2BodyDef& body_def);
void readFromSerializer(cefix::Serializer& serializer, b2BodyDef& body_def);

void writeToSerializer(cefix::Serializer& serializer, b2FixtureDef& fixture_def);
void readFromSerializer(cefix::Serializer& serializer, b2FixtureDef& fixture_def);

void writeToSerializer(cefix::Serializer& serializer, b2CircleShape& circle);
void readFromSerializer(cefix::Serializer& serializer, b2CircleShape& circle);
void writeToSerializer(cefix::Serializer& serializer, b2PolygonShape& polygon);
void readFromSerializer(cefix::Serializer& serializer, b2PolygonShape& polygon);

void writeToSerializer(cefix::Serializer& serializer, b2DistanceJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2DistanceJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2RevoluteJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2RevoluteJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2PrismaticJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2PrismaticJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2LineJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2LineJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2PulleyJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2PulleyJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2WeldJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2WeldJointDef& joint_def);

void writeToSerializer(cefix::Serializer& serializer, b2FrictionJointDef& joint_def);
void readFromSerializer(cefix::Serializer& serializer, b2FrictionJointDef& joint_def);


#endif