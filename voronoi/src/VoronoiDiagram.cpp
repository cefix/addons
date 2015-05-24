/*
 *  VoronoiDiagram.cpp
 *  livevoronoi
 *
 *  Created by Stephan Huber on 03.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */
 
//#define VORONOI_DEBUG
//#define VORONOI_DEBUG_CLIPPING
//#define VORONOI_DEBUG_DELAUNAY
//#define VORONOI_DEBUG_EDGES

#include "VoronoiDiagram.h"
 
#include <cefix/DebugGeometryFactory.h>
#include <limits>
#include <osg/TriangleIndexFunctor>
#include <stdint.h>
#include <cefix/StringUtils.h>

namespace cefix {



	
VoronoiDiagram::Region::Region()
:	cefix::LineStripEdgeGroup(),
	_center()
{
	//std::cout << "new region" << std::endl; 
}

VoronoiDiagram::VoronoiDiagram(const osg::BoundingBox& bb)
:	osg::Referenced(),
	_bbox(bb),
	_computeBounds(!bb.valid()),
	_delaunay(new osgUtil::DelaunayTriangulator())
{
}



	
	
class Triangle {

public:
	static osg::Vec3 s_dump_infos_for_point;
	Triangle(const osg::Vec3& p1, const osg::Vec3& p2, const osg::Vec3& p3) 
	{
		/*
		osg::Vec3 n = (p2-p1) ^ (p3-p1);
		n.normalize();
		osg::Vec3 sh1 = (p1+p2) / 2.0;
		osg::Vec3 sh2 = (p2+p3) / 2.0;
		
		osg::Vec3 sh1n = (p2-p1) ^ n;
		osg::Vec3 sh2n = (p3-p2) ^ n;
		
		sh1n.normalize();
		sh2n.normalize();
		
		osg::Vec3 center;
		
		#if defined(VORONOI_DEBUG_DELAUNAY)
		if ((s_dump_infos_for_point == p1) || (s_dump_infos_for_point == p2) || (s_dump_infos_for_point == p3)){
			cefix::DebugGeometryFactory::get("voronoi")->addPoint(sh1, osg::Vec4(0,1,0,1));
			cefix::DebugGeometryFactory::get("voronoi")->addPoint(sh2, osg::Vec4(0,1,0,1));
		}
		#endif
				
		
		if (getLineLineIntersection(sh1, sh1 + sh1n, sh2, sh2 + sh2n, center)) 
		{	
			_valid = true;
			_centroid = center;
			
		} else {
			_valid = false;	
		}
		*/
		
		_centroid = (p1+p2+p3) / 3.0;
		_valid = true;
		
		#if defined(VORONOI_DEBUG_DELAUNAY)
		if ((s_dump_infos_for_point == p1) || (s_dump_infos_for_point == p2) || (s_dump_infos_for_point == p3)){
			if (_valid) {
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(_centroid, osg::Vec4(0,0,0,0.5));
				cefix::DebugGeometryFactory::get("voronoi")->addLine(sh1, _centroid, osg::Vec4(0,1,0,0.5));
				cefix::DebugGeometryFactory::get("voronoi")->addLine(sh2, _centroid, osg::Vec4(0,1,0,0.5));
			} else {
				cefix::DebugGeometryFactory::get("voronoi")->addLine(p1, p2, osg::Vec4(0,_valid,1,1.0));
				cefix::DebugGeometryFactory::get("voronoi")->addLine(p1, p3, osg::Vec4(0,_valid,1,1.0));
				cefix::DebugGeometryFactory::get("voronoi")->addLine(p3, p2, osg::Vec4(0,_valid,1,1.0));
			}
		}
		#endif

	}
	
	void setIndices(unsigned int i1, unsigned int i2, unsigned int i3) { 
		_i1 = i1; _i2 = i2; _i3 = i3; 
	}
	
	unsigned int getOppositeIndex(unsigned int a, unsigned int b) {
	
		if (_i1 == a) {
			if (b == _i2) return _i3;
			if (b == _i3) return _i2;
		} else if (_i2 == a) {
			if (b == _i1) return _i3;
			if (b == _i3) return _i1; 
		} else if (_i3 == a) {
			if (b == _i1) return _i2;
			if (b == _i2) return _i1;
		}
		
		return 0;
	}
	
