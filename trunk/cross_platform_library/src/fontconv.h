/* 
   fontConv.h SVN revision 700
   Copyright (C) 2007-2008 Prince Ka Naung Project
   http://code.google.com/p/kanaung/

   All rights reserved. (see the NOTICE file in the top directory for more information)

   NOTE: Wait Zar maintains its own branch of the Prince KaNaung/Burglish conversion
   sources, mainly for reasons of validity. Also, apart from fixing warnings and deleting
   comments, NO maintenance of this code is undertaken by Wait Zar's developers.
   Those of you interesting in linking in font conversion to your own projects should 
   grab the latest source from:
   http://code.google.com/p/burglish/source/browse/trunk/desktop/bgl_core
*/

#ifndef __FONTCONV_H__
#define __FONTCONV_H__

#include "fontmap.h"
#include "regex.h"
#include "lib.h"

#define CHAR_RANGE 0x3000 /* ASCII ~ UNICODE */
#define CHAR_BUFFER 0xFFFF /* Input string buffer length */
#define VIRTUAL_OFFSET 0x3000 /* Virtual font offset */

#ifndef HIBYTE
	#define LOBYTE(a)           ((unsigned char)(((unsigned long)(a)) & 0xff))
	#define HIBYTE(a)           ((unsigned char)((((unsigned long)(a)) >> 8) & 0xff))
#endif

void convertFont(wchar_t* dst, wchar_t* src, int srcFont, int dstFont);

#endif //__FONTCONV_H__
