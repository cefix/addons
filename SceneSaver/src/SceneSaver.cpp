//
//  SceneSaver.cpp
//  cefix animated scene exporter
//
//  Created by Stephan Maximilian Huber on 11.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "SceneSaver.h"
#include <osgDB/WriteFile>
#include <cefix/Serializer.h>


class SaveNodeJob : public cefix::ThreadPool::Job {
public:
    SaveNodeJob(CapturedSceneAction* action) 
    :   cefix::ThreadPool::Job(), 
        _action(action)
    {
    }
    
    virtual void operator()() {
        _action->saveNode();
        _action->clearNode();
    }
private:
    osg::ref_ptr<CapturedSceneAction> _action;
};

#pragma mark - 


SceneSaver::SceneSaver(const std::string& file_name_prefix, const std::string& file_format, const osg::CopyOp& copy_op) 
:   osg::Referenced(),  
    SceneTimeline::SaveCallback(),
    _fileFormat(file_format),
    _filePrefix(file_name_prefix),
    _startTick(0),
    _lastNodeTick(0),
    _t(),
    _lastNode(),
    _timeline(NULL),
    _copyOp(copy_op),
    _options(NULL),
    _lastViewMatrix(),
    _threadPool(NULL),
    _saveNodesWhileCapturingFlag(false),
    _enabled(true)
{
    setTimeline(new SceneTimeline());
    _threadPool = new cefix::ThreadPool(1);
    _threadPool->startPausing();
    
    _fileCount = _folderCount = 0;
    _filePrefix = osgDB::getFilePath(file_name_prefix);
    _fileName = osgDB::getSimpleFileName(file_name_prefix);
    
    osgDB::makeDirectoryForFile(_filePrefix+"/"+_fileName+".xml");
    
}


std::string SceneSaver::computeNodeFileName() 
{
    std::string file_name = cefix::intToString(_folderCount)+"/"+_fileName+"_"+cefix::intToString(_folderCount)+"_"+cefix::intToString(_fileCount)+"."+_fileFormat;
    _fileCount++;
    if (_fileCount >= 500) {
        _fileCount = 0;
        _folderCount++;
    }
    
    return file_name;
}


void SceneSaver::saveNode(osg::Node* node, const std::string& file_name)
{
    std::cout << "saving " << node << " to " << file_name << std::endl;
    
    osgDB::makeDirectoryForFile(_filePrefix+"/"+file_name);
    osgDB::writeNodeFile(*node, _filePrefix+"/"+file_name, _options);
}


void SceneSaver::finishSave() 
{
    finish();
    std::cout << "remaining nodes to store: " << _threadPool->getRemainingJobs() << std::endl;
    _threadPool->stopPausing();
    _threadPool->waitUntilDone();
    
    cefix::Serializer s;
    s << "timeline" << _timeline;
    s.getPropertyList()->saveAsXML(_filePrefix+"/"+_fileName+".xml");
    _threadPool->startPausing();
    setSaveNodesWhileCapturingFlag(_saveNodesWhileCapturingFlag);
    _enabled = false;
}


void SceneSaver::queueForSaving(CapturedSceneAction* action)
{
    action->setNodeFileName(computeNodeFileName());
    _threadPool->addJob(new SaveNodeJob(action));
}
    