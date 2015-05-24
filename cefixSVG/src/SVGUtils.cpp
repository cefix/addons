//
//  Utils.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 05.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "SVGUtils.h"
#include "SVGRect.h"
#include "SVGPolygon.h"
#include "SVGPolyLine.h"
#include "SVGPath.h"
#include "SVGLine.h"
#include "SVGEllipse.h"
#include "SVGVisitor.h"

#include <cefix/StringUtils.h>
#include <cefix/Spline.h>
#include <osg/Array>


namespace svg {





namespace util {

template <class T> struct ArrayTypeTraits {
    static T convert(const osg::Vec2d& p) { return T(); }
};

template <>struct ArrayTypeTraits<osg::Vec3f> {
    typedef osg::Vec3Array ArrayType;
    static osg::Vec3f convert(const osg::Vec2d& p) { return osg::Vec3f(p.x(), p.y(),0); }
};
template <>struct ArrayTypeTraits<osg::Vec3d> {
    typedef osg::Vec3dArray ArrayType;
    static osg::Vec3d convert(const osg::Vec2d& p) { return osg::Vec3d(p.x(), p.y(),0); }
};
template <>struct ArrayTypeTraits<osg::Vec2d> {
    typedef osg::Vec2dArray ArrayType;
    static osg::Vec2d convert(const osg::Vec2d& p) { return p; }
};
template <>struct ArrayTypeTraits<osg::Vec2f> {
    typedef osg::Vec2Array ArrayType;
    static osg::Vec2f convert(const osg::Vec2d& p) { return osg::Vec2f(p.x(), p.y()); }
};

inline osg::Vec3 asVec3(const osg::Vec2d& p) { return osg::Vec3(p.x(), p.y(), 0); }
inline osg::Vec2d asVec2(const osg::Vec3& p) { return osg::Vec2d(p.x(), p.y()); }


template <class T>
class CreateOutlineVisitor : public Visitor {
public:
    typedef typename ArrayTypeTraits<T>::ArrayType Outline;
    typedef std::vector< osg::ref_ptr<Outline> > Outlines;
    
    CreateOutlineVisitor(VisitMode mode, Outlines& outlines, osg::Geometry* geo, double max_error)
        : Visitor(mode)
        , _outlines(outlines)
        , _current(-1)
        , _geo(geo)
        , _maxError(max_error)
    {
    }
    
    virtual void visit(Node& node)
    {
        osg::ref_ptr<Path> path = Geometry::converToPath(node);
        if (path) {
            visit(*path);
        }
    }
    
    virtual void visit(Path& path) {
        for(unsigned int i = 0; i < path.getNumSubPaths(); ++i)
        {
            Outline* outline = newOutline();
            unsigned int start_ndx = outline->size();
            
            Path::SubPath* sub_path = path.getSubPathAt(i);
            outline->push_back(convert(sub_path->startPoint));
            for(Path::SubPath::Components::iterator j = sub_path->components.begin(); j != sub_path->components.end(); ++j)
            {
                Path::Component* c(*j);
                switch (c->getType()) {
                    case Path::Component::LineComponent:
                        {
                            Path::Line* line = c->asLine();
                            outline->push_back(convert(line->getEnd()));
                        }
                        break;
                        
                    case Path::Component::CubicCurveComponent:
                        {
                            Path::CubicCurve* curve = c->asCubicCurve();
                            
                            cefix::Spline2D spline;
                            spline.setMaxError(_maxError);
                            spline.controlPoints.push_back(curve->getStart());
                            spline.controlPoints.push_back(curve->getControlPoint1());
                            spline.controlPoints.push_back(curve->getControlPoint2());
                            spline.controlPoints.push_back(curve->getEnd());
                            
                            spline.buildArclenTable();     // build the arc length table
                            spline.normalizeArclenTable();
                            
                            cefix::Spline2D::ArclenTable::iterator ai = spline.arclentable.begin();
                            ai++; // skip first vertice
                            while (ai != spline.arclentable.end())
                            {
                                outline->push_back(convert(ai->pos));
                                ++ai;
                            }
                        }
                        break;
                    default:
                        cefix::log::error("CreateOutlineVisitor") << "unhandled path component " << c->getType() << std::endl;
                        break;
                }
            }
            unsigned int end_ndx = outline->size();
            if (_geo.valid()) {
                _geo->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON, start_ndx, end_ndx - start_ndx));
            }
        }
    }
    
    
private:
    inline T convert(const osg::Vec2d& p) { return ArrayTypeTraits<T>::convert(p); }
    
