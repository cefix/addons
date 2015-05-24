//
//  PDFTiled2DGraphics.cpp
//  Tiled2DGraphics
//
//  Created by Stephan Maximilian Huber on 18.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "PDFTiled2DGraphics.h"
#include <cefix/ColorUtils.h>
#include <cefix/Pixel.h>
#include <osg/Geode>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>


#ifdef __APPLE__
#include "PDFAssetAppleImpl.h"
#endif


namespace cefix {



#pragma mark -

PDFAsset::~PDFAsset()
{
    delete _impl;
}
  
const osg::Vec4& PDFAsset::getMediaRect() const { return _impl->getMediaRect(); }

unsigned int PDFAsset::getNumPages() const { return _impl->getNumPages(); }

void PDFAsset::renderInto(osg::ref_ptr<osg::Image>& img, const osg::Vec4d& rect, float dpi)
{
    _impl->renderInto(img, rect, dpi);
}

void PDFAsset::createImpl() 
{
    _impl = new PDFAssetImpl(this);
}

#pragma mark - 

void PDFTiled2DGraphics::ImageProvider::Job::acquireImage()
{
    float layer_scale = _tile->getParentLevel()->getZoomLevel();
    osg::Vec4d tile_rect(_tile->getRect());
    osg::Vec4 full_rect(0,0, _tile->getRoot()->getWidth(), _tile->getRoot()->getHeight());
    osg::Vec4 pdf_rect(_asset->getMediaRect());
    double scale = cefix::Rect::getWidth(full_rect) / cefix::Rect::getWidth(pdf_rect);
    _asset->renderInto(_img, tile_rect / scale, 72 * scale * layer_scale);
}

#pragma mark - 


void PDFTiled2DGraphics::ImageProvider::add(ImageBasedTiled2DGraphics::ImageProviderJob* job) 
{
    static_cast<Job*>(job)->setPDFAsset(_asset);
    ImageBasedTiled2DGraphics::ImageProvider::add(job);
}

#pragma mark - 

void PDFTiled2DGraphics::Tile::show() 
{
    osg::Geode* geode = new osg::Geode();
    _geo = new cefix::Quad2DGeometry(getRect());
    geode->addDrawable(_geo);
    getParentGroup()->addChild(geode);    
    
    if(_image) {
        _geo->setTextureFromImage(_image, false);
		_geo->setTextureRect(osg::Vec4(0,0,1,1));
    }
    else if (osg::ref_ptr<osg::Texture2D> tex = getThumbTexture()) {
		_geo->setTexture(tex);
		_geo->setTextureRect(_thumbTexRect);
    }
	else {
		_geo->setTexture(_fallbackThumb);
		_geo->setTextureRect(_fallBackThumbTexRect);

	}
    
    
}




void PDFTiled2DGraphics::Tile::hide() 
{
    getParentGroup()->removeChildren(0, getParentGroup()->getNumChildren());
    cancelAnyPreloadingActions();
    _geo = NULL;
	_image = NULL;
}

#pragma mark - 

cefix::Tiled2DGraphics* PDFTiled2DGraphics::create(const std::string& filename, const osg::Vec4& constraining_rect, const Options& options)
{
    unsigned int num_threads = options.numLoadingThreads;
    
    std::string file = osgDB::findDataFile(filename);
    if (file.empty())
        return NULL;
    std::string base_path = osgDB::getFilePath(filename);
    
    osg::ref_ptr<PDFAsset> asset = new PDFAsset(filename);
    
    CreateTileCallback z_cb(asset, num_threads);
    Tiled2DGraphics::CreateTileCallback* cb( (options.callback) ? options.callback : &z_cb);
    
    osg::Vec4 rect = asset->getMediaRect();
    unsigned int tile_size = 256;
    float w = cefix::Rect::getWidth(rect) * options.maxDpi / 72.0;
    float h = cefix::Rect::getHeight(rect) * options.maxDpi / 72.0;
    
    osg::ref_ptr<cefix::Tiled2DGraphics> tiledGraphics = new cefix::Tiled2DGraphics(osg::Vec2(w, h), constraining_rect, tile_size, tile_size, options);
    tiledGraphics->setUseCache(options.useCache);
    tiledGraphics->setOrigin(osg::Image::TOP_LEFT);
    tiledGraphics->addLevelsSuitableForZoomify(tile_size, tile_size);
    tiledGraphics->createTiles(*cb);
    return tiledGraphics.release();
}

}