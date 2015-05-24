//
//  SVGText.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 08.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"

namespace svg {
  

class Text : public Node {
public:
    typedef std::vector<double> Positions;

    Text() : Node() {}
    
    void setText(const std::string& text) { _text = text; }
    const std::string& getText() const { return _text; }
    
    void setFontAndSize(const std::string& font_name, double size) { _fontFamily = font_name; _fontSize = size; }
    
    const std::string& getFontFamily() const { return _fontFamily; }
    double getFontSize() const { return _fontSize; }
    
    void setX(const Positions& pos) { _x = pos; }
    void setY(const Positions& pos) { _y = pos; }
    
    void setStartPosition(double x, double y) { _x.clear(); _y.clear(); _x.push_back(x); _y.push_back(y); }
    
    Positions& getX() { return _x; }
    Positions& getY() { return _y; }
    
    const Positions& getX() const { return _x; }
    const Positions& getY() const { return _y; }
    
    osg::Vec2d getStartPosition() const { return osg::Vec2d(_x[0], _y[0]); }
    
    virtual Text* asText() { return this; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    virtual void print(std::ostream& os);
private:
    std::string _text;
    Positions _x;
    Positions _y;
    std::string _fontFamily;
    double _fontSize;
};

}