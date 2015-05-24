//
//  ImageBasedTiled2DGraphics.h
//  Tiled2DGraphics
//
//  Created by Stephan Maximilian Huber on 18.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include <cefix/ThreadPool.h>
#include <cefix/MathUtils.h>
#include "Tiled2DGraphics.h"

namespace  cefix {


class ImageBasedTiled2DGraphics {
public:
    // forward declaration
    class Tile;
    
   
    
    
    /**
     * a threadpool-job-class which handles the generation of an image for a specific tile, this is the base-class, inherited classes
     * have to implement the real logiv of image-creation
     */
    class ImageProviderJob : public cefix::ThreadPool::Job {
    public:
        ImageProviderJob(Tile* tile)
        :   cefix::ThreadPool::Job(),
            _cancelled(false),
            _tile(tile)
        {
        }
        
        virtual void operator()();
        
        void cancel() { _cancelled = true; }
        bool isCancelled() const { return _cancelled; }
        osg::Image* getImage() { return _img; }
    
    protected:
        virtual void acquireImage() = 0;
    
        Tile* _tile;
        osg::ref_ptr<osg::Image> _img;
        bool _cancelled;
    };
    
    
    /**
     * a thread-pool class handling generating of the various images for the tiled2dgraphics
     */
    class ImageProvider : public osg::Referenced {
    public:
	    ImageProvider(unsigned int num_threads);
        
        virtual void add(ImageProviderJob* job) {
            _worker->addJob(job);
        }

    private:
        osg::ref_ptr<cefix::ThreadPool> _worker;
    
    };
    
    /** 
     * a more specific user-data class
     * storing a reference to the imageprovider and to a thumbnail-texture-cache
     */
    class UserData : public Tiled2DGraphics::UserData {
	public:
		typedef cefix::Cache<Tile*, osg::Texture2D> ThumbCache;
		
        UserData() : Tiled2DGraphics::UserData(), _imageProvider(), _thumbCache(), _maxZoomLevelForFallbackThumb(0) {}
		
        void setImageProvider(ImageProvider* ip) { _imageProvider = ip; }		
        ImageProvider* getImageProvider() const { return _imageProvider; }
		
		ThumbCache* getThumbCache() { return _thumbCache; }
        void setThumbCache(ThumbCache* cache) { _thumbCache = cache; }
		
		virtual void clearCache() { _thumbCache->clear(); }
        
        static osg::Texture2D* createTexture(osg::Image* img);
        
        unsigned int getMaxZoomLevelForFallbackThumb() { return _maxZoomLevelForFallbackThumb; }
        void setMaxZoomLevelForFallbackThumb(unsigned int i) { _maxZoomLevelForFallbackThumb = i; }
	
    private:
		osg::ref_ptr<ImageProvider> _imageProvider;
		osg::ref_ptr<ThumbCache> _thumbCache; 
		unsigned int _maxZoomLevelForFallbackThumb;
	};
    
     /**
     * custom tile class providing a jobGotCancelled-method called from the job
     */
    class Tile : public Tiled2DGraphics::Tile {
    public:
        Tile(unsigned int x, unsigned int y);
                
        virtual void startAnyPreloadingAction();
        
        virtual void cancelAnyPreloadingActions();
        
        void setThumb(osg::Texture2D* texture, const osg::Vec4& in_tex_rect, const osg::Vec4& reference_rect, bool is_fallback, unsigned int from_zoom_level);
        
        void applyTextureToChildTiles(osg::Texture2D* tex) ;
        
        virtual void jobGotCancelled(ImageBasedTiled2DGraphics::ImageProviderJob* job)
        {
            if (job->isCancelled())
                _cancelled = true;
            else if(job->getImage() == NULL )
                _error = true;
        }
        
        osg::Texture2D* getFallbackThumbTexture() { return _fallbackThumb.get(); }
        osg::Texture2D* getThumbTexture() { return _data->getThumbCache()->get(this); }
		osg::Image* getImage() { return _image.get(); }
        
        void setUserData(UserData* data) { _data = data; }
        
    protected:
        ~Tile();
        virtual ImageProviderJob* createJob() = 0;
        osg::ref_ptr<osg::Image> _image;   
        osg::ref_ptr<osg::Texture2D> _fallbackThumb;
        osg::Vec4 _fallBackThumbTexRect, _thumbTexRect; 
        
        osg::ref_ptr<ImageProviderJob> _job;
        bool _requested, _cancelled, _error;
        
        UserData* _data;
        
    };
    
    class Options: public Tiled2DGraphics::Options {
    public:
        Options(): Tiled2DGraphics::Options(), numLoadingThreads(2), useCache(true) {}
        
        unsigned int numLoadingThreads;
        bool useCache;
    };
 
    

    
};

}