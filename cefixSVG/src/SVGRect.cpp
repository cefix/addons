//
//  SVGRect.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGRect.h"

void svg::Rect::print(std::ostream& os)
{
    os <<"rect " << getLeft() << "/" << getTop() << " x " << getWidth() << "/" << getHeight();
}