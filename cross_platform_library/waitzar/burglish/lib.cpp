/* 
   lib.cpp SVN revision 700
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

#include "lib.h"

namespace waitzar 
{

//copy string
void cpy(wchar_t* dst, const wchar_t* src){
	while(*dst++=*src++);
}

//substitute string
void sub(wchar_t* dst, wchar_t* src, wchar_t* from, wchar_t* to, bool replaceAll)
{
	wchar_t *cp =dst;//pointer for destination string
	int fromlen=len(from);int tolen=len(to);//store lengths, coz we are moving pointers, so length will effect
	bool once=true;
	do{
		if(cmp(from, src)==0 && fromlen>0 && (once || replaceAll) ){//partial matching from and source string, but parameters are not reversible
			while(*cp++=*to++){src++;};//during copying to string, also move source string pointer one location, coz its replacing :)
			src-=tolen-fromlen;//adjusting depending on from len and to len
			cp--;//move the pointer one character back
			once = false;
		}
	}while(*cp++ = *src++);//copying source to destination string, need do while, instead of while, coz need to compare fomr the first charactar
}

//compare its match or partial match
int cmp(const wchar_t* cs, const wchar_t* ct)
{
  while (*cs == *ct)
  {
	if (*cs == 0) return 0;
	cs++;ct++;
  }
  if(*cs==0) return 0; // enabled  partial match
  return *cs - *ct;  
}

//return length of string
int len(const wchar_t* str){
	int count =0;
	while(*str++){
		count++;
	}
	return count;
}


/* asm - mem related proc */

//int mcmp (wchar_t * m1 ,  wchar_t * m2 , int len){
//    int retval=1;
//    __asm{
//        mov esi , m1
//        mov edi , m2
//        mov ecx , len
//        repe cmps word ptr [esi], word ptr [edi]
//        je  jeq
//        mov eax , 0
//        mov retval,eax
//jeq:
//	}
//    return retval;
//}

//void mset ( wchar_t * dst , int val , int len){
//    __asm{
//        mov eax, val
//        mov edi, dst
//        mov ecx, len
//        rep stos word ptr[edi] // fill with val in memory
//    }
//}

//void mcpy ( wchar_t * dst , wchar_t * src , int len ){
//    __asm{
//		mov esi, src
//		mov edi, dst
//		mov ecx, len
//		rep movs word ptr[edi], word ptr [esi] //copy source to dest
//    }
//}

//int mlen ( wchar_t* src ){
//    int retval;
//    __asm{
//        xor eax , eax
//        mov edi , src
//        mov ecx , 0ffffffffh
//        repne scas word ptr [edi]
//
//        not ecx
//        dec ecx
//        mov retval , ecx
//    }
//    return retval;
//}

} //End waitzar namespace

