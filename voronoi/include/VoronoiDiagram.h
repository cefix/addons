/*
 *  VoronoiDiagram.h
 *  livevoronoi
 *
 *  Created by Stephan Huber on 03.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef CEFIX_VORONOI_DIAGRAM_HEADER_
#define CEFIX_VORONOI_DIAGRAM_HEADER_

#include <map>
#include <osgUtil/DelaunayTriangulator>
#include <cefix/MathUtils.h>
#include "LineStripEdge.h"

namespace cefix {
			
class VoronoiDiagram : public osg::Referenced {
	
public:
	/// defines a region by a center and one or more linestrip surrounding the center
	class Region : public cefix::LineStripEdgeGroup {
	public:
		Region();
		Region(const osg::Vec3& center) : cefix::LineStripEdgeGroup(),  _center(center) {}

		const osg::Vec3& getCenter() { return _center; }
		inline void setCenter(const osg::Vec3& c) {_center = c;  }	
			
	private:
		osg::Vec3 _center;
	};
	
	typedef std::map<osg::Vec3, Region> RegionMap; 
	
	/// ctor
	VoronoiDiagram(const osg::BoundingBox& bb = osg::BoundingBox());
	
	/// computes the voronoi-diagram
	void compute(osg::Vec3Array* points, osg::PrimitiveSet* ps);
	
	void compute(osg::Vec3Array* points) 
	{
		_delaunay = new osgUtil::DelaunayTriangulator();
		_delaunay->setInputPointArray(points);
		_delaunay->triangulate();
		_triangles = _delaunay->getTriangles();
		compute(points, _triangles.get());
	}
	
	osg::DrawElementsUInt* getTriangles() const { return _triangles.get(); }
	
	
	
	// get the number of regions
	unsigned int getNumRegions() const  { return _regions.size(); }
	
	// get the region for a specific center
	inline bool hasRegionFor(const osg::Vec3& p) {
		return (_regions.find(p) != _regions.end());
	}
	inline Region* getRegionFor(const osg::Vec3& p) { 
		RegionMap::iterator itr = _regions.find(p);
		if (itr == _regions.end())
			_regions.insert(std::make_pair(p,Region(p)));
		return (&_regions[p]); 
	} 
	
	/// get the region-map
	inline RegionMap& getRegions() { return _regions; }

	
	osg::BoundingBox& getBoundingBox() { return _bbox; }
	bool getComputeBounds() { return _computeBounds; }
	void setComputeBounds(bool b) { _computeBounds = b; }
	
	void setDumpInfosForPoint(const osg::Vec3 p) { _dumpInfosForPoint = p; }
	
private:
	
	RegionMap	_regions;
	osg::BoundingBox	_bbox;
	bool				_computeBounds;		
	osg::Vec3			_dumpInfosForPoint;
	
	osg::ref_ptr<osgUtil::DelaunayTriangulator> _delaunay;
	osg::ref_ptr<osg::DrawElementsUInt> _triangles;

};



}


#endif