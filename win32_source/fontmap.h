/* 
   fontMap.h SVN revision 700
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

#ifndef __FONTMAP_H__
#define __FONTMAP_H__

#define Zawgyi_One 0
#define M_Myanmar1 1
#define UniBurma 2
#define WinInnwa 3
#define Parabaik 4
#define Metrix_1 5
#define Kingmyanmarsar 6
#define PadaukOT 7
#define Kannaka_Unknown 8
#define Gandamar_Letter1 9
#define MyaZedi 10
#define Academy 11
#define Myanmar3 12
#define Wwin_Burmese 13
#define MS_HEAVY 14
#define CECLASSIC 15

#define FLEN 16
#define FVLEN 226  //+ 5 /*Addition*/32,13,10,47,9}

//#include <tchar.h>

enum FM_FONTTYPE{UNKNOWN=-1,ASCII, UNICODE_PARTIAL, UNICODE_5_1=51};

struct FontMap_Special_Char{
	unsigned short length; //length of val
	unsigned short keystate;
	unsigned short key;
	wchar_t val[5]; //max length = fixed length-1
};

struct FontMap_Ext{
	unsigned short length; //length of val
	unsigned short key;
	wchar_t val[5];
};

struct FontMap_Reorder_Pair{
	wchar_t key[25]; //currently fixed
	wchar_t val[5]; //currently fixed
};

struct FontMap{
	/* 0 for ASCII, 1 for partial unicode, 51 for unicode 5.1 compatibles */
	unsigned short unicode; 
	
	wchar_t *fontname; 
	
	unsigned short fontsize; 
	
	/*main char list */
	unsigned short val[FVLEN];
	
	/* min, max char code */
	unsigned short min;
	unsigned short max;
	
	/* special input ,  like Ctrl+Alt+5 */
	unsigned short spchar_len;
	union {
		wchar_t *__spchar__; //a trick to the compiler :P
		FontMap_Special_Char *spchar;
	};
	
	/* extended chars for decomposition, 
	 * like "za myin zwel" to "sa lone + ya pin", 
	 * coz non-unicode fonts need that  */
	unsigned short ext_len;
	union {
		wchar_t *__ext__; //a trick to the compiler :P
		FontMap_Ext *ext;
	};
	
	/* consonent forward re-ordering , used when non-5.1 to 5.1 */
	unsigned short fwd_len;
	union {
		wchar_t* __fwd__;
		FontMap_Reorder_Pair* fwd;
	};
	
	/* consonent reverse re-ordering , used when 5.1 to non-5.1 */
	unsigned short rev_len;
	union {
		wchar_t* __rev__;
		FontMap_Reorder_Pair* rev;
	};
	
	/* vowel re-ordering , should be call after consonent is properly ordered */
	unsigned short vowel_len;
	wchar_t* vowel;
	
	/* after vowel re-ordering , need for some adjustments */
	unsigned short after_len;
	union {
		wchar_t* __after__;
		FontMap_Reorder_Pair* after;
	};
	
};

extern FontMap _f[FLEN];

#endif // __FONTMAP_H__

