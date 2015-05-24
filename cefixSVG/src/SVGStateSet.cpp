//
//  SVGStateSet.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGStateSet.h"
#include <osg/ref_ptr>

namespace svg {

StateSet::StateSet()
    : osg::Referenced()
    , _fillColor(Color("none"))
    , _strokeColor(Color("none"))
    , _fillOpacity(-1.0)
    , _strokeOpacity(-1.0)
    , _fillRule(FR_Inherit)
    , _strokeWidth(-1.0)
    , _strokeMiterLimit(-1.0)
    , _strokeLineCap(LC_Inherit)
    , _strokeLineJoin(LJ_Inherit)
    , _display(DM_Inherit)
{

}

StateSet* StateSet::merge(StateSet* left, StateSet* right)
{
    osg::ref_ptr<StateSet> result = new StateSet(*left);
    if (!right)
        return result.release();
    
    result->setFillColor(right->getFillColor());
    result->setStrokeColor(right->getStrokeColor());
    if (right->hasFillOpacity())
        result->setFillOpacity(right->getFillOpacity());
    if (right->hasStrokeOpacity())
        result->setStrokeOpacity(right->getStrokeOpacity());
    
    if(right->getFillRule() != FR_Inherit)
        result->setFillRule(right->getFillRule());
    if (right->getStrokeLineCap() != LC_Inherit)
        result->setStrokeLineCap(right->getStrokeLineCap());
    if (right->getStrokeLineJoin() != LJ_Inherit)
        result->setStrokeLineJoin(right->getStrokeLineJoin());
    if (right->hasStrokeWidth())
        result->setStrokeWidth(right->getStrokeWidth());
    if (right->hasStrokeMiterLimit())
        result->setStrokeMiterLimit(right->getStrokeMiterLimit());
    if(right->getDisplayMode() != DM_Inherit)
        result->setDisplayMode(right->getDisplayMode());
    
    return result.release();
}
 
 
}

std::ostream& operator<<(std::ostream& os, const svg::StateSet& ss)
{
    os  << "(fill: " << ss.getFillColor();
    if (ss.hasFillOpacity())
        os << " / " <<  ss.getFillOpacity();
    if (ss.getFillRule() != svg::StateSet::FR_Inherit)
        os << ", fill-rule: " << ss.getFillRule();
    
    os << ", stroke: " << ss.getStrokeColor();
    if (ss.hasStrokeOpacity())
        os << "/" << ss.getStrokeOpacity();
    
    if (ss.hasStrokeWidth())
        os << ", stroke-width: " << ss.getStrokeWidth();
    if (ss.hasStrokeMiterLimit())
        os << ", stroke-miterlimit: " << ss.getStrokeMiterLimit();
    if (ss.getStrokeLineCap() != svg::StateSet::LC_Inherit)
        os << ", stroke-linecap: " << ss.getStrokeLineCap();
    if (ss.getStrokeLineJoin() != svg::StateSet::LJ_Inherit)
        os << ", stroke-linejoin: " << ss.getStrokeLineJoin();
    os << ") ";
    
    return os;
}