//
//  NativeFont.h
//  cefix_native_font
//
//  Created by Stephan Huber on 09.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <cefix/Font.h>
#include <cefix/RectanglePacking.h>

class NativeFontCharData : public cefix::FontCharData {
public:
    NativeFontCharData() : cefix::FontCharData() {}
    
    void setFontData(cefix::FontGlobalData* data) { _fontData = data; }
    void setTexRect(int l, int t, int r, int b) { _texLeft = l; _texTop = t; _texWidth = r-l; _texHeight = b-t; }
    void setTexId(int tex_id) { _texId = tex_id; }
    void setBase(unsigned int l, unsigned int t) { _baseLeft = l; _baseTop = t; }
    void setCharWidth(float char_width) { _charWidth = char_width; }
    void clearKernings() { _kernings.clear(); }
    void removeKerningTo(unsigned int cp) {
        KerningTable::iterator i = _kernings.find(cp);
        if (i != _kernings.end())
            _kernings.erase(i);
    }
    void addKerning(unsigned int code_point, float w) {
        if (w != charWidth())
            _kernings[code_point] = w - charWidth();
    }
};



class NativeFont : public cefix::Font {
    
public:
    
    class Implementation : public osg::Referenced {
    public:
        static Implementation* getInstance();
        virtual cefix::FontCharData* createFontCharData(NativeFont* font, unsigned int ch) = 0;
    protected:
        osg::Image* createImage(NativeFont* font);
    };
    
    class TexData: public osg::Referenced {
    public:
        typedef cefix::RectanglePacking<NativeFontCharData> Packing;
        
        TexData(unsigned int w, unsigned int h) : osg::Referenced(), _packing(w, h) {}
        
        Packing& getPacking() { return _packing; }
        const Packing& getPacking() const { return _packing; }
    private:
        Packing _packing;
    };

    
    
    NativeFont(const std::string& font_name, unsigned int base_font_size, unsigned int img_width = 512, unsigned int img_height = 512);
    
    void setImplementation(Implementation* impl) { _impl = impl; }
    
    void addChar(unsigned int code_point, NativeFontCharData* data) {
        _chars[code_point] = data;
        data->setFontData(_fontData);
    }
    
    void setComputeKerning(bool f) { _computeKerning = f; }
    bool getComputeKerning() const { return _computeKerning; }

    void addTexData(TexData* data) { _texDatas.push_back(data); }
    TexData* getTexData(unsigned int i) { return _texDatas[i]; }
    unsigned int getNumTexDatas() const { return _texDatas.size(); }
    
protected:
    virtual cefix::FontCharData* createFontCharData(unsigned int ch) {
        return _impl->createFontCharData(this, ch);
    }
    
    
    
    osg::Image* createNewImage();
    osg::ref_ptr<Implementation> _impl;
    bool _computeKerning;
    
    std::vector<osg::ref_ptr<TexData> > _texDatas;
    
    friend class Implementation;
    
};
