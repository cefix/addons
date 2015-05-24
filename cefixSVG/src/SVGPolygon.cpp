//
//  SVGPolygon.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 04.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGPolygon.h"
#include <osg/io_utils>


void svg::Polygon::print(std::ostream& os)
{
    os <<"polygon ";
    for(unsigned int i = 0; i < getNumPoints(); ++i) {
        if (i > 0)
            os << ", ";
        os << getPointAt(i);
    }
}