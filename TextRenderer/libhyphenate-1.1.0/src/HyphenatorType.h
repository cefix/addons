//
//  HyphenatorType.h
//  cefix_native_font
//
//  Created by Stephan Huber on 23.04.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <cefix/StringUtils.h>

namespace Hyphenate {
    
    typedef wchar_t gunichar;
    
    inline gunichar g_unichar_tolower (gunichar c) { return tolower(c); }
    
};