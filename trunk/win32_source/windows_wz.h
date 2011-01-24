/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#ifndef HEADER_WINDOWS_WZ_H_
#define HEADER_WINDOWS_WZ_H_

/**
 * This file exists as a "safe" way of including windows.h in the Wait Zar project.
 * Some problems with just including windows.h carelessly:
 *    * min/max macros are defined, wreaking havoc with the STL
 *    * All of Windows' giant library components (even those we don't use) are included by default
 *    * Stuff missing from MinGW (e.g., OEM_COMMA) must be added in multiple places.
 *    * Unicode might not be defined (although that's usually a project-wide setting)
 * So just include this file where you'd otherwise include windows.h
 */



/////////////////////////////////////
// PRE
/////////////////////////////////////

//Defines for Unicode-enabled functions.
#define _UNICODE
#define UNICODE

//Slim-down our list of definitions. Would you believe that this causes NO
//  noticeable size reduction on Windows XP, VS2003? Perhaps it helps
//  on Vista....
//Anyway, if you add a new function and get an "undefined" error, comment
//  the relevant #define out.
#define NOGDICAPMASKS       //- CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOKEYSTATES         //- MK_* //Needed for mouse cursors
#define NOSYSCOMMANDS       //- SC_*
#define OEMRESOURCE         //- OEM Resource values
#define NOATOM              //- Atom Manager routines
#define NOCLIPBOARD         //- Clipboard routines
//#define NOCOLOR             //- Screen colors
#define NODRAWTEXT          //- DrawText() and DT_*
#define NOKERNEL            //- All KERNEL defines and routines
#define NOMEMMGR            //- GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE          //- typedef METAFILEPICT
#define NOOPENFILE          //- OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL            //- SB_and scrolling routines
#define NOSERVICE           //- All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND             //- Sound driver routines
//#define NOTEXTMETRIC        //- typedef TEXTMETRIC and associated routines
#define NOWH                //- SetWindowsHook and WH_*
//#define NOWINOFFSETS        //- GWL_*, GCL_*, associated routines
#define NOCOMM              //- COMM driver routines
#define NOKANJI             //- Kanji support stuff.
#define NOHELP              //- Help engine interface.
#define NOPROFILER          //- Profiler interface.
#define NODEFERWINDOWPOS    //- DeferWindowPos routines
#define NOMCX               //- Modem Configuration Extensions

//This flag is usually defined by MinGW anyway, so check first.
#ifndef NOMINMAX
  #define NOMINMAX            //- For compatibility with stl::max + using namespace std
#endif

//Which version of the Windows/IE API to use
#define _WIN32_WINNT        0x0500
#define WINVER              _WIN32_WINNT
#define _WIN32_IE           0x0501



///////////////////////////////////////////
//Now include the library
///////////////////////////////////////////
#include <windows.h>
#include <windowsx.h> //For GET_X_LPARAM
#include <Tlhelp32.h> //For getting a list of currently running processes
//#include <wingdi.h> //For the TEXTINFO stuff
#include <shlobj.h> //GetFolderPath
//#include <urlmon.h> //File downloads
#include <direct.h>   //For get_cwd()


//////////////////////////////////////////
//POST
//////////////////////////////////////////


//Fix some defines that mingw misses
#ifndef SHGFP_TYPE_CURRENT
  #define SHGFP_TYPE_CURRENT 0
#endif
#ifndef CSIDL_MYDOCUMENTS
  #define CSIDL_MYDOCUMENTS CSIDL_PERSONAL
#endif
#ifndef MAPVK_VSC_TO_VK
  #define MAPVK_VSC_TO_VK 1
#endif
#ifndef NIN_BALLOONUSERCLICK
  #define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif
#ifndef ODS_HOTLIGHT
  #define ODS_HOTLIGHT        0x0040
#endif
#ifndef VK_OEM_PLUS
  #define VK_OEM_PLUS        0xBB
#endif
#ifndef VK_OEM_MINUS
  #define VK_OEM_MINUS        0xBD
#endif
#ifndef VK_OEM_COMMA
  #define VK_OEM_COMMA        0xBC
#endif
#ifndef VK_OEM_PERIOD
  #define VK_OEM_PERIOD        0xBE
#endif
#ifndef MAPVK_VSC_TO_VK
  #define MAPVK_VSC_TO_VK        1
#endif





#endif /* HEADER_WINDOWS_WZ_H_ */

/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