	const osg::Vec3& getCentroid() { return _centroid; }
	bool isValid() const { return _valid; }
	
private:
	osg::Vec3 _centroid;
	bool		_valid;
	unsigned int _i1, _i2, _i3; 
};


osg::Vec3 Triangle::s_dump_infos_for_point = osg::Vec3(0,0,0);

struct CollectTriangleOperator
{
	
	typedef std::vector<Triangle*> TrianglesVector;
	typedef std::map<unsigned int, TrianglesVector> IndicesMap;
	typedef std::map<unsigned long long, std::pair<Triangle*, Triangle*> > EdgeMap;

    CollectTriangleOperator():_vd(0) {}
	
	~CollectTriangleOperator() 
	{
		for(TrianglesVector::iterator i = _triangles.begin(); i != _triangles.end(); ++i) {
			delete (*i);
		}
	}

    void setVoronoiDiagram(VoronoiDiagram* vd) { _vd = vd; }
    
	inline void operator()(unsigned int ndx1, unsigned int ndx2, unsigned int ndx3) {
		
		if ((ndx1==ndx2) || (ndx2==ndx3) || (ndx1==ndx3)) {
			//degeneriertes dreick
			return;
		}
		Triangle::s_dump_infos_for_point = _dumpInfosForPoint;
		
		Triangle* t = new Triangle((*_points)[ndx1], (*_points)[ndx2], (*_points)[ndx3]);
		
		#if defined(VORONOI_DEBUG_DELAUNAY)
			int ndxs[3] = { ndx1, ndx2, ndx3};
			osg::Vec3 pts[3] = { (*_points)[ndx1], (*_points)[ndx2], (*_points)[ndx3] };
			for(unsigned k = 0; k < 3; ++k) 
			{
				if (pts[k] == Triangle::s_dump_infos_for_point){
					cefix::DebugGeometryFactory::get("voronoi")->addText(pts[k], cefix::intToString(ndxs[k]), osg::Vec4(1,1,1,0.5), "system.xml", 2);
				}
			}

		#endif

		t->setIndices(ndx1, ndx2, ndx3);
		if (t->isValid()) {
			_triangles.push_back(t);
			
			addNeighbors(ndx1, ndx2, t);
			addNeighbors(ndx2, ndx3, t);
			addNeighbors(ndx3, ndx1, t);
		} else {
			delete t;
		}
	}
	
	void addNeighbors(unsigned long ndx1, unsigned long ndx2, Triangle* t) {
		
		unsigned long long edgeNdx = (ndx1 < ndx2) ? ((long long)ndx1 << 16) + ndx2 : ((long long)ndx2 << 16) + ndx1;
		// std::cout << std::hex << edgeNdx << " " << ndx1 << "+" << ndx2 << std::endl;
		EdgeMap::iterator i = _edges.find(edgeNdx);
		if (i != _edges.end()) {
			_edges[edgeNdx].second = t;
		}
		else
			_edges[edgeNdx] = std::pair<Triangle*, Triangle*>(t, NULL);
	}
	
	virtual void setVertexArray(osg::Vec3Array* v) { _points = v; }

    void computeRegions(osg::BoundingBox& bbox);
	
	void setVertices(osg::Vec3Array* v) { _points = v; }
	
