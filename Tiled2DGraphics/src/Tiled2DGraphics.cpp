/*
 *  Tiled2DGraphicsManager.cpp
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 22.10.11.
 *  Copyright 2011 Digital Mind. All rights reserved.
 *
 */

#include "Tiled2DGraphics.h"
#include <sstream>
#include <cefix/MathUtils.h>
#include <cefix/AnimationFactory.h>
#include <cefix/Quad2DGeometry.h>
#include <cefix/DebugGeometryFactory.h>
#include <osg/io_utils>


namespace cefix {


template<typename T>
T nextPowerOfTwo(T n) {
    --n;
    for(T k=1;!(k&(1<<(sizeof(n)+1)));k<<=1) n|=n>>k;
    return ++n;
}


#pragma mark -


/**************************************************************************
 * Tiled2DGraphicsUpdateAnimation
 **************************************************************************/
 
class Tiled2DGraphicsUpdateCallback : public osg::NodeCallback {
public:
	Tiled2DGraphicsUpdateCallback(Tiled2DGraphics* parent)
    :   osg::NodeCallback(), 
        _parent(parent) 
    {
    }
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		traverse(node, nv);
        if (_parent.valid()) {
			if (_parent->dirty())
                _parent->update();
        }
	}
private:
	osg::observer_ptr<Tiled2DGraphics> _parent;
};

#pragma mark -

void Tiled2DGraphics::Tile::requestMergeWithSceneGraph() 
{
    if (getRoot() && getRoot()->getMergeController())
        getRoot()->getMergeController()->addTileToMerge(this);
}


#pragma mark -

/**************************************************************************
 * 
 **************************************************************************/
   
Tiled2DGraphics::Level::Level(float zoom_level, unsigned int num_x_tiles, unsigned int num_y_tiles) 
:	osg::Group(), 
	_numXTiles(num_x_tiles),
	_numYTiles(num_y_tiles),
    _zoomLevel(zoom_level)
{
}



/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::Level::createQuadTree(std::vector<Group*>& result, osg::Group* parent,unsigned int target_level, unsigned int current_level)
{
	osg::Group* tl = new osg::Group();
	osg::Group* tr = new osg::Group();
	osg::Group* bl = new osg::Group();
	osg::Group* br = new osg::Group();
	
	parent->addChild(tl);
	parent->addChild(tr);
	parent->addChild(bl);
	parent->addChild(br);
	
	if (current_level < target_level) {
		createQuadTree(result, tl, target_level, current_level+1);
		createQuadTree(result, tr, target_level, current_level+1);
		createQuadTree(result, bl, target_level, current_level+1);
		createQuadTree(result, br, target_level, current_level+1);
	} else {
		result.push_back(tl);
		result.push_back(tr);
		result.push_back(bl);
		result.push_back(br);
	}
	// std::cout << result.size() << " c: " << current_level << " t: " << target_level << std::endl;
}



/**************************************************************************
 * 
 **************************************************************************/
 
void Tiled2DGraphics::Level::createTiles(CreateTileCallback& callback)
{
	unsigned int dest_level = std::max(nextPowerOfTwo(_numXTiles), nextPowerOfTwo(_numYTiles));
	unsigned int num_iterations = 0;
	unsigned int current_level = 1;
	while(current_level < dest_level) 
	{ 
		++num_iterations; current_level += current_level; 
	}
	
	std::vector<osg::Group*> result;
	
	if (dest_level > 1) {
		createQuadTree(result, this, num_iterations, 1);
	} else {
        osg::Group* g = new osg::Group();
        addChild(g);
		result.push_back(g);
	}
	double w(_parent->getSize()[0]), h(_parent->getSize()[1]);
	double t_w(_parent->getTileWidth());
	double t_h(_parent->getTileHeight());
	
	t_w = (t_w < 0) ? 1 / static_cast<float>(_numXTiles) * _parent->getSize()[0] : t_w / _zoomLevel;
	t_h = (t_h < 0) ? 1 / static_cast<float>(_numYTiles) * _parent->getSize()[1] : t_h / _zoomLevel;
	

	
	for(unsigned int y = 0; y < _numYTiles; ++y)
	{
		for(unsigned int x = 0; x < _numXTiles; ++x) 
        {
			Tile* tile = callback.createTile(x, y);
			if (_parent->getOrigin() == osg::Image::BOTTOM_LEFT){
				tile->setRect(x * t_w, y * t_h, std::min(t_w, w - (x*t_w)), std::min(t_h, h - (y*t_h)));
			} else {
				float t = std::min(t_h, h - (y*t_h));
				tile->setRect(x * t_w, h - t - y * t_h, std::min(t_w, w - (x*t_w)), t);
			}

			tile->setParentLevel(this);
            tile->setParentGroup(result[y*dest_level + x]);
            
			callback(result[y*dest_level + x], tile);
            
            _tiles.push_back(tile);
		}
	}
    _potSize = dest_level;
}


