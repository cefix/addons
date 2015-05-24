//
//  WebAlterableControllerImplementation.h
//  alterable
//
//  Created by Stephan Huber on 29.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <cefix/AlterableController.h>

namespace cefix { namespace net {

class WebAlterableControllerImplementation : public AlterableControllerImplementation {
public:
    class WebServer;
    WebAlterableControllerImplementation(unsigned int port = 8888, const std::string& support_files_dir = "frontend/");
    ~WebAlterableControllerImplementation();
    virtual void rebuild();
    AlterableController* getController() { return AlterableControllerImplementation::getController(); }
protected:
    WebServer* _server;
};


}
}