	inline bool clipLineOnPlane(const osg::Plane& plane, osg::Vec3& v1, osg::Vec3& v2) 
	{	
		double dist1 = plane.distance(v1);
		double dist2 = plane.distance(v2);
		if (dist1*dist2 >= 0) 
		{
			#if defined(VORONOI_DEBUG_CLIPPING)
			cefix::DebugGeometryFactory::get("voronoi")->addPoint(v1, osg::Vec4(1,0,0,1));
			cefix::DebugGeometryFactory::get("voronoi")->addPoint(v2, osg::Vec4(1,0,0,1));
			cefix::DebugGeometryFactory::get("voronoi")->addLine(v1, v2, osg::Vec4(1,0,0,1));
			#endif
			return false; // punkte schneiden diese plane nicht
		}
		osg::Vec3 vLineDir = v2-v1;
		vLineDir.normalize();
		if (dist1 > 0) {
			double numerator = -dist1;
			double denominator = plane.getNormal() * vLineDir;
			
			if (0.0 != denominator)  {
				#if defined(VORONOI_DEBUG_CLIPPING)
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v1, osg::Vec4(1,0,0,1));
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v2, osg::Vec4(0,1,0,1));
				#endif
				double dist = numerator/denominator;
				v1 = v1 + (vLineDir*dist);
				#if defined(VORONOI_DEBUG_CLIPPING)
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v1, osg::Vec4(1,1,0,1));
				#endif
			}
		}
		
		if (dist2 > 0) {
			double numerator = -dist2;
			double denominator = plane.getNormal() * vLineDir;
			
			if (0.0 != denominator)  {
				#if defined(VORONOI_DEBUG_CLIPPING)
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v1, osg::Vec4(0,1,1,1));
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v2, osg::Vec4(1,0,1,1));
				#endif
				double dist = numerator/denominator;
				v2 = v2 + (vLineDir*dist);
				#if defined(VORONOI_DEBUG_CLIPPING)
				cefix::DebugGeometryFactory::get("voronoi")->addPoint(v2, osg::Vec4(1,1,1,1));
				#endif
			}
		}
		return true;
	}

	osg::Vec3 _dumpInfosForPoint;
	
