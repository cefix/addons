//
//  HalfEdgeMeshIterators.h
//  cefixSketch
//
//  Created by Stephan Maximilian Huber on 16.05.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

namespace Iterators {

template <class Mesh, class Attribute, class AttributeHandle>
class BaseIterator {

public:
    
    typedef Attribute                       value_type;
    typedef AttributeHandle                 value_handle;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef std::ptrdiff_t                  difference_type;
    typedef value_type&                     reference;
    typedef value_type*                     pointer;
    typedef Mesh*                           mesh_ptr;
    typedef Mesh&                           mesh_ref;
    
    typedef std::vector<Attribute>          array_type;
    typedef array_type*                     array_ptr;
    typedef array_type&                     array_ref;
    
    
    
    BaseIterator() : _mesh(0) {}
    BaseIterator(mesh_ptr mesh, array_ref arr, value_handle handle, bool skip = false) 
    :   _mesh(mesh), 
        _arr(&arr),
        _handle(handle), 
        _skip(skip) 
    {
    }
    
    BaseIterator(const BaseIterator& rhs) 
    : _mesh(rhs._mesh), _arr(rhs._arr), _handle(rhs._handle), _skip(rhs._skip)
    {}
    
    BaseIterator& operator=(const BaseIterator<Mesh, Attribute, AttributeHandle>& rhs) 
    {
        _mesh       = rhs._mesh;
        _arr        = rhs._arr;
        _handle     = rhs._handle;
        _skip       = rhs._skip;
        return *this;
    }
    
    //friend class ConstBaseIterator<Mesh, Array, Attribute, AttributeHandle>;
    
    reference operator*()  const { return _mesh->get(_handle); }
   
    pointer   operator->() const { return &(_mesh->get(_handle)); }
    
    value_handle handle() const { return _handle; }
    
    operator value_handle() const { return _handle; }
    
    bool operator==(const BaseIterator& rhs) const 
    { return ((_mesh == rhs._mesh) && (_handle == rhs._handle)); }
    
    bool operator!=(const BaseIterator& rhs) const 
    { return !operator==(rhs); }
    
    BaseIterator& operator++() 
    { _handle.__increment(); if (_skip) skipForward(); return *this; }
 
    BaseIterator& operator--() 
    { _handle.__decrement(); if (_skip) skipBackward(); return *this; }
    
    
private:
    void skipForward() {}
    void skipBackward() {}
    
    mesh_ptr        _mesh;
    array_ptr       _arr;
    value_handle    _handle;
    bool            _skip;
};


template <class Mesh, class Attribute, class AttributeHandle>
class ConstBaseIterator {

public:
    
    typedef const Attribute                       value_type;
    typedef const AttributeHandle                 value_handle;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef std::ptrdiff_t                  difference_type;
    typedef value_type&                     reference;
    typedef value_type*                     pointer;
    typedef const Mesh*                           mesh_ptr;
    typedef const Mesh&                           mesh_ref;
    
    typedef const std::vector<Attribute>          array_type;
    typedef array_type*                     array_ptr;
    typedef array_type&                     array_ref;
    
    
    
    ConstBaseIterator() : _mesh(0) {}
    ConstBaseIterator(mesh_ptr mesh, array_ref arr, value_handle handle, bool skip = false) 
    :   _mesh(mesh), 
        _arr(&arr),
        _handle(handle), 
        _skip(skip) 
    {
    }
    
    ConstBaseIterator(const ConstBaseIterator& rhs) 
    : _mesh(rhs._mesh), _arr(rhs._arr), _handle(rhs._handle), _skip(rhs._skip)
    {}
    
    ConstBaseIterator& operator=(const BaseIterator<Mesh, Attribute, AttributeHandle>& rhs) 
    {
        _mesh       = rhs._mesh;
        _arr        = rhs._arr;
        _handle     = rhs._handle;
        _skip       = rhs._skip;
        return *this;
    }
    
    //friend class ConstBaseIterator<Mesh, Array, Attribute, AttributeHandle>;
    
    reference operator*()  const { return _mesh->get(_handle); }
   
    pointer   operator->() const { return &(_mesh->get(_handle)); }
    
    value_handle handle() const { return _handle; }
    
    operator value_handle() const { return _handle; }
    
    bool operator==(const ConstBaseIterator& rhs) const 
    { return ((_mesh == rhs._mesh) && (_handle == rhs._handle)); }
    
    bool operator!=(const ConstBaseIterator& rhs) const 
    { return !operator==(rhs); }
    
    ConstBaseIterator& operator++() 
    { _handle.__increment(); if (_skip) skipForward(); return *this; }
 
    ConstBaseIterator& operator--() 
    { _handle.__decrement(); if (_skip) skipBackward(); return *this; }
    
    
private:
    void skipForward() {}
    void skipBackward() {}
    
    mesh_ptr        _mesh;
    array_ptr       _arr;
    AttributeHandle    _handle;
    bool            _skip;
};



template<class Mesh>
class ConstHalfEdgeFaceCirculator {

public:
    
