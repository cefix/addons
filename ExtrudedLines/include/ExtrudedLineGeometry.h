//
//  ExtrudedLineGeometry.h
//  cefixSketch
//
//  Created by Stephan Huber on 06.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

namespace cefix {

namespace ExtrudedLine {


/** base class storing a point on a line with two normals pointing up and left and a width and a height. 
 *  the normals are computed on the fly
 */
struct Point  
{    
    Point() : point(), hN(), vN(), width(0), height(0), up(osg::Y_AXIS) {}
    Point(const osg::Vec3& in_p, double in_w, double in_h, const osg::Vec3& in_up = osg::Y_AXIS) 
    :   point(in_p), 
        hN(), 
        vN(), 
        width(in_w), 
        height(in_h), 
        up(in_up)
    {
    }
    
    osg::Vec3 point;
    osg::Vec3 hN, vN;
    double width, height;
    osg::Vec3 up;
};

/** base line geometry, storing a vector of Points and providing an interface to add, get and set points */
class BaseGeometry : public osg::Geometry {
public:
    /// ctor
    BaseGeometry() 
    :   osg::Geometry(), 
        _lineVertices(),
        _defaultUpVector(osg::Z_AXIS),
        _useCaps(false)
    {
    }
    typedef std::vector<Point> LineVertices;
    
    /// get num of line vertices
    unsigned int getNumLineVertices() { return _lineVertices.size() + (_useCaps * 2); }
    
    /// get a point at a specific index
    Point& getAt(unsigned int i) 
    { 
        if (!_useCaps)
            return _lineVertices[i];
        if (i == 0)
            return _startCaps;
        if (i < getNumLineVertices()-1)
            return _lineVertices[i-1];
        return _endCaps;
    }
    
    /// get all line vertices
    LineVertices& getLineVertices() { return _lineVertices; }
    const LineVertices& getLineVertices() const { return _lineVertices; }
    
    /// clear all line vertices
    void clear() { _lineVertices.clear(); }
    
    /// set the line-vertices
    void setLineVertices(const LineVertices& v) { _lineVertices = v; update(); }
    
    /// add a point
    void add(const Point& p) { _lineVertices.push_back(p); }
   
    /// add a point
    void add(const osg::Vec3& in_p, double in_w, double in_h, const osg::Vec3& in_up = osg::Y_AXIS) {
        add(Point(in_p, in_w, in_h, in_up));
    }
    
    // set a point at a specific index
    void setAt(unsigned int i, const Point& p) { _lineVertices[_useCaps ? i-1 : i] = p; }
    
    
    // update the geometry from the list of points
    virtual void update() = 0;
    
    /// set the default up-vector
    void setDefaultUpVector(const osg::Vec3& p) { _defaultUpVector = p; }
    
    /// set use  of caps. If set to true, the line has two additional points, one
    /// at the beginning, one at the line-ending
    void setUseCaps(bool b) { _useCaps = b; update(); }
protected:
    void computeCaps();
    
    LineVertices _lineVertices;
    osg::Vec3 _defaultUpVector;
    bool _useCaps;
    Point _startCaps, _endCaps;
};


/**
 * ColorPolicyBase class, color-policies are specific classes creating color-arrays
 * for a ExtrudedLineGeometry
 */
class ColorPolicyBase {
public:
    /// ctor
    ColorPolicyBase() {}
    
    /// setupColors, called at init-time
    void setupColors(BaseGeometry* geo, unsigned int num_vertices_per_slice) {}
    
    /// begin with generation
    void beginColors(BaseGeometry* geo, unsigned int num_vertices_per_slice) {} 
    
    // generate a specific color for a point
    void computeColors(BaseGeometry* geo, unsigned int i, const Point& p) {}
    
    // finished with generation
    void finishColors(BaseGeometry* geo, unsigned int num_vertices_per_slice) {}
};


/**
 * TexturePolicyBase class, texture-policies are specific classes creating texture-coordinate-arrays
 * for a ExtrudedLineGeometry
 */
 
class TexturePolicyBase {
public:
    /// ctor
    TexturePolicyBase() {}
    
    // called on init
    void setupTextureCoords(BaseGeometry* geo, unsigned int num_vertices_per_slice) {}
    
    // called on begin oft geometry generation
    void beginTextureCoords(BaseGeometry* geo, unsigned int num_vertices_per_slice) {}
    
    // generate texcords for a specifc point
    void computeTextureCoords(BaseGeometry* geo, unsigned int i, const Point& p) {}
    
    // finished with generation
    void finishTextureCoords(BaseGeometry* geo, unsigned int num_vertices_per_slice) {}
};


/**
 * No color policy, if used, the geometry has no color 
 */
class NoColorPolicy : public ColorPolicyBase {
public:
};


/**
 * No texture policy, if used, the geometry has no tex-coords 
 */

class NoTexturePolicy : public TexturePolicyBase {

};


/**
 * single color policy, the color of the geometry is based on a single color
 */
class SingleColorPolicy : public ColorPolicyBase {
public:
    SingleColorPolicy() 
    :   ColorPolicyBase() 
    { 
        _colors = new osg::Vec4Array(1);
    }
    
