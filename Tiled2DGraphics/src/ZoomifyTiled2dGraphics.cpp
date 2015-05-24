/*
 *  ZoomifyTiledGraphics.cpp
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 25.10.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "ZoomifyTiled2DGraphics.h"
#include <cefix/StringUtils.h>
#include <cefix/tinyxml.h>
#include <cefix/log.h>
#include <cefix/MathUtils.h>
#include <cefix/Pixel.h>
#include <cefix/DebugGeometryFactory.h>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>
#include <cefix/SharedThreadPool.h>
#include <cefix/AllocationObserver.h>

namespace cefix {


ZoomifyTiled2DGraphics::ImageLoader::Job::Job(Tile* tile)
:   ImageBasedTiled2DGraphics::ImageProviderJob(tile)
{
}


void ZoomifyTiled2DGraphics::ImageLoader::Job::acquireImage()
{
    _img = osgDB::readImageFile(static_cast<ZoomifyTiled2DGraphics::Tile*>(_tile)->getFileName(), _options.get());
    _previewImg = _img.get();
}


ZoomifyTiled2DGraphics::ImageLoader::ImageLoader(unsigned int num_threads, osgDB::Options* options)
:   ImageBasedTiled2DGraphics::ImageProvider(num_threads),
    _options(options)
{
}

void ZoomifyTiled2DGraphics::ImageLoader::add(Job* job) 
{
    job->setOptions(_options);
    ImageBasedTiled2DGraphics::ImageProvider::add(job);
}



#pragma mark -

ZoomifyTiled2DGraphics::Tile::Tile(unsigned int x, unsigned int y) 
:	cefix::ImageBasedTiled2DGraphics::Tile(x,y), 
	_geo(NULL) 
{
	/*
	static unsigned int num_tiles(0);
	std::cout << num_tiles << " => " << num_tiles*sizeof(Tile) << std::endl;
	num_tiles++;
	*/
}

void ZoomifyTiled2DGraphics::Tile::show() 
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




void ZoomifyTiled2DGraphics::Tile::hide() {
    getParentGroup()->removeChildren(0, getParentGroup()->getNumChildren());
    cancelAnyPreloadingActions();
    _geo = NULL;
	_image = NULL;
}





std::string ZoomifyTiled2DGraphics::Tile::computeFilename(const std::string& base_path) const
{
    unsigned int x = getX();
    unsigned int y = getY();
    unsigned int num_tiles = (getParentLevel()->getNumTilesOfLowerLevels() + (y * getParentLevel()->getNumXTiles() + x)) / 256.0;
    
    std::string filename = base_path+"TileGroup"+cefix::intToString(num_tiles)+"/"+cefix::intToString(getParentLevel()->getLevelId()) + "-" + cefix::intToString(x) + "-" + cefix::intToString(y) + ".jpg";
    
    /*
	if (!osgDB::fileExists(filename)) {
        std::cout << "could not find " << _filename << " num_tiles: " << num_tiles << std::endl;
    }
	*/
	
	return filename;
}


void ZoomifyTiled2DGraphics::Tile::debug() 
{
    bool states[7] = { _requested, _cancelled, _error, getFallbackThumbTexture(), getThumbTexture(), _image.valid(), _job.valid() };
    float s = getParentLevel()->getZoomLevel();
    osg::Vec2 p= cefix::Rect::getTopLeft(getRect()) + osg::Vec2(20/s, 20/s);
    for(unsigned int i = 0; i < 7; ++i) {
        cefix::DebugGeometryFactory::get("tiles_2d")->addPoint(p, osg::Vec4(!states[i], states[i], 0, 0.5));
        p += osg::Vec2(15/s,0);
    }
}


#pragma mark -


cefix::Tiled2DGraphics* ZoomifyTiled2DGraphics::create(const std::string& filename, const osg::Vec4& constraining_rect, const Options& options)
{
    unsigned int num_threads = options.numLoadingThreads;
    
    std::string file = osgDB::findDataFile(filename);
    if (file.empty())
        return NULL;
    std::string base_path = osgDB::getFilePath(filename);
    
    // ggf. zip öffnen
    std::string archive_file_name = base_path.empty() ? "ImageTiles.zip" : base_path+"/ImageTiles.zip";
   
    osg::ref_ptr<osgDB::Archive> archive = osgDB::openArchive(archive_file_name, osgDB::Archive::READ);
    if (archive.valid()) {
        osgDB::Registry::instance()->addToArchiveCache("Archiv.zip", archive);
        num_threads = 1;
    }
        
    
    ZoomifyCreateCallback z_cb(base_path.empty() ? "" : base_path+"/", num_threads, options.fileOptions);
    Tiled2DGraphics::CreateTileCallback* cb( (options.callback) ? options.callback : &z_cb);
    
    
    TiXmlDocument doc;
    if (!doc.LoadFile(file)) {
        cefix::log::error("ZoomifyTiled2DGraphics") << "could not parse xml-file" << std::endl;
        return NULL;
    }
    
    int w, h, tile_size;
    doc.RootElement()->QueryIntAttribute("WIDTH", &w);
    doc.RootElement()->QueryIntAttribute("HEIGHT", &h);
    doc.RootElement()->QueryIntAttribute("TILESIZE", &tile_size);
    
    
    osg::ref_ptr<cefix::Tiled2DGraphics> tiledGraphics = new cefix::Tiled2DGraphics(osg::Vec2(w, h), constraining_rect, tile_size, tile_size, options);
    tiledGraphics->setUseCache(options.useCache);
    tiledGraphics->setOrigin(osg::Image::TOP_LEFT);
    tiledGraphics->addLevelsSuitableForZoomify(tile_size, tile_size);
    tiledGraphics->createTiles(*cb);
    return tiledGraphics.release();
}


}