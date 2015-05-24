//
//  SceneSaver.h
//  cefix animated scene exporter
//
//  Created by Stephan Maximilian Huber on 11.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include <cefix/ThreadPool.h>

#include "SceneTimeline.h"

class SceneSaver : public osg::Referenced, public SceneTimeline::SaveCallback {

public:
    
    SceneSaver(const std::string& file_name_prefix, const std::string& file_format, const osg::CopyOp& copy_op = osg::CopyOp::DEEP_COPY_ALL);
    
    ~SceneSaver() {
        finish();
    }
    
    void setTimeline(SceneTimeline* tl) 
    {
        tl->setSaveCallback(this);
        _timeline = tl;
    }
    
    
    void capture(osg::Node* node, osg::Timer_t tick = 0)
    {
        if (!_enabled) return;
        osg::Timer_t now(checkNow(tick));
        
        if(_lastNode) {
            osg::ref_ptr<CapturedSceneAction> action = new CapturedSceneAction(_lastNode, _t.delta_m(_startTick, _lastNodeTick), _t.delta_m(_lastNodeTick, now));
            _timeline->add(action);
            queueForSaving(action);
        }
        _lastNode = node ? osg::clone(node, _copyOp) : NULL;
        _lastNodeTick = now;
        
        std::cout << "captured " << _t.delta_m(_startTick, _lastNodeTick) << " msecs ..." << std::endl;
    }
    
    void capture(osgGA::CameraManipulator* manip, osg::Timer_t tick = 0) 
    {
        if (!_enabled) return;
        
        osg::Timer_t now(checkNow(tick));
        
        if (!_viewMatrixAction) {
            _viewMatrixAction = new CapturedViewMatrixAction();
            _timeline->add(_viewMatrixAction);
        }
        
        _viewMatrixAction->addKeyframe(_t.delta_m(_startTick, now), manip->getMatrix());
    }
    
    void finish(osg::Timer_t tick = 0) {
        capture((osg::Node*)NULL, tick);
    }
    

    void finishSave();
    
    void setOptions(osgDB::Options* o) { _options = o; }
    void setSaveNodesWhileCapturingFlag(bool f) { _saveNodesWhileCapturingFlag = f; if (f) _threadPool->stopPausing(); }
    void setEnabledFlag(bool f) { _enabled = f; }
    bool isEnabled() const { return _enabled; }
    
protected:
    
    void queueForSaving(CapturedSceneAction* action);
    std::string computeNodeFileName();
    virtual void saveNode(osg::Node* node, const std::string& node_file_name);
    
    osg::Timer_t checkNow(osg::Timer_t tick) {
        osg::Timer_t t = (tick == 0) ? _t.tick() : tick;
        
        if (_startTick == 0)
            _startTick = t;
        return t;
    }
private:
    std::string _fileFormat, _filePrefix, _fileName;
    unsigned int _fileCount, _folderCount;
    osg::Timer_t _startTick, _lastNodeTick;
    osg::Timer _t;
    osg::ref_ptr<osg::Node> _lastNode;
    osg::ref_ptr<SceneTimeline> _timeline;
    osg::CopyOp _copyOp;
    osg::ref_ptr<osgDB::Options> _options;
    osg::Matrixd _lastViewMatrix;
    osg::ref_ptr<CapturedViewMatrixAction> _viewMatrixAction;
    
    osg::ref_ptr<cefix::ThreadPool> _threadPool;
    bool _saveNodesWhileCapturingFlag, _enabled;

};