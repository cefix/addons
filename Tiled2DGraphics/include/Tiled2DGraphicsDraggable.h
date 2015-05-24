/*
 *  Tiled2DGraphicsDraggable.h
 *  Tiled2DGraphics
 *
 *  Created by Stephan Huber on 26.10.11.
 *  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#pragma once
#include <cefix/Draggable.h>
#include <cefix/RingBuffer.h>


#include "Tiled2DGraphics.h"

namespace cefix {

class Tiled2DGraphicsDraggable : public cefix::Draggable, public osg::NodeCallback {
public:
    enum Mode { NONE, DRAG, ZOOM_IN, ZOOM_OUT};
    Tiled2DGraphicsDraggable(Tiled2DGraphics* graphics);
    
    static osg::Geode* createGeode(Tiled2DGraphics* graphics, const osg::Vec4& rect, float loc_z = 900);
    
    virtual void down();
    virtual void up(bool inside);
    
    virtual bool dragStarted();
    virtual bool drag(const osg::Vec3& delta);
    virtual void dragStopped();
    
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    void setZoomSpeed(float f) { _zoomSpeed = f; }
    float getZoomSpeed() const { return _zoomSpeed; }
    
    void setGraphics(Tiled2DGraphics* graphics) { _graphics = graphics; }
private:
    osg::observer_ptr<Tiled2DGraphics> _graphics;
    osg::Vec2 _scrollDelta;
    cefix::RingBuffer<osg::Vec2, 5> _lastDeltas;
    bool _doThrowing;
    osg::Vec2 _lastDelta;
    Mode _mode;
    float _zoomSpeed;
};


}