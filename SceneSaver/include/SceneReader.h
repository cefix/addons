//
//  SceneReader.h
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 12.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include "SceneTimeline.h"

class SceneReader : public osg::Referenced, public SceneTimeline::LoadCallback {
public:
    enum Mode { PRELOAD, PROXY, ON_DEMAND };
    
    SceneReader(Mode mode = ON_DEMAND);
    
    osg::Node* getNode() { return _node; }
    
    SceneTimeline* getTimeline() { return _timeline; }
    
    void setTimeline(SceneTimeline* tl);
    
    void load(const std::string& filename);
    
    void setOptions(osgDB::Options* o) { _options = o; }
    
    void setMode(Mode mode) { _mode = mode; }
    
    void setCameraManipulator(osgGA::CameraManipulator* manip) { _timeline->setCameraManipulator(manip); }

protected:
    virtual osg::Node* loadNode(const std::string& node_file_name, bool is_prelaoding);
    
private:
    osg::ref_ptr<osg::Group> _node;
    osg::ref_ptr<SceneTimeline> _timeline;
    std::string _basePath;
    osg::ref_ptr<osgDB::Options> _options;
    Mode _mode;

};