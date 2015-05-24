//
//  SVGParser.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 02.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGParser.h"
#include <cefix/FileUtils.h>
#include <cefix/Log.h>
#include <cefix/StringUtils.h>

#include <osgDB/FileUtils>

#include "SVGNode.h"
#include "SVGDocument.h"
#include "SVGRect.h"
#include "SVGEllipse.h"
#include "SVGLine.h"
#include "SVGPolyLine.h"
#include "SVGPolygon.h"
#include "SVGPath.h"
#include "SVGText.h"
#include "SVGUtils.h"

namespace svg {

Document* Parser::parse(const std::string& file_name)
{
    std::string contents;
    std::string file = osgDB::findDataFile(file_name);
    if(file.empty())
        return NULL;
    
    cefix::readFileIntoString(file, contents);
    return parseSource(contents);
}


Document* Parser::parseSource(const std::string& text)
{
    TiXmlDocument doc;
    doc.Parse(text.c_str());
    if(doc.Error()) {
        cefix::log::error("svg::Parser") << "could not parse xml " << doc.ErrorDesc() << " at row: " << doc.ErrorRow() << ", col: " << doc.ErrorCol() << std::endl;
        return NULL;
    }
    
    osg::ref_ptr<Node> node = parse(doc.RootElement());
    return node ? node.release()->asDocument() : NULL;
}


Node* Parser::parse(TiXmlElement* elem)
{
    if (!elem)
        return NULL;
    
    std::string elem_name = cefix::strToLower(elem->ValueStr());
    osg::ref_ptr<Node> node(NULL);
    
    // std::cout << "parse " << elem_name << std::endl;
    
    if(elem_name == "svg")
        node = parseDocument(NULL, elem);
    else if(elem_name == "g")
        node = parseGroup(NULL, elem);
    else if(elem_name == "rect")
        node = parseRect(NULL, elem);
    else if(elem_name == "ellipse")
        node = parseEllipse(NULL, elem);
    else if(elem_name == "circle")
        node = parseCircle(NULL, elem);
    else if(elem_name == "line")
        node = parseLine(NULL, elem);
    else if(elem_name == "polygon")
        node = parsePolygon(NULL, elem);
    else if(elem_name == "polyline")
        node = parsePolyLine(NULL, elem);
    else if(elem_name == "path")
        node = parsePath(NULL,elem);
    else if(elem_name == "text")
        node = parseText(NULL,elem);
    if (node) {
        parseNodeCommonAttributes(node, elem);
        parseChilds(node->asGroup(), elem);
    }
    
    if(!node) {
        std::cout << "unknown element: " << elem_name << std::endl;
    }
    return node.release();
}

void Parser::parseChilds(Group* parent, TiXmlElement* elem)
{
    if (!parent) {
        cefix::log::info("svg::parseChilds") << "parent-elem is NULL?! " << elem->ValueStr() << std::endl;
        return;
    }
    
    TiXmlNode* child(NULL);
    
    while( (child = elem->IterateChildren( child )) ) {
        osg::ref_ptr<Node> node = parse(child->ToElement());
        if (node)
            parent->addChild(node);
    }
}


Node* Parser::parseDocument(Document* node, TiXmlElement* elem)
{
    osg::ref_ptr<Document> doc = node ? node : new Document;
    std::string version = elem->Attribute("version");
    
    double x = parseLength(elem->Attribute("x"));
    double y = parseLength(elem->Attribute("y"));
    double width = parseLength(elem->Attribute("width"));
    double height = parseLength(elem->Attribute("height"));
    if (width || height)
        doc->setDimensions(x, y, width, height);
    
    parseGroup(doc, elem);
    
    return doc.release();
}


Node* Parser::parseGroup(Group* node, TiXmlElement* elem)
{
    osg::ref_ptr<Group> group = node ? node : new Group();
    std::vector<double> nums;
    if(parseListOfNumbers(elem->Attribute("viewBox"), nums))
    {
        group->setViewBox(nums[0], nums[1], nums[2], nums[3]);
    }
    
    return group.release();
}

Node* Parser::parseRect(Rect* node, TiXmlElement* elem)
{
    osg::ref_ptr<Rect> rect = node? node : new Rect();
    double l = parseLength(elem->Attribute("x"));
    double t = parseLength(elem->Attribute("y"));
    double w = parseLength(elem->Attribute("width"));
    double h = parseLength(elem->Attribute("height"));
    
    double rx = parseLength(elem->Attribute("rx"));
    double ry = parseLength(elem->Attribute("ry"));
    
    
    rect->set(l, t, w, h);
    rect->setCornerRadius(rx, ry);
    
    return rect.release();
}

Node* Parser::parseEllipse(Ellipse* node, TiXmlElement* elem)
{
    osg::ref_ptr<Ellipse> ellipse = node ? node : new Ellipse;
    
    double cx = parseLength(elem->Attribute("cx"));
    double cy = parseLength(elem->Attribute("cy"));
    double rx = parseLength(elem->Attribute("rx"));
    double ry = parseLength(elem->Attribute("ry"));
    
    ellipse->set(cx, cy, rx, ry);
    
    return ellipse.release();
}

Node* Parser::parseCircle(Circle* node, TiXmlElement* elem)
{
    osg::ref_ptr<Circle> circle = node ? node : new Circle;
    
    double cx = parseLength(elem->Attribute("cx"));
    double cy = parseLength(elem->Attribute("cy"));
    double r = parseLength(elem->Attribute("r"));
    
    circle->set(cx, cy, r);
    
    return circle.release();
}

Node* Parser::parseLine(Line* node, TiXmlElement* elem)
{
    osg::ref_ptr<Line> line = node ? node : new Line();
    
    double x1 = parseLength(elem->Attribute("x1"));
    double y1 = parseLength(elem->Attribute("y1"));
    double x2 = parseLength(elem->Attribute("x2"));
    double y2 = parseLength(elem->Attribute("y2"));
    
    line->set(x1, y1, x2, y2);
    
    return line.release();
}



Node* Parser::parsePolygon(svg::Polygon *node, TiXmlElement *elem)
{
    osg::ref_ptr<Polygon> polygon = node ? node : new Polygon();
    
    std::vector<double> raw_points;
    if (!parseListOfNumbers(elem->Attribute("points"), raw_points))
        return NULL;
    for(unsigned int i = 0; i < raw_points.size(); i += 2) {
        polygon->addPoint(raw_points[i], raw_points[i+1]);
    }
    
    return polygon.release();
}


Node* Parser::parsePolyLine(svg::PolyLine *node, TiXmlElement *elem)
{
    osg::ref_ptr<PolyLine> polyline = node ? node : new PolyLine();
    
    std::vector<double> raw_points;
    if (!parseListOfNumbers(elem->Attribute("points"), raw_points))
        return NULL;
    for(unsigned int i = 0; i < raw_points.size(); i += 2) {
        polyline->addPoint(raw_points[i], raw_points[i+1]);
    }
    
    return polyline.release();
}

Node* Parser::parsePath(Path* node, TiXmlElement* elem)
{
    osg::ref_ptr<Path> path = node ? node : new Path();
    
    double length(0);
    length = parseLength(elem->Attribute("pathLength"));
    
    parsePathComponents(elem->Attribute("d"), path);
    
    return path.release();
}

Node* Parser::parseText(Text* node, TiXmlElement* elem)
{
    if (!elem->GetText()) {
        cefix::log::error("svg::Parser") << "could not parse text-element, perhaps because of not supported TSPAN-elements" << std::endl;
        return NULL;
    }
    
    osg::ref_ptr<Text> text = node ? node : new Text();
    
    std::string str = elem->GetText();
    std::string fontName = elem->Attribute("font-family") ? elem->Attribute("font-family") : "system.xml";
    double fontsize = parseLength(elem->Attribute("font-size"));
    text->setText(str);
    text->setFontAndSize(fontName, fontsize);
    std::vector<double> x, y;
    parseListOfNumbers(elem->Attribute("x"), x);
    parseListOfNumbers(elem->Attribute("y"), y);
    text->setX(x);
    text->setY(y);
    
    return text.release();
}


bool Parser::parsePathComponents(const char* str, Path* path)
{
    if (!str)
        return false;
    std::string components(str), value_str;
    unsigned char last_command(0);
    std::vector<double> params;
    for(std::string::iterator i = components.begin(); i != components.end(); ++i) {
        if(std::isdigit(*i) || ((*i) == '.'))
            value_str += *i;
        else {
            if (!value_str.empty())
                params.push_back(cefix::stringToDouble(value_str));
            if (*i == '-') {
                value_str = *i;
                continue;
            }
            if ((*i == 0x09) || (*i == 0x0D) || (*i == 0x0A) || (*i == 0x20) || (*i == ',')) {
                value_str = "";
                continue;
            } else {
                if (last_command) {
                    addPathComponent(path, last_command, params);
                }
                last_command = *i;
                params.clear();
                value_str = "";
            }
        }
    }
    if (!value_str.empty())
        params.push_back(cefix::stringToDouble(value_str));
    if (last_command) {
        addPathComponent(path, last_command, params);
    }
    return true;
}

void Parser::addPathComponent(Path* path, unsigned char command, const std::vector<double>& params)
{
    unsigned int i(0);
    
    switch(command) {
        case 'm':
        case 'M':
            while(i < params.size()-1) {
                if (i == 0)
                    path->moveTo(params[i], params[i+1], command == 'm');
                else
                    path->lineTo(params[i], params[i+1], command == 'm');
                i+= 2;
            }
            break;
        
        case 'l':
        case 'L':
            while(i < params.size()-1) {
                path->lineTo(params[i], params[i+1], command == 'l');
                i+= 2;
            }
            break;

        case 'H':
        case 'h':
            while(i < params.size()) {
                path->horizontalLineTo(params[i], command == 'h');
                i+= 1;
            }
            break;
        
        case 'V':
        case 'v':
            while(i < params.size()) {
                path->verticalLineTo(params[i], command == 'v');
                i+= 1;
            }
            break;
        
        case 'C':
        case 'c':
            while(i < params.size()-5) {
                path->cubicCurveTo(params[i], params[i+1], params[i+2], params[i+3], params[i+4], params[i+5], command == 'c');
                i+= 6;
            }
            break;
            
        case 'S':
        case 's':
            while(i < params.size()-3) {
                path->shortHandCubicCurveTo(params[i], params[i+1], params[i+2], params[i+3], command == 's');
                i+= 4;
            }
            break;
        case 'Q':
        case 'q':
            while(i < params.size()-3) {
                path->quadraticCurveTo(params[i], params[i+1], params[i+2], params[i+3], command == 'q');
                i+= 4;
            }
            break;
        
        case 'T':
        case 't':
            while(i < params.size()-1) {
                path->shortHandQuadraticCurveTo(params[i], params[i+1], command == 't');
                i+= +2;
            }
            break;
            
        case 'Z':
        case 'z':
            path->close();
            break;
            
        default:
            cefix::log::error("Parser::addPathComponent") << "unhandled/unknown path component: " << command << std::endl;
    }
}


double Parser::parseLength(const char* input)
{
    if (!input) return 0.0;
    
    std::string temp(input);
    temp = cefix::strTrim(temp);
    
    unsigned int unit_pos = std::string::npos;
    unsigned int position(0);
    for (std::string::const_iterator it(temp.begin()); it != temp.end(); ++it, position++)
    {
        if(isdigit(*it) || (*it == '-') || (*it == '.'))
            ;
        else
            unit_pos = position-1;
    }
    
    if(unit_pos == std::string::npos)
        return cefix::stringToDouble(temp);
    else {
        std::string unit = temp.substr(unit_pos, temp.length());
        std::string value = temp.substr(0, unit_pos);
        static std::map<std::string, double> units;
        if(units.size() == 0) {
            units["em"] = 12.5;
            units["ex"] = 12.5;
            units["px"] = 1.0;
            units["in"] = 90.0;
            units["cm"] = 35.43307;
            units["mm"] = 3.543307;
            units["pt"] = 1.25;
            units["pc"] = 15.0;
            units["deg"] = 1.0;
            units["grad"] = 360.0/400.0;
            units["rad"] = 180.0/osg::PI;
        }
        if (units.find(unit) == units.end()) {
            std::cout << "unknown unit: " << unit << std::endl;
            return 0;
        }
        return cefix::stringToDouble(value) * units[unit];
    }
    
    return 0.0;
}



bool Parser::parseTransform(const char* input, osg::Matrixd& m)
{
    if (!input)
        return false;
    
    std::string temp(input);
    temp = cefix::strToLower(cefix::strTrim(temp));
    
    unsigned int par_begin = temp.find('(');
    unsigned int par_end = temp.find(')');
    
    const std::string type = temp.substr(0, par_begin);
    const std::string pars = temp.substr(par_begin+1, par_end-par_begin-1
    );
    
    std::vector<double> nums;
    parseListOfNumbers(pars.c_str(), nums);
    
    if((type == "matrix") && (nums.size() == 6)) {
        m(0,0) = nums[0];
        m(0,1) = nums[1];
        m(1,0) = nums[2];
        m(1,1) = nums[3];
        m(3,0) = nums[4];
        m(3,1) = nums[5];
        // std::cout << m << std::endl;
        return true;
    }
    else if ((type == "translate") and (nums.size() >= 1)) {
        m.makeTranslate(nums[0], nums.size() >= 2 ? nums[1] : 0, 0);
        return true;
    }
    else if ((type == "scale") and (nums.size() >= 1)) {
        m.makeScale(nums[0], nums.size() >= 2 ? nums[1] : nums[0], 1);
        return true;
    }
    else if ((type == "rotate") and (nums.size() == 1)) {
        m.makeRotate(osg::DegreesToRadians(nums[0]), osg::Z_AXIS);
        return true;
    }
    else if ((type == "rotate") and (nums.size() == 3)) {
        m = osg::Matrixd::translate(-nums[1], -nums[2], 1) * osg::Matrixd::rotate(osg::DegreesToRadians(nums[0]), osg::Z_AXIS) * osg::Matrixd::translate(nums[1], nums[2], 0);
        return true;
    }
    else if ((type == "skewX") and (nums.size() == 1)) {
        m(1,0) = tan(nums[0]);
    }
    else if ((type == "skewY") and (nums.size() == 1)) {
        m(0,1) = tan(nums[0]);
    }
    std::cout << "unknown transform: " << temp << std::endl;
    return false;
}

bool Parser::parseTransforms(const char* input, osg::Matrixd& mat)
{
    if (!input)
        return false;
    std::vector<std::string> transform_strings;
    
    parseListOfStrings(input, transform_strings);
    
    for(unsigned int i = 0; i < transform_strings.size(); ++i)
    {
        osg::Matrixd m;
        if (!parseTransform(transform_strings[i].c_str(), m))
            return false;
        mat =   m * mat;
    }
    
    return true;
}


void Parser::parseListOfStrings(const char* input, std::vector<std::string>&tokens)
{
    if(!input)
        return;
    
    std::string current("");
    std::string str(input);
    int paranthesis_depth = 0;
    for (std::string::const_iterator it(str.begin()); it != str.end(); ++it)
    {
        if(*it == '(')
            paranthesis_depth++;
        if (*it == ')')
            paranthesis_depth--;
        
        if ((paranthesis_depth == 0) && ((*it == 0x09) || (*it == 0x0D) || (*it == 0x0A) || (*it == 0x20) || (*it == ',')))
        {
            if (!current.empty())
                tokens.push_back(current);
            current = "";
        }
        else {
            current += *it;
        }
    }
    if (!current.empty())
        tokens.push_back(current);
}



bool Parser::parseListOfNumbers(const char* input, std::vector<double>&output)
{
    if(!input)
        return false;
    
    std::string temp(input);
    std::vector<std::string> tokens;
    
    temp = cefix::strToLower(cefix::strTrim(temp));
    if (temp == "none")
        return false;
    
    parseListOfStrings(temp.c_str(), tokens);
    
    for(std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i) {
        output.push_back(parseLength((*i).c_str()));
    }
    
    return true;
}


bool Parser::parseColor(const char* str, Color& color)
{
    color = Color();
    if(!str)
        return false;
    
    std::string color_str =cefix::strToLower(std::string(str));
    
    if (color_str == "none")
        return true;
    
    if(color_str[0] == '#')
    {
        color = Color(cefix::hexColorToVec4(color_str));
        return true;
    }
    
    else if(color_str.substr(0,3) == "rgb")
    {
        std::size_t opening = color_str.find('(');
        std::size_t closing = color_str.find(')');
        if((opening != std::string::npos) && (closing != std::string::npos)) {
            std::vector<double> values;
            std::string inner = color_str.substr(opening+1, closing-opening-1);
            parseListOfNumbers(inner.c_str(), values);
            if (values.size() == 3)
            {
                color.set(values[0], values[1], values[2]);
                return true;
            }
            else if (values.size() == 4)
            {
                color.set(values[0], values[1], values[2], values[3]);
                return true;
            }
        }
    }
    
    else {
        color = Color(color_str);
        if (color.hasColor())
            return true;
    }
    
    
    cefix::log::error("SVGParser::parseColor") << "could not parse color " << color_str << std::endl;
    return false;
}


void Parser::parseNodeCommonAttributes(Node* node, TiXmlElement* elem)
{
    const char* id = elem->Attribute("id");
    if (id) node->setId(std::string(id));
    
    const char* transform = elem->Attribute("transform");
    if (transform) {
        osg::Matrixd mat;
        if(parseTransforms(transform, mat)) {
            node->setTransform(mat);
        }
    }
    
    Color fill_color, stroke_color;
    if(parseColor(elem->Attribute("fill"), fill_color))
        node->getOrCreateStateSet()->setFillColor(fill_color);
    
    if(parseColor(elem->Attribute("stroke"), stroke_color))
        node->getOrCreateStateSet()->setStrokeColor(stroke_color);
    
    const char* attr(NULL);
    if((attr = elem->Attribute("fill-opacity")))
        node->getOrCreateStateSet()->setFillOpacity(parseLength(attr));
    
    if((attr = elem->Attribute("stroke-opacity")))
        node->getOrCreateStateSet()->setStrokeOpacity(parseLength(attr));
    if ((attr = elem->Attribute("opacity"))) {
        node->getOrCreateStateSet()->setFillOpacity(parseLength(attr));
        node->getOrCreateStateSet()->setStrokeOpacity(parseLength(attr));
    }
    if((attr = elem->Attribute("stroke-width")))
        node->getOrCreateStateSet()->setStrokeWidth(parseLength(attr));
    
    if((attr = elem->Attribute("stroke-miterlimit")))
        node->getOrCreateStateSet()->setStrokeMiterLimit(parseLength(attr));
    
    if (const char* fill_rule = elem->Attribute("fill-rule")) {
        std::string fill_rule_str = cefix::strToLower(std::string(fill_rule));
        if (fill_rule_str == "nonzero")
            node->getOrCreateStateSet()->setFillRule(svg::StateSet::FR_NonZero);
        else if(fill_rule_str == "evenodd")
            node->getOrCreateStateSet()->setFillRule(svg::StateSet::FR_OddEven);
        else if(fill_rule_str == "inherit")
            node->getOrCreateStateSet()->setFillRule(svg::StateSet::FR_Inherit);
        else {
            cefix::log::error("SVGParser::parseNodeCommonAttributes") << "could not parse fill-rule: " << fill_rule_str << std::endl;
        }
    }
    
    if (const char* stroke_linecap = elem->Attribute("stroke-linecap")) {
        std::string stroke_linecap_str = cefix::strToLower(std::string(stroke_linecap));
        if (stroke_linecap_str == "butt")
            node->getOrCreateStateSet()->setStrokeLineCap(svg::StateSet::LC_Butt);
        else if(stroke_linecap_str == "round")
            node->getOrCreateStateSet()->setStrokeLineCap(svg::StateSet::LC_Round);
        else if(stroke_linecap_str == "square")
            node->getOrCreateStateSet()->setStrokeLineCap(svg::StateSet::LC_Square);
        else if(stroke_linecap_str == "inherit")
            node->getOrCreateStateSet()->setStrokeLineCap(svg::StateSet::LC_Inherit);
        else {
            cefix::log::error("SVGParser::parseNodeCommonAttributes") << "could not parse stroke-linecap: " << stroke_linecap_str << std::endl;
        }
    }
    
    if (const char* stroke_linejoin = elem->Attribute("stroke-linejoin")) {
        std::string stroke_linejoin_str = cefix::strToLower(std::string(stroke_linejoin));
        if (stroke_linejoin_str == "miter")
            node->getOrCreateStateSet()->setStrokeLineJoin(svg::StateSet::LJ_Miter);
        else if(stroke_linejoin_str == "round")
            node->getOrCreateStateSet()->setStrokeLineJoin(svg::StateSet::LJ_Round);
        else if(stroke_linejoin_str == "bevel")
            node->getOrCreateStateSet()->setStrokeLineJoin(svg::StateSet::LJ_Bevel);
        else if(stroke_linejoin_str == "inherit")
            node->getOrCreateStateSet()->setStrokeLineJoin(svg::StateSet::LJ_Inherit);
        else {
            cefix::log::error("SVGParser::parseNodeCommonAttributes") << "could not parse stroke-linejoin: " << stroke_linejoin_str << std::endl;
        }
    }
    
    if (const char* display_mode = elem->Attribute("display")) {
        std::string display_mode_str = cefix::strToLower(std::string(display_mode));
        if (display_mode_str == "none")
            node->getOrCreateStateSet()->setDisplayMode(svg::StateSet::DM_None);
        else if(display_mode_str == "inline")
            node->getOrCreateStateSet()->setDisplayMode(svg::StateSet::DM_Inline);
        else if(display_mode_str == "inherit")
            node->getOrCreateStateSet()->setDisplayMode(svg::StateSet::DM_Inherit);
        else {
            cefix::log::error("SVGParser::parseNodeCommonAttributes") << "could not parse display: " << display_mode_str << std::endl;
        }
    }
}


}