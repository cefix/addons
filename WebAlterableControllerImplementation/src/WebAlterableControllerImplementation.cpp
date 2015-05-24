//
//  WebAlterableControllerImplementation.cpp
//  alterable
//
//  Created by Stephan Huber on 29.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "WebAlterableControllerImplementation.h"

#include <cefix/Datafactory.h>
#include <cefix/Log.h>
#include <cefix/JSON.h>
#include <cefix/StringUtils.h>

#include "server.hpp"
#include "request_handler.hpp"
#include "mime_types.hpp"

namespace cefix { namespace net {

class WebAlterableControllerImplementation::WebServer : public OpenThreads::Thread, public http::server2::request_handler::Callback {
public:
    WebServer(WebAlterableControllerImplementation* parent, unsigned int port, const std::string& support_files_dir);
    
    virtual void run();
    void stop() {
        _server.stop(); 
    }
    
    virtual bool operator()(const std::string& request_path, http::server2::reply& rep);
    virtual std::string applyTemplateVars(const std::string& txt);

    void setStringReply(const std::string& str, http::server2::reply& rep);
    bool handleGetStructure(http::server2::reply& rep);
    bool handleGetValueList(http::server2::reply& rep);
    bool handleGetValue(const std::vector<std::string>& tokens, http::server2::reply& rep);
    bool handleSetValue(const std::string& request_key, http::server2::reply& rep);

private:    
    typedef std::map<std::string, std::pair<Alterable*, unsigned int> > AlterableMap;
    
    WebAlterableControllerImplementation* _parent;
    unsigned int _port;
    std::string _supportFilesDir;
    http::server2::server _server;
    
