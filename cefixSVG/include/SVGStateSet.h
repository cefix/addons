//
//  SVGStateSet.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGColor.h"

#include <osg/Referenced>

namespace svg {

class StateSet : public osg::Referenced {
public:
    enum FillRule { FR_Inherit, FR_NonZero, FR_OddEven };
    enum StrokeLineCap { LC_Inherit, LC_Butt, LC_Round, LC_Square };
    enum StrokeLineJoin { LJ_Inherit, LJ_Miter, LJ_Round, LJ_Bevel };
    enum DisplayMode { DM_Inherit, DM_None, DM_Inline };
    StateSet();
    
    const Color& getFillColor() const { return _fillColor; }
    const Color& getStrokeColor() const { return _strokeColor; }
    bool isFilled() const { return _fillColor.hasColor(); }
    bool isStroked() const { return _strokeColor.hasColor(); }
    
    void setFillColor(const Color& color) { _fillColor = color; }
    void setStrokeColor(const Color& color) { _strokeColor = color; }
    
    bool hasFillOpacity() const { return (_fillOpacity >= 0); }
    bool hasStrokeOpacity() const { return (_strokeOpacity >= 0); }
    
    double getFillOpacity() const { return _fillOpacity; }
    double getStrokeOpacity() const { return _strokeOpacity; }
    
    void setFillOpacity(double d) { _fillOpacity = d; }
    void setStrokeOpacity(double d) { _strokeOpacity = d; }
    
    FillRule getFillRule() const { return _fillRule; }
    void setFillRule(FillRule fill_rule) { _fillRule = fill_rule; }
    
    StrokeLineCap getStrokeLineCap() const { return _strokeLineCap; }
    void setStrokeLineCap(StrokeLineCap stroke_line_cap) { _strokeLineCap = stroke_line_cap; }
    
    StrokeLineJoin getStrokeLineJoin() const { return _strokeLineJoin; }
    void setStrokeLineJoin(StrokeLineJoin stroke_line_join) { _strokeLineJoin = stroke_line_join; }
    
    bool hasStrokeWidth() const { return _strokeWidth >= 0; }
    bool hasStrokeMiterLimit() const { return _strokeMiterLimit >= 0; }
    
    double getStrokeWidth() const { return _strokeWidth; }
    double getStrokeMiterLimit() const { return _strokeMiterLimit; }
    
    void setStrokeWidth(double d) { _strokeWidth = d; }
    void setStrokeMiterLimit(double d) { _strokeMiterLimit = d; }
    
    
    static StateSet* merge(StateSet* left, StateSet* right);
    
    void setDisplayMode(DisplayMode display) { _display = display; }
    DisplayMode getDisplayMode() const { return _display; }
    
protected:
    Color _fillColor, _strokeColor;
    double _fillOpacity, _strokeOpacity;
    FillRule _fillRule;
    double _strokeWidth, _strokeMiterLimit;
    StrokeLineCap _strokeLineCap;
    StrokeLineJoin _strokeLineJoin;
    DisplayMode _display;

};

}

std::ostream& operator<<(std::ostream& os, const svg::StateSet&);