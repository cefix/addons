//
//  HalfEdgeMesh.h
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 15.05.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

// http://www.flipcode.com/archives/The_Half-Edge_Data_Structure.shtml

#pragma once

#include <cefix/Log.h>
#include "HalfEdgeMeshIterators.h"

/** helper template traits-structure. As the HalfEdgeMesh-template-class knows nothing about the data it can store, 
 *  the traits-class will help with this and provides some basic typedefs and helper methods
 *  For your own data, create a specialized HalfEdgeMeshTraits-class
 */
template <class VertexData>
class HalfEdgeMeshTraits  {
public:
    /// the type used for indices
    typedef VertexData index_type;
    /// the type for comparing values in the indices
    typedef std::less<VertexData> compare_type;
    
    /// get the index-value for a specific data
    static inline const index_type& getIndex(const VertexData& data)  { return data; }
    
    // get the vertex-data
    static inline const osg::Vec3& getVertice(const VertexData& data) { return data; }
    static inline void setVertice(VertexData& data, const osg::Vec3& v) { data = v; }
};


/// small templated helper struct to check the validity of an item
template <class T>
bool check_for_invalid_item(const T& t) {
    return !t.valid;
}


/** 
 * templated HalfEdgeMesh-class, which is able to store arbitrary data in a halfedge-structure.
 * see http://www.flipcode.com/archives/The_Half-Edge_Data_Structure.shtml to get a basic idea
 */
template <class VertexData, class Traits = HalfEdgeMeshTraits<VertexData> >
class HalfEdgeMesh {
public:
    typedef HalfEdgeMesh<VertexData, Traits> base_class;
    typedef VertexData value_type;
    
    /** 
     *  the HalfEdgeMesh stores a lot of handles to the actual data. this is the base-class for all handles
     *  an invalid handle points to a really large integer, for simplicity std::string::npos is used.
     */
    struct BaseHandle {
        /// ctor
        BaseHandle() : _ndx(std::string::npos) {}
        /// ctor
        BaseHandle(std::size_t ndx) : _ndx(ndx) {}
        /// copy-ctor
        BaseHandle(const BaseHandle& rhs) : _ndx(rhs._ndx) {}
        /// returns true, if the handle is valid
        bool valid() const { return _ndx != std::string::npos; }
        /// invalidates the handle
        void invalidate() { _ndx = std::string::npos; }
        /// used for conversion
        operator std::size_t() const {
            return _ndx;
        }
        /// get the stored value
        std::size_t value() const { return _ndx; }
        
        /// comparision operator, two handles are equal, if their value is equal
        bool operator==(const BaseHandle& rhs) const
        { return _ndx == rhs._ndx; }
        
        void __increment() { _ndx++; }
        void __decrement() { _ndx--; }
        
        private:
            std::size_t _ndx;
    };
    
    /// a vertex handle "points" to a vertex
    struct VertexHandle : BaseHandle {
        VertexHandle(std::size_t ndx) : BaseHandle(ndx) {}
        VertexHandle() : BaseHandle() {}
    };
    
    /// a FaceHandle "points" to a face
    struct FaceHandle : BaseHandle {
        FaceHandle(std::size_t ndx) : BaseHandle(ndx) {}
        FaceHandle() : BaseHandle() {}
    };
    
    /// a HalfEdgeHandle "points" to a half-edge
    struct HalfEdgeHandle : BaseHandle {
            HalfEdgeHandle(std::size_t ndx) : BaseHandle(ndx) {}
            HalfEdgeHandle() : BaseHandle() {}
}   ;
    
    typedef std::vector<VertexHandle> VertexHandleList;
    typedef std::vector<HalfEdgeHandle> HalfEdgeHandleList;
    
    struct Vertex;
    struct HalfEdge;
    struct Face;
    
    /// a struct combining all necessary data for a vertex inside the halfedge-mesh
    struct Vertex {
        
        /// the arbitrary data
        VertexData data;
        
        /// the handle to a corresponding half-edge
        HalfEdgeHandle edge;
        
        ///ctor
        Vertex(const VertexData& in_data) : data(in_data), edge(), valid(true), numOutgoingHalfEdges(0), numIncomingHalfEdges(0) {}
        
