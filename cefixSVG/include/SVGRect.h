//
//  SVGRect.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "SVGNode.h"
#include <cefix/MathUtils.h>

namespace svg {

class Rect : public Node {
public:
    Rect() : Node(), _rect(0,0,0,0), _cornerRadius(0,0) {}
    
    void set(double left, double top, double width, double height) {
        _rect.set(left, top, left+width, top+height);
    }
    
    void setCornerRadius(double x, double y) {
        _cornerRadius.set((x != 0) ? x : y, (y != 0) ? y : x);
        _cornerRadius[0] = osg::clampTo<double>(_cornerRadius[0], 0, getWidth() / 2.0);
        _cornerRadius[1] = osg::clampTo<double>(_cornerRadius[1], 0, getHeight() / 2.0);
        
    }
    
    double getLeft() const { return cefix::Rect::getLeft(_rect); }
    double getTop() const { return cefix::Rect::getTop(_rect); }
    double getWidth() const { return cefix::Rect::getWidth(_rect); }
    double getHeight() const { return cefix::Rect::getHeight(_rect); }
    
    const osg::Vec2d& getCornerRadius() const { return _cornerRadius; }
    double getCornerRadiusX() const { return _cornerRadius.x(); }
    double getCornerRadiusY() const { return _cornerRadius.y(); }
    
    
    virtual Rect* asRect() { return this; }
    const osg::Vec4d& get() const { return _rect;  }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }

    virtual void print(std::ostream& where);


protected:
    osg::Vec4d _rect;
    osg::Vec2d _cornerRadius;
};

}
