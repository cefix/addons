//
//  NativeFont.cpp
//  cefix_native_font
//
//  Created by Stephan Huber on 09.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "NativeFont.h"
#include <cefix/AllocationObserver.h>
#include <cefix/Settings.h>
#ifdef CEFIX_FOR_IPHONE
#include "NativeIOSFontImplementation.h"
#else
#include "NativeOSXFontImplementation.h"
#endif


NativeFont::Implementation* NativeFont::Implementation::getInstance() {

    static osg::ref_ptr<Implementation> impl(NULL);
    #ifdef __APPLE__
        if (!impl) impl = new NativeAppleFontImplementation();
    #endif
    return impl;
}

osg::Image* NativeFont::Implementation::createImage(NativeFont* font) {
    return font->createNewImage();
}


class NativeFontGlobalData : public cefix::FontGlobalData {
public:
    NativeFontGlobalData(const std::string& font_name, unsigned int base_font_size, unsigned int img_width, unsigned int img_height)
        : cefix::FontGlobalData()
    {
        _fontName = font_name;
        _nativeFontSize = base_font_size;
        _blurAmount = 0;
        _textureWidth = img_width;
        _textureHeight = img_height;
        _absoluteTexCoordsFlag = false;
        _textureCount = 0;
        _bleed = std::min<int>(1, base_font_size / 8);
    }
};


NativeFont::NativeFont(const std::string& font_name, unsigned int base_font_size, unsigned int img_width, unsigned int img_height)
    : cefix::Font()
    , _impl(Implementation::getInstance())
{
    setComputeKerning(true);
    
    _fontData = new NativeFontGlobalData(font_name, base_font_size, img_width, img_height);
    createNewImage();
}


osg::Image* NativeFont::createNewImage()
{
    osg::ref_ptr<osg::Image> image = new osg::Image();
    image->allocateImage(_fontData->getTextureWidth(), _fontData->getTextureHeight(), 1,  GL_ALPHA, GL_UNSIGNED_BYTE);
    _images.push_back(image.get());
    
    
    osg::Texture2D* texture = new osg::Texture2D();
    texture->setImage(image);
    
    texture->setFilter(osg::Texture::MIN_FILTER, cefix::Settings::instance()->useMipMappedFonts() ? osg::Texture::LINEAR_MIPMAP_LINEAR : osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    
    
    cefix::AllocationObserver::instance()->observe("FontTexture", texture);
    cefix::AllocationObserver::instance()->observe("FontImage", image);
    
    
    _textures.push_back(texture);
    
    return image;
}
