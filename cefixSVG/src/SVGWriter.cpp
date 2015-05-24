//
//  SVGWriter.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 07.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGWriter.h"
#include "SVGDocument.h"
#include "SVGRect.h"
#include "SVGEllipse.h"
#include "SVGLine.h"
#include "SVGPolygon.h"
#include "SVGPolyLine.h"
#include "SVGPath.h"
#include "SVGText.h"

#include <cefix/tinyxml.h>
#include <cefix/StringUtils.h>
#include <cefix/Version.h>
#include <cefix/EnumUtils.h>
#include <cefix/Log.h>

namespace svg {

class WriteVisitor : public Visitor
{
public:
    WriteVisitor(bool convert_to_paths)
        : Visitor(VisitAllElements)
        , _xml()
        , _convertToPaths(convert_to_paths)
    {
        _xml.InsertEndChild(TiXmlDeclaration("1.0", "utf-8", "yes"));
        const std::string generator = "Generator: "+ cefixGetVersionDetailed();
        _xml.InsertEndChild(TiXmlComment(generator.c_str()));
    }


    TiXmlElement* newElement(const std::string& name)
    {
        TiXmlElement* elem = new TiXmlElement(name);
        
        int i = _elemStack.size() - 1;
        TiXmlElement* parent(NULL);
        while (i >= 0 && !parent) {
            parent = _elemStack[i];
            i--;
        }
        if (!parent)
            _xml.LinkEndChild(elem);
        else {
            parent->LinkEndChild(elem);
        }
        _elemStack[_elemStack.size()-1] = elem;
        return elem;
    }
   
    virtual void pushState(Node& node) {
        Visitor::pushState(node);
        _elemStack.push_back(NULL);
    }
    
    virtual void popState(Node& node) {
        Visitor::popState(node);
        _elemStack.pop_back();
    }
    void addColorAttribute(TiXmlElement* elem, const std::string& name, const svg::Color& color) {
        if (!color.hasColor())
            elem->SetAttribute(name, "none");
        else
            elem->SetAttribute(name, cefix::vec4ToHexColor(color.get(), false));
    }
    
    void addViewBoxAttribute(TiXmlElement* elem, const osg::Vec4& rect)
    {
        std::string vb;
        vb += cefix::doubleToString(cefix::Rect::getLeft(rect));
        vb += " " + cefix::doubleToString(cefix::Rect::getTop(rect));
        vb += " " + cefix::doubleToString(cefix::Rect::getWidth(rect));
        vb += " " + cefix::doubleToString(cefix::Rect::getHeight(rect));
        elem->SetAttribute("viewBox", vb);
    }
    
