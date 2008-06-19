/* 
   lib.h SVN revision 700
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

#ifndef __LIB_H__
#define __LIB_H__

#include <tchar.h>

void cpy(wchar_t* dst, wchar_t* src);
void sub(wchar_t* dst, wchar_t* src, wchar_t* from, wchar_t* to, bool replaceAll=false);
int cmp(wchar_t* cs, wchar_t* ct);
int len(wchar_t* str);

//int mcmp (wchar_t * m1 ,  wchar_t * m2 , int len);
//void mset ( wchar_t * dst , int val , int len);
//void mcpy ( wchar_t * dst , wchar_t * src , int len );
//int mlen ( wchar_t* src );

#endif //end define __LIB_H__
