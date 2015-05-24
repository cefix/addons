//
//  HalfMeshGeometryCreator.h
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 16.05.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once


class HalfEdgeMeshGeometryCreator {

public:
    template <class Mesh, class GeometryCreator>
    static osg::Geometry* create(const Mesh& mesh, GeometryCreator& creator )
    {
        osg::Geometry* geo = new osg::Geometry();
        creator.create(geo);
        
        update(mesh, creator, geo, false);
        return geo;
    }
    
    template <class Mesh, class GeometryCreator>
    static void update(const Mesh& mesh, GeometryCreator& creator, osg::Geometry* geo, bool include_invalid_faces ) 
    {
        creator.init(geo);
        typename Mesh::HalfEdgeHandleList half_edge_handles;
        for(typename Mesh::ConstFaceIterator f_itr = mesh.facesBegin(); f_itr != mesh.facesEnd(); ++f_itr) 
        {
            half_edge_handles.resize(0);
            
            typename Mesh::ConstHalfEdgeFaceCirculator hec = mesh.getHalfEdgeFaceCirculator(f_itr.handle());
            
            do {
                half_edge_handles.push_back(hec.edgeHandle());
                ++hec;
            } while ((hec.lapCount() < 1) && hec.edgeHandle().valid());
            
            if (include_invalid_faces || (*f_itr).valid)
                creator.addFace(mesh, half_edge_handles, (*f_itr).valid );
        }
        creator.finish(); 
    }

};

template <class Mesh>
class HalfEdgeMeshDebugGeometryCreator {
public:
    
    void create(osg::Geometry* geo) 
    {
        _vertices = new osg::Vec3Array();
        _normals = new osg::Vec3Array();
        _colors = new osg::Vec4Array();
        
        geo->setVertexArray(_vertices);
        //geo->setNormalArray(_normals);
        geo->setColorArray(_colors);
        //geo->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        
        _triangles = new osg::DrawElementsUShort(GL_TRIANGLES);
        _lines = new osg::DrawElementsUShort(GL_LINES);
        
        geo->addPrimitiveSet(_triangles);
        geo->addPrimitiveSet(_lines);
    }
    
    void init(osg::Geometry* geo) 
    {
        _vertices = dynamic_cast<osg::Vec3Array*>(geo->getVertexArray());
        _normals = dynamic_cast<osg::Vec3Array*>(geo->getNormalArray());
        _colors = dynamic_cast<osg::Vec4Array*>(geo->getColorArray());
        
        _vertices->resize(0);
        _colors->resize(0);
        //_normals->resize(0);
        
        _triangles = dynamic_cast<osg::DrawElementsUShort*>(geo->getPrimitiveSet(0));
        _lines = dynamic_cast<osg::DrawElementsUShort*>(geo->getPrimitiveSet(1));
        
        _triangles->resize(0);
        _lines->resize(0);
        
        geo->dirtyDisplayList();
        geo->dirtyBound();
    }
    
    void finish() {
    }
    
    void addFace(const Mesh& mesh, const typename Mesh::HalfEdgeHandleList& he_handles, bool valid) 
    {
        // dreiecke
        if(he_handles.size() == 3) {
            for(typename Mesh::HalfEdgeHandleList::const_iterator i = he_handles.begin(); i != he_handles.end(); ++i) 
            {
                const typename Mesh::HalfEdge& edge = mesh.get(*i);
                osg::Vec3 v = mesh.getVertice(edge.vertex);
                
                _vertices->push_back(v);
                _colors->push_back(valid ? osg::Vec4(0.5,0.5,0.5,0.3): osg::Vec4(0.5,0.0,0.0,0.3));
                _triangles->push_back(_vertices->size() - 1);
            }
        } else if(he_handles.size() == 4) {
            for(typename Mesh::HalfEdgeHandleList::const_iterator i = he_handles.begin(); i != he_handles.end(); ++i) 
            {
                const typename Mesh::HalfEdge& edge = mesh.get(*i);
                osg::Vec3 v = mesh.getVertice(edge.vertex);
                
                _vertices->push_back(v);
                _colors->push_back(osg::Vec4(1,1,1,0.5));
            }
            
            _triangles->push_back(_vertices->size() - 4);
            _triangles->push_back(_vertices->size() - 3);
            _triangles->push_back(_vertices->size() - 2);
            
            _triangles->push_back(_vertices->size() - 4);
            _triangles->push_back(_vertices->size() - 2);
            _triangles->push_back(_vertices->size() - 1);
        } 
        // linien
        for(typename Mesh::HalfEdgeHandleList::const_iterator i = he_handles.begin(); i != he_handles.end(); ++i) 
        {
            const typename Mesh::HalfEdge& edge = mesh.get(*i);
            const typename Mesh::HalfEdge& next_edge = mesh.get(edge.next);
            
            osg::Vec3 v1 = mesh.getVertice(edge.vertex);
            osg::Vec3 v2 = mesh.getVertice(next_edge.vertex);
                        
            _vertices->push_back(v1);
            _vertices->push_back(v2);
            
            _colors->push_back(edge.pair.valid() ? osg::Vec4(0,1,0,0.7) : osg::Vec4(1,0,0,0.7));
            _colors->push_back(edge.pair.valid() ? osg::Vec4(0,1,0,0.7) : osg::Vec4(1,0,0,0.7));
             
            _lines->push_back(_vertices->size() - 2);
            _lines->push_back(_vertices->size() - 1);
        }
        
    }
    
private:
    osg::Vec3Array *_vertices, *_normals;
    osg::Vec4Array *_colors;
    osg::DrawElementsUShort *_triangles, *_lines;

};