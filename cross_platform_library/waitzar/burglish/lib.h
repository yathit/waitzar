/* 
   lib.h SVN revision 700
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

#ifndef __LIB_H__
#define __LIB_H__

#include <wchar.h>

namespace waitzar 
{

void cpy(wchar_t* dst, const wchar_t* src);
void sub(wchar_t* dst, wchar_t* src, wchar_t* from, wchar_t* to, bool replaceAll=false);
int cmp(const wchar_t* cs, const wchar_t* ct);
int len(const wchar_t* str);

//int mcmp (wchar_t * m1 ,  wchar_t * m2 , int len);
//void mset ( wchar_t * dst , int val , int len);
//void mcpy ( wchar_t * dst , wchar_t * src , int len );
//int mlen ( wchar_t* src );

} //End waitzar namespace
	
#endif //end define __LIB_H__
