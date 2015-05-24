//
//  VectorRenderer2D.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 18.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <osgViewer/Viewer>
#include <osg/ValueObject>
#include <cefix/Frustum.h>
#include "TemplatedPrimitiveIndexFunctor.h"

class VectorRenderer2DCullCallback;
class VectorRenderer2D : public osg::Referenced {

public:
    class Segment {
    public:
        struct Line {
            osg::Vec3 start_point, end_point;
            osg::Vec3::value_type z;
            Line() : start_point(0,0,0), end_point(0,0,0), z(0) {}
            Line(const osg::Vec3& s, const osg::Vec3& e, osg::Vec3::value_type in_z) : start_point(s), end_point(e), z(in_z) {}
        };
        
        typedef std::vector<Line> Lines;
        typedef Lines::iterator iterator;
        typedef Lines::const_iterator const_iterator;
        
        Segment() : _lines(), _zComputed(false), _closed(false), _filled(false), _color(osg::Vec4(-1, 0,0,0)), _z(0), _clipCount(0) {}
        
        void add(const osg::Vec3& a, const osg::Vec3& b, osg::Vec3::value_type z) { _lines.push_back(Line(a, b, z)); _zComputed = false;}
        
        osg::Vec3::value_type getZ() const {
            
            if (_zComputed)
                return _z;
            if (_lines.size() == 0)
                _z = 0;
            else {
                _z = 0;
                for(Lines::const_iterator i = _lines.begin(); i != _lines.end(); ++i) {
                    _z += (*i).z;
                }
                _z /= _lines.size();
            }
            _zComputed = true;
            
            return _z;
        }
        
        void incClipCount() { _clipCount++; }
        void setColor(const osg::Vec4& color) { _color = color; }
        bool hasColor() const { return (_color[0] >= 0); }
        const osg::Vec4& getColor() const { return _color; }
        
        inline bool operator<(Segment& rhs) { return getZ() < rhs.getZ(); }
        
        void setIsFilled(bool f) { _filled = f; }
        bool isFilled() const { return _filled; }
        bool isClosed() const { return _closed; }
        bool isClipped() const { return _clipCount == _lines.size(); }
        bool isValid() const { return (_lines.size() > 0) && !isClipped(); }
        
        void setIsClosed() { _closed = true; }
        
        unsigned int getNumLines() const { return _lines.size(); }
        const Line& getLineAt(unsigned int ndx) const { return _lines[ndx]; }
        
        const Lines& getLines() const { return _lines; }
        Lines& getLines() { return _lines; }
        
        iterator begin() { return _lines.begin(); }
        iterator end() { return _lines.end(); }

        const_iterator begin() const { return _lines.begin(); }
        const_iterator end() const { return _lines.end(); }

    private:
        
        Lines _lines;
        mutable bool _zComputed;
        bool _closed, _filled;
        osg::Vec4 _color;
        mutable osg::Vec3::value_type _z;
        unsigned int _clipCount;
    };
    
    typedef std::list<Segment> Segments;
    
    
    class Backend: public osg::Referenced {
    public:
        Backend() : osg::Referenced() {}
        virtual void reset() = 0;
        virtual void init(unsigned int w, unsigned int h) = 0;
        virtual void render(const Segments& segments) = 0;
        
    };
    
    VectorRenderer2D(Backend* backend);
    
    Backend* getBackend() { return _backend.get(); }
    
    
    VectorRenderer2DCullCallback* attachTo(osgViewer::View* view, bool permanently = false);
    void setUseClipping(bool b) { _functor->setUseClipping(b); }
    void setSplitLines(bool b) { _functor->setSplitLines(b); }
    
protected:
    void render(osgUtil::RenderBin* stage);
    void render(osg::Camera* camera);
    void render(osgViewer::View* view);
    void render(osgUtil::RenderStage* stage, osg::Viewport* vp);
    
    
    void renderStateGraph(osgUtil::StateGraph* graph);
    void renderRenderLeaf(osgUtil::RenderLeaf* rl);
        
    class Operator : public osg::Referenced {
    public:
        
        Operator() : _currentModelView(), _currentProjection(), _parent(NULL), _useClipping(true), _splitLines(true),_useSharedSegment(false) {}
        void setParent(VectorRenderer2D* parent) { _parent = parent; }
        void setGeometry(osg::Geometry* geo) { _primitiveNdx = -1; _primitiveSetNdx = -1; _geo = geo; _v = NULL; }
        
        void operator()(unsigned int  p1);
        void operator()(unsigned int  p1, unsigned int  p2);
        void operator()(unsigned int  p1, unsigned int  p2, unsigned int  p3);
        void operator()(unsigned int  p1, unsigned int  p2, unsigned int  p3, unsigned int  p4);
        
        void tryToSetColor(Segment& p, unsigned int ndx);

        void handleNewPrimitiveSet(GLenum mode);
        void handlePrimitiveSetFinished();
        
        void setModelViewMatrix(osg::RefMatrix* m) { _currentModelView = m; }
        void setProjectionMatrix(osg::RefMatrix* m) { _currentProjection = m; _frustum.setMatrix(m); }
        
        void setViewport(osg::Viewport* vp) {
            _viewport = vp;
        }
        osg::Viewport* getViewport() const { return _viewport; }
        
        void setUseClipping(bool b) { _useClipping = b; }
        void setSplitLines(bool b) { _splitLines = b; }
        
    private:
        void addLine(Segment& p, unsigned int ndx1, unsigned int ndx2);
        inline void applyProjection(osg::Vec3& v) const {
            v = v * (*_currentProjection);
            v[0] *= getViewport()->width()  / 2.0;
            v[1] *= getViewport()->height() / 2.0;
            v[2] = 0;
        }
        osg::RefMatrix *_currentModelView, *_currentProjection;
        osg::Geometry* _geo;
        osg::Vec3Array* _v;
        int _primitiveSetNdx,_primitiveNdx;
        VectorRenderer2D* _parent;
        osg::ref_ptr<osg::Viewport> _viewport;
        cefix::Frustum _frustum;
        bool _useClipping, _splitLines, _useSharedSegment;
        Segment _sharedSegment;
    };
    
private:
    void addSegment(const Segment& segment) { _segments.push_back(segment); }
    
    osg::ref_ptr<Backend> _backend;
    osg::ref_ptr<TemplatedPrimitiveIndexFunctor<Operator> > _functor;
    
    Segments _segments;
    
    friend class VectorRenderer2DCullCallback;
    friend class Operator;
};