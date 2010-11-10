/* 
   fontConv.h SVN revision 700
   Copyright (C) 2007-2008 Prince Ka Naung Project
   http://code.google.com/p/kanaung/

   All rights reserved. This code is included under the terms of the GNU General Public License,
   version 3.0. (The original code is GPL2; WaitZar has re-licensed this code with the expressed
   written permission of the original copyright holder.)

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

namespace waitzar 
{

#define CHAR_RANGE 0x3000 /* ASCII ~ UNICODE */
#define CHAR_BUFFER 0xFFFF /* Input string buffer length */
#define VIRTUAL_OFFSET 0x3000 /* Virtual font offset */

//#ifndef HIBYTE
#define LOBYTE2(a)           ((unsigned char)(((unsigned long)(a)) & 0xff))
#define HIBYTE2(a)           ((unsigned char)((((unsigned long)(a)) >> 8) & 0xff))
//#endif

void convertFont(wchar_t* dst, const wchar_t* src, int srcFont, int dstFont);

} //End waitzar namespace
	
#endif //__FONTCONV_H__
