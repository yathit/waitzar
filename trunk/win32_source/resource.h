//
// Resource header file; see WaitZarRes.rc for implementation.
// This file is now maintained entirely by hand.
// 
// WARNING: It seems that the resource compiler ONLY understands preprocessor directives.
//          Using enums, const ints, or anything else will cause the resource to fail to load.
//
// Please see resource_ex.h for naming conventions.
//


////////////////////////////////
// General Resources
////////////////////////////////

//Icons
#define IDI_WAITZAR            101
#define IDI_ENGLISH            102
#define IDI_MYANMAR            103
#define IDI_LOADING            104

//Embedded Data Files
#define IDR_WAITZAR_MODEL      201
#define IDR_WAITZAR_EASYPS     202
#define IDR_DEFAULT_CONFIG     203
#define IDR_PADAUK_ZG          204

//Pulp Core Font Files (main)
#define IDR_MAIN_FONT          301
#define IDR_SMALL_FONT         302

//Pulp Core Font Files (help keyboard)
#define IDR_HELP_KEY_FONT      303
#define IDR_HELP_FORE_FONT     304
#define IDR_HELP_BACK_FONT     305
#define IDR_HELP_CORNER_IMG    306

//Pulp Core Font Files (pagination images)
#define IDR_PGDOWN_COLOR       307
#define IDR_PGUP_COLOR         308
#define IDR_PGDOWN_SEPIA       309
#define IDR_PGUP_SEPIA         310



//////////////////////////////////
// Dialogs, Controls, Menu Items
//////////////////////////////////


//Our dummy "blank" dialog, to be filled in programmatically
#define IDD_BLANK              41000


