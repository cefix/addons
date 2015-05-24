/*
 *  ImageFactory.cpp
 *  NativeContext
 *
 *  Created by Stephan Huber on Wed May 28 2003.
 *  Copyright (c) 2003 digital mind. All rights reserved.
 *
 */

#include <string>
#include <map>
#include <cefix/Log.h>
#include <osg/StateSet>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Texture2D>
#include <osg/ImageStream>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/TextureRectangle>

#include <cefix/ImageFactory.h>
#include <cefix/Settings.h>


namespace cefix {

    // ---------------------------------------------------------------------------
    // ImageFactoryMap
    // Zuordnung Texturefilename, textur-Objekt
    // ---------------------------------------------------------------------------
    class ImageFactoryMap : public std::map<std::string, osg::ref_ptr<osg::Texture> > {
		public:
			ImageFactoryMap(bool useAbsolute) : _useAbsoluteTexCoords(useAbsolute) {};
			
			inline osg::Texture* get(std::string &fileName);
			
		protected:
			bool _useAbsoluteTexCoords;
	
	};
	
    // ---------------------------------------------------------------------------
    // get
    // erzeugt ggf. textur, falls noch nicht bereits geladen und liefert diese zurück
    // ---------------------------------------------------------------------------
	osg::Texture* ImageFactoryMap::get(std::string &fileName) {
		
        std::string fileFound = fileName;
        
        if (!osgDB::fileExists(fileFound)) {
            fileFound = osgDB::findDataFile(fileName);
            if (fileFound.empty()) {
                log::error("ImageFactoryMap::get") << "could not locate " << fileName << std::endl;
                return NULL;
            }
        }
		
		iterator itr = find(fileFound);
		if (itr != end())
			return itr->second.get();
		
		// so, muessen datei jetzt erstmal laden, gibts noch nicht
		log::info("ImageFactoryMap::get") << "loading " << fileFound << std::endl;
		
		osg::Image* image = osgDB::readImageFile(fileFound);
        
        if (!image) 
            return NULL;
        
		osg::ref_ptr<osg::Texture> texture;
		
		if (_useAbsoluteTexCoords) {
            osg::TextureRectangle* t = new osg::TextureRectangle();
            t->setImage(image);
            texture = t;
        } else {
            osg::Texture2D* t = new osg::Texture2D();
            t->setImage(image);
            texture = t;
        }
        
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		insert(std::pair<std::string, osg::ref_ptr<osg::Texture> >(fileFound, texture));
		
		return texture.get();
	}
	
	#pragma mark -
    
	// ---------------------------------------------------------------------------
	// Konstruktor
	// ---------------------------------------------------------------------------
	ImageFactory::ImageFactory() : 
        osg::Referenced(), 
        _useAbsoluteTexCoords(true),
        _renderPlane(PLANE_X_Y)
    
    {
       
        _useAbsoluteTexCoords = cefix::Settings::instance()->getImageFactoryUseTextureRectangle();
		_textures = new ImageFactoryMap(_useAbsoluteTexCoords);
		
	}


	// ---------------------------------------------------------------------------
	// Destruktor
	// ---------------------------------------------------------------------------
	ImageFactory::~ImageFactory() {
	
		_textures->clear();
		delete _textures;
		
	}
		
	// ---------------------------------------------------------------------------
	// instance 
	// liefert statische Instanz zurück
	// ---------------------------------------------------------------------------
	ImageFactory* ImageFactory::instance() {
	
		static osg::ref_ptr<ImageFactory> s_imageFactoryPtr = new ImageFactory();		
		return s_imageFactoryPtr.get();
	}
    
    // ---------------------------------------------------------------------------
    // getTexture
    // ---------------------------------------------------------------------------
	osg::Texture* ImageFactory::getTexture(std::string &textureFileName) {
        return _textures->get(textureFileName);
    }
    
	// ---------------------------------------------------------------------------
	// getImage
	// ---------------------------------------------------------------------------

