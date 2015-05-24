//
//  VectorRendererPrimitiveIndexFunctor.h
//  cefix_svg_parser
//
//  Created by Stephan Huber on 18.05.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

template <class T>
class TemplatedPrimitiveIndexFunctor : public osg::PrimitiveIndexFunctor, public T
{
public:


    virtual void setVertexArray(unsigned int,const osg::Vec2*)
    {
    }

    virtual void setVertexArray(unsigned int ,const osg::Vec3* )
    {
    }

    virtual void setVertexArray(unsigned int,const osg::Vec4* )
    {
    }

    virtual void setVertexArray(unsigned int,const osg::Vec2d*)
    {
    }

    virtual void setVertexArray(unsigned int ,const osg::Vec3d* )
    {
    }

    virtual void setVertexArray(unsigned int,const osg::Vec4d* )
    {
    }

    virtual void begin(GLenum mode)
    {
        _modeCache = mode;
        _indexCache.clear();
    }

    virtual void vertex(unsigned int vert)
    {
        _indexCache.push_back(vert);
    }

    virtual void end()
    {
        if (!_indexCache.empty())
        {
            drawElements(_modeCache,_indexCache.size(),&_indexCache.front());
        }
    }

    virtual void drawArrays(GLenum mode,GLint first,GLsizei count)
    {
        this->handleNewPrimitiveSet(mode);
        switch(mode)
        {
            case(GL_TRIANGLES):
            {
                unsigned int pos=first;
                for(GLsizei i=2;i<count;i+=3,pos+=3)
                {
                    this->operator()(pos,pos+1,pos+2);
                }
                break;
            }
            case(GL_TRIANGLE_STRIP):
             {
                unsigned int pos=first;
                for(GLsizei i=2;i<count;++i,++pos)
                {
                    if ((i%2)) this->operator()(pos,pos+2,pos+1);
                    else       this->operator()(pos,pos+1,pos+2);
                }
                break;
            }
            case(GL_QUADS):
            {
                unsigned int pos=first;
                for(GLsizei i=3;i<count;i+=4,pos+=4)
                {
                    this->operator()(pos,pos+1,pos+2);
                    this->operator()(pos,pos+2,pos+3);
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
                unsigned int pos=first;
                for(GLsizei i=3;i<count;i+=2,pos+=2)
                {
                    this->operator()(pos,pos+1,pos+2);
                    this->operator()(pos+1,pos+3,pos+2);
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
                unsigned int pos=first+1;
                for(GLsizei i=2;i<count;++i,++pos)
                {
                    this->operator()(first,pos,pos+1);
                }
                break;
            }
            case(GL_POINTS): {
                unsigned int pos=first;
                for(GLsizei i=0;i<count;i++,pos++)
                {
                    this->operator()(pos);
                }
                break;
            }
            case(GL_LINES): {
                unsigned int pos=first;
                for(GLsizei i=1;i<count;i+=2,pos+=2)
                {
                    this->operator()(pos,pos+1);
                }
                break;
            }
            case(GL_LINE_STRIP): {
                unsigned int ilast = first+count-1;
                for(unsigned int i = first;i < ilast; i+=1)
                    this->operator()(i,i+1);
                break;
            }
            case(GL_LINE_LOOP): {
                unsigned int ilast = first+count-1;
                for(unsigned int i = first;i < ilast;i+=1)
                    this->operator()(i,i+1);
                this->operator()(ilast,first);
                break;
            }
            default:
                // can't be converted into to triangles.
                break;
        }
        this->handlePrimitiveSetFinished();
    }
    
    
    template <class U>
    void drawElementsImpl(GLenum mode,GLsizei count,const U* indices)
    {
        if (indices==0 || count==0) return;
        
        this->handleNewPrimitiveSet(mode);
        
        typedef U Index;
        typedef const Index* IndexPointer;

        switch(mode)
        {
            case(GL_TRIANGLES):
            {
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    this->operator()(*iptr,*(iptr+1),*(iptr+2));
                break;
            }
            case(GL_TRIANGLE_STRIP):
            {
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    if ((i%2)) this->operator()(*(iptr),*(iptr+2),*(iptr+1));
                    else       this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                }
                break;
            }
            case(GL_QUADS):
            {
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2), *(iptr+3));
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2), *(iptr+3));
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
                IndexPointer iptr = indices;
                Index first = *iptr;
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    this->operator()(first,*(iptr),*(iptr+1));
                }
                break;
            }
            case(GL_POINTS):
            {
                IndexPointer iptr = indices;
                ++iptr;
                for(GLsizei i=0;i<count;++i,++iptr)
                {
                    this->operator()(*(iptr));
                }
                break;
            }
            case(GL_LINES):
            {
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=2)
                    this->operator()(*iptr,*(iptr+1));
                break;
            }
            case(GL_LINE_STRIP): {
                IndexPointer ilast = &indices[count-1];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=1)
                    this->operator()(*iptr,*(iptr+1));
                break;
            }
            case(GL_LINE_LOOP): {
                IndexPointer ilast = &indices[count-1];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=1)
                    this->operator()(*iptr,*(iptr+1));
                this->operator()(*(ilast),indices[0]);
                break;
            }
            default:
                // can't be converted into to triangles.
                break;
        }
        
        this->handlePrimitiveSetFinished();
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
    {
        drawElementsImpl<GLubyte>(mode, count, indices);
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
    {
        drawElementsImpl<GLushort>(mode, count, indices);
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
    {
        drawElementsImpl<GLuint>(mode, count, indices);
    }

    GLenum               _modeCache;
    std::vector<GLuint>  _indexCache;
};



