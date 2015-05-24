//
//  ImageBasedTiled2DGraphics.cpp
//  Tiled2DGraphics
//
//  Created by Stephan Maximilian Huber on 18.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "ImageBasedTiled2DGraphics.h"
#include <cefix/SharedThreadPool.h>
#include <cefix/AllocationObserver.h>


namespace cefix {

ImageBasedTiled2DGraphics::ImageProvider::ImageProvider(unsigned int num_threads)
:   osg::Referenced()
{
    _worker = (num_threads > 1) ? cefix::SharedThreadPool::instance() : new cefix::ThreadPool(num_threads);
}

#pragma mark - 

void ImageBasedTiled2DGraphics::ImageProviderJob::operator()()
{
    if (!_cancelled) {
        acquireImage();
        //OpenThreads::Thread::microSleep(1000*1000);
    }
    if (!_cancelled && _img.valid() && _tile->getRoot()->isEnabled())
        _tile->requestMergeWithSceneGraph();  
    else 
        callOnMainThread(_tile, &Tile::jobGotCancelled, this);  
}


#pragma mark - 

ImageBasedTiled2DGraphics::Tile::Tile(unsigned int x, unsigned int y) 
:   Tiled2DGraphics::Tile(x,y) 
{
}





void ImageBasedTiled2DGraphics::Tile::startAnyPreloadingAction() 
{
    // image ggf laden
    if (!_image && !_job) {
        _requested = true;
        _cancelled = _error = false;
        _job = createJob();
        _data->getImageProvider()->add(_job);
    }
}

void ImageBasedTiled2DGraphics::Tile::setThumb(osg::Texture2D* texture, const osg::Vec4& in_tex_rect, const osg::Vec4& reference_rect, bool is_fallback, unsigned int from_zoom_level) 
{ 
    bool save_to_fallback = is_fallback || (from_zoom_level <= _data->getMaxZoomLevelForFallbackThumb());
    
    if(!is_fallback) {
        _data->getThumbCache()->add(this, texture); 
    }
    
	if (save_to_fallback)	
		_fallbackThumb = texture; 
		
    osg::Vec4 rect = getRect();
    osg::Vec4 tex_rect;
    float s_w = cefix::Rect::getWidth(in_tex_rect) / cefix::Rect::getWidth(reference_rect);
    float s_h = cefix::Rect::getHeight(in_tex_rect) / cefix::Rect::getHeight(reference_rect);
    
    tex_rect[0] = in_tex_rect[0] + (rect[0] - reference_rect[0]) * s_w;
    tex_rect[2] = in_tex_rect[0] + (rect[2] - reference_rect[0]) * s_w;
    tex_rect[1] = in_tex_rect[1] + (rect[1] - reference_rect[1]) * s_h;
    tex_rect[3] = in_tex_rect[1] + (rect[3] - reference_rect[1]) * s_h;
    
	if(!is_fallback)
		_thumbTexRect = tex_rect;
	
    if (save_to_fallback)
		_fallBackThumbTexRect = tex_rect; 
}



void ImageBasedTiled2DGraphics::Tile::cancelAnyPreloadingActions() 
{ 
    if (_job) {
        _job->cancel(); 
        _cancelled = true;
    }
    _job = NULL; 
}


void ImageBasedTiled2DGraphics::Tile::applyTextureToChildTiles(osg::Texture2D* tex) 
{
    std::list<Tiled2DGraphics::Tile*> tiles;
    unsigned int start_level = getParentLevel()->getLevelId()+1;
    unsigned int end_level = getRoot()->getLastLevelId();
    for(unsigned int i = start_level; i <= end_level; ++i)
        getRoot()->getTilesContaining(getRect(), i, tiles);
    
    for(std::list<Tiled2DGraphics::Tile*>::iterator i = tiles.begin(); i != tiles.end(); ++i) {
        if(cefix::Rect::contains(getRect(), (*i)->getRect())) {
            static_cast<ImageBasedTiled2DGraphics::Tile*>(*i)->setThumb(tex, osg::Vec4(0,0,1,1), getRect(), false, getParentLevel()->getLevelId());
        }
    }
}


ImageBasedTiled2DGraphics::Tile::~Tile() 
{
    cancelAnyPreloadingActions();
}






osg::Texture2D* ImageBasedTiled2DGraphics::UserData::createTexture(osg::Image* img) 
{
    osg::Texture2D* tex = new osg::Texture2D(img);
    tex->setResizeNonPowerOfTwoHint(false);
#if defined(OSG_GLES1_AVAILABLE)
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
#endif
    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    
    cefix::AllocationObserver::instance()->observe(tex);
    return tex;
}

}