    void addCommonAttributes(TiXmlElement* elem, Node& node)
    {
        if (!node.getId().empty())
            elem->SetAttribute("id", node.getId());
        if(!node.getTransform().isIdentity())
        {
            const osg::Matrixd& m(node.getTransform());
            std::vector<std::string> ss;
            ss.push_back(cefix::doubleToString(m(0,0)));
            ss.push_back(cefix::doubleToString(m(0,1)));
            ss.push_back(cefix::doubleToString(m(1,0)));
            ss.push_back(cefix::doubleToString(m(1,1)));
            ss.push_back(cefix::doubleToString(m(3,0)));
            ss.push_back(cefix::doubleToString(m(3,1)));
            
            elem->SetAttribute("transform", "matrix("+cefix::strJoin(ss,",")+")");
        }
        
        StateSet* ss = node.getStateSet();
        if (ss) {
            addColorAttribute(elem, "fill", ss->getFillColor());
            addColorAttribute(elem, "stroke", ss->getStrokeColor());
            
            if(ss->isFilled() && ss->hasFillOpacity())
                elem->SetAttribute("fill-opacity", cefix::doubleToString(ss->getFillOpacity()));
            
            if(ss->isStroked() && ss->hasStrokeOpacity())
                elem->SetAttribute("stroke-opacity", cefix::doubleToString(ss->getStrokeOpacity()));
            
            if(ss->hasStrokeWidth())
                elem->SetAttribute("stroke-width", cefix::doubleToString(ss->getStrokeWidth()));
            
            if (ss->hasStrokeMiterLimit())
                elem->SetAttribute("stroke-miterlimit", cefix::doubleToString(ss->getStrokeMiterLimit()));
            
            if (ss->getFillRule() != StateSet::FR_Inherit) {
                static StateSet::FillRule s_fr[2] = { StateSet::FR_OddEven, StateSet::FR_NonZero };
                static const char* s_fr_str[2] = { "oddeven", "nonzero" };
                cefix::EnumAsString<StateSet::FillRule, 2> eas(s_fr, s_fr_str);
                elem->SetAttribute("fill-rule", eas.string(ss->getFillRule()));
            }
            if (ss->getStrokeLineCap() != StateSet::LC_Inherit) {
                static StateSet::StrokeLineCap s_lc[3] = { StateSet::LC_Butt, StateSet::LC_Round, StateSet::LC_Square };
                static const char* s_lc_str[3] = { "butt", "round", "square" };
                cefix::EnumAsString<StateSet::StrokeLineCap, 3> eas(s_lc, s_lc_str);
                elem->SetAttribute("stroke-linecap", eas.string(ss->getStrokeLineCap()));
            }
            if (ss->getStrokeLineJoin() != StateSet::LJ_Inherit) {
                static StateSet::StrokeLineJoin s_lj[3] = { StateSet::LJ_Bevel, StateSet::LJ_Miter, StateSet::LJ_Round };
                static const char* s_lj_str[3] = { "bevel", "miter", "round" };
                cefix::EnumAsString<StateSet::StrokeLineJoin, 3> eas(s_lj, s_lj_str);
                elem->SetAttribute("stroke-linejoin", eas.string(ss->getStrokeLineJoin()));
            }
            if (ss->getDisplayMode() != StateSet::DM_Inherit) {
                static StateSet::DisplayMode s_dm[3] = { StateSet::DM_Inherit, StateSet::DM_None, StateSet::DM_Inline };
                static const char* s_dm_str[3] = { "inherit", "none", "inline" };
                cefix::EnumAsString<StateSet::DisplayMode, 3> eas(s_dm, s_dm_str);
                elem->SetAttribute("display", eas.string(ss->getDisplayMode()));
            }
        }
    }
    
    virtual void visit(Document& node) {
        
        TiXmlElement* elem = newElement("svg");
        elem->SetAttribute("x", cefix::doubleToString(node.getLeft())+"px");
        elem->SetAttribute("y", cefix::doubleToString(node.getTop())+"px");
        elem->SetAttribute("width", cefix::doubleToString(node.getWidth())+"px");
        elem->SetAttribute("height", cefix::doubleToString(node.getHeight())+"px");
        elem->SetAttribute("version", "1.1");
        elem->SetAttribute("xmlns", "http://www.w3.org/2000/svg");
        elem->SetAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
        if(node.hasViewBox())
            addViewBoxAttribute(elem, node.getViewBox());
        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Group& node) {
        
        TiXmlElement* elem = newElement("g");
        if(node.hasViewBox())
            addViewBoxAttribute(elem, node.getViewBox());
        
        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Rect& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("rect");
        elem->SetAttribute("x", cefix::doubleToString(node.getLeft()));
        elem->SetAttribute("y", cefix::doubleToString(node.getTop()));
        elem->SetAttribute("width", cefix::doubleToString(node.getWidth()));
        elem->SetAttribute("height", cefix::doubleToString(node.getHeight()));

        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Ellipse& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("ellipse");
        elem->SetAttribute("cx", cefix::doubleToString(node.getCenter().x()));
        elem->SetAttribute("cy", cefix::doubleToString(node.getCenter().y()));
        elem->SetAttribute("rx", cefix::doubleToString(node.getRadiusX()));
        elem->SetAttribute("ry", cefix::doubleToString(node.getRadiusY()));

        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Circle& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("circle");
        elem->SetAttribute("cx", cefix::doubleToString(node.getCenter().x()));
        elem->SetAttribute("cy", cefix::doubleToString(node.getCenter().y()));
        elem->SetAttribute("r", cefix::doubleToString(node.getRadius()));

        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Line& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("line");
        elem->SetAttribute("x1", cefix::doubleToString(node.getStart().x()));
        elem->SetAttribute("y1", cefix::doubleToString(node.getStart().y()));
        elem->SetAttribute("x2", cefix::doubleToString(node.getEnd().x()));
        elem->SetAttribute("y2", cefix::doubleToString(node.getEnd().y()));

        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Polygon& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("polygon");
        std::string points;
        for(unsigned int i = 0; i < node.getNumPoints(); ++i) {
            points += cefix::doubleToString(node.getPointAt(i).x()) + " " + cefix::doubleToString(node.getPointAt(i).y()) + " ";
        }
        elem->SetAttribute("points", cefix::strTrim(points));
        
        addCommonAttributes(elem, node);
    }
    
