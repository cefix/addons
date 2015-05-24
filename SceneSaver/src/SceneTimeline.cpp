//
//  SceneTimeline.cpp
//  cefix animated scene exporter
//
//  Created by Stephan Maximilian Huber on 11.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "SceneTimeline.h"
#include <cefix/Serializer.h>


void CapturedSceneAction::init() {
    cefix::Timeline::Action::init();
     _node = static_cast<SceneTimeline*>(getParentTimeline())->readNode(_nodeFileName, true);
}



void CapturedSceneAction::enter() 
{ 
    SceneTimeline* tl = static_cast<SceneTimeline*>(getParentTimeline());
    
    if ((!_node) && !_nodeFileName.empty())
        _node = tl->readNode(_nodeFileName, false);
    if (_parent && _node) tl->setLastNode(_parent, _node); 
} 

void CapturedSceneAction::leave()
{
    if (static_cast<SceneTimeline*>(getParentTimeline())->getHoldReadNodes() == false)
        _node = NULL;
}

void CapturedSceneAction::saveNode() {
    if (_node) {
        static_cast<SceneTimeline*>(getParentTimeline())->saveNode(_node, _nodeFileName);
    }
}

void CapturedSceneAction::writeTo(cefix::Serializer& s)
{
    cefix::Timeline::Action::writeTo(s);
    saveNode();
    s << "node_file_name" << _nodeFileName;
}


void CapturedSceneAction::readFrom(cefix::Serializer& s)
{
    cefix::Timeline::Action::readFrom(s);
    s >> "node_file_name" >> _nodeFileName;
}

#pragma mark -

void CapturedViewMatrixAction::init()
{
    _manip = static_cast<SceneTimeline*>(getParentTimeline())->getCameraManipulator();
    
}
    
void CapturedViewMatrixAction::within(double scalar)
{
    if (_manip) _manip->setByMatrix(sample(scalar));
}



#pragma mark -

SceneTimeline::SceneTimeline() 
:   cefix::Timeline(), 
    _saveCallback(NULL), 
    _loadCallback(NULL),
    _cameraManipulator(NULL),
    _lastNode(NULL)
{
    setCallMissedActionsFlag(false, 0);
    clearLastNode();
}


void SceneTimeline::writeTo(cefix::Serializer& s)
{
    s.registerClass<CapturedSceneAction>("captured_scene");
    s.registerClass<CapturedViewMatrixAction>("captured_view");
    cefix::Timeline::writeTo(s);
}


void SceneTimeline::readFrom(cefix::Serializer& s)
{
    s.registerClass<CapturedSceneAction>("captured_scene");
    s.registerClass<CapturedViewMatrixAction>("captured_view");
    
    cefix::Timeline::readFrom(s);
}