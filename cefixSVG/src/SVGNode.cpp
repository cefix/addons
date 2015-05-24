//
//  SVGNode.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGNode.h"

std::ostream& operator<<(std::ostream& os, svg::Node& node)
{
    node.print(os);
    
    if (!node.getId().empty())
        os << "id: " << node.getId();
    if (node.getStateSet())
        os << " " << (*node.getStateSet());
    
    return os;
}