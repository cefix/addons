//
//  VectorRenderer2DSVGBackend.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 20.05.13.
//
//

#pragma once

#include "VectorRenderer2D.h"
#include <cefix/DataFactory.h>
#include <cefix/Version.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "SVGDocument.h"
#include "SVGPath.h"
#include "SVGWriter.h"

class VectorRenderer2DSVGBackend : public VectorRenderer2D::Backend {
public:
    VectorRenderer2DSVGBackend(bool wireframe = false)
        : VectorRenderer2D::Backend()
        , _count(0)
        , _baseName(cefix::DataFactory::instance()->getApplicationPath()+"/output_")
        , _colorRange(osg::Vec4(0.5, 0.5, 0.5, 1), osg::Vec4(0,0,0,1))
        , _wireframe(wireframe)
    {
    }
    
    void setColorRange(const cefix::vec4Range& range) { _colorRange = range; }
    void setFilePathAndName(const std::string& fn) { _baseName = fn; }
    
    virtual void reset()
    {
        _count++;
    }
    
    virtual void init(unsigned int w, unsigned int h)
    {
        _doc = new svg::Document();
        _doc->setDimensions(0,0, w, h);
        _group = new svg::Group();
        _doc->addChild(_group);
        _group->setTransform(osg::Matrix::scale(1, -1, 1) * osg::Matrix::translate(w/2, h/2,0));
        _group->getOrCreateStateSet()->setStrokeWidth(0.1);
    }

    virtual void render(const VectorRenderer2D::Segments& segments)
    {
        
        double min_z = segments.front().getZ();
        double max_z = segments.back().getZ();
        
        for(VectorRenderer2D::Segments::const_iterator i = segments.begin(); i != segments.end(); ++i)
        {
            double z = i->getZ();
            double scalar = (max_z - min_z) > 0.0001 ? (z - min_z) / (max_z - min_z) : 1;
            osg::Vec4 color = i->hasColor() ? i->getColor() : _colorRange.interpolate(scalar);
            
            svg::Path* path = new svg::Path();
            
            for(unsigned int j = 0; j != i->getNumLines(); ++j)
            {
                const VectorRenderer2D::Segment::Line& l = i->getLineAt(j);
                
                if ((j == 0) || ((j > 0) && (i->getLineAt(j-1).end_point != l.start_point)))
                    path->moveTo(l.start_point.x(), l.start_point.y());
                
                path->lineTo(l.end_point.x() , l.end_point.y());
               
            }
            if (i->isClosed())
                path->close();
            
            if(i->isClosed())
            {
                path->getOrCreateStateSet()->setFillColor(svg::Color(color));
                if (_wireframe)
                {
                    color =  _colorRange.interpolate(scalar);
                    path->getOrCreateStateSet()->setStrokeColor(svg::Color(color));
                }
            }
            else
                path->getOrCreateStateSet()->setStrokeColor(svg::Color(color));
            
            _group->addChild(path);
     
        }

        std::stringstream ss;
        ss << _baseName << std::setw(4) << std::setfill('0') << _count << ".svg";
    
        svg::Writer::write(_doc, ss.str(), false);
    }

private:
    unsigned int _count;
    std::string _baseName;
    osg::ref_ptr<svg::Document> _doc;
    svg::Group* _group;
    cefix::vec4Range _colorRange;
    bool _wireframe;
};