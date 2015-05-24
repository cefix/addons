//
//  VectorRenderer2D.cpp
//  cefix_svg_parser
//
//  Created by Stephan Huber on 18.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "VectorRenderer2D.h"
#include <osgViewer/Renderer>
#include "VectorRenderer2DCullCallback.h"


void VectorRenderer2D::Operator::addLine(Segment& p, unsigned int ndx1, unsigned int ndx2)
{
    if (_v == 0) _v = dynamic_cast<osg::Vec3Array*>(_geo->getVertexArray());
    if (_v == 0)
        return;
    
    osg::Vec3 v1((*_v)[ndx1]);
    osg::Vec3 v2((*_v)[ndx2]);

    v1 = v1 * (*_currentModelView);
    v2 = v2 * (*_currentModelView);
    
    osg::Vec3::value_type z = (v1[2] + v2[2]) / 2.0;
    
    if (1 || !_useClipping) {
        applyProjection(v1);
        applyProjection(v2);
        p.add(v1, v2, z);
    }
    else {
        if (!_frustum.project(v1, v2)) {
            p.incClipCount();
        } 
        applyProjection(v1);
        applyProjection(v2);
    
        p.add(v1, v2, z);
    }
    
}


void VectorRenderer2D::Operator::handleNewPrimitiveSet(GLenum mode)
{
    _primitiveSetNdx++;
    _useSharedSegment = (!_splitLines) && ((mode == GL_LINE_STRIP) || (mode == GL_LINE_STRIP));
    if(_useSharedSegment) {
        Segment p;
        _sharedSegment = p;
    }
}


void VectorRenderer2D::Operator::handlePrimitiveSetFinished()
{
    if (_useSharedSegment)
        _parent->addSegment(_sharedSegment);
    _useSharedSegment = false;
}


void VectorRenderer2D::Operator::tryToSetColor(Segment& p, unsigned int ndx)
{
    osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(_geo->getColorArray());
    if (!colors)
        return;
    
    switch (_geo->getColorBinding()) {
        case osg::Geometry::BIND_OVERALL:
            p.setColor((*colors)[0]);
            break;
        case osg::Geometry::BIND_PER_VERTEX:
            p.setColor((*colors)[ndx]);
            break;
        case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            p.setColor((*colors)[_primitiveSetNdx]);
            break;
        case osg::Geometry::BIND_PER_PRIMITIVE:
            p.setColor((*colors)[_primitiveNdx]);
            break;
        default:
            break;
    }
}


void VectorRenderer2D::Operator::operator()(unsigned int p1)
{
    _primitiveNdx++;
    Segment p;
    addLine(p, p1, p1);
    
    tryToSetColor(p, p1);
    
    _parent->addSegment(p);
}

void VectorRenderer2D::Operator::operator()(unsigned int p1, unsigned int p2)
{
    _primitiveNdx++;
    Segment pp;
    Segment& p((_useSharedSegment) ? _sharedSegment : pp);
    addLine(p, p1, p2);
    tryToSetColor(p, p1);
    
    if (!_useSharedSegment)
        _parent->addSegment(p);
}
void VectorRenderer2D::Operator::operator()(unsigned int p1,unsigned int p2, unsigned int p3)
{
    _primitiveNdx++;
    Segment p;
    addLine(p, p1, p2);
    addLine(p, p2, p3);
    addLine(p, p3, p1);
    p.setIsClosed();
    tryToSetColor(p, p1);
    _parent->addSegment(p);
}

void VectorRenderer2D::Operator::operator()(unsigned int p1,unsigned int p2,unsigned int p3,unsigned int p4)
{
    _primitiveNdx++;
    Segment p;
    
    addLine(p, p1, p2);
    addLine(p, p2, p3);
    addLine(p, p3, p4);
    addLine(p, p4, p1);
    p.setIsClosed();
    tryToSetColor(p, p1);
    _parent->addSegment(p);
}




VectorRenderer2D::VectorRenderer2D(Backend* backend)
    : osg::Referenced()
    , _backend(backend)
    , _functor(new TemplatedPrimitiveIndexFunctor<Operator>())
{
    _functor->setParent(this);
}

void VectorRenderer2D::render(osgViewer::View* view)
{
    render(view->getCamera());
}

void VectorRenderer2D::render(osg::Camera* camera)
{
    osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(camera->getRenderer());
    if (renderer) {
        _functor->setViewport(renderer->getSceneView(0)->getViewport());
        render(renderer->getSceneView(0)->getRenderStage());
    }
}


