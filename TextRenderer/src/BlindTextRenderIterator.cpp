//
//  BlindTextRenderIterator.cpp
//  cefix_native_font
//
//  Created by Stephan Huber on 16.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "BlindTextRenderIterator.h"

osg::StateSet* BlindTextRenderIteratorHelper::getOrCreateBlindTextStateSet()
{
    static osg::ref_ptr<osg::StateSet> ss(NULL);
    if (!ss) {
        ss = new osg::StateSet();
        ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    }
    return ss.get();
}