/*
 *  VoronoiDiagramGeometry.cpp
 *  cefixSketch
 *
 *  Created by Stephan Huber on 08.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "VoronoiSplineGeometry.h"
#include <cefix/ColorUtils.h>

#include <cefix/Spline.h>

namespace cefix {
VoronoiSplineGeometry::VoronoiSplineGeometry(cefix::VoronoiDiagram* v, bool include_open_regions) 
:	osg::Geometry(), 
    _v(v),
    _includeOpenRegions(include_open_regions) 
{
    _vertices = new osg::Vec3Array();
    setVertexArray(_vertices);
    _edges = new osg::DrawElementsUInt(GL_TRIANGLES);
    addPrimitiveSet(_edges);
    setUseDisplayList(false);
    
    _colors = new osg::Vec4Array();
    setColorArray(_colors);
    setColorBinding(BIND_PER_VERTEX);
}



void VoronoiSplineGeometry::update() 
{
	_vertices->resize(0);
	_edges->resize(0);
	_colors->resize(0);
	
	for(cefix::VoronoiDiagram::RegionMap::iterator i = _v->getRegions().begin(); i != _v->getRegions().end(); ++i) 
	{
		cefix::VoronoiDiagram::Region& r = (*i).second;
		
		osg::Vec4 color = cefix::rgb(0xffffff);
		if (!r.isClosed()) 
			continue;
			
		cefix::Spline3D spline;
		spline.setMode(cefix::BSpline);
		spline.setSplineClosed(true);
		
		osg::Vec3 c = r.getCenter();
		_vertices->push_back(c);
		_colors->push_back(color);
		
		
		unsigned int center_ndx = _vertices->size() - 1;
		
		cefix::LineStripEdge* edge = r.getStartEdge();
		while (edge) {
			//spline.controlPoints.push_back(edge->left());
			while(edge) 
			{								
				osg::Vec3 l(edge->left());
				osg::Vec3 r(edge->right());
				
				l = c +(l-c) * 0.9;
				r = c +(r-c) * 0.9;
				
				spline.controlPoints.push_back(edge->right());
				
				edge = edge->getRightEdge();
			}
			
			edge = r.getNextEdge();
		}
		
		if (spline.controlPoints.size() < 3) 
			continue;
	
	
		spline.buildArclenTable();     // build the arc length table
		spline.normalizeArclenTable();
		 
		cefix::Spline3D::ArclenTable::iterator ai= spline.arclentable.begin();
		bool first(true);
		osg::Vec3 previous_vertex;
		while (ai != spline.arclentable.end())
		{
			if (!first) {
				_vertices->push_back(previous_vertex);
				_vertices->push_back(ai->pos);
				
				
				_colors->push_back(color);
				_colors->push_back(color);
				
				_edges->push_back(center_ndx);
				_edges->push_back(_vertices->size() - 1);
				_edges->push_back(_vertices->size() - 2);
				
			}
			first = false;
			previous_vertex = ai->pos;
			++ai;
		}
		
	}
	
	dirtyBound();

}

}