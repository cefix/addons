//
//  SceneReader.cpp
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 12.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "SceneReader.h"
#include <cefix/Serializer.h>

SceneReader::SceneReader(Mode mode)
:   osg::Referenced(),
    SceneTimeline::LoadCallback(),
    _node(new osg::Group()),
   _timeline(NULL),
   _mode(mode)
{

}


void SceneReader::load(const std::string& filename)
{
    _basePath = osgDB::getFilePath(filename);
    cefix::Serializer s(filename);
    s >> "timeline" >> _timeline;
    
    setTimeline(_timeline);
}

void SceneReader::setTimeline(SceneTimeline* tl) { 
    _timeline = tl; 
    _timeline->setLoadCallback(this);
    
    for(SceneTimeline::iterator i = _timeline->begin(); i != _timeline->end(); ++i) {
        CapturedSceneAction* a = dynamic_cast<CapturedSceneAction*>(i->second.get());
        if (a) {
            a->setParent(_node);
        }
    }
    
    _timeline->setHoldReadNodesFlag(_mode == PRELOAD);
}

osg::Node* SceneReader::loadNode(const std::string& node_file_name, bool is_preloading)
{
    std::string full_node_file = _basePath + "/" + node_file_name;
    if (is_preloading && (_mode != PRELOAD))
        return NULL;
    
    // std::cout << "reading " << node_file_name << std::endl;
    
    if (_mode == PROXY) {
        osg::ProxyNode* pn = new osg::ProxyNode();
        pn->setLoadingExternalReferenceMode(osg::ProxyNode::DEFER_LOADING_TO_DATABASE_PAGER);
        pn->setFileName(0, full_node_file);
        return pn;
    }
    osg::Node* node = osgDB::readNodeFile(full_node_file, _options);
    if(!node) {
        cefix::log::error("SceneReader") << "could not load " << full_node_file << std::endl;
    }
    return node;
}
