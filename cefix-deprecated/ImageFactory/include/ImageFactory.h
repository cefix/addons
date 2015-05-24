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

#ifndef __IMAGE_FACTORY_HEADER__
#define __IMAGE_FACTORY_HEADER__

#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec4>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <cefix/Export.h>


namespace cefix {
	class ImageFactoryMap;
	/**
	 * this class implements a simple image cache
	 *
	 * @author Stephan Maximilian Huber
	 */
	class CEFIX_EXPORT ImageFactory : public osg::Referenced {
		public:
        
            enum RenderPlane { 
                PLANE_X_Z,  /**< renders the image on the XZ-Plane */
                PLANE_X_Z_NEG, /**< renders the image on the XZ-plane to the opposite */
                PLANE_X_Y,   /**< renders the image on the XY-Plane */
                PLANE_Y_Z   /**< renders the image on the YZ-Plane */
            };
			
			/** Constructor */
			ImageFactory();
			
			/** @return the ImageFactory-singleton */
			static ImageFactory* instance();
			
			/**
			 * loads an image and returns a texture-object
			 * @param textureFileName the file to open
			 * @return a newly created texture
			 */
            osg::Texture* getTexture(std::string &textureFileName);
			
			/**
			 * creates a quad and use a texture to texture ist
			 * @param tex Texture to use
			 * @param origin origin of the quad
			 * @param imgSourceRect source-rect of the image (you can clip the image here)
			 * @scalar scalar by this scalar the quads get scaled
			 * @return a newly created quad as geometry
			 */
            osg::Geometry* getImage(osg::Texture* tex, osg::Vec3 origin, osg::Vec4 imgSourceRect, float scalar = 1.0f);
            
			/**
			 * creates a quad and use a texture to texture ist
			 * @param tex Texture to use
			 * @param origin origin of the quad
			 * @param imgSourceRect source-rect of the image (you can clip the image here)
			 * @scalar scalar by this scalar the quads get scaled
			 * @return a newly created quad as geometry
			 */
            osg::Geometry* getImage(osg::Texture* tex, osg::Vec2 origin, osg::Vec4 imgSourceRect, float scalar = 1.0f) {
                return getImage(tex,osg::Vec3(origin.x(), origin.y(), 0), imgSourceRect, scalar);
            }
			
			/**
			 * creates a quad and loads an image which is applied as a texture
			 * @param textureFileName image to load
			 * @param origin origin of the quad
			 * @param imgSourceRect source-rect of the image (you can clip the image here)
			 * @scalar scalar by this scalar the quads get scaled
			 * @return a newly created quad as geometry
			 */
            osg::Geometry* getImage(std::string textureFileName, osg::Vec3 origin, osg::Vec4 imgSourceRect, float scalar = 1.0f ) {
				//osg::Texture* tex = NULL;//;
				//osg::Geometry* geo = getImage(tex, origin, rect, scalar);
				return getImage(getTexture(textureFileName), origin, imgSourceRect, scalar);
            }
			
			/**
			 * creates a quad and loads an image which is applied as a texture
			 * @param textureFileName image to load
			 * @param origin origin of the quad
			 * @param imgSourceRect source-rect of the image (you can clip the image here)
			 * @scalar scalar by this scalar the quads get scaled
			 * @return a newly created quad as geometry
			 */
            inline osg::Geometry* getImage(std::string textureFileName, osg::Vec2 origin, osg::Vec4 imgSourceRect, float scalar = 1.0f) {
                return getImage(getTexture(textureFileName), origin, imgSourceRect, scalar);
            }
            
			/**
			 * creates a quad and loads an image which is applied as a texture
			 * @param textureFileName image to load
			 * @scalar scalar by this scalar the quads get scaled
			 * @return a newly created quad as geometry
			 */
            osg::Geometry* getImage(std::string textureFileName, float scalar = 1.0f) {
                osg::Texture* tex = getTexture(textureFileName);
                
                if (!tex) 
                    return NULL;
                
                float w,h;
                if (_useAbsoluteTexCoords) {
                    osg::TextureRectangle* tr = dynamic_cast<osg::TextureRectangle*>(tex);
                    w = tr->getImage()->s() * scalar;
                    h = tr->getImage()->t() * scalar;
                } else {
                    w = 1;
                    osg::Texture2D* t2d = dynamic_cast<osg::Texture2D*>(tex);
                    h = t2d->getImage()->t() / t2d->getImage()->s();
                }
                
                return getImage(tex,osg::Vec2(-(w/2.0f), -(h/2.0f)), osg::Vec4(0,0,w, h),scalar);
            }
			/**
			 * set this flag to true, if you want to use absolute texture coordinates and rectangular tectures
			 */
			void setUseAbsoluteTextCoordsFlag(bool aflag) { _useAbsoluteTexCoords = aflag; }
            
            /**
             sets the rendering-plane, usually X - Y -plane
            */
            void setRenderPlane(RenderPlane plane) {
                _renderPlane = plane;
            }
			
		protected:
			virtual ~ImageFactory();
			
			ImageFactoryMap* _textures;
			bool _useAbsoluteTexCoords;
            RenderPlane _renderPlane;
	};

} // namespace
#endif
