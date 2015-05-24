/*
 *
 *      (\(\
 *     (='.')           cefix --
 *    o(_")")           a multipurpose library on top of OpenSceneGraph
 *  
 *
 *                      Copyright 2001-2011, stephanmaximilianhuber.com 
 *                      Stephan Maximilian Huber. 
 *
 *                      All rights reserved.
 *
 */

#ifndef TRANSFORM_VERTICES_VISITOR_HEADER
#define TRANSFORM_VERTICES_VISITOR_HEADER
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/NodeVisitor>

namespace cefix {
/** this NodeVisitor will transform all found vertices with a matrix */
class TransformVerticesVisitor : public osg::NodeVisitor {
	
	public:
		/** ctor */
		TransformVerticesVisitor(osg::NodeVisitor::TraversalMode mode = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) :
			osg::NodeVisitor(mode),
			_m(osg::Matrix::identity()),
			_transformNormals(true)
		{
			
		}
		/** sets the matrix */
		void setMatrix(osg::Matrix m) { _m = m; }
		
		void setTransformNormals(bool f) { _transformNormals = f; }
		
		/** we are interested in geodes and their drawables */
		virtual void apply(osg::Geode& geode) {
		
			for (unsigned int i = 0; i < geode.getNumDrawables(); ++i) {
				osg::Geometry* geo = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
				if (geo) {
					transformArray(dynamic_cast<osg::Vec3Array*>(geo->getVertexArray()));
					if (_transformNormals) transformArray(dynamic_cast<osg::Vec3Array*>(geo->getNormalArray()));
					geo->dirtyDisplayList();
					geo->dirtyBound();
				}
			}
		}
		
	private:
	
		void transformArray(osg::Vec3Array* a) {
			if (a == NULL) return;
			
			for(unsigned int j = 0; j < a->size(); ++j) {
				(*a)[j] = _m * (*a)[j];
			}
		
		}
		
		osg::Matrix _m;
		bool		_transformNormals;

};

}

#endif