/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::Level::getTilesInRect(const osg::Vec4& rect, TilesList& result, bool get_only_hidden_tiles)
{
    int x1,y1,x2,y2;
    float w(_parent->getWidth());
    float h(_parent->getHeight());
    Tiled2DGraphics::Origin o(_parent->getOrigin());
    x1 = std::max<int>(0,              std::floor(cefix::Rect::getLeft(rect) / w * _numXTiles)-1);
    x2 = std::min<int>(_numXTiles-1,   std::ceil(cefix::Rect::getRight(rect) / w * _numXTiles));
    
    y1 = std::max<int>(0,              std::floor(cefix::Rect::getTop(rect)   / h * _numYTiles)-1);
    y2 = std::min<int>(_numYTiles-1,   std::ceil(cefix::Rect::getBottom(rect) / h * _numYTiles));
    
    // std::cout << rect << " " << x1 << " " << y1 << " " << x2 << " " << y2 << std::endl;
    
    for(int y = y1; y <= y2; ++y) 
    {
        for(int x = x1; x <= x2; ++x) 
        {
            Tile* tile =  (o == osg::Image::TOP_LEFT) ? _tiles[(_numYTiles - 1 -y) * _numXTiles + x] : _tiles[y * _numXTiles + x];
            if ((!get_only_hidden_tiles) || (!tile->inVisibleList()))
            {
                result.push_back(tile);
            }
        }
    }
}

#pragma mark -


Tiled2DGraphics::TiledCacheItem::~TiledCacheItem() 
{
    if ((_parent->getCurrentLevel() !=  _tile->getParentLevel() ) || (!cefix::Rect::isIntersecting(_parent->getViewPort(), _tile->getRect()))) 
    {
        _tile->setIsInConstrainingRect(false);
    }
}

#pragma mark -

Tiled2DGraphics::MergeController::MergeController(Tiled2DGraphics* parent, unsigned int num_tiles_to_merge_per_frame)
:   osg::Referenced(),
    _parent(parent),
    _tiles(),
    _numTilesPerFrame(num_tiles_to_merge_per_frame)
{
}

void Tiled2DGraphics::MergeController::merge() 
{
    typedef std::multimap<float, Tiles::iterator, std::less<float> >TilesMap;
    TilesMap tiles;
        
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_tiles.size() == 0)
        return;
    unsigned int dead(0);   
    
    osg::Vec2 vp_center = cefix::Rect::getCenter(_parent->getViewPort());
     
    for(Tiles::iterator i = _tiles.begin(); i != _tiles.end(); ) {
        Tile* tile = (*i);
        if (tile->inVisibleList()) 
        {
            // in die map mit der distanz tile - center als key
            float distance = (cefix::Rect::getCenter((*i)->getRect()) - vp_center).length2();
            tiles.insert(std::make_pair(distance, i));
            i++;
        } else {
            i = _tiles.erase(i);
            dead++;
        }
    }
    
    unsigned int added(0);
    for(TilesMap::iterator i = tiles.begin(); i != tiles.end(); ++i) {
        if (added < _numTilesPerFrame) {
            const Tiles::iterator j = i->second;
            (*j)->mergeWithSceneGraph();
            _tiles.erase(i->second);
            added++;
        }
    }
    
    // std::cout << "merged " << added << " tiles, " << dead << " dead, " << _tiles.size() << " still waiting ..." << std::endl;
}


