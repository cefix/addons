//
//  BlindTextRenderIterator.h
//  cefix_native_font
//
//  Created by Stephan Huber on 16.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include <cefix/Font.h>


class BlindTextRenderIteratorHelper {
public:
    static osg::StateSet* getOrCreateBlindTextStateSet();
private:
    BlindTextRenderIteratorHelper() {}
    
};

template <class BaseClass>
class BlindTextRenderIterator : public BaseClass {
public:
    
    enum RenderMode { RenderWords, RenderLines } ;
    enum State { Unknown, Start, Middle, End } ;
    
    BlindTextRenderIterator()
        : BaseClass()
        , _renderMode(RenderWords)
        , _state(Unknown)
        , _dummyVertices(new osg::Vec3Array())
    {
    }

    BlindTextRenderIterator(const BaseClass& rhs)
        : BaseClass(rhs)
        , _renderMode(RenderWords)
        , _state(Unknown)
        , _dummyVertices(new osg::Vec3Array())
    {
    }

    
    virtual void begin() {
        BaseClass::begin();
        _state = Unknown;
    }
    
    inline virtual void advance(const unsigned int &left, const unsigned int &right)
    {
        BaseClass::advance(left, right);
    }
    
    inline virtual void render(const unsigned int &ch)
    {
        float delta = 0;
        switch (this->_textAlignment) {
            case BaseClass::ALIGN_LEFT:
                break;
                
            case BaseClass::ALIGN_CENTER:
                delta = this->_lineWidths[this->_currentLine] / 2.0;
                if (this->getSubPixelAlignment() == false)
                    delta = (int)delta;
                break;
            case BaseClass::ALIGN_RIGHT:
                delta = this->_lineWidths[this->_currentLine];
                break;
        }
        if (ch > 32) {
            
            if ((_state == Unknown) || (_state == End))
            {
                cefix::FontCharData* charData = this->_font->getCharData('x');
                this->_container->select(charData->getTexID());
                _dummyVertices->clear();
                charData->addVertices(this->_dummyVertices, this->_position[0] - delta, this->_position[1], this->_renderPlane, this->_startPos);
                _tl = (*_dummyVertices)[1];
                _bl = (*_dummyVertices)[2];
                _state = Start;
            }
        } else {
            if (((ch == 9) || (ch == 32)) && (_renderMode == RenderWords)) {
                finishLine();
            }
        }
        
    }
    
    void finishLine()
    {
        if (_state != Start)
            return;
        
        float delta = 0;
        switch (this->_textAlignment) {
            case BaseClass::ALIGN_LEFT:
                break;
                
            case BaseClass::ALIGN_CENTER:
                delta = this->_lineWidths[this->_currentLine] / 2.0;
                if (this->getSubPixelAlignment() == false)
                    delta = (int)delta;
                break;
            case BaseClass::ALIGN_RIGHT:
                delta = this->_lineWidths[this->_currentLine];
                break;
        }
        
        cefix::FontCharData* charData = this->_font->getCharData('x');
        this->_container->select(charData->getTexID());
        _dummyVertices->clear();
        charData->addVertices(this->_dummyVertices, this->_position[0] - delta, this->_position[1], this->_renderPlane, this->_startPos);
        _tr = (*_dummyVertices)[1];
        _br = (*_dummyVertices)[2];
        _state = End;
        osg::Vec3Array* v = this->_container->getVertices();
        v->push_back(_tl);
        v->push_back(_tr);
        v->push_back(_br);
        v->push_back(_bl);
        charData->addTexCoords(this->_container->getTexCoords());

    }
    
    virtual void finish() {
        
        if((_state != End) && (_state != Unknown))
            render(10);
        
        BaseClass::finish();
        this->clearStateSets();
        this->setStateSet(BlindTextRenderIteratorHelper::getOrCreateBlindTextStateSet());
    }
    
    virtual void startNewLine() {
        finishLine();
    }
    
    void setRenderMode(RenderMode render_mode) { _renderMode = render_mode; this->setDirty(true); }
    RenderMode getRenderMode() const { return _renderMode; }
protected:
    osg::StateSet* getOrCreateBlindTextStateSet();
private:
    RenderMode _renderMode;
    State _state;
    osg::Vec3 _tl, _tr, _bl, _br;
    osg::ref_ptr<osg::Vec3Array> _dummyVertices;
    
    
};
