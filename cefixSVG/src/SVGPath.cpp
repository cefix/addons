//
//  SVGPath.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGPath.h"

namespace svg {

void Path::moveTo(double x, double y, bool relative)
{
    osg::Vec2 p = makeP(x,y, relative);
    //_currentSubPath->components.push_back(new MoveTo(p));
    _currentPoint = p;
    _currentSubPath = new SubPath(p);
    _subPaths.push_back(_currentSubPath);
}


void Path::lineTo(double x, double y, bool relative)
{
    osg::Vec2 p = makeP(x,y, relative);
    _currentSubPath->components.push_back(new Line(_currentPoint, p));
    _currentPoint = p;
}


void Path::horizontalLineTo(double x, bool relative) {
    lineTo(x, relative ? 0 : _currentPoint.y(), relative);
}


void Path::verticalLineTo(double y, bool relative) {
    lineTo(relative ? 0 : _currentPoint.x(), y, relative);
}


void Path::close()
{
    if (_currentSubPath) {
        lineTo(_currentSubPath->startPoint.x(), _currentSubPath->startPoint.y(), false);
        _currentSubPath->closed = true;
    }
}


void Path::cubicCurveTo(double x1, double y1, double x2, double y2, double x, double y, bool relative) {
    
    osg::Vec2 cp_1 = makeP(x1,y1, relative);
    osg::Vec2 cp_2 = makeP(x2,y2, relative);
    osg::Vec2 p = makeP(x, y, relative);
    
    _currentSubPath->components.push_back(new CubicCurve(_currentPoint, cp_1, cp_2, p));
    _currentPoint = p;
}


void Path::shortHandCubicCurveTo(double x2, double y2, double x, double y, bool relative) {
    osg::Vec2d cp_1(_currentPoint);
    if ((_currentSubPath->components.size() > 0) && (_currentSubPath->components[_currentSubPath->components.size() - 1]->asCubicCurve())) {
        CubicCurve* c = _currentSubPath->components[_currentSubPath->components.size() - 1]->asCubicCurve();
        cp_1 = c->getEnd() - (c->getControlPoint2() - c->getEnd());
    }
    osg::Vec2d cp_2 = makeP(x2, y2, relative);
    osg::Vec2d p = makeP(x,y, relative);
    _currentSubPath->components.push_back(new CubicCurve(_currentPoint, cp_1, cp_2, p));
    _currentPoint = p;
}


void Path::quadraticCurveTo(double x1, double y1, double x, double y, bool relative) {
    
    osg::Vec2 cp = makeP(x1,y1, relative);
    osg::Vec2 p = makeP(x, y, relative);
    
    _currentSubPath->components.push_back(new QuadraticCurve(_currentPoint, cp, p));
    _currentPoint = p;
}


void Path::shortHandQuadraticCurveTo(double x, double y, bool relative) {
    osg::Vec2d cp(_currentPoint);
    if ((_currentSubPath->components.size() > 0) && (_currentSubPath->components[_currentSubPath->components.size() - 1]->asQuadraticCurve())) {
        QuadraticCurve* c = _currentSubPath->components[_currentSubPath->components.size() - 1]->asQuadraticCurve();
        cp = c->getEnd() - (c->getControlPoint() - c->getEnd());
    }

    osg::Vec2d p = makeP(x,y, relative);
    _currentSubPath->components.push_back(new QuadraticCurve(_currentPoint, cp, p));
    _currentPoint = p;
}

void Path::print(std::ostream& os)
{
    os << "path ";
    for(unsigned int i = 0; i < getNumSubPaths(); ++i) {
        SubPath* s = getSubPathAt(i);
        for(SubPath::Components::iterator j = s->components.begin(); j != s->components.end(); ++j) {
            os << (**j) << "   ";
        }
    }
}


}


std::ostream& operator<<(std::ostream& os, svg::Path::Component& c) {
    c.print(os);
    return os;
}
