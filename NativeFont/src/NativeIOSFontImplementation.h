//
//  NativeAppleFontImplementation.h
//  cefix_native_font
//
//  Created by Stephan Huber on 09.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "NativeFont.h"
#include <cefix/RectanglePacking.h>


class NativeAppleFontImplementation : public NativeFont::Implementation {
public:
    class Context;
    
        
    
    
    NativeAppleFontImplementation();
    virtual cefix::FontCharData* createFontCharData(NativeFont* font, unsigned int ch);
private:
    void computeKerning(Context* context, unsigned int cp_1, unsigned int cp_2);
    void renderCodePoint(Context*, unsigned int code_point, osg::Image* cp_image, float& char_width, int& dx, int& dy);
    
    void copyPixels(osg::Image* src, osg::Image* dst, unsigned int dx, unsigned dy);
    
};


