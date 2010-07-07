//
// Resource header file; see WaitZarRes.rc for implementation.
// This file is now maintained entirely by hand.
// 
// WARNING: It seems that the resource compiler ONLY understands preprocessor directives.
//          Using enums, const ints, or anything else will cause the resource to fail to load.
//
// According to Technical Note 20: ID Naming & Numbering Conventions
//  http://msdn.microsoft.com/en-us/library/t2zechd4%28v=VS.80%29.aspx
// ..which is for MFC, but is somewhat "standard" for Windows, and thus a good idea.
//  1) Variable prefix strings:
//     IDC_  for cursors
//     IDI_  for icons
//     IDB_  for bitmaps
//     IDM_  for menu items
//     //Dialog-specific:
//     IDD_  for dialog resources
//     IDC_  for dialog controls (exception: IDOK|IDCANCEL)
//  2) Variable ranges by prefix string:
//     IDC_,IDI_,IDB_ 1 -> 0x6FFF
//     //Dialog-specific:
//     IDD_  1 -> 0x6FFF
//     IDC_  8 -> 0xDFFF
//     ???   1 -> 7 (IDOK|IDCANCEL, more... should be in Windows.h anyway)
//     IDM_  (seems to share with IDD)
//  3) My interpretation:
//     Start Icons/Cursors/Bitmaps/CustomResources at 101. Use IDR_ for embedded files.
//     Start Dialogs, Controls, and Menu Items at 40001.
//


//Include guard
#pragma once

//Needed by an inner class
#include <string>


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

//Main Context Menu & its Menu Items
#define IDM_CONTEXT_MENU       40001
#define IDM_HELP               40002
#define IDM_SETTINGS           40003
#define IDM_EXIT               40004
#define IDM_ENGLISH            40005
#define IDM_MYANMAR            40006
#define IDM_DELETE_ME          40007
#define IDM_LOOKUP             40008

//"Help/About" Dialog
#define IDD_HELP_ABOUT         41000
#define IDC_HELP_L1            41001
#define IDC_HELP_H1            41002
#define IDC_HELP_L2            41003
//#define IDC_HELP_L3            41004
#define IDC_HELP_L4            41005
#define IDC_HELP_L5A           41006
#define IDC_HELP_L5B           41007
#define IDC_HELP_H5            41008
#define IDC_HELP_L6            41009
#define IDC_HELP_H6            41010
#define IDC_HELP_BKGRD         41011
#define IDC_HELP_ICON          41012

//"Settings" Dialog
#define IDD_SETTINGS           42000
#define IDC_SETTINGS_L1        42001
#define IDC_SETTINGS_CB1       42002

//Our dummy "blank" dialog, to be filled in programmatically
#define IDD_BLANK              43000



//Small class used to add Window components
class WControl {
public:
	unsigned int id;
	std::wstring text;
	std::wstring type;
	bool convertToHyperlink;
	size_t x;
	size_t y;
	size_t w;
	size_t h;
	unsigned int iconID;

	WControl(unsigned int id, std::wstring text, std::wstring type, bool convertToHyperlink=false, size_t x=0, size_t y=0, size_t w=0, size_t h=0) 
		: id(id), text(text), type(type), convertToHyperlink(convertToHyperlink), x(x), y(y), w(w), h(h), iconID(0)
	{}
};

