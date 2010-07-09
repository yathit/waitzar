//
// Extended resource header file. Contains more resource definitions, removed
//   from resource.h to avoid straining the Resource Compiler.
// Also ensures that we don't accidentally use an enum-generated value inside one of
//   the resource definitions, which will cause a run-time error. (By separating
//   files we generate a compile-time error, I think.)
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


#pragma once


//Needed by an inner class
#include <string>

//Include original resource file.
#include "resource.h"


//Dialog resources. We restart numbering
// for each main dialog; this makes debugging
// a bit easier.
enum {
	//Context menu
    IDM_HELP                 = 40001,
    IDM_SETTINGS,
    IDM_EXIT,
    IDM_ENGLISH,
    IDM_MYANMAR,
    IDM_LOOKUP,

	//Where to add "typing" sub-menu items.
	IDM_TYPING_SUBMENU_START = 41001,

	//"Help/About" Dialog
	IDC_HELP_L1              = 42001,
    IDC_HELP_H1,
    IDC_HELP_L2,
    IDC_HELP_L4,
    IDC_HELP_L5A,
    IDC_HELP_L5B,
    IDC_HELP_H5,
    IDC_HELP_L6,
    IDC_HELP_H6,
    IDC_HELP_BKGRD,
    IDC_HELP_ICON,

	//"Settings" Dialog
	IDC_SETTINGS_L1          = 43001,
	IDC_SETTINGS_CB1,
	IDC_SETTINGS_BKGRD,
    IDC_SETTINGS_MAINTAB,
};



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
