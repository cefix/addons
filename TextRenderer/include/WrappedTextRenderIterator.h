//
//  WrappedTextRenderIterator.h
//  cefix_native_font
//
//  Created by Stephan Huber on 22.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <vector>
#include <map>
#include <osg/Vec2>

template<class RenderIterator>
class WrappedTextRenderIterator : public RenderIterator {
public:
    struct WordBounds {
        float width;
        float isHyphenation;
        WordBounds() : width(0), isHyphenation(false) {}
    };
    WrappedTextRenderIterator()
        : RenderIterator()
        , _maxLineWidth(200)
        , _minLineWidth(150)
        , _wordWidths()
        , _index()
        , _wordNdx(0)
        , _hyphenationHintCode(0x00AD)
        , _suppressHyphenationRendering(true)
    {
    }
    
    WrappedTextRenderIterator(const RenderIterator& rhs)
        : RenderIterator(rhs)
        , _maxLineWidth(200)
        , _minLineWidth(150)
        , _wordWidths()
        , _index()
        , _wordNdx(0)
        , _hyphenationHintCode(0x00AD)
        , _suppressHyphenationRendering(true)
    {
    }

    
    void setMaxLineWidth(float f) { _maxLineWidth = f;  this->setDirty(true);  }
    void setMinLineWidth(float f) { _minLineWidth = f;  this->setDirty(true); }
    void setLineWidth(float min_width, float max_width) { _maxLineWidth = max_width; _minLineWidth = min_width;  this->setDirty(true); }
    
    virtual void reset() {
        RenderIterator::reset();
        
    }
    
    virtual void begin() {
        RenderIterator::begin();
        _wordWidths.clear();
        _index.clear();
        _wordWidths.push_back(WordBounds());
        _wordNdx = 0;
        _index[0] = 0;
    }

    virtual bool needsPreflight() { return true; }
    
    virtual void preflight(const unsigned int &left, const unsigned int &right)
    {
        RenderIterator::preflight(left, right);
        osg::Vec2 old_pos(this->_position), new_pos;
        advance(left, right);
        new_pos = this->_position;
        
        if(isWordBoundary(left))
        {
            if (left == getHyphenationHintCode())
                _wordWidths[_wordNdx].isHyphenation = true;
            
            _wordWidths.push_back(WordBounds());
            _wordNdx++;
            _index[this->_textPosition] = _wordNdx;
            
            if (left == getHyphenationHintCode())
                _wordWidths[_wordNdx].isHyphenation = true;
            
        }
        else
            _wordWidths[_wordNdx].width += (new_pos - old_pos)[0];
        this->_position = old_pos;
            
    }
    
    virtual void advance(const unsigned int &left, const unsigned int &right)
    {
        std::map<unsigned int, unsigned int>::iterator itr = _index.find(this->_textPosition);
        if (itr != _index.end()) _wordNdx = itr->second;
        const WordBounds& wb(_wordWidths[_wordNdx]);
        
        if(isWordBoundary(left) && (this->_position[0] + wb.width > _minLineWidth)) {
            RenderIterator::advance(10,10);
        } else {
            if(left != getHyphenationHintCode())
                RenderIterator::advance(left, right);
        }
    }
    
    virtual void render(const unsigned int &ch)
    {
        std::map<unsigned int, unsigned int>::iterator itr = _index.find(this->_textPosition);
        if (itr != _index.end()) _wordNdx = itr->second;
        const WordBounds& wb(_wordWidths[_wordNdx]);
        
        if (ch == getHyphenationHintCode() && _suppressHyphenationRendering)
        {
            if (wb.isHyphenation && (this->_position[0] + wb.width > _minLineWidth))
                RenderIterator::render('-');
            
        } else
            RenderIterator::render(ch);
    }
    
    virtual void finish() {
        RenderIterator::finish();
    }
    
    bool isWordBoundary(unsigned int ch)
    {
        return (ch == 32) || (ch == '-') || (ch == 9) || (ch == 10) || (ch == _hyphenationHintCode);
    }
    
    void setHyphenationHintCode(unsigned int ch) { _hyphenationHintCode = ch; this->setDirty(true); }
    unsigned int getHyphenationHintCode() const { return _hyphenationHintCode; }
    void setSuppressHyphenationRendering(bool f) { _suppressHyphenationRendering = f;  this->setDirty(true);  }
protected:
    float _maxLineWidth, _minLineWidth;
    std::vector<WordBounds> _wordWidths;
    std::map<unsigned int, unsigned int> _index;
    unsigned int _wordNdx;
    unsigned int _hyphenationHintCode;
    bool _suppressHyphenationRendering;
};