private:
	VoronoiDiagram* _vd;   
	TrianglesVector	_triangles;
	osg::ref_ptr<osg::Vec3Array> _points;
	EdgeMap			_edges;
	
	

};
	
	
void CollectTriangleOperator::computeRegions(osg::BoundingBox& bbox) 
{
	VoronoiDiagram::RegionMap& regions = _vd->getRegions();
	
	std::vector<osg::Plane> planes;
	
	if (bbox.valid()) {
		planes.push_back( osg::Plane( osg::Vec3(-1, 0, 0), osg::Vec3(bbox.xMin(), 0, 0) ) );
		planes.push_back( osg::Plane( osg::Vec3( 1, 0, 0), osg::Vec3(bbox.xMax(), 0, 0) ) );
		planes.push_back( osg::Plane( osg::Vec3( 0,-1, 0), osg::Vec3(0, bbox.yMin(), 0) ) );
		planes.push_back( osg::Plane( osg::Vec3( 0, 1, 0), osg::Vec3(0, bbox.yMax(), 0) ) );
		
		planes.push_back( osg::Plane( osg::Vec3( 0, 0,-1), osg::Vec3(0, 0, bbox.zMin()) ) );
		planes.push_back( osg::Plane( osg::Vec3( 0, 0, 1), osg::Vec3(0, 0, bbox.zMax()) ) );
	}
	
	osg::notify(osg::INFO) << "# triangles " << _triangles.size() << " # edges: " << _edges.size() << " # points: " << _points->size() << std::endl;
	
	// alle edges durchgehen und die centroiden der beiden 
	while(!_edges.empty()) 
	{
		EdgeMap::iterator itr = _edges.begin();
		Triangle* t1( itr->second.first );
		Triangle* t2( itr->second.second );
		unsigned long long ndx = itr->first;
		ndx = ndx & 0xffff0000;
		ndx = ndx >> 16;
		unsigned int ndx1 = ndx;
		unsigned int ndx2 = (itr->first & 0x0000ffff);
		osg::Vec3 p1( (*_points)[ndx1]);
		osg::Vec3 p2( (*_points)[ndx2]);
		

		VoronoiDiagram::RegionMap::iterator ritr = regions.find(p1);
		
		if (ritr == regions.end()) {
			regions.insert(std::make_pair(p1,VoronoiDiagram:: Region(p1)));
		}
		
		ritr = regions.find(p2);
		if (ritr == regions.end()) {
			regions.insert(std::make_pair(p2, VoronoiDiagram::Region(p2)));
		}	
		
		osg::Vec3 res1, res2;
		
		if (t2 == NULL) {
			// kein benachbartes dreieck vorhanden, also am rand
			osg::Vec3 c = t1->getCentroid();
			osg::Vec3 sh = (p1 + p2) / 2;
			
			osg::Vec3d p3 = (*_points)[t1->getOppositeIndex(ndx1, ndx2)];
			osg::Vec3d v = (p2 - p1) ^ (p3 - p1);
			v.normalize();
			osg::Vec3d n = v ^ (p2 - p1); // n steht jetzt senkrecht auf p1p2 und sollte von p3 wegzeigen
			n.normalize();
			#if defined(VORONOI_DEBUG_DELAUNAY)
			if ((_dumpInfosForPoint == p1) || (_dumpInfosForPoint == p2)){
				cefix::DebugGeometryFactory::get("voronoi")->addNormal(sh, -n*30, osg::Vec4(1,0,0,1));
			}
			#endif
			
			osg::Vec3 result = sh + (-n*10000);
						
			res1 = c;
			res2 = result;
		} else {
			res1 = t1->getCentroid();
			res2 = t2->getCentroid();
		}
		bool outside = false;
		
		for(std::vector<osg::Plane>::iterator p = planes.begin(); p != planes.end(); ++p) {
			double dist1 = (*p).distance(res1);
			double dist2 = (*p).distance(res2);
			if ((dist1 > 0) && (dist2 > 0)) {
				outside = true;
				break;
			}
			if (!outside && (dist1*dist2 < 0)) 
				clipLineOnPlane( *p, res1, res2);
		}
		
		if (!outside) {
			regions[p1].addEdge(res1, res2);
			regions[p2].addEdge(res1, res2);
		}
		
		#if defined(VORONOI_DEBUG_EDGES)
		if ((_dumpInfosForPoint == p1) || (_dumpInfosForPoint == p2)){
			regions[_dumpInfosForPoint].setDebug(true);
			std::cout << "adding " << res1 << " - " << res2 << std::endl;
			std::cout << "result " << regions[_dumpInfosForPoint] << std::endl << std::endl;
			cefix::DebugGeometryFactory::get("voronoi")->addText(res1, cefix::intToString(res1[0]) + "/" + cefix::intToString(res1[1]), osg::Vec4(1,1,1,0.3), "system.xml", 2);
			cefix::DebugGeometryFactory::get("voronoi")->addText(res2, cefix::intToString(res2[0]) + "/" + cefix::intToString(res2[1]), osg::Vec4(1,1,1,0.3), "system.xml", 2);
		}
		#endif

	
		
		#if defined(VORONOI_DEBUG_EDGES)
		//cefix::DebugGeometryFactory::get("voronoi")->addLine(p1 + (res1-p1) * 0.95 , p1 + (res2-p1) * 0.95, osg::Vec4(1,outside,0,1));
		//cefix::DebugGeometryFactory::get("voronoi")->addLine(p2 + (res1-p2) * 0.95 , p2 + (res2-p2) * 0.95, osg::Vec4(0,outside,1,1));
		#endif
		_edges.erase(itr);
	}
	#if defined(VORONOI_DEBUG_EDGES)
		std::cout << (regions[_dumpInfosForPoint]) << std::endl;
	#endif
	
}
	
	
	



void VoronoiDiagram::compute(osg::Vec3Array* points, osg::PrimitiveSet* ps) {

	_regions.clear(); 
	
	if (_computeBounds) {
		for(osg::Vec3Array::iterator itr = points->begin(); itr != points->end(); ++itr) {
			_bbox.expandBy(*itr);
		}
	}
	
	typedef osg::TriangleIndexFunctor<CollectTriangleOperator> CollectTriangleFunctor;
	
	CollectTriangleFunctor collectTriangles;
	collectTriangles._dumpInfosForPoint = _dumpInfosForPoint;
	collectTriangles.setVertices(points);
    collectTriangles.setVoronoiDiagram(this);
	
	if (ps)
		ps->accept(collectTriangles);
	
	collectTriangles.computeRegions(_bbox);

}




}