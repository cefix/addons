/*
 *  Tiled2DGraphicsManager.h
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 22.10.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#pragma once

#include <cefix/Quad2DGeometry.h>
#include <cefix/Cache.h>
#include <osg/MatrixTransform>

namespace cefix {



/**
 * Tiled2DGraphics 
 */ 
class Tiled2DGraphics : public osg::MatrixTransform {
public:
	typedef osg::Image::Origin Origin;
   

    class Level;
    class MergeController;
	
    /**
	 * Tile
	 */ 
	class Tile : public osg::Referenced {
	public:
        /// ctor
		Tile(unsigned int x, unsigned int y) : osg::Referenced(), _x(x), _y(y), _parent(NULL), _parentGroup(NULL), _inConstrainingRect(false), _inVisibleList(false) {}
		
        /// sets the rect for this tile
        void setRect(float x, float y, float w, float h) 
		{ 
			_rect.set(x, y, x+w, y+h); 
		}
        /// get the rect
		const osg::Vec4& getRect() const { return _rect; }
		
        /// get the x-index of the grid-placement
        unsigned int getX() const { return _x; }
		
        /// get the y-index of the grid-placement
        unsigned int getY() const { return _y; }
        
        /// get the parent-level
		Level* getParentLevel() const { return _parent; }
		
        // called, when the tile enters the constraining-rect, 
        virtual void show() {}
        
        // called when the tile leaves the constraining-rect (may be called deferred)
        virtual void hide() {}
        
        // called when a tile should cancel all action like loading images etc
        virtual void cancelAnyPreloadingActions() {}
        
        virtual void startAnyPreloadingAction() {}
        
        virtual void mergeWithSceneGraph() {}
        
        /// return true if the tile is insid the constraining-rect
        bool isInConstrainingRect() const { return _inConstrainingRect; }
        
        // debug
        virtual void debug() {}
        
        void requestMergeWithSceneGraph();
        
        Tiled2DGraphics* getRoot() { return _parent->getRoot(); }
    
    protected:
        osg::Group* getParentGroup() { return _parentGroup; }
        
	private:
        void setParentGroup(osg::Group* group) { _parentGroup = group; }
        void setIsInConstrainingRect(bool f) 
        { 
            if (_inConstrainingRect != f) 
            {
                _inConstrainingRect = f;
                if (f) 
                    show();
                else {
                    _inVisibleList = false;
                    hide();
                }
            }
                    
        }
		void setParentLevel(Level* level) { _parent = level; }
        void setIsInVisibleList(bool f) { _inVisibleList = f; }
        bool inVisibleList() const { return _inVisibleList; }
		osg::Vec4 _rect;
		unsigned int _x, _y;
		Level* _parent;
        osg::Group* _parentGroup;
        bool _inConstrainingRect, _inVisibleList;
        
		
	friend class Tiled2DGraphics;
	friend class Level;
	};
	
    /**
     * CreateTileCallback
     * helper class to customize the creation of tiles
     */
	class CreateTileCallback {
	public:
        virtual void init() = 0;
		virtual Tile* createTile(unsigned int x, unsigned int y) = 0;
		virtual void operator()(osg::Group* parent_group, Tile* tile) = 0;
        virtual void clearCache() = 0;
        
		virtual ~CreateTileCallback() {}
        
        Tiled2DGraphics* getTiled2DGraphics() const { return _graphics; }
        void setTiled2DGraphics(Tiled2DGraphics* g) { _graphics = g; }
    private:
        Tiled2DGraphics* _graphics;
	};
	
    
	/**
	 * Level
     * the level-class encapsulates a X x Y tiles in a bunch of groups which are organized as a quad-tree, to get best performance
	 */ 
	class Level : public osg::Group {
	public:
		typedef std::vector<osg::ref_ptr<Tile> > Tiles;
        typedef std::list<Tile*> TilesList;
        /// c'tor
		Level(float zoom_level, unsigned int num_x_tiles, unsigned int num_y_tiles) ;
		
        /// get the zoom-level
        float getZoomLevel() const { return _zoomLevel; }
        
        /// get the level-id (0 is the level with the smalles zoom-level)
        unsigned int getLevelId() const { return _levelId; }
        
        /// get num tiles in x-direction
        unsigned int getNumXTiles() const { return _numXTiles; }
        
