//
//  CreateSceneGraphFromSVGVisitor.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGVisitor.h"
#include "SVGStateSet.h"
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <vector>

class CreateSceneGraphFromSVGVisitor : public svg::Visitor {
public:

CreateSceneGraphFromSVGVisitor() : svg::Visitor(VisitVisibleElements), _transformStack() {}
public:
    
    virtual void visit(svg::Document& node);
    virtual void visit(svg::Text& node);
    virtual void visit(svg::Node& node);
    
    virtual void visit(svg::Path& node);

    virtual void pushState(svg::Node& node);
    virtual void popState(svg::Node& node);
    
    osg::Node* getNode() { return _root.get(); }
    
private:
    osg::MatrixTransform* createTransform(svg::Node& node);
    void addDebugOutline(osg::MatrixTransform* mat, svg::Node& node);
    
    osg::MatrixTransform* getTopMost() {
        return _transformStack.back();
    }
    
    void addStroke(svg::Node& node, osg::Geode* geode);
    
    std::vector< osg::ref_ptr<osg::MatrixTransform> > _transformStack;
    osg::ref_ptr< osg::MatrixTransform > _root;
    std::vector< osg::ref_ptr<svg::StateSet> > _statesetStack;
    osg::ref_ptr<svg::StateSet> _currentStateSet;
};