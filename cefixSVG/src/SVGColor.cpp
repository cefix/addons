//
//  SVGColor.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 10.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGColor.h"
#include <map>
#include <cefix/StringUtils.h>

namespace svg {

osg::Vec4d rgb(unsigned int r, unsigned int g, unsigned int b) {
    return osg::Vec4d(r/255.0, g/255.0, b/255.0, 1);
}

Color::Color(const std::string& name)
    : _color(0,0,0,1)
    , _hasColor(false)
{
    std::string lc_name = cefix::strToLower(name);
    if(lc_name == "none")
        return;
    
    typedef std::map<std::string, osg::Vec4d> NameColorMap;
    static NameColorMap name_color_map;
    if (name_color_map.size() == 0) {
        name_color_map["aliceblue"] = rgb(240, 248, 255);
        name_color_map["antiquewhite"] = rgb(250, 235, 215);
        name_color_map["aqua"] = rgb( 0, 255, 255);
        name_color_map["aquamarine"] = rgb(127, 255, 212);
        name_color_map["azure"] = rgb(240, 255, 255);
        name_color_map["beige"] = rgb(245, 245, 220);
        name_color_map["bisque"] = rgb(255, 228, 196);
        name_color_map["black"] = rgb( 0, 0, 0);
        name_color_map["blanchedalmond"] = rgb(255, 235, 205);
        name_color_map["blue"] = rgb( 0, 0, 255);
        name_color_map["blueviolet"] = rgb(138, 43, 226);
        name_color_map["brown"] = rgb(165, 42, 42);
        name_color_map["burlywood"] = rgb(222, 184, 135);
        name_color_map["cadetblue"] = rgb( 95, 158, 160);
        name_color_map["chartreuse"] = rgb(127, 255, 0);
        name_color_map["chocolate"] = rgb(210, 105, 30);
        name_color_map["coral"] = rgb(255, 127, 80);
        name_color_map["cornflowerblue"] = rgb(100, 149, 237);
        name_color_map["cornsilk"] = rgb(255, 248, 220);
        name_color_map["crimson"] = rgb(220, 20, 60);
        name_color_map["cyan"] = rgb( 0, 255, 255);
        name_color_map["darkblue"] = rgb( 0, 0, 139);
        name_color_map["darkcyan"] = rgb( 0, 139, 139);
        name_color_map["darkgoldenrod"] = rgb(184, 134, 11);
        name_color_map["darkgray"] = rgb(169, 169, 169);
        name_color_map["darkgreen"] = rgb( 0, 100, 0);
        name_color_map["darkgrey"] = rgb(169, 169, 169);
        name_color_map["darkkhaki"] = rgb(189, 183, 107);
        name_color_map["darkmagenta"] = rgb(139, 0, 139);
        name_color_map["darkolivegreen"] = rgb( 85, 107, 47);
        name_color_map["darkorange"] = rgb(255, 140, 0);
        name_color_map["darkorchid"] = rgb(153, 50, 204);
        name_color_map["darkred"] = rgb(139, 0, 0);
        name_color_map["darksalmon"] = rgb(233, 150, 122);
        name_color_map["darkseagreen"] = rgb(143, 188, 143);
        name_color_map["darkslateblue"] = rgb( 72, 61, 139);
        name_color_map["darkslategray"] = rgb( 47, 79, 79);
        name_color_map["darkslategrey"] = rgb( 47, 79, 79);
        name_color_map["darkturquoise"] = rgb( 0, 206, 209);
        name_color_map["darkviolet"] = rgb(148, 0, 211);
        name_color_map["deeppink"] = rgb(255, 20, 147);
        name_color_map["deepskyblue"] = rgb( 0, 191, 255);
        name_color_map["dimgray"] = rgb(105, 105, 105);
        name_color_map["dimgrey"] = rgb(105, 105, 105);
        name_color_map["dodgerblue"] = rgb( 30, 144, 255);
        name_color_map["firebrick"] = rgb(178, 34, 34);
        name_color_map["floralwhite"] = rgb(255, 250, 240);
        name_color_map["forestgreen"] = rgb( 34, 139, 34);
        name_color_map["fuchsia"] = rgb(255, 0, 255);
        name_color_map["gainsboro"] = rgb(220, 220, 220);
        name_color_map["ghostwhite"] = rgb(248, 248, 255);
        name_color_map["gold"] = rgb(255, 215, 0);
        name_color_map["goldenrod"] = rgb(218, 165, 32);
        name_color_map["gray"] = rgb(128, 128, 128);
        name_color_map["grey"] = rgb(128, 128, 128);
        name_color_map["green"] = rgb( 0, 128, 0);
        name_color_map["greenyellow"] = rgb(173, 255, 47);
        name_color_map["honeydew"] = rgb(240, 255, 240);
        name_color_map["hotpink"] = rgb(255, 105, 180);
        name_color_map["indianred"] = rgb(205, 92, 92);
        name_color_map["indigo"] = rgb( 75, 0, 130);
        name_color_map["ivory"] = rgb(255, 255, 240);
        name_color_map["khaki"] = rgb(240, 230, 140);
        name_color_map["lavender"] = rgb(230, 230, 250);
        name_color_map["lavenderblush"] = rgb(255, 240, 245);
        name_color_map["lawngreen"] = rgb(124, 252, 0);
        name_color_map["lemonchiffon"] = rgb(255, 250, 205);
        name_color_map["lightblue"] = rgb(173, 216, 230);
        name_color_map["lightcoral"] = rgb(240, 128, 128);
        name_color_map["lightcyan"] = rgb(224, 255, 255);
        name_color_map["lightgoldenrodyellow"] = rgb(250, 250, 210);
        name_color_map["lightgray"] = rgb(211, 211, 211);
        name_color_map["lightgreen"] = rgb(144, 238, 144);
        name_color_map["lightgrey"] = rgb(211, 211, 211);
        name_color_map["lightpink"] = rgb(255, 182, 193);
        name_color_map["lightsalmon"] = rgb(255, 160, 122);
        name_color_map["lightseagreen"] = rgb( 32, 178, 170);
        name_color_map["lightskyblue"] = rgb(135, 206, 250);
        name_color_map["lightslategray"] = rgb(119, 136, 153);
        name_color_map["lightslategrey"] = rgb(119, 136, 153);
        name_color_map["lightsteelblue"] = rgb(176, 196, 222);
        name_color_map["lightyellow"] = rgb(255, 255, 224);
        name_color_map["lime"] = rgb( 0, 255, 0);
        name_color_map["limegreen"] = rgb( 50, 205, 50);
        name_color_map["linen"] = rgb(250, 240, 230);
        name_color_map["magenta"] = rgb(255, 0, 255);
        name_color_map["maroon"] = rgb(128, 0, 0);
        name_color_map["mediumaquamarine"] = rgb(102, 205, 170);
        name_color_map["mediumblue"] = rgb( 0, 0, 205);
        name_color_map["mediumorchid"] = rgb(186, 85, 211);
        name_color_map["mediumpurple"] = rgb(147, 112, 219);
        name_color_map["mediumseagreen"] = rgb( 60, 179, 113);
        name_color_map["mediumslateblue"] = rgb(123, 104, 238);
        name_color_map["mediumspringgreen"] = rgb( 0, 250, 154);
        name_color_map["mediumturquoise"] = rgb( 72, 209, 204);
        name_color_map["mediumvioletred"] = rgb(199, 21, 133);
        name_color_map["midnightblue"] = rgb( 25, 25, 112);
        name_color_map["mintcream"] = rgb(245, 255, 250);
        name_color_map["mistyrose"] = rgb(255, 228, 225);
        name_color_map["moccasin"] = rgb(255, 228, 181);
        name_color_map["navajowhite"] = rgb(255, 222, 173);
        name_color_map["navy"] = rgb( 0, 0, 128);
        name_color_map["oldlace"] = rgb(253, 245, 230);
        name_color_map["olive"] = rgb(128, 128, 0);
        name_color_map["olivedrab"] = rgb(107, 142, 35);
        name_color_map["orange"] = rgb(255, 165, 0);
        name_color_map["orangered"] = rgb(255, 69, 0);
        name_color_map["orchid"] = rgb(218, 112, 214);
        name_color_map["palegoldenrod"] = rgb(238, 232, 170);
        name_color_map["palegreen"] = rgb(152, 251, 152);
        name_color_map["paleturquoise"] = rgb(175, 238, 238);
        name_color_map["palevioletred"] = rgb(219, 112, 147);
        name_color_map["papayawhip"] = rgb(255, 239, 213);
        name_color_map["peachpuff"] = rgb(255, 218, 185);
        name_color_map["peru"] = rgb(205, 133, 63);
        name_color_map["pink"] = rgb(255, 192, 203);
        name_color_map["plum"] = rgb(221, 160, 221);
        name_color_map["powderblue"] = rgb(176, 224, 230);
        name_color_map["purple"] = rgb(128, 0, 128);
        name_color_map["red"] = rgb(255, 0, 0);
        name_color_map["rosybrown"] = rgb(188, 143, 143);
        name_color_map["royalblue"] = rgb( 65, 105, 225);
        name_color_map["saddlebrown"] = rgb(139, 69, 19);
        name_color_map["salmon"] = rgb(250, 128, 114);
        name_color_map["sandybrown"] = rgb(244, 164, 96);
        name_color_map["seagreen"] = rgb( 46, 139, 87);
        name_color_map["seashell"] = rgb(255, 245, 238);
        name_color_map["sienna"] = rgb(160, 82, 45);
        name_color_map["silver"] = rgb(192, 192, 192);
        name_color_map["skyblue"] = rgb(135, 206, 235);
        name_color_map["slateblue"] = rgb(106, 90, 205);
        name_color_map["slategray"] = rgb(112, 128, 144);
        name_color_map["slategrey"] = rgb(112, 128, 144);
        name_color_map["snow"] = rgb(255, 250, 250);
        name_color_map["springgreen"] = rgb( 0, 255, 127);
        name_color_map["steelblue"] = rgb( 70, 130, 180);
        name_color_map["tan"] = rgb(210, 180, 140);
        name_color_map["teal"] = rgb( 0, 128, 128);
        name_color_map["thistle"] = rgb(216, 191, 216);
        name_color_map["tomato"] = rgb(255, 99, 71);
        name_color_map["turquoise"] = rgb( 64, 224, 208);
        name_color_map["violet"] = rgb(238, 130, 238);
        name_color_map["wheat"] = rgb(245, 222, 179);
        name_color_map["white"] = rgb(255, 255, 255);
        name_color_map["whitesmoke"] = rgb(245, 245, 245);
        name_color_map["yellow"] = rgb(255, 255, 0);
        name_color_map["yellowgreen"] = rgb(154, 205, 50);
    }
    
    NameColorMap::iterator itr = name_color_map.find(name);
    if (itr != name_color_map.end()) {
        _color = itr->second;
        _hasColor = true;
    }
}

}


std::ostream& operator<<(std::ostream& os, const svg::Color& color) {
    if(color.hasColor())
        os << color.get();
    else
        os << "none";
    
    return os;
}