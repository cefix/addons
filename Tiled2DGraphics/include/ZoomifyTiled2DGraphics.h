/*
 *  ZoomifyTiledGraphics.h
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 25.10.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */
 
#pragma once

#include "Tiled2DGraphics.h"
#include <cefix/ThreadPool.h>
#include <osgDB/Options>
#include <osgDB/ReadFile>

#include "ImageBasedTiled2DGraphics.h"

namespace cefix {


class ZoomifyTiled2DGraphics {
public:
    class Tile;
    class ImageLoader : public ImageBasedTiled2DGraphics::ImageProvider {
    public:
		
        class Job : public ImageBasedTiled2DGraphics::ImageProviderJob {
        
        public:
            Job(Tile* tile);
            osg::Image* getPreviewImage() { return _previewImg; }
            void setOptions(osgDB::Options* options) { _options = options; }
        
        protected:
            virtual void acquireImage();
            
            osg::ref_ptr<osg::Image> _previewImg;
            osg::ref_ptr<osgDB::Options> _options;
        };
        
        ImageLoader(unsigned int num_threads, osgDB::Options* options = NULL);
        
        void add(Job* job);

    private:
        osg::ref_ptr<osgDB::Options> _options;
    };
	
	class UserData : public ImageBasedTiled2DGraphics::UserData {
	public:
		void setBasePath(const std::string& path) { _basePath = path; }
		const std::string& getBasePath() const { return _basePath; }
		
	private:
		std::string _basePath;
	};

    class Tile : public cefix::ImageBasedTiled2DGraphics::Tile {
    public:
        Tile(unsigned int x, unsigned int y);
      
        virtual void show();        
        virtual void hide();
        
        std::string computeFilename(const std::string& base_path) const;
        std::string getFileName() const { return computeFilename(static_cast<UserData*>(_data)->getBasePath()); }
		
		virtual void debug();
        
        virtual ImageBasedTiled2DGraphics::ImageProviderJob* createJob() {  return new ImageLoader::Job(this); }
        
        virtual void mergeWithSceneGraph()
        {
            if(_geo && _job) 
            {
                _geo->setTextureFromImage(_job->getImage(), false);
                _geo->setTextureRect(0,0,1,1);
                
                applyTextureToChildTiles(_geo->getTexture2D());
                _image = _job->getImage();
            }
            _job = NULL;
        }

    private:
        cefix::Quad2DGeometry* _geo;
        
    };


    class ZoomifyCreateCallback : public Tiled2DGraphics::CreateTileCallback {
    public:
        ZoomifyCreateCallback(const std::string& base_path, unsigned int num_threads = 2, osgDB::Options* options = NULL) 
        :   cefix::Tiled2DGraphics::CreateTileCallback(),
            _basePath(base_path)
        {
            _data = new UserData();
			_data->setBasePath(_basePath);
			_data->setThumbCache(new UserData::ThumbCache());
			_preview = osgDB::readImageFile(base_path + "TileGroup0/0-0-0.jpg", options);
            
            _previewTex = UserData::createTexture(_preview);
            _data->setImageProvider(new ImageLoader(num_threads, options));
        }
        
        virtual void init() 
        {
            getTiled2DGraphics()->setUserData(_data);
			_referenceRect.set(0,0, getTiled2DGraphics()->getWidth(), getTiled2DGraphics()->getHeight());
        }
        
        virtual cefix::Tiled2DGraphics::Tile* createTile(unsigned int x, unsigned int y) { 
            return new ZoomifyTiled2DGraphics::Tile(x,y); 
        }
        
        virtual void operator()(osg::Group* parent_group, cefix::Tiled2DGraphics::Tile* tile)
        {
            static_cast<ZoomifyTiled2DGraphics::Tile*>(tile)->setThumb(_previewTex, osg::Vec4(0, 0, 1,1), _referenceRect, true, 0);
            static_cast<ZoomifyTiled2DGraphics::Tile*>(tile)->setUserData(_data);
        }
        
        virtual void clearCache() {
            _data->clearCache();
        }
    
    private:
        std::string _basePath;
        osg::ref_ptr<osg::Image> _preview;
        osg::ref_ptr<osg::Texture2D> _previewTex;
		osg::ref_ptr<UserData> _data;
        osg::Vec4 _referenceRect;

    };
    
    class Options: public ImageBasedTiled2DGraphics::Options {
    public:
        Options(): ImageBasedTiled2DGraphics::Options(), fileOptions(NULL), callback(NULL) {}
        
        osg::ref_ptr<osgDB::Options> fileOptions;
        Tiled2DGraphics::CreateTileCallback* callback; 
    };
    
    static Tiled2DGraphics* create(const std::string& filename, const osg::Vec4& constraining_rect, const Options& options = Options());
    
private:
     ZoomifyTiled2DGraphics() {}

};

}