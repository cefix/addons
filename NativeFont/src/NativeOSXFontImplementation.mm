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

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>



NSImage* fromOsgImage(osg::Image* o)
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
}

class NativeAppleFontImplementation::Context : public osg::Referenced {
public:
    Context() : nsFont(NULL), font(NULL), textContainer(NULL), layoutManager((NULL) ) {}
    ~Context() {
        [nsFont release];
        [textContainer release];
        [layoutManager release];
    }
    
    NSFont* nsFont;
    NativeFont* font;
    NSTextContainer* textContainer;
    NSLayoutManager* layoutManager;
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
    NSMutableDictionary* _attrs = [NSMutableDictionary dictionaryWithObject:context->nsFont forKey: NSFontAttributeName];
    
    try {
        unsigned int _bleed = context->font->getFontData()->getBleed();
        unsigned int slug = context->font->getFontData()->getNativeFontSize() * 2 +_bleed;
        
        unichar cp = static_cast<unichar>(code_point);
        // std::cout << code_point << "-" << cp << std::endl;
    
        NSString* str = [[NSString alloc] initWithCharacters: &cp length:1 ];
        [str autorelease];
        NSSize size = [str sizeWithAttributes: _attrs];
        
        unsigned int width =  2*slug + size.width;
        unsigned int height = 2*slug + size.height;
        
        if ((width == 0) || (width == 0)) {
            throw std::exception();
        }
        cp_image->allocateImage(width, height, 1, GL_ALPHA, GL_UNSIGNED_BYTE);
        memset(cp_image->data(), 0, width * height);
        
        NSImage* ns_img = fromOsgImage(cp_image);
        [ns_img lockFocus];
        [str drawAtPoint:NSMakePoint(slug, slug) withAttributes:_attrs];
        
        
        /* baseline einzeichnen:
         [NSBezierPath setDefaultLineWidth:1];
         NSPoint p1 = NSMakePoint(slug, slug + [_nsFont ascender]);
         NSPoint p2 = NSMakePoint(slug + size.width, slug + [_nsFont ascender]);
         [NSBezierPath strokeLineFromPoint: p1 toPoint:p2];
         */
        
        [ns_img unlockFocus];
        
        NSBitmapImageRep * imgBitmap =[ [ NSBitmapImageRep alloc ]initWithData: [ ns_img TIFFRepresentation ] ];
        
        int texformat = GL_RGB;
        switch( [ imgBitmap samplesPerPixel ] )
        {
            case 4:
                texformat = GL_BGRA;
                break;
            case 3:
                texformat = GL_RGB;
                break;
            case 2:
                texformat = GL_LUMINANCE_ALPHA;
                break;
            case 1:
                texformat = GL_ALPHA;
                break;
            default:
                break;
        }
        unsigned int tw = [imgBitmap pixelsWide];
        unsigned int th = [imgBitmap pixelsHigh];
        osg::ref_ptr<osg::Image> temp_image = new osg::Image();
        temp_image->setImage(tw, th, 1, texformat, texformat, GL_UNSIGNED_BYTE, [imgBitmap bitmapData], osg::Image::NO_DELETE);
        
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
        
        //_cb->setRenderingImage(ns_img);
        
        [imgBitmap release];
        [ns_img release];
        
        dx =  -cl_left + slug;
        
        dy = ([context->nsFont ascender] + slug - cl_bottom) + cp_image->t();
        
        NSTextStorage *textStore = [[NSTextStorage alloc] initWithString: [[NSString stringWithCharacters:&cp length:1] stringByAppendingString:@"m"]];
        NSTextContainer *textContainer = [[NSTextContainer alloc] init];
        NSLayoutManager *myLayout = [[NSLayoutManager alloc] init];
        [myLayout addTextContainer:textContainer];
        [myLayout setUsesScreenFonts:NO];
        [textStore addAttribute: NSLigatureAttributeName value: [NSNumber numberWithInt:0] range: NSMakeRange(0, 2)];
        [textStore addLayoutManager:myLayout];
        [textStore setFont:context->nsFont];
        
        NSRange glyphRange = [myLayout glyphRangeForTextContainer:textContainer];
        
        [myLayout setLocation: NSMakePoint(0, 0) forStartOfGlyphRange:glyphRange];
        
        char_width = [myLayout locationForGlyphAtIndex: 1].x - [myLayout locationForGlyphAtIndex: 0].x;
        
        [myLayout release];
        [textContainer release];
        [textStore release];
    }
    catch (...) {
    }
    
    [_attrs release];
}

void NativeAppleFontImplementation::computeKerning(Context* context, unsigned int cp_1, unsigned int cp_2)
{
    NativeFontCharData* char_data_1 = dynamic_cast<NativeFontCharData*>(context->font->getCharData(cp_1));
    
    char_data_1->removeKerningTo(cp_2);
    
    if (!context->textContainer) {
        context->textContainer = [[NSTextContainer alloc] init];
        context->layoutManager = [[NSLayoutManager alloc] init];
        [context->layoutManager addTextContainer: context->textContainer];
        [context->layoutManager setUsesScreenFonts:NO];
    }
    
    
    unichar txt[2] = { cp_1, cp_2 };
    NSTextStorage *textStore = [[NSTextStorage alloc] initWithString: [NSString stringWithCharacters:&txt[0] length:2]];
    [textStore addAttribute: NSLigatureAttributeName value: [NSNumber numberWithInt:0] range: NSMakeRange(0, 2)];
    [textStore addLayoutManager:context->layoutManager];
    [textStore setFont: context->nsFont];
    
    NSRange glyphRange = [context->layoutManager glyphRangeForTextContainer: context->textContainer];
    
    [context->layoutManager setLocation: NSMakePoint(0, 0) forStartOfGlyphRange:glyphRange];
    [context->layoutManager ensureLayoutForGlyphRange: glyphRange];
    float char_width = [context->layoutManager locationForGlyphAtIndex: 1].x - [context->layoutManager locationForGlyphAtIndex: 0].x;
    
    char_data_1->addKerning(cp_2, char_width);
    
    [textStore release];
}

cefix::FontCharData* NativeAppleFontImplementation::createFontCharData(NativeFont* font, unsigned int code_point)
{
    osg::ref_ptr<Context> context = new Context();
    context->font = font;
    context->nsFont = [NSFont fontWithName: cefix::PlatformUtils::toNSString(font->getFontData()->fontName()) size:font->getFontData()->getNativeFontSize()];
    
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
    
    if(font->getComputeKerning())
    {
        for(cefix::Font::CharList::iterator i = font->begin(); i != font->end(); ++i) {
            computeKerning(context, i->first, code_point);
            computeKerning(context, code_point, i->first);
        }
    }
    
    return data;
}
