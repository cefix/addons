//
//  PDFAssetAppleImpl.h
//  cefix_presentation_ios
//
//  Created by Stephan Maximilian Huber on 19.03.12.
//  Copyright (c) 2012 stephanmaximilianhuber.com. All rights reserved.
//

#pragma once

#include <osg/Vec4d>
#include <osg/Image>
#include <CoreFoundation/CoreFoundation.h>


typedef struct CGPDFDocument *CGPDFDocumentRef;
typedef struct CGPDFPage *CGPDFPageRef;



namespace cefix {



class PDFAsset;
class PDFAssetImpl {
public:
    PDFAssetImpl(PDFAsset* parent) ;    
    const osg::Vec4& getMediaRect() const { return _mediaRect; }
    unsigned getNumPages() const { return _numPages; }
    
    void renderInto(osg::ref_ptr<osg::Image>& img, const osg::Vec4d& rect, float dpi);
    
    ~PDFAssetImpl() ;
    
    void loadPage(unsigned int p);
    
protected:
    void open(); 
private:
    PDFAsset* _parent;
    osg::Vec4 _mediaRect;
    unsigned int _numPages;
    
    CGPDFDocumentRef _pdf;
    CGPDFPageRef _currentPage;
    
};

}