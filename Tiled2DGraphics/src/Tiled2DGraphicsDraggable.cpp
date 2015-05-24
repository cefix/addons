/*
 *  Tiled2DGraphicsDraggable.cpp
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 26.10.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "Tiled2DGraphicsDraggable.h"
#include <cefix/Quad2DGeometry.h>

namespace cefix {

Tiled2DGraphicsDraggable::Tiled2DGraphicsDraggable(Tiled2DGraphics* graphics)
:   cefix::Draggable(),
    osg::NodeCallback(),
    _graphics(graphics),
    _scrollDelta(0,0),
    _lastDeltas(),
    _doThrowing(false),
    _zoomSpeed(500)
{
}


void Tiled2DGraphicsDraggable::down()
{
    osg::Vec3 p = getCurrentHit().getLocalIntersectPoint();
	_graphics->setZoomRegistrationPoint(osg::Vec2(p[0], p[1]));
	
	_doThrowing = false;
    _lastDeltas.clear();
    stopPropagation();
    cefix::Draggable::down();
}


void Tiled2DGraphicsDraggable::up(bool inside)
{
    _doThrowing = true;
    _mode = NONE;
    cefix::Draggable::up(inside);
}

    
bool Tiled2DGraphicsDraggable::dragStarted()
{
    if (getCurrentPointerData().event->getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT) {
        _mode = ZOOM_IN;
        return false;
    } else if(getCurrentPointerData().event->getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) {
        _mode = ZOOM_OUT;
        return false;
    }
    _mode = DRAG;
    _scrollDelta = _graphics->getScrollDelta();
    _lastDelta.set(0,0);
    return cefix::Draggable::dragStarted();
}


bool Tiled2DGraphicsDraggable::drag(const osg::Vec3& delta)
{
    osg::Vec2 d = osg::Vec2(delta[0], delta[1]);
    d /= _graphics->getCurrentZoomLevel();
    _lastDeltas.add(d-_lastDelta);
    _lastDelta = d;
    
    _graphics->setScrollDelta(_scrollDelta - d);
    
    return true;
}

void Tiled2DGraphicsDraggable::dragStopped() 
{
}

void Tiled2DGraphicsDraggable::operator()(osg::Node* node, osg::NodeVisitor* nv) 
{
    traverse(node, nv);
    
    if (_doThrowing) {
        osg::Vec2 d = _graphics->getScrollDelta();
        osg::Vec2 new_delta = _lastDeltas.getAverage() * 0.8;
        _graphics->setScrollDelta(d-new_delta);
        // std::cout << "avg: " << _lastDeltas.getAverage() << " new_delta: " << new_delta << std::endl;
        
        _lastDeltas.add(new_delta);
    } else if ((_mode == ZOOM_IN) ||(_mode == ZOOM_OUT)) 
    {
        if(0) {
            float current_a = (_graphics->getWidth() *_graphics->getHeight());
            float new_a = (current_a * _graphics->getCurrentZoomLevel()) + (_zoomSpeed * _zoomSpeed) * ((_mode == ZOOM_IN) ? 1 : -1);
            _graphics->setZoomLevel(new_a / current_a);
        } else {
            float current_a = (_graphics->getWidth());
            float new_a = (current_a * _graphics->getCurrentZoomLevel()) + (_zoomSpeed) * ((_mode == ZOOM_IN) ? 1 : -1);
            _graphics->setZoomLevel(new_a / current_a);
        }
    }

}
    
    
osg::Geode* Tiled2DGraphicsDraggable::createGeode(Tiled2DGraphics* graphics, const osg::Vec4& rect, float loc_z)
{
    cefix::Quad2DGeometry* geo = new cefix::Quad2DGeometry(rect);
    geo->setLocZ(loc_z);
    
    osg::ColorMask* mask = new osg::ColorMask(false, false, false, false);
    geo->getOrCreateStateSet()->setAttributeAndModes(mask, osg::StateAttribute::ON);
    
    osg::Geode* geode = new osg::Geode();
    geode->setNodeMask(0x1);
    geode->addDrawable(geo);
    
    Tiled2DGraphicsDraggable* d = new Tiled2DGraphicsDraggable(graphics);
    
    geode->setUserData(d);
    geode->setUpdateCallback(d);
    return geode;
}



}