#pragma mark -

/**************************************************************************
 * 
 **************************************************************************/
 
Tiled2DGraphics::Tiled2DGraphics(const osg::Vec2& size, const osg::Vec4& constraining_rect, double tile_width, double tile_height, const Options& options)
:	osg::MatrixTransform(),
	_size(size),
	_constrainingRect(constraining_rect),
	_minZoomLevel(computeMinZoomLevel()),
	_maxZoomLevel(_minZoomLevel),
	_currentZoomLevel(0.0f),
    _currentVisibleLevel(NULL),
    _scrollDelta(0,0),
    _visibleTiles(),
    _tilesToHide(),
	_tileWidth(tile_width),
	_tileHeight(tile_height),
	_origin(osg::Image::BOTTOM_LEFT),
    _zoomRegistrationPoint(),
    _clampingPolicy(ClampSoft),
    _useCache(true),
    _debug(options.debug),
    _debugTiles(options.debugTiles),
    _tilesComputed(false),
    _lastLevelId(0),
    _enabled(true)
{
	_tilesToHideCache = new TilesToHideCache(options.tilesCacheLifeTime);
    _mergeController = new MergeController(this, options.numTilesToMergePerFrame);
    
    setUpdateCallback(new Tiled2DGraphicsUpdateCallback(this));
	setZoomLevel(_minZoomLevel);
	updateOffset(ClampHard);
    
	setZoomRegistrationPoint(cefix::Rect::getCenter(constraining_rect));
    
    
    
    if (_debug) {
        cefix::DebugGeometryFactory::enable("tiles_2d");
        cefix::DebugGeometryFactory::enable("tiles_2d_overlay");
        cefix::DebugGeometryFactory::get("tiles_2d")->setLocZ(1);
        addChild(cefix::DebugGeometryFactory::get("tiles_2d")->getOrCreateNode());
    }
}



/**************************************************************************
 * 
 **************************************************************************/
 
float Tiled2DGraphics::computeMinZoomLevel()
{
	return std::max(cefix::Rect::getWidth(_constrainingRect) / _size[0], cefix::Rect::getHeight(_constrainingRect) / _size[1]);
}


/**************************************************************************
 * 
 **************************************************************************/
 
Tiled2DGraphics::Level* Tiled2DGraphics::addLevel(Tiled2DGraphics::Level* level)
{
	float zoom_level = level->getZoomLevel();
    
    _maxZoomLevel = std::max(_maxZoomLevel, zoom_level);
	Levels::iterator itr = _levels.find(zoom_level);
	if (itr == _levels.end()) 
	{
		_levels[zoom_level] = level;
		level->setParent(this);
    		
		addChild(level);
        unsigned int k(0);
        unsigned num_tiles(0);
        for(Levels::iterator i= _levels.begin(); i != _levels.end(); ++i){
            i->second->setLevelId(k++);
            i->second->setNumTilesOfLowerLevels(num_tiles);
            num_tiles += i->second->getNumTiles();
        }
		updateLevels();
        
        // std::cout << " adding level id " << level->getLevelId() << " for zoom " << level->getZoomLevel() << " (" << level->getNumXTiles() << " x " << level->getNumYTiles() << ")" << std::endl;
		return level;
	}
	return itr->second;
}


/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::createTiles(CreateTileCallback& callback) 
{
    callback.setTiled2DGraphics(this);
    callback.init();
    unsigned int k(0), num_tiles(0);
    _idMapping.clear();
    
    for(Levels::iterator i= _levels.begin(); i != _levels.end(); ++i)
    {
        _lastLevelId = k;
        _idMapping[k] = i->first;
        
        i->second->setLevelId(k++);
        i->second->createTiles(callback);
        i->second->setNumTilesOfLowerLevels(num_tiles);
        num_tiles += i->second->getNumTiles();
        
    }
    _tilesComputed = true;
}


/**************************************************************************
 * 
 **************************************************************************/
 