    Outline* newOutline()
    {
        if (_geo.valid() && (_current == 0)) {
            return _outlines[_current];
        }
        
        _current++;
        if (_outlines.size() <= _current)
            _outlines.resize(_current+1);
        if (_outlines[_current] == 0)
            _outlines[_current] = new Outline();
        return _outlines[_current];
    }
    
    
protected:
    Outlines& _outlines;
    int _current;
    osg::ref_ptr<osg::Geometry> _geo;
    double _maxError;
    
};


void Geometry::create2DOutlines(Node& node, Outlines2D& outlines, double max_error)
{
    CreateOutlineVisitor<osg::Vec2d> v(Visitor::VisitVisibleElements, outlines, NULL, max_error);
    node.accept(v);
}

void Geometry::create3DOutlines(Node& node, Outlines3D& outlines, double max_error)
{
    CreateOutlineVisitor<osg::Vec3f> v(Visitor::VisitVisibleElements, outlines, NULL, max_error);
    node.accept(v);
}

osg::Geometry* Geometry::createOutlines(Node& node, double max_error)
{
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry();
    Outlines3D outlines;
    {
        CreateOutlineVisitor<osg::Vec3f> v(Visitor::VisitVisibleElements, outlines, geo.get(), max_error);
        node.accept(v);
    }
    if (outlines.size() > 0) {
        geo->setVertexArray(outlines[0]);
        return geo.release();
    }
    
    return NULL;
}


Path* Geometry::converToPath(Node& node)
{
    if (node.asPath())
        return node.asPath();
    
    osg::ref_ptr<Path> path = new Path();
    
    if (Rect* rect = node.asRect())
    {
        double l(rect->getLeft()), t(rect->getTop()), w(rect->getWidth()), h(rect->getHeight());
        path->moveTo(l, t+h);
        path->lineTo(l+w, t+h);
        path->lineTo(l+w, t);
        path->lineTo(l, t);
        path->close();
    }
    else if (Line* line = node.asLine())
    {
        path->moveTo(line->getStart());
        path->lineTo(line->getEnd());
    }
    else if (PolyLine* line = node.asPolyLine())
    {
        path->moveTo(line->getPointAt(0));
        for(unsigned int i = 1; i < line->getNumPoints(); ++i)
        {
            path->lineTo(line->getPointAt(i));
        }
    }
    else if (Polygon* line = node.asPolygon())
    {
        path->moveTo(line->getPointAt(0));
        for(unsigned int i = 1; i < line->getNumPoints(); ++i)
        {
            path->lineTo(line->getPointAt(i));
        }
        path->close();
    }
    else if (Ellipse* e = node.asEllipse())
    {
        static double kappa = 4 * (sqrt(2.0) -1 ) / 3.0;
        double x(e->getCenterX()),y(e->getCenterY()), radius_x(e->getRadiusX()), radius_y(e->getRadiusY());
        double ox(kappa * radius_x), oy(kappa * radius_y);
        
        path->moveTo(x - radius_x, y);
        path->cubicCurveTo(x - radius_x, y - oy, x - ox, y - radius_y, x, y - radius_y);
        path->cubicCurveTo(x + ox, y - radius_y, x + radius_x, y - oy, x + radius_x, y);
        path->cubicCurveTo(x + radius_x, y + oy, x + ox, y + radius_y, x, y + radius_y);
        path->cubicCurveTo(x - ox, y + radius_y, x - radius_x, y + oy, x - radius_x, y);
    }
    else
    {
        return NULL;
    }
    
    return path.release();
}

inline osg::Vec2 rotate2d(const osg::Vec2d& p, double angle)
{
    double t_cos(cos(angle)), t_sin(sin(angle));
    osg::Vec2d result;
    result.x() = p.x() * t_cos - p.y() *t_sin;
    result.y() = p.x() * t_sin + p.y() *t_cos;
    
    return result;
}

