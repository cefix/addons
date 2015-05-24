//
//  VectorRenderer2DPostscriptBackend.h
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

class VectorRenderer2DPostscriptBackend : public VectorRenderer2D::Backend {
public:
    VectorRenderer2DPostscriptBackend(bool wireframe = false)
        : VectorRenderer2D::Backend()
        , _count(0)
        , _baseName(cefix::DataFactory::instance()->getApplicationPath()+"/output_")
        , _fstream()
        , _colorRange(osg::Vec4(0.5, 0.5, 0.5, 1), osg::Vec4(0,0,0,1))
        , _wireframe(wireframe)
    {
    }
    
    void setColorRange(const cefix::vec4Range& range) { _colorRange = range; }
    void setFilePathAndName(const std::string& fn) { _baseName = fn; }
    
    virtual void reset()
    {
        _count++;
        if (_fstream.is_open())
            _fstream.close();
        
    }
    
    virtual void init(unsigned int w, unsigned int h)
    {
        std::stringstream ss;
        ss << _baseName << std::setw(4) << std::setfill('0') << _count << ".eps";
        _fstream.open(ss.str().c_str(), std::ios::out | std::ios::trunc);
        _fstream << "%!PS-Adobe-3.1 EPSF-3.0" << std::endl;
        _fstream << "%%Title: " << ss.str() << std::endl;
        _fstream << "%%Creator: " << cefixGetVersionDetailed() << std::endl;
        _fstream << "%%BoundingBox: 0 0 " << w << " " << h << std::endl;
        _fstream << "gsave" << std::endl;
        _fstream << "0.1 setlinewidth" << std::endl;
        _fstream << "newpath 0 0 moveto " << w << " 0 lineto " << w << " " << h << " lineto 0 " << h << " lineto closepath clippath" << std::endl;
        _fstream << "1 0 0 setrgbcolor" << std::endl;
        _fstream << "newpath 0 0 moveto " << w << " 0 lineto " << w << " " << h << " lineto 0 " << h << " lineto closepath stroke" << std::endl;
        _fstream << "gsave" << std::endl;
        _fstream << w/2.0 << " " << h/2.0 << " translate" << std::endl;
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
            
            _fstream << color[0] << " " << color[1] << " " << color[2] << " setrgbcolor" << std::endl;
            std::stringstream ss;
            ss << "newpath ";
            
            for(unsigned int j = 0; j != i->getNumLines(); ++j)
            {
                const VectorRenderer2D::Segment::Line& l = i->getLineAt(j);
                
                if ((j == 0) || ((j > 0) && (i->getLineAt(j-1).end_point != l.start_point)))
                    ss << l.start_point.x() << " " << l.start_point.y() << " moveto ";
                
                ss << l.end_point.x() << " " << l.end_point.y() << " lineto ";
               
            }
            if (i->isClosed())
                ss << " closepath";
            
            if(i->isClosed())
            {
                _fstream << ss.str() << " fill" << std::endl;
                if (_wireframe)
                {
                    color =  _colorRange.interpolate(scalar);
                    _fstream << color[0] << " " << color[1] << " " << color[2] << " setrgbcolor" << std::endl;
                    _fstream << ss.str() << " stroke" << std::endl;
                }
            }
            else
                _fstream << ss.str() << " stroke" << std::endl;
     
        }

        _fstream << "grestore" << std::endl;
        _fstream << "grestore" << std::endl;
        
    }

private:
    unsigned int _count;
    std::string _baseName;
    std::ofstream _fstream;
    cefix::vec4Range _colorRange;
    bool _wireframe;
};