void Tiled2DGraphics::setZoomLevel(float zoom)
{
	float new_zoom_level = osg::clampTo(zoom, _minZoomLevel, _maxZoomLevel);
	if (new_zoom_level != _currentZoomLevel) {
		_currentZoomLevel = new_zoom_level;
		updateLevels();
		
	}
}



/**************************************************************************
 * 
 **************************************************************************/
 
void Tiled2DGraphics::updateLevels()
{
	if (_levels.size() == 0)
		return;
	
	Level* new_level = NULL;
	Levels::iterator i = _levels.lower_bound(_currentZoomLevel);
	if (i != _levels.begin())
		--i;
	Levels::iterator next_i = i;
	if (next_i != _levels.end()) {
		next_i++;
		if (next_i == _levels.end())
			--next_i;
	}
	
	float lower_d = (_currentZoomLevel - i->first);
	float upper_d = (i->first - _currentZoomLevel);
	new_level =  (lower_d < upper_d) ? i->second : next_i->second;
	    
    if(_currentVisibleLevel != new_level) 
    {
        for(Level::TilesList::iterator i = _visibleTiles.begin(); i != _visibleTiles.end(); ++i) {
            _tilesToHide.push_back(*i);
        }
        _visibleTiles.clear();
        hideOrphanedTiles();
    }
    
    // std::cout << new_level << " lower: " << lower_d << " upper: " << upper_d << std::endl;
    
	for(Levels::iterator itr = _levels.begin(); itr != _levels.end(); ++itr) {
        // std::cout << itr->second->getLevelId() << ": " << ((itr->second == new_level) * 0xFFFF) << std::endl;
		itr->second->setNodeMask( (itr->second == new_level) * 0xFFFF);
	}
    

    
	_currentVisibleLevel = new_level;
    
    if (_tilesComputed)
        updateVisibility();
}


/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::updateVisibility()
{
    _dirty = false;
    updateOffset(_clampingPolicy);
    
    if (_debug) {
        static int cnt=0;
        cnt++;
        std::ostringstream ss;
        ss << "enabled: " << _enabled << std::endl;
        ss << "scroll-delta: " << _scrollDelta << std::endl;
        ss << "zoom-level: " << _currentZoomLevel << std::endl;
        ss << "counter: " << cnt << std::endl;
        cefix::DebugGeometryFactory::get("tiles_2d_overlay")->addText(osg::Vec3(10,40,10), ss.str(), osg::Vec4(0.3,0.3,0.3,1));
    }
    
    if (!_currentVisibleLevel || !_enabled)
		return;
    
    
    // erstmal alle möglichen Tiles durchgehen und wenn sie noch nicht visible sind, zu den visible hinzufügen
    _currentVisibleLevel->getTilesInRect(_viewportRect, _visibleTiles);
    
    for(Level::TilesList::iterator i = _visibleTiles.begin(); i != _visibleTiles.end(); ) 
    {
        if(cefix::Rect::isIntersecting((*i)->getRect(), _viewportRect)) 
        {
            _tilesToHideCache->remove(*i); 
            
            (*i)->setIsInConstrainingRect(true);
            (*i)->setIsInVisibleList(true);
            (*i)->startAnyPreloadingAction();
            
            ++i;
        }
        else {
            if ((*i)->isInConstrainingRect())
                _tilesToHide.push_back(*i);
            i = _visibleTiles.erase(i);
        }
    }
    
    // std::cout << "visible tiles: "<< _visibleTiles.size() << " tiles to hide: " << _tilesToHide.size() << std::endl;
    
    hideOrphanedTiles();
    
    _mergeController->merge();
    
}


/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::hideOrphanedTiles()
{
    for(Level::TilesList::iterator i = _tilesToHide.begin(); i != _tilesToHide.end(); ++i)
    {
        (*i)->cancelAnyPreloadingActions();
        (*i)->setIsInVisibleList(false);
        if (!_useCache) {
            (*i)->setIsInConstrainingRect(false);
        }
        else if (!_tilesToHideCache->has(*i))
            _tilesToHideCache->add(*i, new TiledCacheItem(this, *i));
    }
    _tilesToHide.clear();
}