    typedef const typename Mesh::HalfEdge                         value_type;
    typedef typename Mesh::HalfEdgeHandle                   value_handle;
    typedef std::forward_iterator_tag                       iterator_category;
    typedef std::ptrdiff_t                                  difference_type;
    typedef value_type&                                     reference;
    typedef value_type*                                     pointer;
    typedef const Mesh*                                           mesh_ptr;
    typedef const Mesh&                                           mesh_ref;
    
    ConstHalfEdgeFaceCirculator() : _mesh(0), _rounds(0), _faceHandle(), _edgeHandle() {}
    
    ConstHalfEdgeFaceCirculator(mesh_ptr mesh, typename Mesh::FaceHandle face_handle)
    : _mesh(mesh), _rounds(0), _faceHandle(face_handle) 
    {
        _edgeHandle = _mesh->get(_faceHandle).first;
    }
    
    reference operator*()  const { return _mesh->get(_edgeHandle); }
   
    pointer   operator->() const { return &(_mesh->get(_edgeHandle)); }
    
    int lapCount() const { return _rounds; }
    
    
    bool operator==(const ConstHalfEdgeFaceCirculator& rhs) const 
    { return ((_mesh == rhs._mesh) && (_faceHandle == rhs._faceHandle) && (_edgeHandle == rhs._edgeHandle) && (_rounds == rhs._rounds)); }
    
    bool operator!=(const ConstHalfEdgeFaceCirculator& rhs) const 
    { return !operator==(rhs); }

    typename Mesh::HalfEdgeHandle edgeHandle() const { return _edgeHandle; }
    typename Mesh::FaceHandle faceHandle() const { return _faceHandle; }
    
    ConstHalfEdgeFaceCirculator& operator++() 
    {   
        value_handle next_edge = _mesh->get(_edgeHandle).next;
        if (next_edge == _mesh->get(_faceHandle).first)
            ++_rounds;
        _edgeHandle = next_edge;
        
        return *this; 
    }
    
private:
    mesh_ptr        _mesh;
    int             _rounds;
    typename Mesh::FaceHandle _faceHandle;
    value_handle _edgeHandle;

};


template<class Mesh>
class ConstAdjacentFacesFaceCirculator {

public:
    
    typedef const typename Mesh::Face                       value_type;
    typedef typename Mesh::FaceHandle                       value_handle;
    typedef std::forward_iterator_tag                       iterator_category;
    typedef std::ptrdiff_t                                  difference_type;
    typedef value_type&                                     reference;
    typedef value_type*                                     pointer;
    typedef const Mesh*                                     mesh_ptr;
    typedef const Mesh&                                     mesh_ref;
    
    ConstAdjacentFacesFaceCirculator() : _mesh(0), _rounds(0), _faceHandle(), _adjacentFaceHandle(), _edgeHandle() {}
    
    ConstAdjacentFacesFaceCirculator(mesh_ptr mesh, typename Mesh::FaceHandle face_handle)
    : _mesh(mesh), _rounds(0), _faceHandle(face_handle) 
    {
        _edgeHandle = _mesh->get(_faceHandle).first;
        _adjacentFaceHandle = getAdjacentFaceHande(_edgeHandle);
    }
    
    reference operator*()  const { return _mesh->get(_adjacentFaceHandle); }
   
    pointer   operator->() const { return &(_mesh->get(_adjacentFaceHandle)); }
    
    int lapCount() const { return _rounds; }
    
    
    bool operator==(const ConstAdjacentFacesFaceCirculator& rhs) const 
    { return ((_mesh == rhs._mesh) && (_faceHandle == rhs._faceHandle) && (_edgeHandle == rhs._edgeHandle) && (_rounds == rhs._rounds)); }
    
    bool operator!=(const ConstAdjacentFacesFaceCirculator& rhs) const 
    { return !operator==(rhs); }

    typename Mesh::HalfEdgeHandle edgeHandle() const { return _edgeHandle; }
    typename Mesh::FaceHandle faceHandle() const { return _faceHandle; }
    typename Mesh::FaceHandle handle() const { return _adjacentFaceHandle; }
    
    ConstAdjacentFacesFaceCirculator& operator++() 
    {   
        value_handle next_edge = _mesh->get(_edgeHandle).next;
        if (next_edge == _mesh->get(_faceHandle).first)
            ++_rounds;
        _edgeHandle = next_edge;
        _adjacentFaceHandle = getAdjacentFaceHande(_edgeHandle);
        
        return *this; 
    }
    
private:
    typename Mesh::FaceHandle getAdjacentFaceHandle(typename Mesh::HalfEdgeHandle eh) {
        return _mesh->get(_mesh->get(_edgeHandle).pair).face;
    }
    
    mesh_ptr        _mesh;
    int             _rounds;
    typename Mesh::FaceHandle _faceHandle, _adjacentFaceHandle;
    value_handle _edgeHandle;

};


}
