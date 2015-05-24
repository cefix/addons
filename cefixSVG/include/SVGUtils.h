//
//  Utils.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"
#include <osg/Geometry>
#include <osg/Array>
#include <vector>

namespace svg {



namespace util {

class PointsVector {
public:
    
    typedef std::vector<osg::Vec2d> Points;
    typedef Points::iterator iterator;

    PointsVector(): _points() {}
    virtual ~PointsVector() {}
    
    void setPoints(const Points& points) { _points = points; }
    void addPoint(double x, double y) { addPoint(osg::Vec2d(x, y)); }
    void addPoint(const osg::Vec2d& point) { _points.push_back(point); }
    
    unsigned int getNumPoints() const { return _points.size(); }
    const osg::Vec2d& getPointAt(unsigned int ndx) const { return _points[ndx]; }
    
    Points& getPoints() { return _points; }
    const Points& getPoints() const { return _points; }
    
    iterator begin() { return _points.begin(); }
    iterator end() { return _points.end(); }

protected:
    Points _points;

};

class Geometry {
public:
    typedef osg::Vec2dArray Outline2D;
    typedef std::vector< osg::ref_ptr<Outline2D> > Outlines2D;
    
    typedef osg::Vec3Array Outline3D;
    typedef std::vector< osg::ref_ptr<Outline3D> > Outlines3D;
    
    /// creates a vector of vec2arrays consisting of all outlines of the node
    static void create2DOutlines(Node& node, Outlines2D& outlines, double max_error = 0.001);
    
    /// creates a vector of vec3arrays consisting of all outlines of the node
    static void create3DOutlines(Node& node, Outlines3D& outlines, double max_error = 0.001);
    
    /// creates a geometry with all outlines as polygon-primitivesets 
    static osg::Geometry* createOutlines(Node& node, double max_error = 0.001);
    
    static osg::Geometry* createStroke(Outlines2D& outlines, double stroke_width, double miter_limit, StateSet::StrokeLineJoin line_join, StateSet::StrokeLineCap line_cap, double max_error = 0.001);
    
    static osg::Geometry* createStroke(Node& node, double stroke_width, double miter_length, StateSet::StrokeLineJoin line_join, StateSet::StrokeLineCap line_cap, double max_error = 0.001);
    
    static Path* converToPath(Node& node);
private:
   
    Geometry() {}
};

}
}