/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::addLevelsSuitableForZoomify(float tile_width, float tile_height)
{
    float w(getWidth()), h(getHeight());
    float zoom = 1.0f;
    while ((w > tile_width) || (h > tile_height)) {
        addLevel(new Level(zoom, std::ceil(w/tile_width), std::ceil(h/tile_height)));
        w *= 0.5;
        h *= 0.5;
        zoom *= 0.5;
    }
    
    // so nun letzten level hinzufügen
    addLevel(new Level(w/getWidth(), 1, 1));
}




/**************************************************************************
 * 
 **************************************************************************/


void Tiled2DGraphics::setZoomRegistrationPoint(const osg::Vec2& p) 
{ 
	_zoomRegistrationPoint = p;
	
	const double c_w(cefix::Rect::getWidth(_constrainingRect));
	const double c_h(cefix::Rect::getHeight(_constrainingRect));
	const double c_l(cefix::Rect::getLeft(_constrainingRect));
	const double c_t(cefix::Rect::getTop(_constrainingRect));
	
	
	const double s_x = (_zoomRegistrationPoint.x() - c_l) / c_w;
	const double s_y = (_zoomRegistrationPoint.y() - c_t) / c_h;
	
	_zoomRegistrationPointNative = -_scrollDelta + cefix::Rect::getTopLeft(_viewportRect) + osg::Vec2(s_x * c_w / _currentZoomLevel, s_y * c_h / _currentZoomLevel);
		
}

/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::updateTransform() 
{
	updateOffset(_clampingPolicy);
    	//setMatrix(osg::Matrix::translate(dx, dy, 0) * osg::Matrix::scale(_currentZoomLevel, _currentZoomLevel, 1) * osg::Matrix::translate(scroll_delta_x, scroll_delta_y, 0));

}





/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::updateOffset(ClampingPolicy clampingPolicy) 
{
    const double c_w(cefix::Rect::getWidth(_constrainingRect));
	const double c_h(cefix::Rect::getHeight(_constrainingRect));
	const double c_l(cefix::Rect::getLeft(_constrainingRect));
	const double c_t(cefix::Rect::getTop(_constrainingRect));
	
	
	const double s_x = (_zoomRegistrationPoint.x() - c_l) / c_w;
	const double s_y = (_zoomRegistrationPoint.y() - c_t) / c_h;
	
	const osg::Vec2 tl(_zoomRegistrationPointNative - osg::Vec2(s_x * c_w / _currentZoomLevel, s_y * c_h / _currentZoomLevel));
	bool viewport_computed(false);
	
	// clamping
    osg::Vec2 clamped;
    clamped[0] = osg::clampTo(_scrollDelta[0]+tl[0], 0.0f, getWidth()  - cefix::Rect::getWidth(_constrainingRect) / _currentZoomLevel) - tl[0];
	clamped[1] = osg::clampTo(_scrollDelta[1]+tl[1], 0.0f, getHeight() - cefix::Rect::getHeight(_constrainingRect) / _currentZoomLevel) - tl[1];
    
	switch(clampingPolicy) {
		case ClampHard:
            _scrollDelta = clamped;
            break;
		
		case ClampSoft:
            {				
				_viewportRect = osg::Vec4(0, 0, c_w, c_h) / _currentZoomLevel; 
				_viewportRect = cefix::Rect::offset(_viewportRect, _scrollDelta + tl);
				viewport_computed = false;
                
				_scrollDelta = cefix::interpolate(_scrollDelta, clamped, 0.5);
            }
            break;
            
		case DoNotClamp:
        default:
			break;
	}
	
	// viewport
	if (!viewport_computed) {
		_viewportRect = osg::Vec4(0, 0, c_w, c_h) / _currentZoomLevel; 
		_viewportRect = cefix::Rect::offset(_viewportRect, _scrollDelta + tl);
	}
	
	
	osg::Vec2 d1(-_scrollDelta - _zoomRegistrationPointNative); //cefix::Rect::getTopLeft(_viewportRect));
	osg::Vec2 d2(_zoomRegistrationPoint); //cefix::Rect::getTopLeft(_constrainingRect));
	
	setMatrix(osg::Matrix::translate(d1.x(), d1.y(), 0) * osg::Matrix::scale(_currentZoomLevel, _currentZoomLevel, 1) * osg::Matrix::translate(d2.x(), d2.y(), 0));

	// debug
    if (_debug) 
    {
        float di = 3 / _currentZoomLevel;

        
        for(TilesToHideCache::const_iterator i = _tilesToHideCache->begin(); i != _tilesToHideCache->end(); ++i) 
        {
            Tile* tile = i->second.getReference()->getTile();
            osg::Vec4 r = cefix::Rect::inset(tile->getRect(), 4*di, 4*di);
            osg::Vec4 c = osg::Vec4(1,1,1, 0.3 * (1- (i->second.getAge() / 5.0)));
            cefix::DebugGeometryFactory::get("tiles_2d")->addLine(cefix::Rect::getTopLeft(r), cefix::Rect::getBottomRight(r), c);
            cefix::DebugGeometryFactory::get("tiles_2d")->addLine(cefix::Rect::getTopRight(r), cefix::Rect::getBottomLeft(r), c);
        }
		
		cefix::DebugGeometryFactory::get("tiles_2d")->addRect(_viewportRect, osg::Vec4(0,1,1,1));   
        cefix::DebugGeometryFactory::get("tiles_2d")->addPoint(_zoomRegistrationPointNative, osg::Vec4(0,1,1,1));
        
        if (_debugTiles) {
            for(Level::TilesList::iterator j = _visibleTiles.begin(); j != _visibleTiles.end(); ++j) {
                Tile* tile = (*j);
                drawDebugInfoForTile(tile, di);
            }
		}
    }
}

