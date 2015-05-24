//
//  SVGDocument.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <cefix/MathUtils.h>

#include "SVGGroup.h"

namespace svg {

class Document : public Group {
public:
    
    virtual Document* asDocument() { return this; }
    
    
    void setDimensions(double x, double y, double w, double h) {
        _size.set(x, y, x+w, y+h);
    }
    double getLeft() const { return cefix::Rect::getLeft(_size); }
    double getTop() const { return cefix::Rect::getTop(_size); }
    double getWidth() const { return cefix::Rect::getWidth(_size); }
    double getHeight() const { return cefix::Rect::getHeight(_size); }
    const osg::Vec4d& getDimensions() const { return _size; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    
    virtual void print(std::ostream& where);

    
protected:
    osg::Vec4d _size;
};

}

