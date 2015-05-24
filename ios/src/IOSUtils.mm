//
//  IOSUtils.cpp
//  cefix-iphone-particles-ios
//
//  Created by Stephan Maximilian Huber on 22.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#include "IOSUtils.h"

#include <UIKit/UIKit.h>

namespace cefix {

NSString* IOSUtils::toNSString(const std::string& str)
{
    return [NSString stringWithUTF8String: str.c_str()];
}



std::string IOSUtils::toString(NSString* str)
{
    return str ? std::string([str UTF8String]) : "";
}


UIImage* IOSUtils::createFromOsgImage(osg::Image* img) {
    
    unsigned int bpp = img->getPixelSizeInBits();
    NSInteger length = img->s() * img->t() * bpp / 8;
    GLubyte *buffer = (GLubyte*)malloc(length);
    memcpy(buffer, img->data(), length);
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, length, NULL);
    int bitsPerComponent = 8;
    int bytesPerRow = bpp / 8 * img->s();
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    if (bpp == 32)
        bitmapInfo |= kCGImageAlphaLast;

    // create the CGImage and then the UIImage
    CGImageRef imageRef = CGImageCreate(img->s(), img->t(), bitsPerComponent, bpp, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, kCGRenderingIntentDefault);
    UIImage *image = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGColorSpaceRelease(colorSpaceRef);
    
    return image;
}


void IOSUtils::updateOsgImage(osg::Image* dst, UIImage* src)
{
    int height = src.size.height;
	int width  = src.size.width;
 
	dst->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    if (dst->getPixelSizeInBits() == 32)
        bitmapInfo |= kCGImageAlphaPremultipliedLast;

    CGContextRef contextRef = CGBitmapContextCreate(dst->data(), width, height, 8, dst->getRowSizeInBytes(), colorSpaceRef, bitmapInfo);
	CGImageRef cgImage = src.CGImage;
	
    CGRect rect = CGRectMake(0, 0, width, height);
	CGContextDrawImage(contextRef, rect, cgImage);
    
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpaceRef);

    dst->dirty();
}


std::string IOSUtils::getDocumentsFolder() {
    NSString* doc_folder = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    return toString(doc_folder);
}


void IOSUtils::setStatusbarVisible(bool visible, bool animated)
{
    [[UIApplication sharedApplication] setStatusBarHidden:!visible animated:animated];
}

}