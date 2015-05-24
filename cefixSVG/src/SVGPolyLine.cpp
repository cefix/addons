//
//  SVGPolyLine.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGPolyLine.h"
#include <osg/io_utils>


void svg::PolyLine::print(std::ostream& os)
{
    os <<"polyline ";
    for(unsigned int i = 0; i < getNumPoints(); ++i) {
        if (i > 0)
            os << ", ";
        os << getPointAt(i);
    }
}