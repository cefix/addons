//
//  SVGLine.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"

namespace svg {

class Line : public Node {
public:
    
    Line() : Node(), _start(), _end() {}
    
    void set(double tx1, double ty1, double tx2, double ty2) { _start.set(tx1, ty1); _end.set(tx2, ty2); }
    
    const osg::Vec2d& getStart() const { return _start; }
    const osg::Vec2d& getEnd() const { return _end; }
    
    virtual Line* asLine() { return this; }
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }

    virtual void print(std::ostream& where);

    
private:
    osg::Vec2d _start, _end;

};

}

