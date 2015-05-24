//
//  SVGText.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 08.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGText.h"
#include <osg/io_utils>


void svg::Text::print(std::ostream& os)
{
    os << "text: " << _text << " ("  << _fontFamily << ", " << _fontSize << ", " << getStartPosition() << ") ";
}