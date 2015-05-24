//
//  SVGEllipse.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"

namespace svg {

class Ellipse : public Node {
public:
    Ellipse() : Node(), _center(), _rx(0), _ry(0) {}
    
    void set(double cx, double cy, double rx, double ry) {
        _center.set(cx, cy);
        _rx = rx; _ry = ry;
    }
    
    virtual Ellipse* asEllipse() { return this; }
    
    double getCenterX() const { return _center[0]; }
    double getCenterY() const { return _center[1]; }
    
    double getRadiusX() const { return _rx; }
    double getRadiusY() const { return _ry; }
    
    const osg::Vec2d& getCenter() const { return _center; }
    
    void setRadius(double r) { setRadius(r, r); }
    void setRadius(double rx, double ry) { _rx = rx; _ry = ry; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    
    bool isCircle() const { return _rx == _ry; }

    virtual void print(std::ostream& where);

    
protected:
    osg::Vec2d _center;
    double _rx, _ry;
};



class Circle : public Ellipse {
public:
    Circle() : Ellipse() {}
    
    void set(double cx, double cy, double r) {
        Ellipse::set(cx, cy, r, r);
    }
    
    double getRadius() const { return _rx; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }

    virtual void print(std::ostream& where);

};

}

