//
//  SVGParser.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <cefix/tinyxml.h>
#include <osg/Matrixd>
#include <vector>

namespace svg {

class Node;
class Document;
class Group;
class Rect;
class Ellipse;
class Circle;
class Line;
class PolyLine;
class Polygon;
class Path;
class Color;
class Text;

class Parser {
public:
    Parser() {};
    
    Document* parse(const std::string& file_name);
    Document* parseSource(const std::string& text);
    
protected:
    Node* parseDocument(Document* node, TiXmlElement* elem);
    Node* parseRect(Rect* rect, TiXmlElement* elem);
    Node* parseGroup(Group* group, TiXmlElement* elem);
    Node* parseEllipse(Ellipse* ellipse, TiXmlElement* elem);
    Node* parseCircle(Circle* ellipse, TiXmlElement* elem);
    Node* parseLine(Line* ellipse, TiXmlElement* elem);
    Node* parsePolyLine(PolyLine* poly_line, TiXmlElement* elem);
    Node* parsePolygon(Polygon* ellipse, TiXmlElement* elem);
    Node* parsePath(Path* path, TiXmlElement* elem);
    Node* parseText(Text* text, TiXmlElement* elem);

    double parseLength(const char* input);
    
    bool parseTransform(const char* input, osg::Matrixd& m);
    bool parseTransforms(const char* input, osg::Matrixd& mat);
    bool parseListOfNumbers(const char* input, std::vector<double>&output);
    void parseListOfStrings(const char* str, std::vector<std::string>&tokens);
    bool parseColor(const char* str, Color& color);
    bool parsePathComponents(const char* str, Path* path);
    void addPathComponent(Path* path, unsigned char command, const std::vector<double>& params);
    void parseNodeCommonAttributes(Node* node, TiXmlElement* elem);

    Node* parse(TiXmlElement* root_elem);
    
    void parseChilds(Group* parent, TiXmlElement* elem);


};


}
