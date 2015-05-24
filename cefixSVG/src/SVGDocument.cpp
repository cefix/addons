//
//  SVGDocument.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGDocument.h"


void svg::Document::print(std::ostream& os)
{
    os << "svg " << getLeft() << "/" << getTop() << " x " << getWidth() << "/" << getHeight();
    Group::print(os);
}