inline void addLineJoin(osg::Vec3Array* v, osg::DrawElementsUShort* da, Geometry::Outline2D* outline, svg::StateSet::StrokeLineJoin lj, int l, int r, int m, double miter_limit, const osg::Vec2d& dir_to_curr, const osg::Vec2d& dir_to_next)
{
    switch(lj) {
        
        case StateSet::LJ_Round:
            
            {
                osg::Vec2d mp = asVec2((*v)[m]);
                osg::Vec2d lp_1 = asVec2((*v)[l]);
                osg::Vec2d rp_1 = asVec2((*v)[r]);
                osg::Vec2d dir(lp_1 - mp);
                double angle = cefix::getAngleBetween(dir, rp_1 - mp);
                if (angle > osg::PI) {
                    dir = (rp_1 - mp);
                    angle = 2*osg::PI - angle;
                }
                double delta = osg::PI/100.0;
                
                int steps = std::ceil(angle / delta);
                if (steps <= 1)
                {
                    da->push_back(r);
                    da->push_back(l);
                    da->push_back(m);
                }
                else
                {
                    delta = (angle) / steps;
                    for(unsigned int i = 1; i <= steps; ++i)
                    {
                        double a_s( (i-1) * delta);
                        double a_e( (i-0) * delta);
                        
                        osg::Vec2d l = mp + rotate2d(dir, a_s);
                        osg::Vec2d r = mp + rotate2d(dir, a_e);
                        
                        v->push_back(asVec3(l));
                        v->push_back(asVec3(r));
                        da->push_back(v->size() - 2);
                        da->push_back(v->size() - 1);
                        da->push_back(m);
                    }
                }
            }
            break;
        case StateSet::LJ_Miter:
            {
                osg::Vec2d lp_1 = asVec2((*v)[l]);
                osg::Vec2d rp_1 = asVec2((*v)[r]);
                
                double theta = cefix::getAngleBetween(-dir_to_curr, dir_to_next);
                static double max_error = osg::PI * 0.01;
                if ((theta < osg::PI - max_error) || (theta > osg::PI+max_error)) {
                    double miter_val = 1.0 / sin(theta / 2.0);
                    if (miter_val <= miter_limit) {
                    
                        osg::Vec2d rp_2 = rp_1 - (dir_to_next * miter_limit);
                        osg::Vec2d lp_2 = lp_1 + (dir_to_curr * miter_limit);
                        
                        osg::Vec2d p;
                        
                        if (cefix::getRayRayIntersection(lp_1, lp_2, rp_1, rp_2, p))
                        {
                            v->push_back(asVec3(p));
                            da->push_back(r );
                            da->push_back(v->size() - 1);
                            da->push_back(m);
                            
                            da->push_back(l );
                            da->push_back(v->size() - 1);
                            da->push_back(m);
                            break;
                        }
                    }
                }
                // fall-through to bevel-handling
            }
        default:
            da->push_back(r);
            da->push_back(l);
            da->push_back(m);
            break;
    }
}


void addLineCap(const osg::Vec2d& p, const osg::Vec2d& dir, osg::Vec3Array* v, osg::DrawElementsUShort* da, double stroke_width, svg::StateSet::StrokeLineCap line_cap)
{
    switch (line_cap) {
        case (svg::StateSet::LC_Square):
            {
                osg::Vec2d d(dir);
                d.normalize();
                d *= 0.5 * stroke_width;
                osg::Vec2d perp = osg::Vec2d(-d.y(), d.x());
                
                (*v).push_back(asVec3(p-perp));
                (*v).push_back(asVec3(p+perp));
                (*v).push_back(asVec3(p-perp-d));
                (*v).push_back(asVec3(p+perp-d));
                
                da->push_back(v->size() - 2);
                da->push_back(v->size() - 4);
                da->push_back(v->size() - 3);
                
                da->push_back(v->size() - 1);
                da->push_back(v->size() - 2);
                da->push_back(v->size() - 3);
                
            }
            break;
        case (svg::StateSet::LC_Round):
            {
                double angle = osg::PI;
                double delta = osg::PI/100.0;
                int steps = angle / delta;
                
                delta = (angle) / steps;
                osg::Vec2d d(dir);
                d.normalize();
                d *= 0.5 * stroke_width;
                osg::Vec2d perp = osg::Vec2d(-d.y(), d.x());
                v->push_back(asVec3(p));
                
                unsigned int m = v->size() - 1;
                for(unsigned int i = 1; i <= steps; ++i)
                {
                    double a_s( (i-1) * delta);
                    double a_e( (i-0) * delta);
                    
                    osg::Vec2d l = p + rotate2d(perp, a_s);
                    osg::Vec2d r = p + rotate2d(perp, a_e);
                    
                    v->push_back(asVec3(l));
                    v->push_back(asVec3(r));
                    da->push_back(v->size() - 2);
                    da->push_back(v->size() - 1);
                    da->push_back(m);
                }
            }
            break;
        default:
            break;
    };
}