        /// get num tiles in y-direction
        unsigned int getNumYTiles() const { return _numYTiles; }
        
        /// get num tiles (x*y)
        unsigned int getNumTiles() const { return _numXTiles * _numYTiles;}
        
        // get the number of tiles of lower levels (without this level)
        unsigned int getNumTilesOfLowerLevels() const { return _numTilesOfLowerLevels; }
        
        Tiled2DGraphics* getRoot() { return _parent; }
        
        Tiles& getTiles() { return _tiles; }
        const Tiles& getTiles() const { return _tiles; }
        
	private:	
		void createTiles(CreateTileCallback& callback);
		void createQuadTree(std::vector<Group*>& result, osg::Group* parent, unsigned int target_level, unsigned int currentlevel);
		void setParent(Tiled2DGraphics* parent) { _parent = parent; }
        
        void getTilesInRect(const osg::Vec4& rect, TilesList& result, bool get_only_hidden_tiles = true);
        void setLevelId(unsigned int in_id) { _levelId = in_id; }
        void setNumTilesOfLowerLevels(unsigned int num) { _numTilesOfLowerLevels = num; }
		
		unsigned int _numXTiles, _numYTiles;
		Tiles _tiles;
		float _zoomLevel;
        unsigned int _potSize;
		
		Tiled2DGraphics* _parent;
        unsigned int _levelId,_numTilesOfLowerLevels;
		
	friend 
		class Tiled2DGraphics;
	};
	
    class TiledCacheItem : public osg::Referenced {
    public:
        TiledCacheItem(Tiled2DGraphics* parent, Tile* tile) 
		:	osg::Referenced(), 
			_parent(parent), 
			_tile(tile)
		{
		}
        Tile* getTile() const { return _tile; }
    protected:
        virtual ~TiledCacheItem();
        
    private:
        Tiled2DGraphics* _parent;
        osg::ref_ptr<Tile> _tile;
    };
    
    /**
     * small controller class helping merging loaded tiles with scenegraph
     */
     
    class MergeController : public osg::Referenced {
    public: 
        typedef std::list<Tile*> Tiles;
        
        MergeController(Tiled2DGraphics* parent, unsigned int num_tiles_to_merge_per_frame);
        
        void addTileToMerge(Tile* tile) 
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            _tiles.push_back(tile);
        }
        