void VectorRenderer2D::renderRenderLeaf(osgUtil::RenderLeaf* rl)
 {
    osg::Drawable* dw= rl->_drawable.get();
    
    bool ignore = false;
    if (dw->getUserValue("vector_renderer_2d_ignore", ignore) && ignore)
        return;
     
    if (rl->_modelview.get()) {
        _functor->setModelViewMatrix(rl->_modelview.get());
    }
    if (rl->_projection.get()) {
        _functor->setProjectionMatrix(rl->_projection.get());
    }
    if ((dw) && (dw->asGeometry()))
    {
        osg::Geometry* geo = dw->asGeometry();
        
        _functor->setGeometry(geo);
        
        /*
        osg::Vec3Array* sourceVec = dynamic_cast<osg::Vec3Array*>(geo->getVertexArray());
    
        
        if (!sourceVec) {
            log::error("PostScriptWriter::writeRenderLeaf")  << "could not write drawable, no vertices ..." << std::endl;
            return;
        }
        
        if (geo->getColorBinding() == osg::Geometry::BIND_PER_VERTEX) {
            log::error("PostScriptWriter::writeRenderLeaf") << "Colorbinding BIND_PER_VERTEX not supported! " << std::endl;
        }
        
        // farbe setzen
        if ((geo->getColorBinding() == osg::Geometry::BIND_PER_VERTEX) || (geo->getColorBinding() == osg::Geometry::BIND_OVERALL)) {
            if (geo->getColorArray()) {
                osg::Vec4Array* c = dynamic_cast<osg::Vec4Array*>(geo->getColorArray());
                if ((c) && (c->size() > 0)) _functor->setColor((*c)[0]);
            }
            else
                _functor->setColor(osg::Vec4(0,0,0,1));
        }
    
        _functor->setVertexArray(sourceVec->getNumElements(),static_cast<const osg::Vec3*>(sourceVec->getDataPointer()));
        
        if (geo->getColorArray() && (geo->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE))
            _functor->setColorArray(dynamic_cast<osg::Vec4Array*>(geo->getColorArray()));
        */
        
        // alle Primitivesets durchgehen
        for (unsigned int i = 0; i <  geo->getNumPrimitiveSets(); ++i)
        {
            /*
            if (geo->getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET) {
                osg::Vec4Array* c = dynamic_cast<osg::Vec4Array*>(geo->getColorArray());
                _functor->setColor((*c)[i]);
            }
            */
            osg::PrimitiveSet* ps = geo->getPrimitiveSet(i);
            ps->accept(*_functor);
        }
    }
}


void VectorRenderer2D::renderStateGraph(osgUtil::StateGraph* graph) {

    for(osgUtil::StateGraph::LeafList::iterator dw_itr = graph->_leaves.begin();
        dw_itr != graph->_leaves.end();
        ++dw_itr)
    {
        osgUtil::RenderLeaf* rl = dw_itr->get();
        renderRenderLeaf(rl);
        
    }

}

void VectorRenderer2D::render(osgUtil::RenderBin* bin)
{
    osgUtil::RenderBin::RenderBinList::iterator rbitr;
    osgUtil::RenderBin::RenderBinList binList = bin->getRenderBinList();
    for(rbitr = binList.begin();
        rbitr!=binList.end() && rbitr->first<0;
        ++rbitr)
    {
        render(rbitr->second.get());
    }
    

    // draw fine grained ordering.
    osgUtil::RenderBin::RenderLeafList leafList = bin->getRenderLeafList();
    for(osgUtil::RenderBin::RenderLeafList::iterator rlitr=leafList.begin();
        rlitr!= leafList.end();
        ++rlitr)
    {
        osgUtil::RenderLeaf* rl = *rlitr;
        renderRenderLeaf(rl);
    }

  
    osgUtil::RenderBin::StateGraphList StateGraphList = bin->getStateGraphList();
    for(osgUtil::RenderBin::StateGraphList::iterator oitr=StateGraphList.begin();
        oitr!=StateGraphList.end();
        ++oitr)
    {
        
        renderStateGraph((*oitr));
        
    }
    
    // write post bins.
    for(;
        rbitr!=binList.end();
        ++rbitr)
    {
        render(rbitr->second.get());
    }
}





VectorRenderer2DCullCallback* VectorRenderer2D::attachTo(osgViewer::View *view, bool permanently)
{
   VectorRenderer2DCullCallback* cb = new VectorRenderer2DCullCallback(this);
    view->getCamera()->setCullCallback(cb);
    return cb;
}


void VectorRenderer2D::render(osgUtil::RenderStage* stage, osg::Viewport* vp)
{
    _functor->setViewport(vp);
    _backend->reset();
    _backend->init(vp->width(), vp->height());
    _segments.clear();
    
    render(stage);
    
    for(Segments::iterator i = _segments.begin(); i != _segments.end(); ) {
        if (i->isValid())
            ++i;
        else
            i = _segments.erase(i);
    }
    
    _segments.sort();
    if(_segments.size())
        _backend->render(_segments);
}