    virtual void visit(PolyLine& node)
    {
        if (convertToPaths(node))
            return;
        
        TiXmlElement* elem = newElement("polyline");
        std::string points;
        for(unsigned int i = 0; i < node.getNumPoints(); ++i) {
            points += cefix::doubleToString(node.getPointAt(i).x()) + " " + cefix::doubleToString(node.getPointAt(i).y()) + " ";
        }
        elem->SetAttribute("points", cefix::strTrim(points));
        
        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Text& node) {
        TiXmlElement* elem = newElement("text");
        elem->SetAttribute("font-family", node.getFontFamily());
        elem->SetAttribute("font-size", cefix::doubleToString(node.getFontSize()));
        elem->InsertEndChild(TiXmlText(node.getText()));
        std::string s;
        for(std::vector<double>::iterator i = node.getX().begin(); i != node.getX().end(); ++i) {
            s += cefix::doubleToString(*i)+" ";
        }
        elem->SetAttribute("x", s);
        s.clear();
        
        for(std::vector<double>::iterator i = node.getY().begin(); i != node.getY().end(); ++i) {
            s += cefix::doubleToString(*i)+" ";
        }
        elem->SetAttribute("y", s);
        
        
        addCommonAttributes(elem, node);
    }
    
    virtual void visit(Path& node)
    {
        TiXmlElement* elem = createElemForPath(node);
        addCommonAttributes(elem, node);
    }
    
    bool convertToPaths(Node& node) {
        if (!_convertToPaths)
            return false;
        osg::ref_ptr<Path> path = util::Geometry::converToPath(node);
        if (path) {
            TiXmlElement* elem = createElemForPath(*path);
            addCommonAttributes(elem, node);
            
            return true;
        }
        
        return false;
    }

    
    TiXmlElement* createElemForPath(Path& node)
    {
        TiXmlElement* elem = newElement("path");
        std::stringstream pathdata;
        for(unsigned int i=0; i < node.getNumSubPaths(); ++i) {
            Path::SubPath* sub_path = node.getSubPathAt(i);
            pathdata << "M" << sub_path->startPoint.x() << " " << sub_path->startPoint.y() << " ";
            for(unsigned int j = 0; j < sub_path->components.size(); ++j) {
                Path::Component* component = sub_path->components[j];
                switch (component->getType()) {
                    case Path::Component::LineComponent:
                        pathdata << "L" << component->asLine()->getEnd();
                        break;
                    case Path::Component::CubicCurveComponent:
                        pathdata << "C" << component->asCubicCurve()->getControlPoint1() << " " << component->asCubicCurve()->getControlPoint2() << " " << component->asCubicCurve()->getEnd();
                        break;
                    case Path::Component::QuadraticCurveComponent:
                        pathdata << "Q" << component->asQuadraticCurve()->getControlPoint() << " " << component->asQuadraticCurve()->getEnd();
                        break;
                    default:
                        cefix::log::error("svg::Writer") << "can't handle path component " << component->getType() << std::endl;
                }
                if ( j % 10 == 0) pathdata << std::endl;
            }
        }
        elem->SetAttribute("d", pathdata.str());
        
        return elem;
    }
    
    
    
    bool saveTo(const std::string& file_name) {
        return _xml.SaveFile(file_name);
    }
private:
    TiXmlDocument _xml;
    std::vector<TiXmlElement*> _elemStack;
    bool _convertToPaths;
};

bool Writer::write(Document* doc, const std::string& filename, bool convert_to_paths)
{
    WriteVisitor wv(convert_to_paths);
    doc->accept(wv);
    return wv.saveTo(filename);
}


}