        /// ctor
        Vertex() : data(), edge(), valid(true), numOutgoingHalfEdges(0), numIncomingHalfEdges(0) {}
        
        /// get the Vertice
        const osg::Vec3& getVertice() const { return Traits::getVertice(data); }  
        
        /// returns true if this vertex is not valid, or is not part of an half-edge.
        bool dangling() const { return ((numOutgoingHalfEdges <= 0) && (numIncomingHalfEdges <= 0)) || (!valid); }
        
        bool valid;
        int numOutgoingHalfEdges, numIncomingHalfEdges;
    };
    
    /// a struct combining all necessary data for a half-edge
    struct HalfEdge {
        
        /// handle of the start-point if this half-edge
        VertexHandle vertex;
        
        /// handle to the face,
        FaceHandle face;
        
        /// handle to the nect half-edge
        HalfEdgeHandle next;
        
        /// the "other" half-edge, pointing from the end to the beginning, only valid, if this edge is part of two faces.
        HalfEdgeHandle pair;
        
        bool valid;
        
        /// ctor
        HalfEdge() : vertex(), face(), next(), pair(), valid(true) {}
    };
    
    
    /// a struct combining all data for a face
    struct Face {
    
        /// handle of the first half-edge
        HalfEdgeHandle first; 
        
        bool valid;
        
        /// ctor
        Face() : first(), valid(true) {}
    };
    
    typedef std::map<typename Traits::index_type, std::size_t, typename Traits::compare_type> VertexDataMap;
    typedef std::map< std::pair<VertexHandle, VertexHandle>, HalfEdgeHandle > HalfEdgeMap;
     
    typedef Iterators::BaseIterator<base_class, Vertex, VertexHandle> VertexIterator;
    typedef Iterators::BaseIterator<base_class, HalfEdge, HalfEdgeHandle> HalfEdgeIterator;
    typedef Iterators::BaseIterator<base_class, Face, FaceHandle> FaceIterator;
    
    typedef Iterators::ConstBaseIterator<base_class, Vertex, VertexHandle> ConstVertexIterator;
    typedef Iterators::ConstBaseIterator<base_class, HalfEdge, HalfEdgeHandle> ConstHalfEdgeIterator;
    typedef Iterators::ConstBaseIterator<base_class, Face, FaceHandle> ConstFaceIterator;
    
    typedef Iterators::ConstHalfEdgeFaceCirculator<base_class> ConstHalfEdgeFaceCirculator; 
    typedef Iterators::ConstAdjacentFacesFaceCirculator<base_class> ConstAdjacentFacesFaceCirculator;
            
    
    /// ctor
    HalfEdgeMesh() 
    :   _vertices(), 
        _faces(), 
        _halfEdges(), 
        _index(), 
        _halfEdgeMap(), 
        _invalidVertex(),
        _invalidHalfEdge(),
        _invalidFace(),
        _debug(true) 
    {
    };

    /// copy ctor
    HalfEdgeMesh(const HalfEdgeMesh& rhs) 
    :   _vertices(rhs._vertices), 
        _faces(rhs._faces), 
        _halfEdges(rhs._halfEdges), 
        _index(rhs._index), 
        _halfEdgeMap(rhs._halfEdgeMap), 
        _invalidVertex(),
        _invalidHalfEdge(),
        _invalidFace(),
        _debug(rhs._debug) 
    {
    };

    /// clear this half-edge-mesh
    void clear() { _vertices.clear(); _faces.clear(); _halfEdges.clear(); _index.clear(); _halfEdgeMap.clear(); }
    
    /// adds a triangle 
    FaceHandle addTriangle(const VertexData& p1, const VertexData& p2, const VertexData& p3)
    {
        VertexHandleList list;
        list.push_back(addVertex(p1));
        list.push_back(addVertex(p2));
        list.push_back(addVertex(p3));
        
        return addFace(list);
    }
    
    /// adds a polygon
    template <class Iterator> FaceHandle addPolygon(const Iterator& itr_start, const Iterator& itr_end) 
    {
        const Iterator itr(itr_start);
        VertexHandleList list;
        while(itr != itr_end) {
            list.push_back(addVertex( *itr ));
        }
        return addFace(list);
    }
    
