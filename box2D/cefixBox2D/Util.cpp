/*
 *  Util.cpp
 *  die Physiker
 *
 *  Created by Stephan Huber on 27.09.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#include "Util.h"
#include <cefix/Serializer.h>
#include <cefix/EnumUtils.h>

namespace cefixb2d {


void scaleShapes(b2Body* body, float scale) {
    
    for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext()) 
    {
        b2Shape::Type shapeType = f->GetType();
        if ( shapeType == b2Shape::e_circle )
        {
            b2CircleShape* circleShape = (b2CircleShape*)f->GetShape();
            scaleShape(*circleShape, scale);
        }
        else if ( shapeType == b2Shape::e_polygon )
        {
            b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
            scaleShape(*polygonShape, scale);
        }
    }
}

template<> void scaleJoint(b2PrismaticJointDef* joint_def, float sf) 
{
	joint_def->localAnchorA.x *= sf;
	joint_def->localAnchorA.y *= sf;
	
	joint_def->localAnchorB.x *= sf;
	joint_def->localAnchorB.y *= sf;
	
	joint_def->lowerTranslation *= sf;
	joint_def->upperTranslation *= sf;
}



template<> void scaleJoint(b2LineJointDef* joint_def, float sf) 
{
	joint_def->localAnchorA.x *= sf;
	joint_def->localAnchorA.y *= sf;
	
	joint_def->localAnchorB.x *= sf;
	joint_def->localAnchorB.y *= sf;
	
	joint_def->lowerTranslation *= sf;
	joint_def->upperTranslation *= sf;
}

template<> void scaleJoint(b2DistanceJointDef* joint_def, float sf) 
{
	joint_def->localAnchorA.x *= sf;
	joint_def->localAnchorA.y *= sf;
	
	joint_def->localAnchorB.x *= sf;
	joint_def->localAnchorB.y *= sf;
	
	joint_def->length *= sf;
}

} // end of namespace


cefix::EnumAsString<b2BodyType, 3>& getBodyTypeEnumsAsString()
{
	static b2BodyType enums[3] = { b2_staticBody, b2_kinematicBody, b2_dynamicBody };
	static const char* enum_strs[3] = {"b2_staticBody", "b2_kinematicBody", "b2_dynamicBody" };
	static cefix::EnumAsString<b2BodyType, 3> enum_as_string(enums, enum_strs);
	
	return enum_as_string;
}

cefix::EnumAsString<b2Shape::Type, 4>& getShapeTypeEnumsAsString()
{
	static b2Shape::Type enums[4] = { b2Shape::e_unknown, b2Shape::e_circle, b2Shape::e_polygon, b2Shape::e_typeCount };
	static const char* enum_strs[4] = {"unknown", "circle", "polygon", "typeCount" };
	static cefix::EnumAsString<b2Shape::Type, 4> enum_as_string(enums, enum_strs);
	
	return enum_as_string;
}

void writeToSerializer(cefix::Serializer& serializer, b2Vec2& p)
{
    serializer << "x" << p.x;
    serializer << "y" << p.y;
}


void readFromSerializer(cefix::Serializer& serializer, b2Vec2& p)
{
    serializer >> "x" >> p.x;
    serializer >> "y" >> p.y;
}




void writeToSerializer(cefix::Serializer& serializer, b2BodyDef& body_def)
{
    serializer << "position"        << body_def.position;
	serializer << "angle"           << body_def.angle;
    serializer << "linear_velocity" << body_def.linearVelocity;
    serializer << "linear_damping"  << body_def.linearDamping;
    serializer << "angular_damping" << body_def.angularDamping;
    serializer << "allow_sleep"     << body_def.allowSleep;
    serializer << "awake"           << body_def.awake;
    serializer << "fixed_rotation"  << body_def.fixedRotation;
    serializer << "bullet"          << body_def.bullet;
    serializer << "type"            << getBodyTypeEnumsAsString().string(body_def.type);
    serializer << "active"          << body_def.active;
    serializer << "inertia_scale"   << body_def.inertiaScale;
}

void readFromSerializer(cefix::Serializer& serializer, b2BodyDef& body_def)
{
    serializer >> "position"        >> body_def.position;
	serializer >> "angle"           >> body_def.angle;
    serializer >> "linear_velocity" >> body_def.linearVelocity;
    serializer >> "linear_damping"  >> body_def.linearDamping;
    serializer >> "angular_damping" >> body_def.angularDamping;
    serializer >> "allow_sleep"     >> body_def.allowSleep;
    serializer >> "awake"           >> body_def.awake;
    serializer >> "fixed_rotation"  >> body_def.fixedRotation;
    serializer >> "bullet"          >> body_def.bullet;
    
    std::string type_str;
    serializer >> "type"            >> type_str;
    body_def.type = getBodyTypeEnumsAsString().value(type_str, b2_staticBody);
    
    serializer >> "active"          >> body_def.active;
    serializer >> "inertia_scale"   >> body_def.inertiaScale;

}

void writeToSerializer(cefix::Serializer& serializer, b2Filter& filter) 
{
    serializer << "category_bits"    << filter.categoryBits;
    serializer << "mask_bits"        << filter.maskBits;
    serializer << "group_index"      << filter.groupIndex;
}

void readFromSerializer(cefix::Serializer& serializer, b2Filter& filter) 
{
    serializer >> "category_bits"    >> filter.categoryBits;
    serializer >> "mask_bits"        >> filter.maskBits;
    serializer >> "group_index"      >> filter.groupIndex;
}

void writeToSerializer(cefix::Serializer& serializer, b2FixtureDef& fixture_def)
{
    serializer << "friction"                << fixture_def.friction;
    serializer << "restitution"             << fixture_def.restitution;
    serializer << "density"                 << fixture_def.density;
    serializer << "filter"                  << fixture_def.filter;
    serializer << "is_sensor"               << fixture_def.isSensor;
}

void readFromSerializer(cefix::Serializer& serializer, b2FixtureDef& fixture_def)
{
    serializer >> "friction"                >> fixture_def.friction;
    serializer >> "restitution"             >> fixture_def.restitution;
    serializer >> "density"                 >> fixture_def.density;
    serializer >> "filter"                  >> fixture_def.filter;
    serializer >> "is_sensor"               >> fixture_def.isSensor;
}

void writeToSerializer(cefix::Serializer& serializer, b2Shape& shape)
{
    serializer << "type"    << getShapeTypeEnumsAsString().string(shape.m_type);
    serializer << "radius"  << shape.m_radius;    
}

void readFromSerializer(cefix::Serializer& serializer, b2Shape& shape)
{
    std::string type_str;
    serializer >> "type"    >> type_str;
    serializer >> "radius"  >> shape.m_radius;
    
    shape.m_type = getShapeTypeEnumsAsString().value(type_str, b2Shape::e_unknown);
}

void writeToSerializer(cefix::Serializer& serializer, b2CircleShape& circle)
{
    writeToSerializer(serializer,static_cast<b2Shape&>(circle));
    
	serializer << "center" << circle.m_p;
    
}

void readFromSerializer(cefix::Serializer& serializer, b2CircleShape& circle)
{
    readFromSerializer(serializer,static_cast<b2Shape&>(circle));
    
	serializer >> "center" >> circle.m_p;
}

void writeToSerializer(cefix::Serializer& serializer, b2PolygonShape& polygon)
{
    writeToSerializer(serializer,static_cast<b2Shape&>(polygon));
    
	serializer << "centroid"		<< polygon.m_centroid;
	serializer << "vertex_count"	<< polygon.m_vertexCount;
	
	std::vector<b2Vec2> vertices(polygon.m_vertices, polygon.m_vertices + polygon.m_vertexCount);
	std::vector<b2Vec2> normals(polygon.m_normals, polygon.m_normals + polygon.m_vertexCount);
	
	serializer << "vertices"		<< vertices;
	serializer << "normals"			<< normals;
}

void readFromSerializer(cefix::Serializer& serializer, b2PolygonShape& polygon)
{
    readFromSerializer(serializer,static_cast<b2Shape&>(polygon));
    serializer >> "centroid"		>> polygon.m_centroid;
	serializer >> "vertex_count"	>> polygon.m_vertexCount;
	
	std::vector<b2Vec2> vertices;
	std::vector<b2Vec2> normals;
	
	serializer >> "vertices"		>> vertices;
	serializer >> "normals"			>> normals;
	
	for(int i = 0; i < polygon.m_vertexCount; ++i) {
		polygon.m_vertices[i] = vertices[i];
		polygon.m_normals[i]  = normals[i];
	}
}


void writeToSerializer(cefix::Serializer& serializer, b2JointDef& joint_def)
{
	serializer << "collide_connected" << joint_def.collideConnected;
}


void readFromSerializer(cefix::Serializer& serializer, b2JointDef& joint_def)
{
	serializer >> "collide_connected" >> joint_def.collideConnected;
}

void writeToSerializer(cefix::Serializer& serializer, b2DistanceJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"	<< joint_def.localAnchorA; 
	serializer << "local_anchor_b"  << joint_def.localAnchorB;
	serializer << "length"			<< joint_def.length;
	serializer << "frequency_hz"	<< joint_def.frequencyHz;
	serializer << "damping_ratio"	<< joint_def.dampingRatio;

}


void readFromSerializer(cefix::Serializer& serializer, b2DistanceJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"	>> joint_def.localAnchorA; 
	serializer >> "local_anchor_b"  >> joint_def.localAnchorB;
	serializer >> "length"			>> joint_def.length;
	serializer >> "frequency_hz"	>> joint_def.frequencyHz;
	serializer >> "damping_ratio"	>> joint_def.dampingRatio;
}


void writeToSerializer(cefix::Serializer& serializer, b2RevoluteJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "reference_angle"		<< joint_def.referenceAngle;
	serializer << "lower_angle"			<< joint_def.lowerAngle;
	serializer << "upper_angle"			<< joint_def.upperAngle;
	serializer << "max_motor_torque"	<< joint_def.maxMotorTorque;
	serializer << "motor_speed"			<< joint_def.motorSpeed;
	serializer << "enable_limit"		<< joint_def.enableLimit;
	serializer << "enable_motor"		<< joint_def.enableMotor;
}


void readFromSerializer(cefix::Serializer& serializer, b2RevoluteJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "reference_angle"		>> joint_def.referenceAngle;
	serializer >> "lower_angle"			>> joint_def.lowerAngle;
	serializer >> "upper_angle"			>> joint_def.upperAngle;
	serializer >> "max_motor_torque"	>> joint_def.maxMotorTorque;
	serializer >> "motor_speed"			>> joint_def.motorSpeed;
	serializer >> "enable_limit"		>> joint_def.enableLimit;
	serializer >> "enable_motor"		>> joint_def.enableMotor;

}


void writeToSerializer(cefix::Serializer& serializer, b2PrismaticJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "local_axis_1"		<< joint_def.localAxis1;
	serializer << "reference_angle"		<< joint_def.referenceAngle;
	serializer << "lower_translation"	<< joint_def.lowerTranslation;
	serializer << "upper_translation"	<< joint_def.upperTranslation;
	serializer << "max_motor_force"		<< joint_def.maxMotorForce;
	serializer << "motor_speed"			<< joint_def.motorSpeed;
	serializer << "enable_limit"		<< joint_def.enableLimit;
	serializer << "enable_motor"		<< joint_def.enableMotor;
}


void readFromSerializer(cefix::Serializer& serializer, b2PrismaticJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "local_axis_1"		>> joint_def.localAxis1;
	serializer >> "reference_angle"		>> joint_def.referenceAngle;
	serializer >> "lower_translation"	>> joint_def.lowerTranslation;
	serializer >> "upper_translation"	>> joint_def.upperTranslation;
	serializer >> "max_motor_force"		>> joint_def.maxMotorForce;
	serializer >> "motor_speed"			>> joint_def.motorSpeed;
	serializer >> "enable_limit"		>> joint_def.enableLimit;
	serializer >> "enable_motor"		>> joint_def.enableMotor;

}

void writeToSerializer(cefix::Serializer& serializer, b2LineJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "local_axis_a"		<< joint_def.localAxisA;
	serializer << "lower_translation"	<< joint_def.lowerTranslation;
	serializer << "upper_translation"	<< joint_def.upperTranslation;
	serializer << "max_motor_force"		<< joint_def.maxMotorForce;
	serializer << "motor_speed"			<< joint_def.motorSpeed;
	serializer << "enable_limit"		<< joint_def.enableLimit;
	serializer << "enable_motor"		<< joint_def.enableMotor;
}


void readFromSerializer(cefix::Serializer& serializer, b2LineJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "local_axis_a"		>> joint_def.localAxisA;
	serializer >> "lower_translation"	>> joint_def.lowerTranslation;
	serializer >> "upper_translation"	>> joint_def.upperTranslation;
	serializer >> "max_motor_force"		>> joint_def.maxMotorForce;
	serializer >> "motor_speed"			>> joint_def.motorSpeed;
	serializer >> "enable_limit"		>> joint_def.enableLimit;
	serializer >> "enable_motor"		>> joint_def.enableMotor;

}


void writeToSerializer(cefix::Serializer& serializer, b2PulleyJointDef& joint_def)
{
	/*groundAnchorA.Set(-1.0f, 1.0f);
		groundAnchorB.Set(1.0f, 1.0f);
		localAnchorA.Set(-1.0f, 0.0f);
		localAnchorB.Set(1.0f, 0.0f);
		lengthA = 0.0f;
		maxLengthA = 0.0f;
		lengthB = 0.0f;
		maxLengthB = 0.0f;
		ratio = 1.0f;
	*/
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "ground_anchor_a"		<< joint_def.groundAnchorA;
	serializer << "ground_anchor_b"		<< joint_def.groundAnchorB;
	serializer << "length_a"			<< joint_def.lengthA;
	serializer << "max_length_a"		<< joint_def.maxLengthA;
	serializer << "length_b"			<< joint_def.lengthB;
	serializer << "max_length_b"		<< joint_def.maxLengthB;
	serializer << "ratio"				<< joint_def.ratio;
}



