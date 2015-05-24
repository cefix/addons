//
//  PDFTiled2DGraphics.h
//  Tiled2DGraphics
//
//  Created by Stephan Maximilian Huber on 18.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include "ImageBasedTiled2DGraphics.h"
#include <cefix/MathUtils.h>

namespace cefix {

class PDFAssetImpl;

class PDFAsset : public osg::Referenced {
public:

    PDFAsset(const std::string& filename): osg::Referenced(), _filename(filename) { createImpl(); }
    
    const osg::Vec4& getMediaRect() const;
    
    const std::string& getFileName() const { return _filename; }
    unsigned int getNumPages() const; 
    void renderInto(osg::ref_ptr<osg::Image>& img, const osg::Vec4d& rect, float dpi = 72);

protected:
    ~PDFAsset();
    
private:
    void createImpl();
    std::string _filename;
    PDFAssetImpl* _impl;
    
};


class PDFTiled2DGraphics {
public:
    class Tile;
    
    class ImageProvider : public ImageBasedTiled2DGraphics::ImageProvider {
    
    public:
        class Job : public ImageBasedTiled2DGraphics::ImageProviderJob {
        public:
            Job(Tile* tile) : ImageBasedTiled2DGraphics::ImageProviderJob(tile), _asset(NULL) {}
            virtual void acquireImage();
            void setPDFAsset(PDFAsset* asset) { _asset = asset; }
        private:
            PDFAsset* _asset;
        };
    
        ImageProvider(unsigned int num_threads, PDFAsset* asset) : ImageBasedTiled2DGraphics::ImageProvider(num_threads), _asset(asset) {}
        
        virtual void add(ImageBasedTiled2DGraphics::ImageProviderJob* job);
    private:
        osg::ref_ptr<PDFAsset> _asset;
    };
    
    class Tile : public ImageBasedTiled2DGraphics::Tile {
    public:
        Tile(unsigned int w, unsigned h) : ImageBasedTiled2DGraphics::Tile(w, h) {}
        virtual ImageBasedTiled2DGraphics::ImageProviderJob* createJob() { return new ImageProvider::Job(this); }
        virtual void show();        
        virtual void hide();        
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
    
    class UserData : public ImageBasedTiled2DGraphics::UserData {
    public:
        UserData() : ImageBasedTiled2DGraphics::UserData(), _asset() {}
        void setPDFAsset(PDFAsset* asset) { _asset = asset; }
        PDFAsset* getPDFAsset() const { return _asset; }
        
    private:
        osg::ref_ptr<PDFAsset> _asset; 
    };
    
    class CreateTileCallback : public Tiled2DGraphics::CreateTileCallback {
    public:
        CreateTileCallback(PDFAsset* asset, unsigned int num_threads = 2) 
        :   cefix::Tiled2DGraphics::CreateTileCallback()
        {
            _data = new UserData();
			_data->setPDFAsset(asset);
			_data->setThumbCache(new UserData::ThumbCache());
            
            osg::Vec4 media_rect = asset->getMediaRect();
            float w = cefix::Rect::getWidth(media_rect);
            float h = cefix::Rect::getHeight(media_rect);
            
            asset->renderInto(_preview, osg::Vec4d(0, 0, w, h));
            
            _previewTex = UserData::createTexture(_preview);
            _data->setImageProvider(new ImageProvider(num_threads, asset));
        }
        
        virtual void init() 
        {
            getTiled2DGraphics()->setUserData(_data);
			_referenceRect.set(0,0, getTiled2DGraphics()->getWidth(), getTiled2DGraphics()->getHeight());
        }
        
        virtual cefix::Tiled2DGraphics::Tile* createTile(unsigned int x, unsigned int y) { 
            return new Tile(x,y); 
        }
        
        virtual void operator()(osg::Group* parent_group, cefix::Tiled2DGraphics::Tile* tile)
        {
            static_cast<Tile*>(tile)->setThumb(_previewTex, osg::Vec4(0, 0, 1,1), _referenceRect, true, 0);
            static_cast<Tile*>(tile)->setUserData(_data);
        }
        
        virtual void clearCache() {
            _data->clearCache();
        }
    
    private:
        osg::ref_ptr<osg::Image> _preview;
        osg::ref_ptr<osg::Texture2D> _previewTex;
		osg::ref_ptr<UserData> _data;
        osg::Vec4 _referenceRect;

    };
    
    class Options : public ImageBasedTiled2DGraphics::Options {
    public:
        Options() : ImageBasedTiled2DGraphics::Options(), maxDpi(600), callback(NULL) {}
        
        float maxDpi;
        Tiled2DGraphics::CreateTileCallback* callback;
    };
    
    static Tiled2DGraphics* create(const std::string& filename, const osg::Vec4& constraining_rect, const Options& options = Options());
private:
    PDFTiled2DGraphics() {}

};


}