    /// adds a face from another half-edge-mesh
    template<class Mesh>
    FaceHandle addFaceFromMesh(const Mesh& mesh, const FaceHandle& face_handle)
    {
        VertexHandleList list;
        HalfEdgeHandleList he_list;
        mesh.getSurroundingHalfEdgesForFace(face_handle, he_list);
        
        for(typename HalfEdgeHandleList::iterator i = he_list.begin(); i != he_list.end(); ++i) 
        {
            list.push_back(mesh.get(*i).vertex);
        }
        return addFace(list);
    }
    
    /// adds a face from a VertexHandleList
    FaceHandle addFace(const VertexHandleList& list) 
    {
        HalfEdgeHandle prev_he;
        HalfEdgeHandle first_he;
        HalfEdgeHandleList he_handles;
        typename VertexHandleList::const_iterator prev_v(list.begin());
        for(unsigned int i = 0; i < list.size(); ++i) 
        {
            VertexHandle v1(list[i]);
            VertexHandle v2(list[ (i+1 >= list.size()) ? 0 : i+1]);
            
            HalfEdgeHandle he = addHalfEdge(v1, v2);
            if (!he.valid()) {
                if (_debug)
                    cefix::log::error("HalfEdgeMesh") << "could not add face, invalid edge" << std::endl;
                return FaceHandle();
            }
                
            if (!first_he.valid())
                first_he = he;
                
            if (prev_he.valid())
                get(prev_he).next = he;
            
            get(he).vertex = v1;
            prev_he = he;
            get(he).next = first_he;
            
            he_handles.push_back(he);
        }
        
        // so, face erzeugen
        FaceHandle face_handle = addFace(Face());
        get(face_handle).first = first_he;
        for(typename HalfEdgeHandleList::const_iterator itr = he_handles.begin(); itr != he_handles.end(); itr++) {
            get(*itr).face = face_handle;
        }
        return face_handle;
    }
    
    /// add a vertex to the half-edge-mesh
    VertexHandle addVertex(const VertexData& p) 
    {
        VertexHandle h = get(p);
        if (h.valid()) return h;
        
        // nicht gefunden, neu erzeugen
        
        _vertices.push_back(Vertex(p));
        std::size_t ndx = _vertices.size()-1;
        _index.insert(std::make_pair(Traits::getIndex(p), ndx));
        return ndx;
    }
    
    /// get the half-edge from a handle
    const HalfEdge& get(const HalfEdgeHandle& heh) const { return (heh.valid()) ? _halfEdges[heh] : invalidHalfEdge(); }
    
    /// get the face from a handle
    const Face& get(const FaceHandle& heh) const { return (heh.valid()) ?_faces[heh] : invalidFace(); }
    
    /// get the vertex from a handle
    const Vertex& get(const VertexHandle& heh) const { return (heh.valid()) ?_vertices[heh] : invalidVertex(); }
    
    /// get the half-edge from a handle
    HalfEdge& get(const HalfEdgeHandle& heh) { return (heh.valid()) ? _halfEdges[heh] : invalidHalfEdge(); }
    
    /// get the face from a handle
    Face& get(const FaceHandle& heh) { return (heh.valid()) ? _faces[heh] : invalidFace(); }
    
    /// get the vertex from a handle
    Vertex& get(const VertexHandle& heh) { return (heh.valid()) ? _vertices[heh] : invalidVertex(); }
    
    /// returns true, if the mesh contains already v
    bool includes(const VertexData& v) const 
    {
        return get(v).valid();
    }
    
    /// get the VertexHandle for a given VertexData
    VertexHandle get(const VertexData& v) const 
    {
        typename VertexDataMap::const_iterator i = _index.find( Traits::getIndex(v) );
        return (i != _index.end() ) ? VertexHandle(i->second) : VertexHandle();
    }
    
    
    std::size_t getNumVertices() const { return _vertices.size(); }
    std::size_t getNumHalfEdges() const  { return _halfEdges.size(); }
    std::size_t getNumFaces() const { return _faces.size(); }
    
    VertexIterator vertexBegin() { return VertexIterator(this, _vertices, 0); }
    VertexIterator vertexEnd() { return VertexIterator(this, _vertices, _vertices.size()); }
    