        void merge();
    private:
        Tiled2DGraphics* _parent;
        Tiles _tiles;
        unsigned int _numTilesPerFrame;
        OpenThreads::Mutex _mutex;
    };
	
	
	class UserData : public osg::Referenced {
	public:
		virtual void clearCache() = 0;
	};
    
	typedef std::map<float, Level*> Levels;
    typedef std::map<unsigned int, float> LevelIdsZoomLevelMapping;
    typedef cefix::Cache<Tile*, TiledCacheItem> TilesToHideCache;
    
    enum ClampingPolicy { DoNotClamp, ClampHard, ClampSoft };
    
    class Options {
    public:
        Options() : debug(false), debugTiles(false), numTilesToMergePerFrame(4), tilesCacheLifeTime(5.0f) {}
        
        bool debug, debugTiles;
        unsigned int numTilesToMergePerFrame;
        float tilesCacheLifeTime;
    };
    
    /// ctor
	Tiled2DGraphics(const osg::Vec2& size, const osg::Vec4& constraining_rect, double tile_width = -1, double tile_height = -1, const Options& = Options());
	
    /// get the size
	const osg::Vec2& getSize() const  {return _size; }
    
    /// get the width
    float getWidth() const { return _size[0]; }
    
    /// get the height
    float getHeight() const { return _size[1]; }
    	
    /// add a new level for a specific zoom-level
	Level* addLevel(Level* level);
    
    /// create all tiles for all levels
    void createTiles(CreateTileCallback& callback);
	
    /// set the zoom-level
	void setZoomLevel(float zoom);
	
    /// set the minimum zoom level
    void setMinZoomLevel(float zoom_level) { _minZoomLevel = zoom_level; }
	
    /// set the maximum zoom level
    void setMaxZoomLevel(float zoom_level) { _maxZoomLevel = zoom_level; }
	
    /// get the current zoom level
	float getCurrentZoomLevel() const { return _currentZoomLevel; }
	
    /// get the maximum zoom-level
    float getMinZoomLevel() const { return _minZoomLevel; }
	/// get the minimum zoom level
    float getMaxZoomLevel() const { return _maxZoomLevel; }
	/// compute the minimum zoom level, so that the image is not smaller than the constraining-rect
    float computeMinZoomLevel();
	
    /// get the constraining-rect, all tiles which intersects with the constraining-rect are shown
	const osg::Vec4& getConstrainingRect() const { return _constrainingRect; }
    
    /// set the constraining rect
	void setConstrainingRect(const osg::Vec4& rect) { _constrainingRect = rect;  }
	
    /// get the scroll-delta
	osg::Vec2 getScrollDelta() const { return _scrollDelta; }
	
    /// set the scroll-delta
    void setScrollDelta(osg::Vec2 delta) { _scrollDelta = delta; }
    void forceScrollDelta(osg::Vec2 delta) { _scrollDelta = delta; updateOffset(ClampHard); }
	
    /// create levels suitable for zoomify
	void addLevelsSuitableForZoomify(float tile_width, float tile_height);
	
    /// get the width of a tile (if < 0, the tile width gets computed automatically)
	double getTileWidth() const { return _tileWidth; }
	
    /// get the hright of a tile (if < 0, the tile height gets computed automatically)
    double getTileHeight() const { return _tileHeight; }
	
    /// set the origin-mode
	void setOrigin(Origin origin) { _origin = origin; }
	
    /// get the origin-mode
    Origin getOrigin() const { return _origin; }
    
    void setZoomRegistrationPoint(const osg::Vec2& p);
    const osg::Vec2& getZoomRegistrationPoint() const { return _zoomRegistrationPoint; }
    
    void getTilesContaining(const osg::Vec4& rect, unsigned int level_id, Level::TilesList& result); 
	
    void setClampingPolicy(ClampingPolicy policy) { _clampingPolicy = policy; }
    
    ClampingPolicy getClampingPolicy() const { return _clampingPolicy; }
    
    void clearCache() { _tilesToHideCache->clear(); if (_userData) _userData->clearCache(); }
    
    void setUseCache(bool use_cache) { _useCache = use_cache; if (!use_cache) _tilesToHideCache->clear(); }
    bool isCacheUsed() const { return _useCache; }
    
    const osg::Vec4& getViewPort() const { return _viewportRect; }
	
	Levels& getLevels() { return _levels; }
	Level* getCurrentLevel() { return _currentVisibleLevel; }
	
	void setUserData(UserData* ref) { _userData = ref; }
    
    MergeController* getMergeController() { return _mergeController; }
    
    unsigned int getLastLevelId() const { return _lastLevelId; }
    
    void clearAnyPreloadingActions();
    
    void setEnabledFlag(bool f);
    bool isEnabled() const { return _enabled; }
    
    bool dirty() const { return _dirty; }
    
    void update() {
        updateVisibility();
        _dirty = false;
    }
    
private:
	void updateLevels();
	void updateVisibility();
	void updateTransform();
    void hideOrphanedTiles();
    void updateOffset(ClampingPolicy clampingPolicy);
	void drawDebugInfoForTile(Tile* tile, float di);
	osg::Vec2 _size;
	osg::Vec4 _constrainingRect;
	float _minZoomLevel, _maxZoomLevel, _currentZoomLevel;
	
	Levels _levels;
	Level* _currentVisibleLevel;
	
	osg::Vec2 _scrollDelta;
    Level::TilesList _visibleTiles, _tilesToHide;
    double _tileWidth, _tileHeight;
	
	osg::Image::Origin _origin;
    
    osg::Vec2 _zoomRegistrationPoint, _zoomRegistrationPointNative, _offset;
	
    LevelIdsZoomLevelMapping _idMapping;
    ClampingPolicy _clampingPolicy;
    osg::Vec4 _viewportRect;
    
    osg::ref_ptr<TilesToHideCache> _tilesToHideCache;
    bool _useCache, _debug, _debugTiles, _tilesComputed;
    osg::ref_ptr<UserData> _userData;
    osg::ref_ptr<MergeController> _mergeController;
    unsigned int _lastLevelId;
    bool _enabled, _dirty;
friend
	class Tiled2DGraphicsUpdateCallback;
};

}