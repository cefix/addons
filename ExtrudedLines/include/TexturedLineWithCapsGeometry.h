//
//  TexturedLineWithCapsGeometry.h
//  cefixSketch
//
//  Created by Stephan Huber on 12.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once


#include "ExtrudedLineGeometry.h"

namespace cefix {

namespace TexturedLineWithCapsImpl {


class TexturePolicy : public ExtrudedLine::TexturePolicyBase {
public:
    TexturePolicy() : ExtrudedLine::TexturePolicyBase(), _texCoords(new osg::Vec2Array()) {}
    void setupTextureCoords(cefix::ExtrudedLine::BaseGeometry* geo, unsigned int num_vertices_per_slice) 
    {
        geo->setTexCoordArray(0, _texCoords);
        _numVerticesPerSlice = num_vertices_per_slice;
        
    }
    void beginTextureCoords(cefix::ExtrudedLine::BaseGeometry* geo, unsigned int num_vertices_per_slice) {
        _texCoords->resize(geo->getNumLineVertices() * num_vertices_per_slice);
        _numVerticesPerSlice = num_vertices_per_slice;
    }
    
    void computeTextureCoords(cefix::ExtrudedLine::BaseGeometry* geo, unsigned int i, const cefix::ExtrudedLine::Point& p) 
    {
        const double scalar = 1 / static_cast<double>(_numVerticesPerSlice-1);
        float x_scalar = (i == 0) ? 0 : (i < geo->getNumLineVertices()-1) ? 0.5 : 1;
        for(unsigned int k = 0; k < _numVerticesPerSlice; ++k) {
            (*_texCoords)[i * _numVerticesPerSlice + k].set( x_scalar, k * scalar);
        }
    }
private:
    osg::Vec2Array* _texCoords;
    unsigned int _numVerticesPerSlice;

};

}


class TexturedLineWithCapsGeometry : public ExtrudedLineGeometryT<
        cefix::ExtrudedLine::LineSliceGenerator, 
        cefix::ExtrudedLine::SingleColorPolicy,
        TexturedLineWithCapsImpl::TexturePolicy> {

public:
    TexturedLineWithCapsGeometry() : ExtrudedLineGeometryT<
        cefix::ExtrudedLine::LineSliceGenerator, 
        cefix::ExtrudedLine::SingleColorPolicy,
        TexturedLineWithCapsImpl::TexturePolicy>()
    {
        setUseCaps(true);
    }
};



}