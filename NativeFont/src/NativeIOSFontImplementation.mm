//
//  NativeAppleFontImplementation.cpp
//  cefix_native_font
//
//  Created by Stephan Huber on 09.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "NativeOSXFontImplementation.h"
#include <cefix/Pixel.h>
#include <cefix/PlatformUtils.h>

#import <UIKit/UIKit.h>



/*NSImage* fromOsgImage(osg::Image* o)
{
    unsigned char* ptr = o->data();
    unsigned char* planes[1] = { ptr };
    
    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
                             initWithBitmapDataPlanes:planes
                             pixelsWide:o->s()
                             pixelsHigh:o->t()
                             bitsPerSample:8
                             samplesPerPixel: o->getPixelSizeInBits()/8
                             hasAlpha:NO
                             isPlanar:NO
                             colorSpaceName: @"NSDeviceBlackColorSpace"
                             bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
                             bytesPerRow:o->getRowSizeInBytes()
                             bitsPerPixel: o->getPixelSizeInBits()
                             ];
    
    NSImage* img = [[NSImage alloc] initWithSize: NSMakeSize(o->s(), o->t())];
    [img addRepresentation: rep];
    [img setFlipped: YES];
    
    return img;
}*/

UIImage* fromOsgImage(osg::Image* img) {
    
    unsigned int bpp = img->getPixelSizeInBits();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, img->data(), img->s() * img->t(), NULL);
    int bitsPerComponent = 8;
    int bytesPerRow = img->s();
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceGray();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    
    // create the CGImage and then the UIImage
    CGImageRef imageRef = CGImageCreate(img->s(), img->t(), bitsPerComponent, bpp, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, kCGRenderingIntentDefault);
    UIImage *image = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    return image;
}

class NativeAppleFontImplementation::Context : public osg::Referenced {
public:
    Context()
        : nsFont(NULL)
        , font(NULL)
        //, textContainer(NULL)
        //, layoutManager(NULL)
    {
    }
    ~Context() {
        //[nsFont release];
        //[textContainer release];
        //[layoutManager release];
    }
    
    UIFont* nsFont;
    NativeFont* font;
    //NSTextContainer* textContainer;
    //NSLayoutManager* layoutManager;
};

NativeAppleFontImplementation::NativeAppleFontImplementation()
:   NativeFont::Implementation()
{
}

void NativeAppleFontImplementation::copyPixels(osg::Image* src, osg::Image* dst, unsigned int dx, unsigned dy)
{
    cefix::Pixel s(src);
    cefix::Pixel d(dst);
    
    for(unsigned int y = 0; y < s.t(); ++y) {
        for(unsigned int x = 0; x < s.s(); ++x) {
            d.setRGBA(x+dx, (d.t()-1) - (y+dy), s.getRGBA(x,y));
        }
    }
    dst->dirty();
}



void NativeAppleFontImplementation::renderCodePoint(Context* context, unsigned int code_point, osg::Image* cp_image, float& char_width, int& dx,  int& dy)
{
    try {
        unsigned int _bleed = context->font->getFontData()->getBleed();
        unsigned int slug = context->font->getFontData()->getNativeFontSize() * 2 +_bleed;
        
        unichar cp = static_cast<unichar>(code_point);
        // std::cout << code_point << "-" << cp << std::endl;
    
        NSString* str = [[NSString alloc] initWithCharacters: &cp length:1 ];
        [str autorelease];
        CGSize size = [str sizeWithFont: context->nsFont];
        
        unsigned int width =  2*slug + size.width;
        unsigned int height = 2*slug + size.height;
        
        if ((width == 0) || (width == 0)) {
            throw std::exception();
        }
        
        unsigned char *rawData = (unsigned char*) calloc(height * width, sizeof(unsigned char));
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
        CGContextRef img_context = CGBitmapContextCreate(rawData, width, height,
                                                     8, width, colorSpace,
                                                     kCGImageAlphaNone);
        
        UIGraphicsPushContext(img_context);
        CGContextSaveGState(img_context);

        CGContextSetRGBFillColor(img_context, 1,1,1, 1);
        CGContextFillRect(img_context, (CGRect){ CGPointZero, CGSizeMake(width, height) });
        CGContextSetRGBFillColor(img_context,0,0,0, 1);

        [str drawAtPoint:CGPointMake(slug, slug) withFont:context->nsFont];
        CGContextRestoreGState(img_context);
        UIGraphicsPopContext();
        
        
        unsigned int tw = width;
        unsigned int th = height;
        osg::ref_ptr<osg::Image> temp_image = new osg::Image();
        temp_image->setImage(tw, th, 1, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, rawData, osg::Image::NO_DELETE);
        temp_image->flipVertical();
        
        cefix::Pixel s(temp_image);
        
        // neue breite + höhe berechnen
        int cl_left(s.s()), cl_top(s.t()), cl_right(0), cl_bottom(0), cl_bottom_saved;
        bool empty= true;
        for(int y = 0; y < s.t(); ++y)
        {
            for(int x = 0; x < s.s(); ++x)
            {
                unsigned char color = s.r(x,y);
                if (color < 255)
                {
                    cl_left = std::min(x,cl_left);
                    cl_top = std::min(y, cl_top);
                    cl_right = std::max(x, cl_right);
                    cl_bottom = std::max(y, cl_bottom);
                    empty = false;
                }
            }
        }
        if (empty)
        {
            cl_left = cl_top = 0;
            cl_right = cl_bottom = cl_bottom_saved = (2 * _bleed) - 1;
        }
        else
        {
            cl_bottom_saved = cl_bottom;
            cl_left = std::max<int>(0, cl_left - _bleed);
            cl_top = std::max<int>(0, cl_top - _bleed);
            cl_right = std::min<int>(width-1, cl_right + _bleed);
            cl_bottom = std::min<int>(height-1, cl_bottom + _bleed);
        }
        
        /* clipping deaktivieren
         cl_top = 0;
         cl_bottom = height-1;
         */
        
        cp_image->allocateImage(cl_right - cl_left+1, cl_bottom - cl_top+1, 1, GL_ALPHA, GL_UNSIGNED_BYTE);
        
        
        cefix::Pixel d(cp_image);
        for(unsigned y = cl_top; y <= cl_bottom; ++y)
        {
            for(unsigned x = cl_left; x <= cl_right; ++x) {
                d.setR(x - cl_left, y - cl_top, 255-s.r(x,y));
            }
        }
        
        
        temp_image = NULL;
        CGContextRelease(img_context);
        free(rawData);
        
        
        dx =  -cl_left + slug;
        
        dy = ([context->nsFont ascender] + slug - cl_bottom) + cp_image->t();
        
        {
            unichar chrs1[2] = { cp, 'm' };
            NSString* str1 = [[NSString alloc] initWithCharacters: &chrs1[0] length:2 ];
            [str1 autorelease];
            CGSize size1 = [str1 sizeWithFont: context->nsFont];
            
            unichar chrs2[1] = { 'm' };
            NSString* str2 = [[NSString alloc] initWithCharacters: &chrs2[0] length:1 ];
            [str2 autorelease];
            CGSize size2 = [str2 sizeWithFont: context->nsFont];

            char_width = size1.width - size2.width;

        }
    }
    catch (...) {
    }
    
}