    FaceIterator facesBegin() { return FaceIterator(this, _faces, 0); }
    FaceIterator facesEnd() { return FaceIterator(this, _faces, _faces.size()); }
    
    HalfEdgeIterator halfEdgesBegin() { return HalfEdgeIterator(this, _halfEdges, 0); }
    HalfEdgeIterator halfEdgesEnd() { return HalfEdgeIterator(this, _halfEdges, _halfEdges.size()); }
    
    ConstVertexIterator vertexBegin() const { return ConstVertexIterator(this, _vertices, 0); }
    ConstVertexIterator vertexEnd() const { return ConstVertexIterator(this, _vertices, _vertices.size()); }
    
    ConstFaceIterator facesBegin() const { return ConstFaceIterator(this, _faces, 0); }
    ConstFaceIterator facesEnd() const { return ConstFaceIterator(this, _faces, _faces.size()); }
    
    ConstHalfEdgeIterator halfEdgesBegin() const { return ConstHalfEdgeIterator(this, _halfEdges, 0); }
    ConstHalfEdgeIterator halfEdgesEnd() const { return ConstHalfEdgeIterator(this, _halfEdges, _halfEdges.size()); }
    
    ConstHalfEdgeFaceCirculator getHalfEdgeFaceCirculator(FaceHandle handle) const { 
        return ConstHalfEdgeFaceCirculator(this, handle); 
    }
    
    ConstAdjacentFacesFaceCirculator getAdjacentFacesFaceCirculator(FaceHandle handle) const { 
        return ConstAdjacentFacesFaceCirculator(this, handle); 
    }

    /// get the osg::Vec3 for a given VertexHandle
    osg::Vec3& getVertice(VertexHandle vh) { return get(vh).getVertice(); }
    
    /// get the osg::Vec3 for a given VertexHandle
    const osg::Vec3& getVertice(VertexHandle vh) const { return get(vh).getVertice(); }
    
    /// get all half-edges surrounding a given face with handle face_handle
    void getSurroundingHalfEdgesForFace(FaceHandle face_handle, HalfEdgeHandleList& result) const
    {
        ConstHalfEdgeFaceCirculator itr = getHalfEdgeFaceCirculator(face_handle);
        do {
            result.push_back(itr.edgeHandle());
            ++itr;
        } while (itr.lapCount() < 1);
    }
    
    /** removes a face, note: the face is marked as invalid and still part of the mesh. 
     *  If you want to clean the mesh, use compress, to remove all invalid entities
     */
    bool removeFace(FaceHandle face_handle) 
    {
        if (!face_handle.valid())
            return false;
            
        get(face_handle).valid = false;
            
        HalfEdgeHandleList he_handles;
        getSurroundingHalfEdgesForFace(face_handle, he_handles);
        for(typename HalfEdgeHandleList::iterator i = he_handles.begin(); i != he_handles.end(); ++i) {
            removeHalfEdge(*i);
        }
        
        return true;
    }
    
    /** removes a half-edge, note: the half-edge is marked as invalid and still part of the mesh. 
     *  If you want to clean the mesh, use compress, to remove all invalid entities
     */
    bool removeHalfEdge(HalfEdgeHandle he_handle) 
    {
        if(!he_handle.valid())
            return false;
        HalfEdge& he = get(he_handle);
        
        VertexHandle start_vertex = he.vertex;
        VertexHandle end_vertex = get(he.next).vertex;
        
        if (he.pair.valid()) {
            // invalidate other
            get(he.pair).pair = HalfEdgeHandle();
            he.pair = HalfEdgeHandle();
        }
        typename HalfEdgeMap::iterator i = _halfEdgeMap.find(std::make_pair(start_vertex, end_vertex));
        if (i != _halfEdgeMap.end())
            _halfEdgeMap.erase(i);
            
        he.valid = false;
        HalfEdgeHandle prev_he_handle = he_handle;
        
        // find half-edge prior to he_handle
        /*
        do {
            prev_he_handle = get(prev_he_handle).next;
        } while (prev_he_handle.valid() && get(prev_he_handle).next != he_handle);
        
        // invalidate next-pointer of prev half edge
        if(prev_he_handle.valid())
            get(prev_he_handle).next = HalfEdgeHandle();
        */
            
        get(start_vertex).numOutgoingHalfEdges--;
        get(end_vertex).numIncomingHalfEdges--;
        
        if (get(start_vertex).dangling())
            removeVertex(start_vertex);
        
        if (get(end_vertex).dangling())
            removeVertex(end_vertex);
        
        return true;
    }
    
