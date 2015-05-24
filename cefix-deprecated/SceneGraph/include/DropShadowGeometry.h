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

#ifndef CEFIX_DROP_SHADOW_GEOMETRY_HEADER
#define CEFIX_DROP_SHADOW_GEOMETRY_HEADER

#include <osgDB/ReadFile>
#include <cefix/Resizable9x9TexturedGeometry.h>

namespace cefix {
	
	class DropShadowGeometry : public Resizable9x9TexturedGeometry {
	public:
		DropShadowGeometry(const osg::Vec4& r, float locz, float fw, const std::string& shadowFileName, float shadowWidth = 0) 
		:	Resizable9x9TexturedGeometry(osg::Vec4(r[0] - fw/2, r[1] - fw/2, r[2] + fw/2, r[3] + fw/2), locz, fw, fw)
		{
			float d ( (shadowWidth != 0) ? shadowWidth : fw*2);
			setTexFrameWidthAndHeight(d,d);
			
			osg::Image* img = osgDB::readImageFile(shadowFileName);
			if (img) {
				setTextureFromImage(img, false);
				setTextureRect(osg::Vec4(0,0,1,1) );
			}
			
			setColor(osg::Vec4(0,0,0,1));
			getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		}
		
		virtual void setRect(const osg::Vec4& r, bool resettexRect=false) {
			float fw = _frameWidth /4.0;
			Resizable9x9TexturedGeometry::setRect(osg::Vec4(r[0] - fw, r[1] - fw, r[2] + fw, r[3] + fw), resettexRect);
		}
	};
}

#endif