    void setupColors(BaseGeometry* geo, unsigned int num_vertices_per_slice)
    {
        geo->setColorArray(_colors);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);
    }
    
    void setColor(const osg::Vec4& c) { (*_colors)[0] = c; }
     
private:
    osg::Vec4Array* _colors;
};



/**
 * Base class for generating vertices for a extruded slice along the line
 */
class SliceGeneratorBase {
public:
    /// ctor
    SliceGeneratorBase(unsigned int num_vertices_per_slice) : _numVerticesPerSlice(num_vertices_per_slice) {}
    
    /// get num vertices per slice
    unsigned int getNumVerticesPerSlice() const { return _numVerticesPerSlice; }
    
    /// set num vertices per slice
    void setNumVerticesPerSlice(unsigned int p) { _numVerticesPerSlice = p; }
    
    /// setup
    void setup(BaseGeometry* geo) {}
    
    /// compute the slice-vertices for a specific point
    void compute(unsigned int i, const Point& p) {}
    
    /// begin the computation
    void begin(BaseGeometry* geo) {}
    
    /// finish the computation
    void finish(BaseGeometry* geo) {}
    
private:
    unsigned int _numVerticesPerSlice;
};


/**
 * a slice generator generating debug output, for every point two normals are 
 * generated and the line itself
 */
class DebugGenerator : public SliceGeneratorBase {
public:
    DebugGenerator();
    
    void setup(BaseGeometry* geo);
    void begin(BaseGeometry* geo);
    void compute(BaseGeometry* geo, unsigned int i, const Point& p);
    void finish(BaseGeometry* geo);
    
private:
    osg::Vec3Array* _vertices;
    osg::Vec4Array* _colors;
    osg::DrawArrays* _da;

};

/**
 * a slice generator, creating the slice from a given polygon 
 */
class PolygonSliceGenerator : public SliceGeneratorBase {
public:
    typedef std::vector<osg::Vec2> Polygon;
    
    /// ctor
    PolygonSliceGenerator(bool closed) : SliceGeneratorBase(0), _closed(closed) {}
    
    /// set the polygon-data
    void setPolygon(Polygon& p) { _polygon = p; setNumVerticesPerSlice(p.size()); }
    
    void setup(BaseGeometry* geo);
    void begin(BaseGeometry* geo);
    void compute(BaseGeometry* geo, unsigned int i, const Point& p);
    void finish(BaseGeometry* geo);

    /// true, if the polyon is closed, or not (e.g. for lines)
    bool isClosed() const { return _closed; }
protected:
    unsigned int getNumIndicesPerSlice() const 
    {
        return isClosed() ? 6 * getNumVerticesPerSlice() : 6 * (getNumVerticesPerSlice()-1);
    }
    
    Polygon _polygon;
    osg::Vec3Array* _vertices; 
    osg::Vec3Array* _normals;
    osg::DrawElementsUShort* _indices;
    osg::DrawArrays* _points;
    bool _closed;
};


/**
 * creates a quad per slice 
 */
class QuadSliceGenerator : public PolygonSliceGenerator {
public:
    QuadSliceGenerator() 
    :   PolygonSliceGenerator(true) 
    {
        Polygon p;
        p.resize(4);
        p[0].set(-1,-1);
        p[1].set( 1,-1);
        p[2].set( 1, 1);
        p[3].set(-1, 1);
        setPolygon(p);
    }
};


/** 
 * creates a line per slice
 */
class LineSliceGenerator : public PolygonSliceGenerator {
public:
    LineSliceGenerator() 
    :   PolygonSliceGenerator(false) 
    {
        Polygon p;
        p.resize(2);
        p[0].set(-1,0);
        p[1].set( 1,0);
        setPolygon(p);
    }
};


/**
 * template class creating a circle-slice generator with NUM_INDICES vertices per slice
 */
template<int NUM_INDICES>
class CircleSliceGenerator : public PolygonSliceGenerator {
public:
    CircleSliceGenerator() 
    :   PolygonSliceGenerator(true) 
    {
        Polygon p;
        p.resize(NUM_INDICES);
        for(unsigned int i=0; i < NUM_INDICES; ++i) {
            double scalar = i / static_cast<double>(NUM_INDICES);
            p[i].set(cos(scalar * 2 * osg::PI), sin(scalar * 2 * osg::PI));
        }
        setPolygon(p);
    }
};


/**
 * template class creating a star-slice generator with NUM_INDICES vertices per slice
 */
template<int NUM_INDICES>
class StarSliceGenerator : public PolygonSliceGenerator {
public:
    StarSliceGenerator(double inner_scale = 0.5) 
    :   PolygonSliceGenerator(true)
    {
        Polygon p;
        p.resize(NUM_INDICES);
        for(unsigned int i=0; i < NUM_INDICES; ++i) {
            double scalar = i / static_cast<double>(NUM_INDICES);
            p[i].set(cos(scalar * 2 * osg::PI), sin(scalar * 2 * osg::PI));
            if (i % 2) p[i] *= inner_scale;
        }
        setPolygon(p);
    }
private:
};

}


