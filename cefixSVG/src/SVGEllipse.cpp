//
//  SVGEllipse.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGEllipse.h"
#include <osg/io_utils>

void svg::Ellipse::print(std::ostream& os)
{
    os << "ellipse " << getCenter()  << " rx: " << getRadiusX() << " ry: " << getRadiusY();
}

void svg::Circle::print(std::ostream& os)
{
    os << "circle " << getCenter()  << " r: " << getRadius();
}