/*
 *  Tiled2DGraphicsMultiTouchDraggable.cpp
 *  cefix_presentation
 *
 *  Created by Stephan Huber on 14.11.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "Tiled2DGraphicsMultiTouchDraggable.h"

#include <cefix/Quad2DGeometry.h>
#include <math.h>
#include <cefix/MathUtils.h>
#include <osg/Geode>

namespace cefix {

Tiled2DGraphicsMultiTouchDraggable::Tiled2DGraphicsMultiTouchDraggable(Tiled2DGraphics* graphics)
:   cefix::Pickable(),
    osg::NodeCallback(),
    _graphics(graphics),
    _startScrollDelta(),
    _start(),
    _lastDeltas(),
    _doThrowing(false),
    _lastDelta(),
    _startZoom(),
    _dragInProgress(false),
    _currentZoom(),
    _currentDelta(),
    _allowSingleTouch(false),
    _valid(false),
    _zoomInProgress(false),
    _zoomIn(true),
    _lastTouchTimeStamp(0),
    _maxNumTouchPoints(0),
    _lastTouchPos()
{
}


void Tiled2DGraphicsMultiTouchDraggable::down()
{
    std::cout << "Tiled2DGraphicsMultiTouchDraggable::touchBegan" << std::endl;
    
    _doThrowing = false;
    _valid = false;
    _zoomInProgress = false;
      
    if (getNumPointers() > 2) {
        allowPropagation();
        return;
    }
    if (!_allowSingleTouch && getNumPointers() < 2) {
        allowPropagation();
        return;
    }
	
	_dragInProgress = true;
    
    _startScrollDelta = _graphics->getScrollDelta();
	_startZoom = _currentZoom = _graphics->getCurrentZoomLevel();
    _lastDelta.set(0,0);
	_currentDelta.set(0,0);
    
    for(unsigned int i = 0; i < getNumPointers(); ++i) {
        osg::Vec3 v(getCurrentHit().localIntersectionPoint);
        _start[i].set(v[0], v[1]);
    }
    if (_allowSingleTouch && getNumPointers() == 1) {
        _start[1] = _start[0];
    }
    _graphics->setZoomRegistrationPoint((_start[0] + _start[1]) / 2.0);
	_lastDeltas.clear();
    
  
    _valid = true;
    stopPropagation();
}


void Tiled2DGraphicsMultiTouchDraggable::up(bool inside)
{
    //std::cout << "Tiled2DGraphicsMultiTouchDraggable::touchEnded" << std::endl;
    
    if (!_valid) 
    {
        // sicherheitshalber alles ausschalten...
        if (getNumPointers() == 1) 
        {
            _dragInProgress = _zoomInProgress = _doThrowing = false;
        }
        allowPropagation();
    }
        
    _zoomInProgress = false;
    osg::Vec3 t_p(getCurrentHit().localIntersectionPoint);
    float delta2 = (_lastTouchPos - osg::Vec2(t_p[0], t_p[1])).length2();
    _lastTouchPos = osg::Vec2(t_p[0], t_p[1]);
    
    if (
        (_maxNumTouchPoints==1) && 
        (osg::Timer::instance()->delta_s(_lastTouchTimeStamp, osg::Timer::instance()->tick())< 0.3) &&
        (delta2 < 15*15)) 
    {
        _zoomInProgress = true;
        const float max_l = _graphics->getMaxZoomLevel();
        const float min_l = _graphics->getMinZoomLevel();
        const float mid_l = cefix::interpolate(min_l, max_l, 0.3f);
        float c_l = _graphics->getCurrentZoomLevel();
        float old_zoom = _currentZoom;
        if (_zoomIn) 
        {
            if (fabs(c_l - min_l) < 0.1)
                _currentZoom = mid_l;
            else if (fabs(c_l - mid_l) < 0.1)
                _currentZoom = max_l;
            else if(fabs(c_l - max_l) < 0.1) {
                _zoomIn = false;
                _currentZoom = mid_l;
            }
            
            
        } 
        else 
        {
            if (fabs(c_l - min_l) < 0.1) {
                _currentZoom = mid_l;
                _zoomIn = true;
            }
            else if (fabs(c_l - mid_l) < 0.1)
                _currentZoom = min_l;
            else if(fabs(c_l - max_l) < 0.1) {
                _currentZoom = mid_l;
            }
        }
        
        if (old_zoom == _currentZoom) {
            float lvls[3] = {min_l, mid_l, max_l};
            float max_diff(1000);
            for(unsigned int i = 0; i < 3; ++i) {
                float d = fabs(lvls[i] - c_l);
                if (d < max_diff) {
                    _currentZoom = lvls[i];
                    max_diff = d;
                }
            }            
        }
        
    }
    if ((_maxNumTouchPoints == 1) && (!_zoomInProgress) && (_lastDeltas.getAverage().length2() > 5*5))
        _doThrowing = true;
        
	_dragInProgress = false;
    
    if(getNumPointers() == 1) 
        _maxNumTouchPoints = 0;
        
    _lastTouchTimeStamp = osg::Timer::instance()->tick();
    
    if (_touchEndedCallback)
        _touchEndedCallback->operator()(this);
    
    stopPropagation();
}


void Tiled2DGraphicsMultiTouchDraggable::handle()
{
    if (!_valid)
        return;
        
    _maxNumTouchPoints = std::max(_maxNumTouchPoints, getNumPointers());
    
	osg::Vec2 now[2];
    osg::Vec2 delta[2];
    osg::Vec2 complete_delta(0,0);
    unsigned int num_touches_ended(0);
    
    for(unsigned int i = 0; i < getNumPointers(); ++i) {
        const cefix::Pickable::PointerData& tp = getNthPointerData(i);
        if (tp.phase == cefix::Pickable::PointerData::UP)
            num_touches_ended++;

    }
    
    for(unsigned int i = 0; i < getNumPointers(); ++i) {
        const cefix::Pickable::PointerData& tp = getNthPointerData(i);
        //std::cout << tp << " " << i << " id " << tp->id << std::endl;
        osg::Vec3 v = getNthHit(i).getLocalIntersectPoint();
        now[i].set(v[0], v[1]);
        
        // ist scale beendet, dann startpunkte resetten
        if ((getNumPointers() == 2) && (num_touches_ended > 0))
        {
            _start[i].set(v[0], v[1]);
        }
        delta[i] = (now[i]-_start[i]);
    }
    
    
    // bei nur einem touch, 2ten touch faken.
    if (_allowSingleTouch && getNumPointers() == 1) 
    {
        now[1] = now[0];
        delta[1] = delta[0];
    }
    
    bool is_scaling(getNumPointers() > 1);
    
    // wir draggen nur, wenn die deltas beider touch-punkte sich in ungefähr die gleiche richtung bewegen
    if((delta[0].length2() > 3*3) && (delta[1].length2() > 3*3)) {
        
        float angle = cefix::getAngleBetween(delta[0], delta[1]);
        
        if ((-osg::PI_4 < angle) && ( angle < osg::PI_4))
            is_scaling = false;
    }
    
    if(!is_scaling) 
    {
        complete_delta = _allowSingleTouch ? delta[0] : (delta[0] + delta[1]) / 2.0;
        complete_delta /= _graphics->getCurrentZoomLevel();
        _currentDelta = complete_delta;
    }
    	
    if (is_scaling)
	{
        float divisor  = (now[0]    - now[1]).length() / _graphics->getCurrentZoomLevel();
        float divident = (_start[0] - _start[1]).length() / _startZoom;
        
        _currentZoom = cefix::interpolate(_currentZoom, _currentZoom * divisor/divident, 0.2);
        _currentZoom = osg::clampTo(_currentZoom, _graphics->getMinZoomLevel(), _graphics->getMaxZoomLevel());
    }
    	
    if (is_scaling) {
        _lastDeltas.add(osg::Vec2(0,0));
    } else {
        _lastDeltas.add(complete_delta - _lastDelta);
    }
    _lastDelta = complete_delta;
}



void Tiled2DGraphicsMultiTouchDraggable::operator()(osg::Node* node, osg::NodeVisitor* nv) 
{
    traverse(node, nv);
    if (_zoomInProgress)
    {
        //std::cout << "zoom in progress " << _currentZoom << std::endl;
        _graphics->setZoomLevel(cefix::interpolate<float>(_currentZoom, _graphics->getCurrentZoomLevel(), 0.9f));
    }
    if (_dragInProgress) 
	{
		_graphics->setScrollDelta(_startScrollDelta - _currentDelta);
		_graphics->setZoomLevel(_currentZoom);
		// std::cout << "zoom: " << _currentZoom << " delta: " << _currentDelta << std::endl;
	}
    else if (_doThrowing) {
        osg::Vec2 d = _graphics->getScrollDelta();
        osg::Vec2 new_delta = _lastDeltas.getAverage() * 0.95;
        _graphics->setScrollDelta(d-new_delta);
        // std::cout << "avg: " << _lastDeltas.getAverage() << " new_delta: " << new_delta << std::endl;
        
        _lastDeltas.add(new_delta);
    } 
}

    
    
osg::Geode* Tiled2DGraphicsMultiTouchDraggable::createGeode(Tiled2DGraphics* graphics, bool allow_single_touch, const osg::Vec4& rect, float loc_z)
{
    cefix::Quad2DGeometry* geo = new cefix::Quad2DGeometry(rect);
    geo->setLocZ(loc_z);
    
    osg::ColorMask* mask = new osg::ColorMask(false, false, false, false);
    geo->getOrCreateStateSet()->setAttributeAndModes(mask, osg::StateAttribute::ON);
    
    osg::Geode* geode = new osg::Geode();
    geode->setNodeMask(0x1);
    geode->addDrawable(geo);
    Tiled2DGraphicsMultiTouchDraggable* d = new Tiled2DGraphicsMultiTouchDraggable(graphics);
    
    geode->setUserData(d);
    geode->setUpdateCallback(d);
    d->setAllowSingleTouch(allow_single_touch);
    return geode;
}



}