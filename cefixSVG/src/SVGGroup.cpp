//
//  Group.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGGroup.h"
#include <iomanip>

void svg::Group::print(std::ostream& os)
{
    static int level=0;
    os << std::setw(3*level) << " " << "group start" << std::endl;
    level++;
    for(unsigned int i = 0; i < getNumChildren(); ++i) {
        os << std::setw(3*level) << " " << (*getChildAt(i)) << std::endl;
    }
    level--;
    os << std::setw(3*level) << " " << "group end ";
    
}