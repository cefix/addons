//
//  ExtrudedLineGeometry.cpp
//  cefixSketch
//
//  Created by Stephan Huber on 06.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ExtrudedLineGeometry.h"

namespace cefix {



namespace ExtrudedLine {


void BaseGeometry::computeCaps()
{
    if(_lineVertices.size() < 2)
        return;
    
    _startCaps = _lineVertices[0];
    osg::Vec3 dir(_lineVertices[1].point - _lineVertices[0].point);
    dir.normalize();
    dir *= _startCaps.width * 0.5;
    _startCaps.point = _lineVertices[0].point - dir;
    
    _endCaps = _lineVertices[_lineVertices.size()-1];
    dir = _lineVertices[_lineVertices.size()-1].point - _lineVertices[_lineVertices.size()-2].point;
    dir.normalize();
    dir *= _endCaps.width * 0.5;
    _endCaps.point = _lineVertices[_lineVertices.size()-1].point + dir;
}

DebugGenerator::DebugGenerator()
:   SliceGeneratorBase(6)
{
}
    
void DebugGenerator::setup(BaseGeometry* geo)
{
    _vertices = new osg::Vec3Array();
    _colors = new osg::Vec4Array();
    
    geo->setVertexArray(_vertices);
    geo->setColorArray(_colors);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    
    _da = new osg::DrawArrays(GL_LINES);
    geo->addPrimitiveSet(_da);
    
    geo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geo->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    geo->getOrCreateStateSet()->setRenderBinDetails(20, "DepthSortedBin");
}


void DebugGenerator::begin(BaseGeometry* geo) 
{
    _vertices->resize(geo->getNumLineVertices() * getNumVerticesPerSlice());
    _colors->resize(geo->getNumLineVertices() * getNumVerticesPerSlice());
    _da->setCount(geo->getNumLineVertices() * getNumVerticesPerSlice());
}



void DebugGenerator::compute(BaseGeometry* geo, unsigned int i, const Point& p)
{
    const unsigned int pps = getNumVerticesPerSlice();
    osg::Quat q;
    q.makeRotate(osg::Y_AXIS, p.up);
    
    osg::Vec3 d1(osg::X_AXIS * p.width * 0.5);
    osg::Vec3 d2(osg::Y_AXIS * p.height * 0.5);
        
    d1 = q * d1;
    d2 = q * d2;
    
    
    (*_vertices)[i * pps + 0] = p.point;
    if (i > 0)
        (*_vertices)[i * pps + 1] = geo->getAt(i-1).point;
    else
        (*_vertices)[i * pps + 1] = p.point;
    
    (*_colors)[i * pps + 0].set(1,1,1,0.5);
    (*_colors)[i * pps + 1].set(1,1,1,0.5);
    
    
    (*_vertices)[i * pps + 2] = p.point;
    (*_vertices)[i * pps + 3] = p.point + (p.hN*d1.x()) + (p.vN*d1.y());
    
    (*_colors)[i * pps + 2].set(1,0,0,0.5);
    (*_colors)[i * pps + 3].set(1,0,0,0.5);
    
    (*_vertices)[i * pps + 4] = p.point;
    (*_vertices)[i * pps + 5] = p.point + (p.hN*d2.x()) + (p.vN*d2.y());
    
    (*_colors)[i * pps + 4].set(0, 1,0,0.5);
    (*_colors)[i * pps + 5].set(0, 1,0,0.5);
    
}

void DebugGenerator::finish(BaseGeometry* geo)
{
    geo->dirtyDisplayList();
    geo->dirtyBound();
}


#pragma mark -

void PolygonSliceGenerator::setup(BaseGeometry* geo)
{
    _vertices = new osg::Vec3Array();
    geo->setVertexArray(_vertices);  
    
    _normals = new osg::Vec3Array();
    geo->setNormalArray(_normals);
    geo->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
     
    _indices = new osg::DrawElementsUShort(GL_TRIANGLES);
    geo->addPrimitiveSet(_indices);

    _points = new osg::DrawArrays(GL_POINTS);
    geo->addPrimitiveSet(_points);


}

void PolygonSliceGenerator::begin(BaseGeometry* geo)
{
    _vertices->resize(geo->getNumLineVertices() * getNumVerticesPerSlice());
    _normals->resize(geo->getNumLineVertices() * getNumVerticesPerSlice());
    
    unsigned num_indices_per_slice = getNumIndicesPerSlice();    
    _indices->resize((geo->getNumLineVertices()-1) * (num_indices_per_slice));
    _points->setCount(_vertices->size());
}


void PolygonSliceGenerator::compute(BaseGeometry* geo, unsigned int i, const Point& p)
{
    const unsigned int pps = getNumVerticesPerSlice();
    const unsigned int np(_polygon.size());
    osg::Quat q;
    q.makeRotate(osg::Y_AXIS, p.up);
        
    for(unsigned int j = 0; j < np; ++j) 
    {
        osg::Vec3 d(
            _polygon[j][0] * p.width * 0.5,
            _polygon[j][1] * p.height * 0.5,
            0);
        d = q * d;
        osg::Vec3 v_x(p.hN * d.x());
        osg::Vec3 v_y(p.vN * d.y());
        
        (*_vertices)[i * pps + j] = p.point + v_x + v_y;
        osg::Vec3 n(p.vN);
        if (pps > 2)
            n = (*_vertices)[i * pps + j] - p.point;
        n.normalize();
        (*_normals)[i * pps + j] = n;
    }
    
    if (i > 0) 
    {
        unsigned int i1,i2,i3,i4;
        
        unsigned int start_ndx = isClosed() ? 0 : 1;
        unsigned int num_indices_per_slice = getNumIndicesPerSlice();
                
        for(unsigned int j=start_ndx; j < _polygon.size(); ++j) 
        {
            unsigned int base_ndx = (i-1) * num_indices_per_slice + (j-start_ndx)*6;
            
            if (isClosed()) {
                i1 = (i-1) * np + j;
                i2 = (i-1) * np + (j+1) % np;
                i3 = (i-0) * np + j;
                i4 = (i-0) * np + (j+1) % np;
            } else {
                i1 = (i-1) * np + j;
                i2 = (i-1) * np + (j-1);
                i3 = (i-0) * np + j;
                i4 = (i-0) * np + (j-1);
            }
            (*_indices)[base_ndx + 0] = i1;
            (*_indices)[base_ndx + 1] = i2;
            (*_indices)[base_ndx + 2] = i3;
            
            (*_indices)[base_ndx + 3] = i3;
            (*_indices)[base_ndx + 4] = i2;
            (*_indices)[base_ndx + 5] = i4;
        }
            
    }
}


void PolygonSliceGenerator::finish(BaseGeometry* geo)
{
    geo->dirtyDisplayList();
    geo->dirtyBound();        
}


}


}