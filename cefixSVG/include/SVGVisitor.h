//
//  SVGVisitor.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGStateSet.h"

#include <vector>

namespace svg {

class Node;
class Group;
class Rect;
class Ellipse;
class Circle;
class Polygon;
class Line;
class PolyLine;
class Path;
class Text;
class Document;


class Visitor {
public:
    typedef std::vector<Node*> NodeStack;
    enum VisitMode { VisitVisibleElements, VisitAllElements };
    Visitor(VisitMode mode) : _mode(mode) {}
    
    virtual void visit(Node& node);
    virtual void visit(Group& node);
    virtual void visit(Rect& node);
    virtual void visit(Ellipse& node);
    virtual void visit(Circle& node);
    virtual void visit(Polygon& node);
    virtual void visit(Line& node);
    virtual void visit(PolyLine& node);
    virtual void visit(Path& node);
    virtual void visit(Document& node);
    virtual void visit(Text& node);
    
    virtual void pushState(Node& node) { _nodeStack.push_back(&node); }
    virtual void popState(Node& node) { _nodeStack.pop_back(); }
    
    template <class T> void pushVisitAndPop(T& node) {
        if(_mode == VisitVisibleElements && node.getStateSet() && (node.getStateSet()->getDisplayMode() == StateSet::DM_None))
            return;
        
        pushState(node);
        visit(node);
        if(node.asGroup())
            traverse(*node.asGroup());
        popState(node);
    }
    
    virtual void traverse(Group& group);
protected:
    NodeStack _nodeStack;
    VisitMode _mode;
};

}