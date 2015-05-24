//
//  SVGWriter.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 07.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <string>

namespace svg {

class Document;


class Writer {
public:
    /**
     * write a svg::Document as a svg-file into filename, if convert_to_path is true all shapes get converted to paths
     */
    static bool write(Document* doc, const std::string& filename, bool convert_to_paths);

};


}