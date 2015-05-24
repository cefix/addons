/*
 *  VoronoiDiagramGeometry.h
 *  cefixSketch
 *
 *  Created by Stephan Huber on 08.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef VORONOI_DIAGRAM_GEOMETRY_HEADER
#define VORONOI_DIAGRAM_GEOMETRY_HEADER

#include <osg/Geometry>
#include "VoronoiDiagram.h"

namespace cefix {

class VoronoiDiagramGeometry : public osg::Geometry {

public:
	VoronoiDiagramGeometry(VoronoiDiagram* v, float scale = 1.0, bool include_open_regions = true) 
	:	osg::Geometry(), 
		_v(v),
        _scale(scale),
		_includeOpenRegions(include_open_regions) 
	{
		_vertices = new osg::Vec3Array();
		setVertexArray(_vertices);
		_edges = new osg::DrawElementsUInt(GL_LINES);
		addPrimitiveSet(_edges);
		setUseDisplayList(false);
        
    }
	
	void update();
	
	void setIncludeOpenRegions(bool f) { _includeOpenRegions = f; }
protected:
	osg::ref_ptr<VoronoiDiagram> _v;
	osg::Vec3Array* _vertices;
	osg::DrawElementsUInt* _edges;
    float _scale;
	bool	_includeOpenRegions;
};
}

#endif