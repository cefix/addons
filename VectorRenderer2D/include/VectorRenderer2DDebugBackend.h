//
//  VectorRenderer2DDebugBackend.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 19.05.13.
//
//

#pragma once

#include "VectorRenderer2D.h"
#include <osgDB/WriteFile>
#include <cefix/DataFactory.h>

class VectorRenderer2DDebugBackend : public VectorRenderer2D::Backend {
public:
    VectorRenderer2DDebugBackend()
        : VectorRenderer2D::Backend()
        , _geode(new osg::Geode())
        , _geometry(new osg::Geometry())
        , _vertices(new osg::Vec3Array())
        , _colors(new osg::Vec4Array())
        , _da(new osg::DrawArrays(GL_LINES, 0, 0))
        , _colorRange(osg::Vec4(0.5, 0.5, 0.5, 0.5), osg::Vec4(1, 1, 1, 0.5))
    {
        _geode->addDrawable(_geometry);
        _geometry->setUserValue("vector_renderer_2d_ignore", true);
        _geometry->setName("VectorRenderer2DDebugBackend");
        _geometry->setVertexArray(_vertices);
        _geometry->setColorArray(_colors);
        _geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        _geometry->addPrimitiveSet(_da);
    }
    
    osg::Node* getNode() { return _geode; }

    virtual void reset()
    {
        _geode->setNodeMask(0x0);
        _vertices->resize(0);
        _colors->resize(0);
    }
    
    virtual void init(unsigned int w, unsigned int h)
    {
       _anchor.set(w / 2.0, h / 2.0, 0);
    }

    virtual void render(const VectorRenderer2D::Segments& segments)
    {
        
        double min_z = segments.front().getZ();
        double max_z = segments.back().getZ();
        std::cout << "num segments " << segments.size() << std::endl;
        for(VectorRenderer2D::Segments::const_iterator i = segments.begin(); i != segments.end(); ++i)
        {
            double z = i->getZ();
            double scalar = (max_z - min_z) > 0.0001 ? (z - min_z) / (max_z - min_z) : 1;
            
            osg::Vec4 color = i->hasColor() ? i->getColor() : _colorRange.interpolate(scalar);
            
            for(VectorRenderer2D::Segment::Lines::const_iterator j = i->begin(); j != i->end(); ++j)
            {
                _vertices->push_back(_anchor + j->start_point);
                _vertices->push_back(_anchor + j->end_point);
                
                
                _colors->push_back(color);
                _colors->push_back(color);
            }
        }
        _da->setCount(_vertices->size());
        _geometry->dirtyDisplayList();
        _geometry->dirtyBound();
        _geode->setNodeMask(0xffff);
        osgDB::writeNodeFile(*_geode, cefix::DataFactory::instance()->getApplicationPath()+"/2d-rendering.osgt");
    }

private:
    osg::ref_ptr<osg::Geode> _geode;
    osg::Geometry* _geometry;
    osg::Vec3Array* _vertices;
    osg::Vec4Array* _colors;
    osg::DrawArrays* _da;
    cefix::vec4Range _colorRange;
    osg::Vec3 _anchor;
};