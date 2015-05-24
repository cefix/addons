//
//  SVGPolygon.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 04.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"
#include "SVGUtils.h"

namespace svg {

class Polygon : public Node, public util::PointsVector {
public:
        
    Polygon() : Node(), util::PointsVector() {}
    
    virtual Polygon* asPolygon() { return this; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }

    virtual void print(std::ostream& where);

};

}

