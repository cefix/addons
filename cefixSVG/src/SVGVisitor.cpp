//
//  SVGVisitor.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGVisitor.h"
#include "SVGGroup.h"
#include "SVGRect.h"
#include "SVGEllipse.h"
#include "SVGPolygon.h"
#include "SVGLine.h"
#include "SVGPolyLine.h"
#include "SVGPath.h"
#include "SVGText.h"
#include "SVGDocument.h"


namespace svg {

void Visitor::visit(Node& node)
{
}


void Visitor::visit(Group& node)
{
    visit(static_cast<Node&>(node));
}


void Visitor::visit(Rect& node)
{
    visit(static_cast<Node&>(node));
}


void Visitor::visit(Ellipse& node)
{
    visit(static_cast<Node&>(node));
}

void Visitor::visit(Circle& node)
{
    visit(static_cast<Ellipse&>(node));
}

void Visitor::visit(Polygon& node)
{
    visit(static_cast<Node&>(node));
}


void Visitor::visit(Line& node)
{
    visit(dynamic_cast<Node&>(node));
}


void Visitor::visit(PolyLine& node)
{
    visit(static_cast<Node&>(node));
}

void Visitor::visit(Path& node)
{
    visit(dynamic_cast<Node&>(node));
}

void Visitor::visit(Document& node)
{
    visit(dynamic_cast<Group&>(node));
}


void Visitor::visit(Text& node)
{
    visit(static_cast<Node&>(node));
}


void Visitor::traverse(Group& group)
{    
    for(Group::iterator i =group.begin(); i != group.end(); ++i)
    {
        (*i)->accept(*this);
    }
}


}