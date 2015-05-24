//
//  Group.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SVGNode.h"
#include <cefix/MathUtils.h>


namespace svg {

class Group: public Node {
public:
    typedef std::vector<osg::ref_ptr<Node> > Children;
    typedef Children::iterator iterator;
    
    Group() : Node(), _children(), _viewbox(0,0,0,0) {}
    
    void addChild(Node* node) { _children.push_back(node); }
    
    unsigned int getNumChildren() const { return _children.size(); }
    
    Node* getChildAt(unsigned int ndx) { return _children[ndx]; }
    
    bool removeChild(Node* node) {
        iterator itr = std::find(_children.begin(), _children.end(), node);
        if(itr == _children.end())
            return false;
        _children.erase(itr);
        return true;
    }
    
    bool removeChildAt(unsigned int ndx)
    {
        if (ndx >= _children.size())
            return false;
        _children.erase(_children.begin() + ndx);
        
        return true;
    }
    
    iterator begin() { return _children.begin(); }
    iterator end() { return _children.end(); }
    
    virtual Group* asGroup() { return this; }
    
    void setViewBox(double x, double y, double w, double h)
    {
        _viewbox.set(x, y, x+w, y+h);
    }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    
    virtual void print(std::ostream& where);
    
    
    const osg::Vec4d& getViewBox() const { return _viewbox; }
    bool hasViewBox() { return (cefix::Rect::getWidth(_viewbox) != 0) || (cefix::Rect::getHeight(_viewbox) != 0); }
    
protected:
    Children _children;
    osg::Vec4d _viewbox;

};

}