    /** removes a vertex, note: the vertex is marked as invalid and still part of the mesh. 
     *  If you want to clean the mesh, use compress, to remove all invalid entities
     */
    bool removeVertex(VertexHandle vertex_handle) 
    {
        Vertex& v = get(vertex_handle);
        if (!v.dangling()) return false;
        
        v.valid = false;
        typename VertexDataMap::iterator itr = _index.find(Traits::getIndex(v.data));
        if (itr != _index.end())
            _index.erase(itr);
        return true;
    }
    
    
    /// remove all invalid entities from the mesh
    void compress() 
    {
        std::vector< std::size_t > vertex_mapping(_vertices.size());
        std::vector< std::size_t > halfEdges_mapping(_halfEdges.size());
        std::vector< std::size_t > face_mapping(_faces.size());
        std::size_t j(0);
        
        
        // first create mapping from old handles to new handles
        
        for(unsigned int i = 0; i < _vertices.size(); ++i) {
            vertex_mapping[i] = j;
            if (get(VertexHandle(i)).valid)
                j++;
        }
        
        j = 0;
        for(unsigned int i = 0; i < _halfEdges.size(); ++i) {
            halfEdges_mapping[i] = j;
            if (get(HalfEdgeHandle(i)).valid)
                j++;
        }
        
        j = 0;
        for(unsigned int i = 0; i < _faces.size(); ++i) {
            face_mapping[FaceHandle(i)] = j;
            if (get(FaceHandle(i)).valid)
                j++;
        }
        
        // remove invalid items
        
        {
            typename std::vector<Face>::iterator itr = std::remove_if(_faces.begin(), _faces.end(), check_for_invalid_item<Face>);
            _faces.erase(itr, _faces.end());
        }
        {
            typename std::vector<HalfEdge>::iterator itr = std::remove_if(_halfEdges.begin(), _halfEdges.end(),  check_for_invalid_item<HalfEdge>);
            _halfEdges.erase(itr, _halfEdges.end());
        }
        {
            typename std::vector<Vertex>::iterator itr = std::remove_if(_vertices.begin(), _vertices.end(), check_for_invalid_item<Vertex>);
            _vertices.erase(itr, _vertices.end());
        }
        
        // adjust handles
        
        for(VertexIterator i = vertexBegin(); i != vertexEnd() ; ++i) {
            Vertex& v = *i;
            if(v.edge.valid())
                v.edge = halfEdges_mapping[v.edge];
        }
        
        for(FaceIterator i = facesBegin(); i != facesEnd() ; ++i) {
            Face& f = *i;
            if (f.first.valid())
                f.first = halfEdges_mapping[f.first];
        }
        
        for(HalfEdgeIterator i = halfEdgesBegin(); i != halfEdgesEnd(); ++i) {
            HalfEdge& he = *i;
            if (he.next.valid())
                he.next = halfEdges_mapping[he.next];
            if (he.pair.valid())
                he.pair = halfEdges_mapping[he.pair];
            if (he.face.valid())
                he.face = face_mapping[he.face];
            if (he.face.valid())
                he.vertex = vertex_mapping[he.vertex];
        }
    }
    
private:
    Face& invalidFace() {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid face requested" << std::endl;
        return _invalidFace;
    }
    
    const Face& invalidFace() const {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid face requested" << std::endl;
        return _invalidFace;
    }
    HalfEdge& invalidHalfEdge() {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid half edge requested" << std::endl;
        return _invalidHalfEdge;
    }
    
    const HalfEdge& invalidHalfEdge() const {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid half edge requested" << std::endl;
        return _invalidHalfEdge;
    }
    
    
    Vertex& invalidVertex() {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid vertex requested" << std::endl;
        return _invalidVertex;
    }
    
    const Vertex& invalidVertex() const {
        if (_debug)
            cefix::log::error("HalfEdge") << "invalid vertex requested" << std::endl;
        return _invalidVertex;
    }

