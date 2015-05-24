//
//  VetctorRenderer2DCullVisitor.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 18.05.13.
//
//

#pragma once

#include "VectorRenderer2D.h"

class VectorRenderer2DCullCallback : public osg::NodeCallback {
public:
    VectorRenderer2DCullCallback(VectorRenderer2D* renderer, bool permanent = false)
    : osg::NodeCallback()
    , _renderer(renderer)
    , _permanent(permanent)
    , _renderNextFrame(!permanent)
    {
    }
    
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        traverse(node, nv);
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv && _renderNextFrame && _renderer.valid()) {
            _renderer->render(cv->getRenderStage(), cv->getViewport());
        }
        if (!_permanent) {
            node->setCullCallback(NULL);
        }
        _renderNextFrame = false;
    }
    
    void renderNextFrame(bool b) { _renderNextFrame = b; }
private:
    osg::observer_ptr<VectorRenderer2D> _renderer;
    bool _permanent, _renderNextFrame;
};