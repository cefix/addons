//
//  CreateScenegGraphFromSVGVisitor.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "CreateSceneGraphFromSVGVisitor.h"

#include "SVGNode.h"
#include "SVGDocument.h"
#include "SVGGroup.h"
#include "SVGRect.h"
#include "SVGEllipse.h"
#include "SVGText.h"
#include "SVGPath.h"
#include "SVGUtils.h"

#include <cefix/Quad2DGeometry.h>
#include <cefix/Ellipse2DGeometry.h>
#include <cefix/TextGeode.h>

#include <osgUtil/Tessellator>


osg::MatrixTransform* CreateSceneGraphFromSVGVisitor::createTransform(svg::Node& node)
{
    osg::MatrixTransform* mat = new osg::MatrixTransform();
    mat->setMatrix(node.getTransform());
    mat->setName(node.getId());
    
    return mat;
}

void CreateSceneGraphFromSVGVisitor::addDebugOutline(osg::MatrixTransform* mat, svg::Node& node)
{
    svg::util::Geometry::Outlines3D outlines;
    svg::util::Geometry::create3DOutlines(node, outlines);
    if (outlines.size() == 0)
        return;
    
    osg::Geode* geode = new osg::Geode();
    mat->addChild(geode);
    for(unsigned int i = 0; i < outlines.size(); ++i)
    {
        if (outlines[i]->size() == 0)
            continue;
        
        osg::Geometry* geo = new osg::Geometry();
        geo->setVertexArray(outlines[i].get());
        osg::Vec4Array* c = new osg::Vec4Array();
        c->push_back(osg::Vec4(0,1,0,1));
        geo->setColorArray(c);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);
        geo->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, outlines[i]->size()));
        geode->addDrawable(geo);
    }
    geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
}


void CreateSceneGraphFromSVGVisitor::pushState(svg::Node& node)
{
    svg::Visitor::pushState(node);
    
    if (_statesetStack.size() == 0) {
        _currentStateSet = new svg::StateSet();
        _currentStateSet->setFillRule(svg::StateSet::FR_NonZero);
        _currentStateSet->setFillColor(svg::Color("black"));
        _currentStateSet->setStrokeColor(svg::Color("none"));
        _currentStateSet->setStrokeLineJoin(svg::StateSet::LJ_Miter);
        _currentStateSet->setStrokeLineCap(svg::StateSet::LC_Butt);
        _currentStateSet->setDisplayMode(svg::StateSet::DM_Inline);
    } else {
        _currentStateSet = _statesetStack[_statesetStack.size()-1];
    }
    _currentStateSet = svg::StateSet::merge(_currentStateSet, node.getStateSet());
    _statesetStack.push_back(_currentStateSet);
    
    osg::MatrixTransform* mat = createTransform(node);
    if(_transformStack.size() > 0)
        _transformStack[_transformStack.size()-1]->addChild(mat);
    if(!_root)
        _root = mat;
    // addDebugOutline(mat, node);
    
    _transformStack.push_back(mat);
}

void CreateSceneGraphFromSVGVisitor::addStroke(svg::Node& node, osg::Geode* geode)
{
    if (_currentStateSet->isStroked()) {
        osg::ref_ptr<osg::Geometry> geo = svg::util::Geometry::createStroke(node, _currentStateSet->getStrokeWidth(), _currentStateSet->getStrokeMiterLimit(), _currentStateSet->getStrokeLineJoin(), _currentStateSet->getStrokeLineCap());
        osg::Vec4Array* colors = new osg::Vec4Array();
        colors->push_back(_currentStateSet->getStrokeColor().get());
        geo->setColorArray(colors);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);
        geode->addDrawable(geo.get());
    }
}


void CreateSceneGraphFromSVGVisitor::popState(svg::Node& node)
{
    svg::Visitor::popState(node);
    _transformStack.pop_back();
    _statesetStack.pop_back();
    if (_statesetStack.size() > 0) {
        _currentStateSet = _statesetStack[_statesetStack.size() - 1];
    }
}


void CreateSceneGraphFromSVGVisitor::visit(svg::Document& node)
{
    osg::Matrixd m = osg::Matrixd::scale(1, -1, 1) * osg::Matrixd::translate(-node.getLeft(), -node.getTop() + node.getHeight(), 0);
    getTopMost()->setMatrix(m);
}



void CreateSceneGraphFromSVGVisitor::visit(svg::Text& node)
{
   
    cefix::Utf8TextGeode* geode = new cefix::Utf8TextGeode(node.getFontFamily(), node.getFontSize(), node.getText());
    geode->setRenderPlane(cefix::FontCharData::NEGATIVE_XY_PLANE);
    if (_currentStateSet->isFilled())
        geode->setTextColor(_currentStateSet->getFillColor().get());
    if (_currentStateSet->hasFillOpacity())
        geode->setAlpha(_currentStateSet->getFillOpacity());
    osg::Vec2d v = node.getStartPosition();
    geode->setPosition(osg::Vec3(v[0], v[1], 0));
    getTopMost()->addChild(geode);
}


void CreateSceneGraphFromSVGVisitor::visit(svg::Node& node)
{
    osg::ref_ptr<svg::Path> path = svg::util::Geometry::converToPath(node);
    if (path.valid())
        visit(*path);
}

void CreateSceneGraphFromSVGVisitor::visit(svg::Path& node)
{    
    osg::Geode* geode = new osg::Geode();
    getTopMost()->addChild(geode);
    
    if (_currentStateSet->isFilled())
    {
        osg::Vec4 fill_color = _currentStateSet->getFillColor().get();
        if (_currentStateSet && _currentStateSet->hasFillOpacity())
            fill_color[3] = _currentStateSet->getFillOpacity();

        osg::ref_ptr<osg::Geometry> geo = svg::util::Geometry::createOutlines(node, 0.01);
        if (!geo.valid())
            return;
        osg::Vec4Array* c = new osg::Vec4Array();
        c->push_back(fill_color);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);
        geo->setColorArray(c);
        
        
        geode->addDrawable(geo);
        geode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
        geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
        
        
        osg::ref_ptr<osgUtil::Tessellator> tscx=new osgUtil::Tessellator; // the v1.2 multi-contour Tessellator.
        // we use the geometry primitives to describe the contours which are tessellated.
        // Winding odd means leave hole in surface where there are 2,4,6... contours circling the point.
        tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        tscx->setBoundaryOnly(false);
        if (_currentStateSet->getFillRule() == svg::StateSet::FR_NonZero)
            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_NONZERO);
        else
            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD);
        
        tscx->retessellatePolygons(*geo);
        
    }
    addStroke(node, geode);
}