void readFromSerializer(cefix::Serializer& serializer, b2PulleyJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "ground_anchor_a"		>> joint_def.groundAnchorA;
	serializer >> "ground_anchor_b"		>> joint_def.groundAnchorB;
	serializer >> "length_a"			>> joint_def.lengthA;
	serializer >> "max_length_a"		>> joint_def.maxLengthA;
	serializer >> "length_b"			>> joint_def.lengthB;
	serializer >> "max_length_b"		>> joint_def.maxLengthB;
	serializer >> "ratio"				>> joint_def.ratio;
}


void writeToSerializer(cefix::Serializer& serializer, b2WeldJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "reference_angle"		<< joint_def.referenceAngle;
}

void readFromSerializer(cefix::Serializer& serializer, b2WeldJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "reference_angle"		>> joint_def.referenceAngle;
}


void writeToSerializer(cefix::Serializer& serializer, b2FrictionJointDef& joint_def)
{
	writeToSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer << "local_anchor_a"		<< joint_def.localAnchorA;
	serializer << "local_anchor_b"		<< joint_def.localAnchorB;
	serializer << "max_torque"			<< joint_def.maxTorque;
	serializer << "max_force"			<< joint_def.maxForce;
}

void readFromSerializer(cefix::Serializer& serializer, b2FrictionJointDef& joint_def)
{
	readFromSerializer(serializer,static_cast<b2JointDef&>(joint_def));
	
	serializer >> "local_anchor_a"		>> joint_def.localAnchorA;
	serializer >> "local_anchor_b"		>> joint_def.localAnchorB;
	serializer >> "max_torque"			>> joint_def.maxTorque;
	serializer >> "max_force"			>> joint_def.maxForce;
}

