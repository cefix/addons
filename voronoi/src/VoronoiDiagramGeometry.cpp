/*
 *  VoronoiDiagramGeometry.cpp
 *  cefixSketch
 *
 *  Created by Stephan Huber on 08.09.10.
 *  Copyright 2010 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "VoronoiDiagramGeometry.h"
namespace cefix {

void VoronoiDiagramGeometry::update() 
{
	_vertices->resize(0);
    _edges->resize(0);
	
	for(VoronoiDiagram::RegionMap::iterator i = _v->getRegions().begin(); i != _v->getRegions().end(); ++i) 
	{
		cefix::VoronoiDiagram::Region& region = (*i).second;
		
		if (!region.isClosed() && (_includeOpenRegions == false)) 
			continue;
		
        cefix::LineStripEdge* edge = region.getStartEdge();
		while (edge) {
			osg::Vec3 c (region.getCenter());
			
			while(edge) 
			{								
				osg::Vec3 l(edge->left());
				osg::Vec3 r(edge->right());
				
                l = c + (l-c) * _scale;
                r = c + (r-c) * _scale;

				//_voronoiVertices->push_back(l-delta);
				_vertices->push_back(l);
				_vertices->push_back(r);
                            
				_edges->push_back(_vertices->size()-2);
				_edges->push_back(_vertices->size()-1);
				
				edge = edge->getRightEdge();
			}
			
			edge = region.getNextEdge();
		}
	}
   
}

}