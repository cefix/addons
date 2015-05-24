//
//  SVGLine.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGLine.h"
#include <osg/io_utils>


void svg::Line::print(std::ostream& os)
{
    os <<"line " << getStart() << " - " << getEnd();
}