//
//  SceneTimeline.h
//  cefix animated scene exporter
//
//  Created by Stephan Maximilian Huber on 11.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include <osgGA/CameraManipulator>
#include <osg/Group>

#include <cefix/Timeline.h>
#include <cefix/KeyframeBasedAction.h>

class CapturedSceneAction: public cefix::Timeline::Action {

public:
    CapturedSceneAction(): Action(), _node(NULL), _nodeFileName(), _parent(NULL) {}
    CapturedSceneAction(osg::Node* node, double starttime, double duration)
    :   Action(starttime, duration),
        _node(node),
        _nodeFileName(),
        _parent(NULL)
    {
    }
    
    void setParent(osg::Group* group) 
    {
        _parent = group;
    }
    
    virtual void init();
    
    virtual void enter();
    virtual void leave();
    
    virtual void writeTo(cefix::Serializer& s);
    virtual void readFrom(cefix::Serializer& s);
    
    void setNodeFileName(const std::string& node_file_name) { _nodeFileName = node_file_name; }
     
protected:
    void saveNode();
    void clearNode() { _node = NULL; }
private:
    osg::ref_ptr<osg::Node> _node;
    std::string _nodeFileName;
    osg::Group* _parent;
    
friend class SaveNodeJob;

};


class CapturedViewMatrixAction : public cefix::KeyframeBasedAction<osg::Matrixd> {

public:
    CapturedViewMatrixAction()
    :   cefix::KeyframeBasedAction<osg::Matrixd>(), 
        _manip(NULL)
    {
    }
    
    virtual void init();
    
    virtual void within (double scalar);
    
private:
    osgGA::CameraManipulator* _manip;

};

class SceneTimeline : public cefix::Timeline {

public:  
    class SaveCallback {
    public:
        virtual void saveNode(osg::Node* node, const std::string& node_file_name) = 0;
        virtual ~SaveCallback() {}
    };
    
    class LoadCallback  {
    public:
        virtual osg::Node* loadNode(const std::string& node_file_name, bool is_prelaoding) = 0;
        virtual ~LoadCallback() {}
    };
    
    SceneTimeline();
    
    virtual void writeTo(cefix::Serializer& s);
    virtual void readFrom(cefix::Serializer& s);
    
    void setSaveCallback(SaveCallback* cb) { _saveCallback = cb; }
    void setLoadCallback(LoadCallback* cb) { _loadCallback = cb; }  
    
    void setHoldReadNodesFlag(bool b) { _holdReadNodes = b; }
    bool getHoldReadNodes() const { return _holdReadNodes; }
    
    osgGA::CameraManipulator* getCameraManipulator() { return _cameraManipulator; }
    void setCameraManipulator(osgGA::CameraManipulator* manip) { _cameraManipulator = manip; }
    
private:
    void setLastNode(osg::Group* parent, osg::Node* node) { 
        if (_lastNode) parent->removeChild(_lastNode);
        _lastNode = node; 
        parent->addChild(node);
    }
        
    void clearLastNode() { _lastNode = 0; }
    void saveNode(osg::Node* node, const std::string& nodeFileName)
    {
        if (_saveCallback)  _saveCallback->saveNode(node, nodeFileName);
    }
    
    osg::Node* readNode(const std::string& nodeFileName, bool is_preloading)
    {
        return (_loadCallback) ? _loadCallback->loadNode(nodeFileName, is_preloading) : NULL;
    }
    
    SaveCallback* _saveCallback;
    LoadCallback* _loadCallback;
    
    bool _holdReadNodes;
    osgGA::CameraManipulator* _cameraManipulator;
    osg::Node* _lastNode;
    
    friend class CapturedSceneAction;

};