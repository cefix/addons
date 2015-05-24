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

#ifndef GEOMETRY_COLOR_BLEND_CULL_CALLBACK_HEADER_
#define GEOMETRY_COLOR_BLEND_CULL_CALLBACK_HEADER_

#include <osg/NodeCallback>
#include <osgUtil/CullVisitor>
#include <osg/observer_ptr>
#include <cefix/Mathutils.h>

namespace cefix {

class GeometryColorBlendCullCallback : public osg::NodeCallback {
	private:
		osg::observer_ptr<osg::Node>		_node; 
		osg::observer_ptr<osg::Vec4Array>	_colors;
		float								_maxDistance, _maxBlend;

	public:
		GeometryColorBlendCullCallback(	osg::Node* node, 
										osg::Vec4Array* colors, 
										float maxDistance, 
										float maxBlend) 
		: _node(node), _colors(colors), _maxDistance(maxDistance), _maxBlend(maxBlend) {}
		
		void setMaxDistance(float distance) { _maxDistance = distance; }
		float getMaxDistance() const { return _maxDistance; }
		void resetMaxDistance() { _maxDistance = -1.0f; }
        
        void setColors(osg::Vec4Array* colors) { _colors = colors; }
		
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
			
			if ((nv) && (_colors.valid()) && (node == _node.get())) {
				osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
				if (cv) {
					osg::Vec3 eye = nv->getEyePoint();
					
					if (_maxDistance > 0.0f) {
						// ok, wir machen eine Art LOD mit dem Movie: wenn er weiter als _maxDistance weg ist, 
						// dann movie stoppen, sonst ggf. starten und lautstärke anpassen
						float scalar = osg::clampTo(eye.length() / _maxDistance, 0.0f, 1.0f);
						(*_colors)[0][3] = _maxBlend * (1 - scalar);
						
					}
				}
			}
		}
	protected:
		~GeometryColorBlendCullCallback() {}

};

}

#endif
