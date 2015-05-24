/*
 *  Tiled2DGraphicsMultiTouchDraggable.h
 *  cefix_presentation
 *
 *  Created by Stephan Huber on 14.11.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#pragma once
#include <cefix/Pickable.h>
#include <cefix/RingBuffer.h>


#include "Tiled2DGraphics.h"

namespace cefix {

class Tiled2DGraphicsMultiTouchDraggable : public cefix::Pickable, public osg::NodeCallback {
public:

    class TouchEndedCallback : public osg::Referenced {
    public:
        virtual void operator()(Tiled2DGraphicsMultiTouchDraggable* draggable) = 0;
    };

    Tiled2DGraphicsMultiTouchDraggable(Tiled2DGraphics* graphics);
    
    static osg::Geode* createGeode(Tiled2DGraphics* graphics, bool allow_single_touch, const osg::Vec4& rect, float loc_z = 900);
    
    virtual void down();
    virtual void up(bool inside);
    
    virtual void handle();
    
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    
    void setGraphics(Tiled2DGraphics* graphics) { _graphics = graphics; }
    
    void setAllowSingleTouch(bool b) { _allowSingleTouch = b; }
    bool getAllowSingleTouch() const { return _allowSingleTouch; }
    
    void setTouchEndedCallback(TouchEndedCallback* cb ) { _touchEndedCallback = cb; }
    
private:
    osg::observer_ptr<Tiled2DGraphics> _graphics;
    osg::Vec2 _startScrollDelta;
    osg::Vec2 _start[2];
    cefix::RingBuffer<osg::Vec2, 5> _lastDeltas;
    bool _doThrowing;
    osg::Vec2 _lastDelta;
	float _startZoom;
	bool _dragInProgress;
	float _currentZoom;
	osg::Vec2 _currentDelta;
    bool _allowSingleTouch, _valid, _zoomInProgress, _zoomIn;
    osg::Timer_t _lastTouchTimeStamp;
    unsigned int _maxNumTouchPoints;
    osg::Vec2 _lastTouchPos;
    
    osg::ref_ptr<TouchEndedCallback> _touchEndedCallback;
    
};

}
