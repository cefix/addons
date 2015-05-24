//
//  HalfEdgeMeshCreator.h
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 16.05.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once
#include "HalfEdgeMesh.h"

struct GeometryVertexData {
    unsigned int ndx;
    
    GeometryVertexData() : ndx(0), _geo(0), _vertices(0) {}
    
    GeometryVertexData(unsigned int in_ndx, osg::Geometry* geo, osg::Vec3Array* vertices)
    : ndx(in_ndx), _geo(geo), _vertices(vertices) {}
    
    const osg::Vec3& getVertice() const { return (*_vertices)[ndx]; }
private:
    osg::ref_ptr<osg::Geometry> _geo;
    osg::Vec3Array* _vertices;
};

inline std::ostream& operator<<(std::ostream& os, GeometryVertexData& data) {
    os << data.ndx;
    return os;
}

template <>
class HalfEdgeMeshTraits<GeometryVertexData>  {
public:
    
    typedef osg::Vec3 index_type;
    typedef std::less<index_type> compare_type;
    
    static inline const index_type& getIndex(const GeometryVertexData& data)  { return data.getVertice(); }
    static inline const osg::Vec3& getVertice(const GeometryVertexData& data)  { return data.getVertice(); }
        
};

template<class Mesh>
class HalfEdgeMeshCreator : public osg::NodeVisitor {
public:
    struct TriangleIndicesCollector
    {
        TriangleIndicesCollector():
            _mesh(0),
            _geo(0),
            _vertices(0)
        {
        }
        
        void setMeshAndGeometry(Mesh* mesh, osg::Geometry* geo) {
            _mesh = mesh;
            _geo = geo;
            _vertices = dynamic_cast<osg::Vec3Array*>(geo->getVertexArray());
        }

        inline void operator () (unsigned int p0, unsigned int p1, unsigned int p2)
        {
            _mesh->addTriangle(
                GeometryVertexData(p0, _geo, _vertices),
                GeometryVertexData(p1, _geo, _vertices),
                GeometryVertexData(p2, _geo, _vertices));
        }
        
        Mesh* _mesh;
        osg::ref_ptr<osg::Geometry> _geo;
        osg::Vec3Array* _vertices;

    };
    HalfEdgeMeshCreator(Mesh& mesh, TraversalMode tm=TRAVERSE_ACTIVE_CHILDREN) : osg::NodeVisitor(tm), _mesh(mesh) {}
    
    virtual void apply(osg::Geode &node) 
    {
        for(unsigned int i = 0; i < node.getNumDrawables(); ++i) {
            osg::Geometry* geo = node.getDrawable(i)->asGeometry();
            if (geo) {
                osg::TriangleIndexFunctor<TriangleIndicesCollector> collectTriangleIndices;


                collectTriangleIndices.setMeshAndGeometry(&_mesh, geo);
                geo->accept(collectTriangleIndices);
            }
        }
        osg::NodeVisitor::apply(node);
    }
private:
    Mesh& _mesh;
    osg::Vec3Array* _vertices;
};