    AlterableMap _alterableMap;
};


WebAlterableControllerImplementation::WebServer::WebServer(WebAlterableControllerImplementation* parent, unsigned int port, const std::string& support_files_dir)
:   OpenThreads::Thread(),
    http::server2::request_handler::Callback(),
    _parent(parent),
    _port(port),
    _supportFilesDir(osgDB::findDataFile(support_files_dir)),
    _server("0.0.0.0", cefix::intToString(port), _supportFilesDir, 2)
    
{
    if (_supportFilesDir.empty()) {
        cefix::log::error("WebAlterableControllerImplementation") << "could not find support files directory at " << support_files_dir << std::endl;
    }
    _server.setCallback(this);
    start();
}

void WebAlterableControllerImplementation::WebServer::run() 
{
    _server.run();
}


std::string WebAlterableControllerImplementation::WebServer::applyTemplateVars(const std::string& txt)
{
    std::string result(txt);
    result = cefix::strReplaceAll<std::string>(result, "{{application_name}}", cefix::DataFactory::instance()->getApplicationName());
    
    return result;
}


bool WebAlterableControllerImplementation::WebServer::operator()(const std::string& request_path, http::server2::reply& rep)
{
    // std::cout << request_path << std::endl;
    std::vector<std::string> tokens;
    cefix::strTokenize(request_path, tokens, "/");
    if (tokens.size() < 1)
        return false;
        
    if (tokens[0] == "get_structure") {
        return handleGetStructure(rep);
    } else if (tokens[0] == "get_value_list") {
        return handleGetValueList(rep);
    } else if (tokens[0] == "get_value") {
        return handleGetValue(tokens, rep);
    } else if (tokens[0] == "set_value") {
        return handleSetValue(request_path, rep);
    }
    
    return false;
}


void  WebAlterableControllerImplementation::WebServer::setStringReply(const std::string& str, http::server2::reply& rep)
{
    rep.content = str;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = cefix::intToString(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = http::server2::mime_types::extension_to_type("html");
    
    rep.status = http::server2::reply::ok;
}


bool WebAlterableControllerImplementation::WebServer::handleGetStructure(http::server2::reply& rep)
{
    AlterableController* ctrl = _parent->getController();
    
    if (ctrl == NULL) {
        setStringReply("{ \"result\": {} }", rep);
        return true;
    }
    
    
    _alterableMap.clear();
    osg::ref_ptr<cefix::PropertyList> pl = new cefix::PropertyList();
    
    for(AlterableController::iterator i = ctrl->begin(); i != ctrl->end(); ++i) 
    {
       cefix::PropertyList* sub_pl = new cefix::PropertyList();
       sub_pl->add("name", i->first);
       
       
        AlterableGroup::List& l = i->second.items;
        for(AlterableGroup::List::iterator j = l.begin(); j != l.end(); ++j) 
        {
            cefix::Alterable* a = *j;
            cefix::PropertyList* values_pl = new cefix::PropertyList();
            sub_pl->add("values", values_pl);
            
            switch(a->getRepresentationType()) {
                case Alterable::VALUE:
                    values_pl->add("representation", "value");
                    values_pl->add("min_range", a->getSliderRange().min());
                    values_pl->add("max_range", a->getSliderRange().max());

                    break;
                case Alterable::SWITCH:
                    values_pl->add("representation", "switch");
                    break;
                case Alterable::TEXT:
                    values_pl->add("representation", "text");
                    break;
                case Alterable::BUTTON:
                    values_pl->add("representation", "button");
                    break;
            }
            values_pl->add("name", a->getName());
            for(unsigned int k = 0; k < a->getNumComposites(); ++k) {
                cefix::PropertyList* comp_pl = new cefix::PropertyList();
                comp_pl->add("value", a->getFloatValueAt(k));
                comp_pl->add("name", a->getCompositeName(k));
                comp_pl->add("key", a->getCompositeKey(k));
                values_pl->add("values", comp_pl);
                _alterableMap[a->getCompositeKey(k)] = std::make_pair(a, k);
            }
        }
       
       pl->add("groups", sub_pl);
    }   
    

    setStringReply(cefix::JSON::stringifyToUTF8("result", pl), rep);
    return true;
}

static inline std::string getValueString(Alterable* a, unsigned int ndx) 
{
    if (a->getRepresentationType() == Alterable::TEXT) {
        return "\""+dynamic_cast<AlterableT<std::string>*>(a)->getValue()+"\"";
    }
    return cefix::floatToString(a->getFloatValueAt(ndx));
}


bool WebAlterableControllerImplementation::WebServer::handleGetValue(const std::vector<std::string>& tokens, http::server2::reply& rep)
{
    std::string key("/");
    for(unsigned int i = 1; i < tokens.size(); ++i) {
        key += tokens[i];
        if (i < tokens.size()-1)
            key += "/";
    }
    
    AlterableMap::iterator i = _alterableMap.find(key);
    if (i == _alterableMap.end()) {
        setStringReply("{ \"result\": null, \"error\": \"key "+key+" not found\"}", rep);
        return true;
    }
    
    setStringReply("{ \"result\": "+getValueString(i->second.first, i->second.second)+" }", rep);
    return true;
}


bool WebAlterableControllerImplementation::WebServer::handleGetValueList(http::server2::reply& rep)
{
    std::string result;
    for(AlterableMap::iterator i = _alterableMap.begin(); i != _alterableMap.end(); ++i) 
    {
        if (!result.empty()) 
            result += ",\n";
            
        result += "\""+i->first+"\": "+getValueString(i->second.first, i->second.second);
    }
    setStringReply("{ \"result\": { "+result+"} }", rep);
    return true;
}



bool WebAlterableControllerImplementation::WebServer::handleSetValue(const std::string& request_key, http::server2::reply& rep)
{
    std::vector<std::string> tokens;
    cefix::strTokenize(request_key, tokens, "=");
    if (tokens.size() < 2) {
    setStringReply("{ \"result\": null, \"error\": \"syntax error in request\"}", rep);
        return true;
    }
    
    std::string key = cefix::strReplaceAll<std::string>(tokens[0], "/set_value", "");
    
    AlterableMap::iterator i = _alterableMap.find(key);
    if (i == _alterableMap.end()) {
        setStringReply("{ \"result\": null, \"error\": \"key "+key+" not found\"}", rep);
        return true;
    }
    Alterable* a = i->second.first;
    unsigned int ndx = i->second.second;
    a->setFloatValueAt(ndx, cefix::stringToFloat(tokens[1]));
    if (a->getRepresentationType() == Alterable::BUTTON) {
        AlterableCallFunctor* acf = dynamic_cast<AlterableCallFunctor*>(a);
        if (acf) 
            acf->call(cefix::stringToInt(tokens[1]));
    }
    setStringReply("{ \"result\": "+cefix::floatToString(a->getFloatValueAt(ndx))+" }", rep);
    
    return true;
}

#pragma mark -

WebAlterableControllerImplementation::WebAlterableControllerImplementation(unsigned int port, const std::string& support_files_dir)
:   AlterableControllerImplementation(),
    _server(new WebServer(this, port, support_files_dir))
{
}

WebAlterableControllerImplementation::~WebAlterableControllerImplementation()
{
    _server->stop();
    _server->join();
    delete _server;
}

void WebAlterableControllerImplementation::rebuild() 
{
    http::server2::reply rep;
    _server->handleGetStructure(rep);
}

}
}