osg::Geometry* Geometry::createStroke(Outlines2D& outlines, double stroke_width, double miter_limit, StateSet::StrokeLineJoin line_join, StateSet::StrokeLineCap line_cap, double max_error)
{
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry();
    osg::DrawElementsUShort* da = new osg::DrawElementsUShort(GL_TRIANGLES);
    osg::Vec3Array* v = new osg::Vec3Array();
    geo->setVertexArray(v);
    geo->addPrimitiveSet(da);
    
    for(unsigned int i = 0; i < outlines.size(); ++i) {
        Outline2D* outline = outlines[i];
        unsigned num_p = outline->size();
        if (num_p < 2)
            continue;
        bool is_closed = ((*outline)[0] == (*outline)[num_p-1]);
        
        unsigned int max =  num_p;
        if(is_closed) {
            while((*outline)[max-1] == (*outline)[0])
                max--;
        }
        
        int prev_tl_ndx(-1), prev_bl_ndx(-1), prev_tr_ndx(-1), prev_br_ndx(-1), prev_ml_ndx(-1), prev_mr_ndx(-1);
        int curr_tl_ndx(-1), curr_bl_ndx(-1), curr_tr_ndx(-1), curr_br_ndx(-1), curr_ml_ndx(-1), curr_mr_ndx(-1);
        int first_tl_ndx(-1), first_bl_ndx(-1), first_tr_ndx(-1), first_br_ndx(-1), first_ml_ndx(-1), first_mr_ndx(-1);
        
        osg::Vec2d prev_tl, prev_bl, prev_tr, prev_br;
        osg::Vec2d first_tl, first_bl, first_tr, first_br;
        
        for(unsigned int j = 0; j < max; ++j)
        {
            unsigned int prev_ndx = (j > 0) ? j-1 : j;
            unsigned int next_ndx = (j < max-1) ? j+1 : is_closed ? 0 : j;
            // std::cout << j << " -> " << next_ndx << std::endl;
            osg::Vec2d current((*outline)[j]);
            osg::Vec2d next_tl, next_bl, next_tr, next_br;
            
            if (next_ndx != j) {
                osg::Vec2d dir_to_next = (*outline)[next_ndx] - current;
                osg::Vec2d dir_to_curr = current - (*outline)[prev_ndx];
                
                dir_to_next.normalize();
                dir_to_curr.normalize();
            
                osg::Vec2d perpendicular = osg::Vec2d(-dir_to_next.y(), dir_to_next.x());
            
                osg::Vec2d half_width = perpendicular * stroke_width * 0.5;
                next_tl = current + half_width;
                next_bl = current - half_width;
                next_tr = (*outline)[next_ndx] + half_width;
                next_br = (*outline)[next_ndx] - half_width;
                bool join_line_top(false), join_line_bottom(false);
                
                if(prev_ndx != j) {
                    
                    osg::Vec2d result_t, result_b;
                    
                    if (cefix::getLineLineIntersection(prev_tl, prev_tr, next_tl, next_tr, result_t, max_error)) {
                        next_tl = result_t;
                        (*v)[prev_tr_ndx].set(result_t.x(), result_t.y(), 0);
                    } else {
                        join_line_top = true;
                    }
                    if (cefix::getLineLineIntersection(prev_bl, prev_br, next_bl, next_br, result_b, max_error)) {
                        next_bl = result_b;
                        (*v)[prev_br_ndx].set(result_b.x(), result_b.y(), 0);
                    } else {
                        // add line join bottom
                        join_line_bottom = true;
                    }
                    
                }
                
                v->push_back(asVec3(current));
                v->push_back(asVec3((*outline)[next_ndx]));
                v->push_back(asVec3(next_tl));
                v->push_back(asVec3(next_bl));
                v->push_back(asVec3(next_tr));
                v->push_back(asVec3(next_br));
                
                unsigned int vs = v->size();
                curr_tl_ndx = vs - 4;
                curr_bl_ndx = vs - 3;
                curr_tr_ndx = vs - 2;
                curr_br_ndx = vs - 1;
                curr_ml_ndx = vs - 6;
                curr_mr_ndx = vs - 5;
                
                // add 4 triangles
                da->push_back(curr_tl_ndx);
                da->push_back(curr_ml_ndx);
                da->push_back(curr_tr_ndx);
                
                da->push_back(curr_ml_ndx);
                da->push_back(curr_mr_ndx);
                da->push_back(curr_tr_ndx);
                
                da->push_back(curr_ml_ndx);
                da->push_back(curr_bl_ndx);
                da->push_back(curr_mr_ndx);
                
                da->push_back(curr_bl_ndx);
                da->push_back(curr_br_ndx);
                da->push_back(curr_mr_ndx);
                
                if (join_line_top)
                    addLineJoin(v, da, outline, line_join, prev_tr_ndx, curr_tl_ndx, curr_ml_ndx, miter_limit, dir_to_curr, dir_to_next);
                if (join_line_bottom)
                    addLineJoin(v, da, outline, line_join, prev_br_ndx, curr_bl_ndx, curr_ml_ndx, miter_limit, dir_to_curr, dir_to_next);
                
                prev_ml_ndx = curr_ml_ndx;
                prev_mr_ndx = curr_mr_ndx;
                prev_tl_ndx = curr_tl_ndx;
                prev_bl_ndx = curr_bl_ndx;
                prev_tr_ndx = curr_tr_ndx;
                prev_br_ndx = curr_br_ndx;
                
                prev_tl = next_tl;
                prev_bl = next_bl;
                prev_tr = next_tr;
                prev_br = next_br;

                if ((j == 0) && is_closed) {
                    first_ml_ndx = curr_ml_ndx;
                    first_mr_ndx = curr_mr_ndx;
                    first_tl_ndx = curr_tl_ndx;
                    first_bl_ndx = curr_bl_ndx;
                    first_tr_ndx = curr_tr_ndx;
                    first_br_ndx = curr_br_ndx;
                    
                    first_tl = next_tl;
                    first_bl = next_bl;
                    first_tr = next_tr;
                    first_br = next_br;
                }
            }
                        
        }
        if(is_closed) {
            // fix last corner
            
            osg::Vec2d result_t, result_b;
            bool join_line_top(false), join_line_bottom(false);
                    
            if (cefix::getLineLineIntersection(prev_tl, prev_tr, first_tl, first_tr, result_t, max_error)) {
                (*v)[first_tl_ndx].set(result_t.x(), result_t.y(), 0);
                (*v)[prev_tr_ndx].set(result_t.x(), result_t.y(), 0);
            } else {
                join_line_top = true;
            }
            if (cefix::getLineLineIntersection(prev_bl, prev_br, first_bl, first_br, result_b, max_error)) {
                (*v)[first_bl_ndx].set(result_b.x(), result_b.y(), 0);
                (*v)[prev_br_ndx].set(result_b.x(), result_b.y(), 0);
            } else {
                // add line join bottom
                join_line_bottom = true;
            }
            
            osg::Vec2d dir_to_next = (*outline)[1] - (*outline)[0];
            osg::Vec2d dir_to_curr = (*outline)[0] - (*outline)[max-1];
            
            dir_to_next.normalize();
            dir_to_curr.normalize();
        
            if (join_line_top)
                addLineJoin(v, da, outline, line_join, prev_tr_ndx, first_tl_ndx, first_ml_ndx, miter_limit, dir_to_curr, dir_to_next);
            if (join_line_bottom)
                addLineJoin(v, da, outline, line_join, prev_br_ndx, first_bl_ndx, first_ml_ndx, miter_limit, dir_to_curr, dir_to_next);
            
        }
        else {
            addLineCap((*outline)[0], (*outline)[1] - (*outline)[0], v, da, stroke_width, line_cap);
            addLineCap((*outline)[num_p-1], (*outline)[num_p-2] - (*outline)[num_p-1], v, da, stroke_width, line_cap);
        }
    }
    
    return geo.release();
}


 osg::Geometry* Geometry::createStroke(Node& node, double stroke_width, double miter_limit, StateSet::StrokeLineJoin line_join, StateSet::StrokeLineCap line_cap, double max_error)
{
    Outlines2D outlines;
    create2DOutlines(node, outlines, max_error);
    
    return createStroke(outlines, stroke_width, miter_limit, line_join, line_cap);
}

} // end namespace util
} // end namespace svg

