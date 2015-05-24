//
//  SVGColor.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 10.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <osg/Vec4>
#include <osg/Vec4d>
#include <ostream>

namespace svg {

class Color  {
public:
    Color() : _color(0,0,0,0), _hasColor(false) {}
    Color(const std::string& name);
    Color(int r, int g, int b, double a = 1.0) : _color(r/255.0, g/255.0, b/255.0, a), _hasColor(true) {}
    Color(float r, float g, float b, float a = 1) : _color(r,g,b,a), _hasColor(true) {}
    Color(const osg::Vec4d& c) : _color(c), _hasColor(true) {}
    Color(const osg::Vec4f& c) : _color(c), _hasColor(true) {}
    
    void set(int r, int g, int b, double a = 1.0) { _color.set(r/255.0, g/255.0, b/255.0, a); _hasColor = true; }
    
    bool hasColor() const { return _hasColor; }
    const osg::Vec4d get() const { return _color; }
    
private:
    osg::Vec4d _color;
    bool _hasColor;
};

}

std::ostream& operator<<(std::ostream& os, const svg::Color& color);