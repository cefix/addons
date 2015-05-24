//
//  IOSVideoReaderWriter.cpp
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 25.07.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include <iostream>

#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include "IOSFullscreenVideo.h"
#include "IOSAVFoundationVideo.h"


class ReaderWriterIOSVideo : public osgDB::ReaderWriter {
public:
    ReaderWriterIOSVideo()
    :   osgDB::ReaderWriter()
    {
        supportsExtension("mov","Movie format");
        supportsExtension("mpg","Movie format");
		supportsExtension("mpv","Movie format");
		supportsExtension("mp4","Movie format");
		supportsExtension("m4v","Movie format");
    }
    
    virtual const char* className() const { return "Default IOS Video reader-writer"; }
    
    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension,"mov") ||
               osgDB::equalCaseInsensitive(extension,"mpg") ||
               osgDB::equalCaseInsensitive(extension,"mpv") ||
               osgDB::equalCaseInsensitive(extension,"mp4") ||
               osgDB::equalCaseInsensitive(extension,"m4v");
    }
    
    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {                    
        
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName(file);
        fileName = osgDB::findDataFile( file, options );
        
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
        
        if (options && (options->getPluginStringData("fullscreen") == "true"))
            return new cefix::IOSFullscreenVideo(fileName);
        else
            return new cefix::IOSAVFoundationVideo(fileName);
            
    }

};

REGISTER_OSGPLUGIN(iosvideo, ReaderWriterIOSVideo)
