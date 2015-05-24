//
//  SVGNode.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGVisitor.h"
#include "SVGStateSet.h"
#include <osg/Matrixd>


namespace svg {

class Document;
class Rect;
class Group;
class Ellipse;
class Circle;
class Polygon;
class Line;
class PolyLine;
class Path;
class Text;


class Node : public osg::Referenced {
public:
    Node() : Referenced(), _id(""), _transform(), _stateSet(NULL) {}
    
    const std::string& getId() const { return _id; }
    void setId(const std::string& in_id) { _id = in_id; }
    
    const osg::Matrixd& getTransform() const { return _transform; }
    void setTransform(const osg::Matrixd& m) { _transform = m; }
    
    virtual Document* asDocument() { return NULL; }
    virtual Rect* asRect() { return NULL; }
    virtual Group* asGroup() { return NULL; }
    virtual Ellipse* asEllipse() { return NULL; }
    virtual Circle* asCirle() { return NULL; }
    virtual Polygon* asPolygon() { return NULL; }
    virtual Line* asLine() { return NULL; }
    virtual PolyLine* asPolyLine() { return NULL; }
    virtual Path* asPath() { return NULL; }
    virtual Text* asText() { return NULL; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    
    StateSet* getOrCreateStateSet() {
        if (!_stateSet)
            _stateSet = new StateSet();
        return _stateSet.get();
    }
    
    StateSet* getStateSet() {
        return _stateSet.get();
    }
    
    void setStateSet(StateSet* ss) { _stateSet = ss; }
    
    virtual void print(std::ostream& where) = 0;
    
private:
    std::string _id;
    osg::Matrixd _transform;
    osg::ref_ptr<StateSet> _stateSet;
};

}

std::ostream& operator<<(std::ostream& os, svg::Node& node);

