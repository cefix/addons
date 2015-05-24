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

#ifndef QUICKTIME_MOVIE_CULL_CALLBACK_HEADER
#define QUICKTIME_MOVIE_CULL_CALLBACK_HEADER

#include <osg/NodeCallback>
#include <osg/observer_ptr>
#include <osgUtil/CullVisitor>
#include <cefix/Log.h>
#include <cefix/QuicktimeMovie.h>
#include <cefix/Mathutils.h>

namespace cefix {

class QuicktimeMovieCullCallback : public osg::NodeCallback {
	private:
		osg::observer_ptr<osg::Node>				_node; 
		osg::observer_ptr<cefix::QuicktimeMovie>	_movie;
		float					_maxDistance;
		bool					_enabled;

	public:
		QuicktimeMovieCullCallback(	osg::Node* node, 
									cefix::QuicktimeMovie* movie) : _node(node), _movie(movie), _maxDistance(-1.0f), _enabled(true) {}
		
		void setMaxDistance(float distance) { _maxDistance = distance; }
		float getMaxDistance() const { return _maxDistance; }
		void resetMaxDistance() { _maxDistance = -1.0f; }
        
        void setMovie(cefix::QuicktimeMovie* movie) { _movie = movie; }
		
		void setEnabledFlag(bool flag) { 
			_enabled = flag; 
			if ((!_enabled) && (_movie.valid()) && (_movie->isPlaying())) 
				_movie->pause();
			
		}
		
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
			
			if ((nv) && (_enabled) &&(_movie.valid()) && (node == _node.get())) {
				osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
				if (cv) {
					osg::Vec3 eye = nv->getEyePoint();
					
					if (_maxDistance > 0.0f) {
						// ok, wir machen eine Art LOD mit dem Movie: wenn er weiter als _maxDistance weg ist, 
						// dann movie stoppen, sonst ggf. starten und lautstärke anpassen
						float scalar = eye.length() / _maxDistance;
						
						if ((scalar > 1.0f) && _movie->isPlaying()) {
							_movie->pause();
						}
						if (scalar <= 1.0f) {
							if (!_movie->isPlaying())
								_movie->play();
							_movie->setVolume(1.0 - scalar);
						}
						
					}
                    //double x_angle = cefix::getAngleBetween(eye, osg::Vec3(0,0,1));
					//osg::notify(osg::ALWAYS) << "eye-point: " << eye << " angle: " << osg::RadiansToDegrees(x_angle) << std::endl; 
				}
			}
		}
	protected:
		virtual ~QuicktimeMovieCullCallback() {} 
};

}

#endif