    FaceHandle addFace(const Face& face) {
        _faces.push_back(face);
        return _faces.size() - 1;
    }
    
    HalfEdgeHandle addHalfEdge(const HalfEdge& he) {
        _halfEdges.push_back(he);
        return _halfEdges.size() - 1;
    }
    
    HalfEdgeHandle addHalfEdge(const VertexHandle& v1, const VertexHandle& v2) {
        
        typename HalfEdgeMap::iterator itr = _halfEdgeMap.find(std::make_pair(v1, v2));
        if (itr != _halfEdgeMap.end())
            return HalfEdgeHandle();
        
        get(v1).numOutgoingHalfEdges++;
        get(v2).numIncomingHalfEdges++;
        
        HalfEdge edge;
        edge.vertex = v1;
        HalfEdgeHandle h = addHalfEdge(edge);
        _halfEdgeMap.insert(std::make_pair(std::make_pair(v1, v2), h));
        
        // std::cout << "adding half edge from " << v1.value() << " to " << v2.value() << ": " << h.value() << std::endl;
        
        get(v1).edge = h;
        
        itr = _halfEdgeMap.find(std::make_pair(v2, v1));
        if (itr != _halfEdgeMap.end()) {
            HalfEdgeHandle other = itr->second;
            get(other).pair = h;
            get(h).pair = other;
        }
        return h;
    }
    
    std::vector<Vertex> _vertices;
    std::vector<Face> _faces;
    std::vector<HalfEdge> _halfEdges;
    
    VertexDataMap _index;
    HalfEdgeMap     _halfEdgeMap;
    
    Vertex      _invalidVertex;
    HalfEdge    _invalidHalfEdge;
    Face        _invalidFace;
    
    bool        _debug;
    
    template <class U, class V>
    friend std::ostream& operator<<(std::ostream& os, const HalfEdgeMesh<U, V>& he_mesh);

};


/// ref-counted Half-Edge-Mesh class, also cloneable
template <class VertexData, class Traits = HalfEdgeMeshTraits<VertexData> >
class HalfEdgeMeshRefCounted : public HalfEdgeMesh<VertexData, Traits>, public osg::Object {
public:
    HalfEdgeMeshRefCounted() : HalfEdgeMesh<VertexData, Traits>(), osg::Object() {}
    HalfEdgeMeshRefCounted(const HalfEdgeMeshRefCounted& rhs,const  osg::CopyOp& copy_op = osg::CopyOp::DEEP_COPY_ALL)
    :   HalfEdgeMesh<VertexData, Traits>(rhs),
        osg::Object(rhs, copy_op)
    {
    }
    META_Object(cefix, HalfEdgeMeshRefCounted);
};


template <class U, class V>
std::ostream& operator<<(std::ostream& os, HalfEdgeMesh<U, V>& he_mesh) {
    os << "vertices: " << std::endl;
    for(typename HalfEdgeMesh<U, V>::VertexIterator i = he_mesh.vertexBegin(); i != he_mesh.vertexEnd(); ++i) {
        os << i.handle() << ": (";
        os << (*i).valid << ") ";
        os << (*i).data;
        os << " he: " << (*i).edge.value();
        os << " num outgoing: " << (*i).numOutgoingHalfEdges << " num incomping: " << (*i).numIncomingHalfEdges << std::endl;
    }
    
    std::cout << std::endl << "faces: " << std::endl;
    for(typename HalfEdgeMesh<U, V>::FaceIterator i = he_mesh.facesBegin(); i != he_mesh.facesEnd(); ++i) {
        os << i.handle() << ": (" << (*i).valid << ") " << (*i).first.value() << std::endl;
    }
    
    os << std::endl << "half-edges: " << std::endl;
    for(typename HalfEdgeMesh<U, V>::HalfEdgeIterator i = he_mesh.halfEdgesBegin(); i != he_mesh.halfEdgesEnd(); ++i) {
        os << i.handle() << ": (" << (*i).valid << ") n: " << (*i).next.value() << " p: " << (*i).pair.value() << " v: " << (*i).vertex.value() << " f: " << (*i).face.value() << std::endl;
    }
    
    return os;
} 


