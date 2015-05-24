//
//  SVGPath.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once


#include "SVGNode.h"
#include <osg/Vec2d>
#include <osg/Vec2>

#include <osg/io_utils>

namespace svg {

class Path : public Node {

public:
    class Line;
    class CubicCurve;
    class QuadraticCurve;
    class Arc;
    
    class Component : public osg::Referenced {
        public:
            enum Type { LineComponent, CubicCurveComponent, QuadraticCurveComponent, ArcComponent };
            Type getType() const { return _type; }
            Component(Type type) : osg::Referenced(), _type(type) {}
            virtual Line* asLine() { return NULL; }
            virtual CubicCurve* asCubicCurve() { return NULL; }
            virtual QuadraticCurve* asQuadraticCurve() { return NULL; }
            virtual void print(std::ostream& os) = 0;
        protected:
        
        private:
            Type _type;
    };
    
    class Line : public Component {
    public:
        Line(const osg::Vec2& s, const osg::Vec2& e) : Component(LineComponent), _start(s), _end(e) {}
        virtual Line* asLine() { return this; }
        const osg::Vec2d& getStart() const { return _start; }
        const osg::Vec2d& getEnd() const { return _end; }
        virtual void print(std::ostream& os) {
            os << "L " << _start << " "<< _end;
        }
    protected:
        osg::Vec2d _start, _end;
    };
    
    class CubicCurve : public Component {
    public:
        CubicCurve(const osg::Vec2d& s, const osg::Vec2& cp_1, const osg::Vec2& cp_2, const osg::Vec2& e)
            : Component(CubicCurveComponent)
            , _start(s)
            , _cp1(cp_1)
            , _cp2(cp_2)
            , _end(e)
        {
        }
        
        virtual CubicCurve* asCubicCurve() { return this; }
        
        const osg::Vec2d& getStart() const { return _start; }
        const osg::Vec2d& getEnd() const { return _end; }
        
        const osg::Vec2d& getControlPoint1() const { return _cp1; }
        const osg::Vec2d& getControlPoint2() const { return _cp2; }
        
        virtual void print(std::ostream& os) {
            os << "C " << _start << " " << _cp1 << " " << _cp2 << " " << _end;
        }
        
    private:
        osg::Vec2d _start, _cp1, _cp2, _end;
    };
    
    class QuadraticCurve : public Component {
    public:
        QuadraticCurve(const osg::Vec2d s, const osg::Vec2d& cp, const osg::Vec2d& e)
            : Component(QuadraticCurveComponent)
            , _start(s)
            , _cp(cp)
            , _end(e)
        {
        }
        
        const osg::Vec2d& getStart() const { return _start; }
        const osg::Vec2d& getEnd() const { return _end; }
        
        const osg::Vec2d& getControlPoint() const { return _cp; }
        
        QuadraticCurve* asQuadraticCurve() { return this; }
        
        virtual void print(std::ostream& os) {
            os << "S " << _start << " " << " " << _cp << " " << _end;
        }
        
    private:
        osg::Vec2d _start, _cp, _end;
    };
    
    class SubPath : public osg::Referenced {
    public:
        typedef std::vector< osg::ref_ptr< Component > > Components;
        SubPath() : osg::Referenced(), components(), startPoint(), closed(false) {}
        SubPath(const osg::Vec2d p) : osg::Referenced(), components(), startPoint(p), closed(false) {}
        
        Components components;
        osg::Vec2d startPoint;
        bool closed;
    };
    
    Path() : Node() {}
    
    void moveTo(double x, double y, bool relative = false);
    void lineTo(double x, double y, bool relative = false);
    void horizontalLineTo(double x, bool relative = false);
    void verticalLineTo(double y, bool relative = false);
    void cubicCurveTo(double x1, double y1, double x2, double y2, double x, double y, bool relative = false);
    void shortHandCubicCurveTo(double x2, double y2, double x, double y, bool relative = false);
    void quadraticCurveTo(double x1, double y1, double x, double y, bool relative = false);
    void shortHandQuadraticCurveTo(double x2, double y2, bool relative = false);
    void close();
    
    void moveTo(const osg::Vec2d& p, bool relative = false) {
        moveTo(p.x(), p.y(), relative);
    }
    void lineTo(const osg::Vec2d& e, bool relative = false) {
        lineTo(e.x(), e.y(), relative);
    }
    
    void cubicCurveTo(const osg::Vec2d& cp1, const osg::Vec2d& cp2, const osg::Vec2d& e, bool relative = false){
        cubicCurveTo(cp1.x(), cp1.y(), cp2.x(), cp2.y(), e.x(), e.y(), relative);
    }
    void shortHandCubicCurveTo(const osg::Vec2d& cp2, const osg::Vec2d& e, bool relative = false) {
        shortHandCubicCurveTo(cp2.x(), cp2.y(), e.x(), e.y(), relative);
    }
    void quadraticCurveTo(const osg::Vec2d& cp1, const osg::Vec2d& e, bool relative = false) {
        quadraticCurveTo(cp1.x(), cp1.y(), e.x(), e.y(), relative);
    }
    void shortHandQuadraticCurveTo(const osg::Vec2& e, bool relative = false) {
        shortHandQuadraticCurveTo(e.x(), e.y(), relative);
    }
    
    
    
    unsigned int getNumSubPaths() const { return _subPaths.size(); }
    SubPath* getSubPathAt(unsigned int ndx) { return _subPaths[ndx]; }
    
    virtual Path* asPath() { return this; }
    
    virtual void accept(Visitor& visitor)
    {
        visitor.pushVisitAndPop(*this);
    }
    
    virtual void print(std::ostream& os);

protected:
    osg::Vec2d makeP(double x, double y, bool relative) {
        return relative ? osg::Vec2(x,y) + _currentPoint : osg::Vec2(x,y);
    }
    
    osg::Vec2d _currentPoint;
    std::vector< osg::ref_ptr< SubPath > > _subPaths;
    SubPath* _currentSubPath;
};


}


std::ostream& operator<<(std::ostream& os, svg::Path::Component& c);
