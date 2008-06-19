/* 
   fontConv.cpp SVN revision 700
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

#include "fontconv.h"

void convertFont(wchar_t* dst, wchar_t* src, int srcFont, int dstFont){
	/* declare hash style buffers */
	wchar_t srcValHash[CHAR_RANGE]={0};
	/*wchar_t dstValHash[CHAR_RANGE]={0};*/
	wchar_t srcExtHash[CHAR_RANGE]={0};
	//wchar_t dstExtHash[CHAR_RANGE]={0};
	
	wchar_t tmpBuffer[CHAR_BUFFER]={0}; /* tmp buffer for converting process */

	for(int i=0; i<FVLEN;i++){
		/* build reverse index for "val" in source, like hash */
		if(_f[srcFont].val[i]!=0x0){
			srcValHash[_f[srcFont].val[i]]= i+VIRTUAL_OFFSET;
		}
		///* build reverse index for "val" in dest, like hash */
		//if(_f[dstFont].val[i]!=0x0){
		//	dstValHash[_f[dstFont].val[i]]= i+VIRTUAL_OFFSET;
		//}
	}

	/* build reverse index for "ext" in source, like hash */
	for(int i=0;i<_f[srcFont].ext_len;i++){
		srcExtHash[_f[srcFont].ext[i].key] = (0xff & srcValHash[_f[srcFont].ext[i].key]) + (i << 8);
	}
	
	/* build reverse index for "ext" in dest, like hash */
	//for(int i=0;i<_f[dstFont].ext_len;i++){
	//	dstExtHash[_f[dstFont].ext[i].key] = (0xff & dstValHash[_f[dstFont].ext[i].key]) + (i << 8);
	//}
	
	wchar_t* srcTmp = src;
	wchar_t* dstTmp = dst;
	
	/* Convert from source font to Global Font  
	 * 1. decompose some char, like "za myin zwel" => "sa lone" + "ya pin" 
	 * 2. add convertable source char + VIRTUAL_OFFSET (0x3000 by default) */
	while(*srcTmp){
	
		/* if known char range */
		if(*srcTmp>=_f[srcFont].min && *srcTmp<=_f[srcFont].max){
			
			/* "ext", decomposition */
			if(srcExtHash[*srcTmp]!=0x0
				&& _f[dstFont].val[LOBYTE(srcExtHash[*srcTmp])]==0x0){ /* only done when dest font dont have this char */
				wchar_t*extval=_f[srcFont].ext[HIBYTE(srcExtHash[*srcTmp])].val;
				while(*extval){ 
					*dstTmp++=*extval++;
				}
				srcTmp++;
				continue;
			}
			
			/* "val", char + 0x3000,
			 * benefits of using VIRTUAL OFFSET 0x3000 is
			 *  - can skip unuse words like 0xD, 0xA, etc.., 
			 *  - can realize very effective converting process later */
			if(srcValHash[*srcTmp]!=0x0){ 
				*dstTmp++=srcValHash[*srcTmp++];
				continue;
			}
			
		} // end if
		
		/* if unknown char or unconvertable char, 
		 * just copy as is*/
		*dstTmp++ = *srcTmp++;
		
	} // end while
	
	*dstTmp=0x0; /* string should b terminated by null char :P */
	
	srcTmp = dst;
	dstTmp = tmpBuffer;
	
	/* Convert from Global font to dest font */
	while(*srcTmp){
		if(*srcTmp>=VIRTUAL_OFFSET){
			if(_f[dstFont].val[*srcTmp-VIRTUAL_OFFSET]!=0x0){
				/* re-combination process */
				bool match=false;
				if(srcTmp[1]!=0x0){ /* no need when string len is 1 */
					for(int i=0;i<_f[dstFont].ext_len;i++){
						if( _f[dstFont].ext[i].length>1 /* no need if ext char length is 1 */
						&& cmp(_f[dstFont].ext[i].val, srcTmp)==0){
							*dstTmp++=_f[dstFont].ext[i].key;
							srcTmp+=_f[dstFont].ext[i].length;
							match=true; /* re-combined */
							break;
						}
					}
				}
				/* if !combined */
				if(!match)
					*dstTmp++=_f[dstFont].val[*srcTmp++-VIRTUAL_OFFSET];
				continue;
			}
		}
		*dstTmp++ = *srcTmp++;
	}
	*dstTmp=0x0;
	
	dstTmp = tmpBuffer;
	
	/* consonent forward re-order */
	for(int i=0;i<_f[dstFont].fwd_len;i++){
		Regex re(_f[dstFont].fwd[i].key,true);
		if(re.test(dstTmp)){
			re.sub(dstTmp,_f[dstFont].fwd[i].val,dstTmp);
		}
	}
	
	/* re-ordering vowel */
	for(int i=0;i<_f[dstFont].vowel_len-1;i++){
		/* prepare the regex pattern. 
		 * var vowel=abcd...z; 
		 * regex = (["+vowel.slice(i+1).join("")+"]+)("+vowel[i]+") 
		 * example regex => ([b-z]+)(a) 
		 * why looping? to generate ([c-z]+)(b) .... ([z]+)(y) */
			
		wchar_t restr[40]=L"(["; wchar_t bracket[]=L"]+)("; 
		cpy(restr+2,&_f[dstFont].vowel[i+1]);// b-z
		cpy(restr+_f[dstFont].vowel_len-i+1,bracket);// ]+)(
		restr[_f[dstFont].vowel_len-i+5]=_f[dstFont].vowel[i]; // tmp[i]
		restr[_f[dstFont].vowel_len-i+6]=')';// )
		
		Regex re(restr,true);
		if(re.test(dstTmp)){
			re.sub(dstTmp,L"\2\1",dstTmp); /* just do re-ordering, according to pattern */
		}
	}
	
	/* adjusting something after vowel re-order */
	for(int i=0;i<_f[dstFont].after_len;i++){
		Regex re(_f[dstFont].after[i].key,true);
		if(re.test(dstTmp)){
			re.sub(dstTmp,_f[dstFont].after[i].val,dstTmp);
		}
	}
	
	/* copy to return string  */
	cpy(dst,dstTmp);
	
	return;
}