/**
 * template class, which combines a slice generator, a color policy and a texcoord policy to a 
 * extruded line geometry
 */
template <
    class SliceGenerator, 
    class ColorPolicy = ExtrudedLine::NoColorPolicy, 
    class TexturePolicy = ExtrudedLine::NoTexturePolicy> 
class ExtrudedLineGeometryT : public ExtrudedLine::BaseGeometry {

public:
    /// ctor
    ExtrudedLineGeometryT(
        const SliceGenerator& g = SliceGenerator(),
        const ColorPolicy& cp = ColorPolicy(),
        const TexturePolicy& tp = TexturePolicy()) 
    :   ExtrudedLine::BaseGeometry(), 
        _generator(g),
        _colorPolicy(cp),
        _texturePolicy(tp),
        _inited(false)
    {
    }
    
    /// update the geometry from the linevertices using the generator and the policies
    virtual void update() {
        if (!this->_inited) {
            this->init();
        }
        computeNormals();
        computeCaps();
        
        unsigned int m(getNumLineVertices());
        _generator.begin(this);
        _colorPolicy.beginColors(this, _generator.getNumVerticesPerSlice());
        _texturePolicy.beginTextureCoords(this, _generator.getNumVerticesPerSlice());
                
        for(unsigned int i = 0; i < m; ++i) 
        {
            _generator.compute(this, i, getAt(i));
            _colorPolicy.computeColors(this, i, getAt(i));
            _texturePolicy.computeTextureCoords(this, i, getAt(i));
        }
        
        _generator.finish(this);
        _colorPolicy.finishColors(this, _generator.getNumVerticesPerSlice());
        _texturePolicy.finishTextureCoords(this, _generator.getNumVerticesPerSlice());
        
    };
    
    /// get the slice generator
    SliceGenerator& getSliceGenerator() { return _generator; }
    
    /// get the color policy
    ColorPolicy& getColorPolicy() { return _colorPolicy; }
    
    /// get the texturecoord policy
    TexturePolicy& getTexturePolicy() { return _texturePolicy; }
    
    
private:
    void init() 
    {
        _inited = true;
        
        _generator.setup(this);
        _colorPolicy.setupColors(this, _generator.getNumVerticesPerSlice());
        _texturePolicy.setupTextureCoords(this, _generator.getNumVerticesPerSlice());
            
        _inited = true;
    }
    
    void computeNormals() 
    {
       
        if (_lineVertices.size() < 2)
            return;
            
        // Normals berechnen
        unsigned int m(_lineVertices.size());
        osg::Vec3 prev_v_axis(this->_defaultUpVector);
        osg::Vec3 prev_h_axis(this->_defaultUpVector ^ (_lineVertices[m-1].point - _lineVertices[0].point) );
        prev_h_axis.normalize();
        
        if (m == 2) {
            _lineVertices[1].hN = prev_h_axis;
            _lineVertices[1].vN = prev_h_axis ^ (_lineVertices[m-1].point - _lineVertices[0].point);
            _lineVertices[1].vN.normalize();
        }
        
        for(unsigned int i = 1; i < m-1; ++i) {
            
            osg::Vec3 t0, t1;
            t0 = _lineVertices[i+1].point - _lineVertices[i].point;
            t1 = _lineVertices[i].point - _lineVertices[i-1].point;
            
            t0.normalize();
            t1.normalize();
            
            osg::Vec3 vAxis = t0 ^ t1;
            vAxis.normalize();
            
            if (vAxis.length2() < 0.1) 
                vAxis = prev_v_axis;
            
            
            // vAxis ggf umdrehen
            if ((vAxis * prev_v_axis) < 0) 
            {
                vAxis *= -1;
            }
            
            osg::Vec3 hAxis = vAxis ^ t0;
            hAxis.normalize();
            if (hAxis.length2() < 0.1)
                hAxis = prev_h_axis;
                
            _lineVertices[i].hN = hAxis;
            _lineVertices[i].vN = vAxis;
            
            prev_v_axis = vAxis;
            prev_h_axis = hAxis;
            
        }
        _lineVertices[0].hN = _lineVertices[1].hN;
        _lineVertices[0].vN = _lineVertices[1].vN;
        
        _lineVertices[m-1].hN = _lineVertices[m-2].hN;
        _lineVertices[m-1].vN = _lineVertices[m-2].vN;
    }
    
    SliceGenerator _generator;
    ColorPolicy _colorPolicy;
    TexturePolicy _texturePolicy;
    bool _inited;    
};

}