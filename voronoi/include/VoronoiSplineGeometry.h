/*
 *  VoronoiDiagramGeometry.h
 *  cefixSketch
 *
 *  Created by Stephan Huber on 08.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef VORONOI_3D_GEOMETRY_HEADER
#define VORONOI_3D_GEOMETRY_HEADER

#include <osg/Geometry>
#include "VoronoiDiagram.h"
#include <cefix/Pixel.h>

namespace cefix {

class VoronoiSplineGeometry : public osg::Geometry {

public:
	VoronoiSplineGeometry(cefix::VoronoiDiagram* v, bool include_open_regions = true) ;
	
	void update();
	
	void setIncludeOpenRegions(bool f) { _includeOpenRegions = f; }
protected:
	osg::ref_ptr<cefix::VoronoiDiagram> _v;
	osg::Vec3Array* _vertices;
	osg::Vec4Array* _colors;
	osg::DrawElementsUInt* _edges;
	bool _includeOpenRegions;
};

}

#endif