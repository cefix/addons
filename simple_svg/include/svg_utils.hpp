#pragma once

#include <osg/Vec4>
#include "simple_svg.hpp"

namespace cefix {

svg::Color toSVGColor(const osg::Vec4& c) {
    return svg::Color(c[0]*255, c[1]*255, c[2]*255, c[3]);
}

svg::Point toSVGPoint(const osg::Vec2& p) {
    return svg::Point(p.x(), p.y());
}

}