void NativeAppleFontImplementation::computeKerning(Context* context, unsigned int cp_1, unsigned int cp_2)
{
    NativeFontCharData* char_data_1 = dynamic_cast<NativeFontCharData*>(context->font->getCharData(cp_1));
    
    char_data_1->removeKerningTo(cp_2);
        
    unichar chrs1[2] = { cp_1, cp_2 };
    NSString* str1 = [[NSString alloc] initWithCharacters: &chrs1[0] length:2 ];
    [str1 autorelease];
    CGSize size1 = [str1 sizeWithFont: context->nsFont];
    
    unichar chrs2[1] = { cp_2 };
    NSString* str2 = [[NSString alloc] initWithCharacters: &chrs2[0] length:1 ];
    [str2 autorelease];
    CGSize size2 = [str2 sizeWithFont: context->nsFont];

    
    
    char_data_1->addKerning(cp_2, size1.width - size2.width);

}

cefix::FontCharData* NativeAppleFontImplementation::createFontCharData(NativeFont* font, unsigned int code_point)
{
    osg::ref_ptr<Context> context = new Context();
    context->font = font;
    context->nsFont = [UIFont fontWithName: cefix::PlatformUtils::toNSString(font->getFontData()->fontName()) size:font->getFontData()->getNativeFontSize()];
    
    if (!context->nsFont) {
        cefix::log::error("NativeFont") <<"Could not generate font-texture, as the referenced font is not active/installed on this computer!" << std::endl;
        return NULL;
    }
    
    unsigned int tex_width = font->getFontData()->getTextureWidth();
    unsigned int tex_height = font->getFontData()->getTextureHeight();
    
    osg::ref_ptr<osg::Image> cp_image(new osg::Image());
    float char_width;
    int delta_x, delta_y;
    
    // codepoint rendern
    renderCodePoint(context, code_point, cp_image, char_width, delta_x, delta_y);
    
    if((cp_image->s() == 0) || (cp_image->t() == 0)) {
        cefix::log::error("NativeFont") << "no rendered glyph for code point (" << code_point <<")" << std::endl;
        return NULL;
    }
    
    if ((cp_image->s() > tex_width) || (cp_image->t() > tex_height))
    {
        cefix::log::error("NativeFont") << "Texture is too small to hold a glyph (" << cp_image->s() << " x " << cp_image->t() <<")" << std::endl;
        return NULL;
    }
        
    NativeFontCharData* data = new NativeFontCharData();
    
    bool place_found = false;
    while (!place_found)
    {
        for(unsigned int tex_num = 0; tex_num < font->getImages().size(); ++tex_num)
        {
            while(tex_num >= font->getNumTexDatas()) {
                font->addTexData(new NativeFont::TexData(tex_width, tex_height));
            }
            
            NativeFont::TexData::Packing::node_type* node = font->getTexData(tex_num)->getPacking().insert(data,cp_image->s(), cp_image->t());
            if (node) {
                place_found = true;
                data->setTexRect(node->getLeft(), node->getTop(), node->getRight(), node->getBottom());
                data->setBase(node->getLeft() + delta_x, node->getTop() + delta_y);
                data->setTexId(tex_num);
                data->setCharWidth(char_width);
                
                copyPixels(cp_image, font->getImages()[tex_num].get(), node->getLeft(), node->getTop());
                
                break;
            }
        }
        if (!place_found)
        {
            // std::cout << "new texture for " << std::hex << code_point << std::endl;
            createImage(font);
        }
    }
    font->addChar(code_point, data);
    
    if(1 || font->getComputeKerning())
    {
        for(cefix::Font::CharList::iterator i = font->begin(); i != font->end(); ++i) {
            computeKerning(context, i->first, code_point);
            computeKerning(context, code_point, i->first);
        }
    }
    
    return data;
}
