//
//  PDFAssetAppleImpl.cpp
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 19.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "PDFAssetAppleImpl.h"
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <cefix/MathUtils.h>
#include "PDFTiled2DGraphics.h"

#ifdef CEFIX_FOR_IPHONE
#import <UIKit/UIKit.h>
#else
#include <ApplicationServices/ApplicationServices.h>
#endif




namespace cefix {


PDFAssetImpl::PDFAssetImpl(PDFAsset* parent) 
:   _parent(parent), 
    _mediaRect(0,0,800,600), 
    _numPages(0),
    _pdf(),
    _currentPage() 
{ 
    open(); 
}
    
PDFAssetImpl::~PDFAssetImpl() 
{
    if (_pdf)
        CGPDFDocumentRelease(_pdf);
        
}


void PDFAssetImpl::renderInto(osg::ref_ptr<osg::Image>& image, const osg::Vec4d& rect, float dpi)
{
    if(image == NULL) {
        image = new osg::Image();
        unsigned int w = cefix::Rect::getWidth(rect) * dpi / 72.0;
        unsigned int h = cefix::Rect::getHeight(rect) * dpi / 72.0;
        image->allocateImage(w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    }
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate ( image->data(),
         image->s(),
         image->t(),
         8,
         image->getRowSizeInBytes(),
         colorSpace,
         kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault);
    
    CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
    CGContextFillRect(context, CGContextGetClipBoundingBox(context));
    
    CGContextTranslateCTM(context, 0.0, image->t());
    CGContextScaleCTM(context, 1.0, -1.0);
    CGContextScaleCTM(context, dpi/72.0, dpi/72.0);
    CGContextTranslateCTM(context, -rect[0], -rect[1]);
    CGRect bounds;
    bounds.origin.x = 0;
    bounds.origin.y = 0;
    bounds.size.width = image->s();
    bounds.size.height = image->s();
    
    //CGContextConcatCTM(context, CGPDFPageGetDrawingTransform(_currentPage, kCGPDFCropBox, bounds, 0, true));
    CGContextDrawPDFPage(context, _currentPage);
    
    image->dirty();
    
    CGColorSpaceRelease( colorSpace );
    CGContextRelease(context);
}


void PDFAssetImpl::open()
{
    std::string filename = osgDB::findDataFile(_parent->getFileName());
    if (filename.empty())
        return;
        
    const UInt8 *buffer = (const UInt8 *)filename.c_str();
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, buffer, filename.length(), false);
    _pdf = CGPDFDocumentCreateWithURL((CFURLRef)url);
    _numPages = CGPDFDocumentGetNumberOfPages(_pdf);
    loadPage(1);
}

void PDFAssetImpl::loadPage(unsigned int p)
{
    _currentPage = CGPDFDocumentGetPage(_pdf, p);
    CGRect nativeMediaRect = CGPDFPageGetBoxRect(_currentPage, kCGPDFCropBox);
    _mediaRect.set(0,0, nativeMediaRect.size.width, nativeMediaRect.size.height); 
}

}