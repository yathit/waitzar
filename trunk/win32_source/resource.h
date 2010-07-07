//
// Resource header file; see WaitZarRes.rc for implementation.
// This file is now maintained entirely by hand.
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


//TODO: Re-order the following, categorize:
#define ICON_WZ                         101
#define ICON_WZ_ENG                     102
#define ICON_WZ_MM                      103
#define WZ_MENU                         104
#define WZ_MODEL                        105
#define ICON_WZ_LOADING                 106
#define WZ_FONT                         107
#define IDR_COREFONT1                   108
#define WZ_SMALL_FONT                   108
#define WZ_HELP_KEY_FONT                109
#define WZ_HELP_FORE_FONT               110
#define WZ_HELP_BACK_FONT               111
#define WZ_HELP_CORNER                  112
#define WZ_EASYPS                       113
#define WZ_DEFAULT_CFG                  114
#define WZ_PGDOWN_COLOR                 115
#define WZ_PGUP_COLOR                   116
#define WZ_PGDOWN_SEPIA                 117
#define WZ_PGUP_SEPIA                   118
#define WZ_PADAUK_ZG                    119
#define IDM_HELP                        40004
#define IDM_MYANMAR                     40006
#define IDM_EXIT                        40008
#define IDM_ENGLISH                     40010
#define IDD_WZ_SETTINGS                 40011
#define ID_SETTINGS_L1                  40012
#define ID_SETTINGS_CB1                 40013
#define ID_SETTINGS_OK                  40014
#define IDM_LOOKUP                      40015
#define IDD_WZ_HELP                     40016
#define ID_HELP_OK                      40017
#define ID_HELP_L1                      40018
#define ID_HELP_BKGRD                   40019
#define ID_HELP_H1                      40020
#define ID_HELP_IC                      40021
#define ID_HELP_L2                      40022
#define ID_HELP_L3                      40023
#define ID_HELP_L4                      40024
#define ID_HELP_L5A                     40025
#define ID_HELP_L5B                     40026
#define ID_HELP_H5                      40027
#define ID_HELP_L6                      40028
#define ID_HELP_H6                      40029
#define ID_DELETE_ME                    40030
#define IDM_SETTINGS_DLG                40031
#define ID_SETTINGS_CNCL                40032



// Next default values for new objects
//
/*#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        120
#define _APS_NEXT_COMMAND_VALUE         40033
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif*/