	osg::Geometry* ImageFactory::getImage(osg::Texture* tex, osg::Vec3 origin, osg::Vec4 arect, float scalar) {
	
		if (!tex)
			return NULL;

		
		osg::Geometry* geo = new osg::Geometry;
		osg::Vec3Array* v = new osg::Vec3Array(4);
        osg::Vec3Array* n = new osg::Vec3Array(1);

        
        osg::Image* img = NULL;
		
		if (dynamic_cast<osg::TextureRectangle*>(tex)) {
			osg::TextureRectangle* t = dynamic_cast<osg::TextureRectangle*>(tex);
			img = t->getImage();
		} else {
			osg::Texture2D* t = dynamic_cast<osg::Texture2D*>(tex);
			img = t->getImage();
		}
		
        osg::Vec4 rect(arect);
        
        if (rect[2] == 0) rect[2] = img->s();
        if (rect[3] == 0) rect[3] = img->t();
		
		float w = (rect[2] - rect[0]) * scalar;
		float h = (rect[3] - rect[1]) * scalar;
        switch (_renderPlane) {
            case PLANE_X_Y:
                (*v)[0].set(origin[0]+ w, origin[1], origin[2]);
                (*v)[1].set(origin[0], origin[1], origin[2] );
                (*v)[2].set(origin[0], origin[1] + h, origin[2] );
                (*v)[3].set(origin[0]+ w, origin[1] + h, origin[2] );
                (*n)[0].set(osg::Vec3(0,0,-1));
                break;
            
            case PLANE_X_Z_NEG:
                (*v)[0].set(origin[0] + w, origin[1], origin[2]);
                (*v)[1].set(origin[0], origin[1], origin[2] );
                (*v)[2].set(origin[0], origin[1], origin[2] + h );
                (*v)[3].set(origin[0] + w, origin[1], origin[2] + h );
                (*n)[0].set(osg::Vec3(0,-1,0));
                break;
            case PLANE_X_Z:
                (*v)[0].set(origin[0], origin[1], origin[2]);
                (*v)[1].set(origin[0] + w, origin[1], origin[2] );
                (*v)[2].set(origin[0] + w, origin[1], origin[2] + h );
                (*v)[3].set(origin[0], origin[1], origin[2] + h );
                (*n)[0].set(osg::Vec3(0,1,0));
                break;
                
            case PLANE_Y_Z:
                (*v)[0].set(origin[0], origin[1]+ w, origin[2]);
                (*v)[1].set(origin[0], origin[1], origin[2] );
                (*v)[2].set(origin[0], origin[1], origin[2] + h );
                (*v)[3].set(origin[0], origin[1]+ w, origin[2] + h );
                (*n)[0].set(osg::Vec3(1,0,0));
                break;
        }
		
		geo->setVertexArray(v);
		geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,v->size()));
        
       geo->setNormalArray(n);
        geo->setNormalBinding(osg::Geometry::BIND_OVERALL);
        
		osg::Vec2Array* texCoords = new osg::Vec2Array;
		texCoords->resize(4);
		
     
		(*texCoords)[0].set(rect[2], img->t() - rect[3]);
		(*texCoords)[1].set(rect[0], img->t() - rect[3]);
		(*texCoords)[2].set(rect[0], img->t() - rect[1]);
		(*texCoords)[3].set(rect[2], img->t() - rect[1]);
		
        // ist das ggf. ein movie?
        if (dynamic_cast<osg::ImageStream*>(img)) {
            (*texCoords)[3].set(rect[2], img->t() - rect[3]);
            (*texCoords)[2].set(rect[0], img->t() - rect[3]);
            (*texCoords)[1].set(rect[0], img->t() - rect[1]);
            (*texCoords)[0].set(rect[2], img->t() - rect[1]);
        }

		
		// ggf. TexturKoordinaten normalisieren
		if (dynamic_cast<osg::Texture2D*>(tex))
			for (osg::Vec2Array::iterator i = texCoords->begin(); i != texCoords->end(); i++) {
			
				(*i)[0] /= img->s();
				(*i)[1] /= img->t();
			}
		
		geo->setTexCoordArray(0, texCoords);
        
        osg::Vec4Array* c = new osg::Vec4Array();
        c->push_back(osg::Vec4(1,1,1,1));
        geo->setColorArray(c);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);
		
        osg::StateSet* state = geo->getOrCreateStateSet();
        state->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
		
		if (dynamic_cast<osg::TextureRectangle*>(tex)) {
			tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
			tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER );
			tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER );
		}
        /*
        state->setMode(GL_BLEND,osg::StateAttribute::ON);
        state->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        
        state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        osg::Depth* depth = new osg::Depth();
        depth->setWriteMask(false);
        state->setAttribute(depth);
        
        osg::BlendFunc* b = new osg::BlendFunc();
        b->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        state->setAttribute(b);

		geo->setStateSet(state);
        */
		
		return geo;
		
		
	}
    

}