/**************************************************************************
 * 
 **************************************************************************/

void Tiled2DGraphics::drawDebugInfoForTile(Tile* tile, float di)
{
	osg::Vec4 r = tile->getRect();
	cefix::DebugGeometryFactory::get("tiles_2d")->addRect(tile->getRect(), osg::Vec4(!tile->isInConstrainingRect(), tile->isInConstrainingRect(), 0, 0.3));
	cefix::DebugGeometryFactory::get("tiles_2d")->addRect(cefix::Rect::inset(r, di*1, di*1), osg::Vec4(!tile->isInConstrainingRect(), tile->isInConstrainingRect(), 0.5, 0.3));
	cefix::DebugGeometryFactory::get("tiles_2d")->addRect(cefix::Rect::inset(r, di*2, di*2), osg::Vec4(!tile->inVisibleList(), tile->inVisibleList(), 0.5, 0.3));
	tile->debug();
}



/**************************************************************************
 * 
 **************************************************************************/
 
void Tiled2DGraphics::getTilesContaining(const osg::Vec4& rect, unsigned int level_id, Level::TilesList& result) 
{
    float zoom_level = _idMapping[level_id];
    Levels::iterator itr = _levels.find(zoom_level);
    if (itr == _levels.end()) {
        return;
    }
    itr->second->getTilesInRect(rect, result, true);
}

void Tiled2DGraphics::clearAnyPreloadingActions()
{
    for(Level::TilesList::iterator i = _visibleTiles.begin(); i != _visibleTiles.end(); ++i) {
        (*i)->cancelAnyPreloadingActions();
    }
    
    for(Level::TilesList::iterator i = _tilesToHide.begin(); i != _tilesToHide.end(); ++i) {
        (*i)->cancelAnyPreloadingActions();
    }
}

void Tiled2DGraphics::setEnabledFlag(bool f)
{
    if (_enabled == f) return;
    std::cout << "enable: " << this << ": " << f << std::endl;
    _enabled = f;
    if (_enabled) 
    {
        updateLevels();
        updateVisibility();
    } 
    else 
    {
        for(Level::TilesList::iterator i = _visibleTiles.begin(); i != _visibleTiles.end(); ++i) {
            _tilesToHide.push_back(*i);
        }
        _visibleTiles.clear();
        hideOrphanedTiles();
    }
}

}
