/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

//Defines for Unicode-enabled text.
//  As far as I know, these must appear _before_ including the windows.h include
//  so that we get the proper Unicode-enabled source.
#define _UNICODE
#define UNICODE



//NOTE: This won't compile unless it's in the main file.... not sure why. :( We'll link it here for now...
#define CRYPTOPP_DEFAULT_NO_DLL
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "CryptoPP/md5.h"
#include "CryptoPP/hex.h"
#include "CryptoPP/files.h"




//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
//#define _CRT_NON_CONFORMING_SWPRINTFS

//Define to require a specific version of Windows.
#define _WIN32_WINNT 0x0500 //Run on Windows 2000, XP, and Vista (haven't tested NT or the "server"s yet)
#define _WIN32_IE 0x0501    //I don't understand why I need this, but the speech balloons won't compile unless I have it.
//Note that version 0x0501 is needed for "clicking on notification balloons". There's reports of this not working on 
//   win2k, but so far we don't have any WZ users on that platform. 

//#define _WIN32_WINNT 0x0410 //Run on Windows 98+, fails for KEYBOARD_INPUT

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


//System includes
#define NOMINMAX
#include <windows.h>
#include <windowsx.h> //For GET_X_LPARAM
//#include <winuser.h> //For colors
#include <psapi.h> //For getting a list of currently running processes
//#include <wingdi.h> //For the TEXTINFO stuff
#include <shlobj.h> //GetFolderPath
#include <urlmon.h> //File downloads
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <list>
#include <limits>
#include <set>
#include <sstream>
#include <fstream>
#include <iostream>

//Our includes
#include "NGram/WordBuilder.h"
#include "NGram/SentenceList.h"
#include "NGram/wz_utilities.h"
#include "Pulp Core/PulpCoreFont.h"
#include "Hyperlinks/Hyperlinks.h"
#include "Settings/ConfigManager.h"
#include "OnscreenKeyboard.h"
#include "MyWin32Window.h"
#include "Hotkeys.h"
#include "MiscUtils.h"
#include "Input/InputMethod.h"
#include "Display/TtfDisplay.h"
#include "Input/RomanInputMethod.h"
#include "Transform/Self2Self.h"
#include "Logger.h"

//VS Includes
#include "resource_ex.h"


using namespace waitzar;
using std::string;
using std::wstring;
using std::vector;
using std::map;
using std::list;
using std::pair;
using std::wstringstream;
using std::ifstream;
using std::ofstream;


//Versioning information & lookup
const wstring WZ_VERSION_MAIN = L"1.7";
const bool WZ_VERSION_IS_NIGHTLY = true;
const wstring WZ_VERSION_FULL = WZ_VERSION_IS_NIGHTLY ? L"NIGHTLY-"+WZ_VERSION_MAIN+L"+" : WZ_VERSION_MAIN;
bool newVersionAvailable = false;
const unsigned int FLASH_SAVE_VERSION_NUMBER = 1;

//Menu item texts
const wstring POPUP_LOOKUP_MM = L"&Look Up Word (F1)";
const wstring POPUP_LOOKUP_EN = L"&Look Up Word";

//Another string...
const wstring ROMAN_INPUT_PROMPT = L"(Press \"Space\" to type this word)";

//Better support for dragging
bool isDragging;
POINT dragFrom;

//Unique IDs
const unsigned int LANG_HOTKEY = 142;
const unsigned int STATUS_NID = 144;

//Custom message IDs
const unsigned int UWM_SYSTRAY = WM_USER+1;
const unsigned int UWM_HOTKEY_UP = WM_USER+2;

//Window IDs for the "Language" sub-menu
const wstring WND_TITLE_LANGUAGE = L"\u1018\u102C\u101E\u102C\u1005\u1000\u102C\u1038";
const wstring WND_TITLE_INPUT = L"Input Method";
const wstring WND_TITLE_OUTPUT = L"Encoding";


//Brushes & Pens
HBRUSH g_WhiteBkgrd;
HBRUSH g_DarkGrayBkgrd;
HBRUSH g_BlackBkgrd;
HBRUSH g_YellowBkgrd;
HBRUSH g_GreenBkgrd;
HBRUSH g_RedBkgrd;
HBRUSH g_DlgHelpBkgrd;
HBRUSH g_DlgHelpSlash;
HBRUSH g_MenuItemBkgrd;
HBRUSH g_MenuItemHilite;
HBRUSH g_MenuDefaultBkgrd;
HBRUSH g_DotHiliteBkgrd;
HPEN g_GreenPen;
HPEN g_BlackPen;
HPEN g_MediumGrayPen;
HPEN g_WhiteThickPen;
HPEN g_EmptyPen;
HPEN g_DotHilitePen;

//Colors
COLORREF cr_MenuItemBkgrd;
COLORREF cr_MenuItemText;
COLORREF cr_MenuItemDisabledText;
COLORREF cr_MenuDefaultBkgrd;
COLORREF cr_MenuDefaultText;
COLORREF cr_MenuDefaultDisabledText;
COLORREF cr_White;
COLORREF cr_Black;

//Global Variables
HINSTANCE hInst;
HICON mmIcon;
HICON engIcon;

//Useful file shorthands
const string cfgDir = "config";
const string fs = "\\";
const string cfgFile = "config.json.txt";

//Saved file names
string pathMainConfig;       //Path to the main config file.
string pathLocalFolder;      //Path to WZ's "local" folder in the AppData directory.
string pathLocalTinySave;    //Path to the "tiny" save file we use for quickly remembering last-used input methods, etc.
string pathLocalLastSavedVersionInfo; //Path to the last locally cached copy of the "latest version" file.
string pathLocalConfig;      //Path to WZ's "local" config.json.txt file in the AppData folder.
string pathUserConfig;       //Path to the user's config.json.txt file in his "My Documents" directory.



void createContextMenu();

//More globals  --  full program customization happens here
InputMethod*       currInput;     //Which of the two next inputs are currently in use?
InputMethod*       currTypeInput;
InputMethod*       currHelpInput; //NULL means disable help
DisplayMethod *mmFont;
DisplayMethod *mmFontSmall;
const Transformation*    input2Uni;
const Transformation*    uni2Output;
const Transformation*    uni2Disp;

//Cache our popup menu
HMENU contextMenu;
HMENU contextMenuPopup;
HMENU typingMenu;

//Some resources, etc., for our popup menu
TtfDisplay* menuFont;
TtfDisplay* sysFont;
unsigned int numInputOptions;


PAINTSTRUCT Ps;
WORD stopChar;
int numConfigOptions;
int numCustomWords;
INPUT inputItems[2000];
KEYBDINPUT keyInputPrototype;
bool helpIsCached;


//Ugh, prototypes...
BOOL CALLBACK SettingsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void checkAllHotkeysAndWindows();


//User-drawn menu data structures
enum WZMI_TYPES {WZMI_SEP, WZMI_HEADER, WZMI_LANG, WZMI_INPUT, WZMI_OUTPUT};
struct WZMenuItem {
	unsigned int menuID;
	WZMI_TYPES type;
	wstring id;
	wstring title;
	bool containsMM;
	bool disabled;

	WZMenuItem(){}

	WZMenuItem(unsigned int menuID, WZMI_TYPES type, const wstring&id, const wstring&title) {
		this->menuID = menuID;
		this->type = type;
		this->id = id;
		this->title = title;
		this->containsMM = false;
		this->disabled = false;
		for (size_t i=0; i<title.length(); i++) {
			if (title[i]>=L'\u1000' && title[i]<=L'\u109F') {
				this->containsMM = true;
				break;
			}
		}
	}
};
WZMenuItem* customMenuItems; //Because Win32 requires a pointer, and using vectors would be hacky at best.
std::map<int, WZMenuItem*> customMenuItemsLookup;
unsigned int totalMenuItems = 0;


//Help Window resources
// NOTE: These need to be pointers, or virtual function calls won't chain properly.
PulpCoreFont *helpFntKeys;
PulpCoreFont *helpFntFore;
PulpCoreFont *helpFntBack;
PulpCoreFont *helpFntMemory;
PulpCoreImage *helpCornerImg;
PulpCoreImage *helpCloseImg;
OnscreenKeyboard* helpKeyboard;

//Page down/up images
PulpCoreImage* pageImages[4] = {NULL, NULL, NULL, NULL};
enum { PGDOWNCOLOR_ID=0, PGUPCOLOR_ID=1, PGDOWNSEPIA_ID=2, PGUPSEPIA_ID=3 };

//BLENDFUNCTION BLEND_FULL = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA }; //NOTE: This requires premultiplied pixel values
//POINT PT_ORIGIN;
HANDLE keyTrackThread;   //Handle to our thread
DWORD  keyTrackThreadID; //Its unique ID (never zero)
CRITICAL_SECTION threadCriticalSec; //Global critical section object
list< pair<unsigned int, VirtKey> > hotkeysDown; //If a hotkey code (wparam) is in this list, its corresponding VKey is being tracked.
bool threadIsActive; //If "false", this thread must be woken to do anything useful


//User keystrokes
wstring userKeystrokeVector;

//Special resources for tracking the caret
//Note: This is run synchronously; it's spawned into its own thread just so we can
//      call "AttachThreadInput()"
HANDLE caretTrackThread;
DWORD caretTrackThreadID;
POINT caretLatestPosition;

//Help window colors
enum {
	COLOR_HELPFNT_KEYS =        0x606060,
    COLOR_HELPFNT_FORE =        0x000000,
    COLOR_HELPFNT_BACK =        0x0019FF,
};

//Our configuration
string getMD5Hash(const std::string& fileName);
ConfigManager config(getMD5Hash);


//These two will take some serious fixing later.
wstring hkString;
string hkRaw;

//To-do: make this settable via the config file; that way,
//   we can simply set it and disable the help window for good.
bool suppressHelpWindow = false;
bool suppressMemoryWindow = false;

//More old-style settings; have to remove somehow.
bool highlightKeys = true;
string fontFileRegular;
string fontFileSmall;

//Another possible setting & its associated thread data
bool checkLatestVersion = true;
HANDLE checkVersionThread;   //Handle to our thread
DWORD  checkVersionThreadID;     //Its unique ID (never zero)


//Partially-managed windows
MyWin32Window* mainWindow = NULL;
MyWin32Window* sentenceWindow = NULL;
MyWin32Window* helpWindow = NULL;
MyWin32Window* memoryWindow = NULL;

//Avoid cyclical messaging:
//bool mainWindowSkipMove = false;
//bool senWindowSkipMove = false;

//Helpful!
size_t changeEncRegionHandle = 0;


//Record-keeping
wstring currStr;
wstring currStrZg;
int currStrDictID;
wstring currLetterSt;
bool mmOn;
bool controlKeysOn = false;
bool numberKeysOn = false;
bool punctuationKeysOn = false;
bool extendedKeysOn = false;
bool helpKeysOn = false;
int prevProcessID;

//Disable input
bool showingHelpPopup = false;
bool showingKeyInputPopup = false;
bool showingSettingsPopup = false;


//Calculate's integers
int firstLineStart;
int secondLineStart;
int thirdLineStart;
int fourthLineStart;
int borderWidth = 2;
int spaceWidth;


//Log file, since the debugger doesn't like multi-process threads
// --Put into its own class
//bool isLogging = false;
//FILE *logFile;

//For testing
//FILETIME startTime;
//FILETIME endTime;
enum test_type {
	none,
	start_up,
	mywords,
	type_all,
	model_print
};
test_type currTest = none;



//Ugh, Windows
template< typename T >
inline T max(const T & a, const T & b) { return std::max(a, b); }
inline long max(const long &a, const int &b) { return max<long>(a, b); }

//Ugh, Windows
template< typename T >
inline T min(const T & a, const T & b) { return std::min(a, b); }
inline long min (const long &a, const int &b) { return min<long>(a,b); }


//Crypto++ implementation of MD5; we'll pass this as a pointer later.
string getMD5Hash(const std::string& fileName) {
	string md5Res;
	CryptoPP::Weak::MD5 hash;
	CryptoPP::FileSource(fileName.c_str(), true, new
		CryptoPP::HashFilter(hash,new CryptoPP::HexEncoder(new CryptoPP::StringSink(md5Res),false)));
	return md5Res;
}

//Means of getting a transformation; we'll have to pass this as a functional pointer later,
//   because of circular dependencies. TODO: Fix this.
const Transformation* ConfigGetTransformation(const Encoding& fromEnc, const Encoding& toEnc) {
	return config.getTransformation(config.activeLanguage, fromEnc, toEnc);
}


/*unsigned long getTimeDifferenceMS(const FILETIME &st, const FILETIME &end)
{
	if (st.dwHighDateTime != end.dwHighDateTime)
		return std::numeric_limits<DWORD>::max();
	return (end.dwLowDateTime - st.dwLowDateTime)/10000L;
}*/



/**
 * This thread is our locus-of-control for carets
 * It should probably always be run synchronously, to prevent another window from
 *   grabbing focus while it's waiting.
 */
DWORD WINAPI UpdateCaretPosition(LPVOID args)
{
	HWND foreWnd = GetForegroundWindow();
	if (IsWindowVisible(foreWnd)==TRUE) {
		DWORD foreID = GetWindowThreadProcessId(foreWnd, NULL);
		if (AttachThreadInput(caretTrackThreadID, foreID, TRUE)) {
			HWND focusWnd = GetFocus();
			HWND activeWnd = GetActiveWindow();

			if (IsWindowVisible(focusWnd)) {
				POINT mousePos;
				RECT clientUL;
				if (GetCaretPos(&mousePos) && GetWindowRect(focusWnd, &clientUL)!=0) {
					caretLatestPosition.x = clientUL.left + mousePos.x;
					caretLatestPosition.y = clientUL.top + mousePos.y;

					int caretHeight = sentenceWindow->getHeight();// SUB_WINDOW_HEIGHT;
					TEXTMETRICW tm;
					HFONT currFont = (HFONT)SendMessage(focusWnd, WM_GETFONT, 0, 0);
					if (mainWindow->getTextMetrics(&tm))
						caretHeight = tm.tmHeight;

					//We actually want the window slightly below this...
					caretLatestPosition.x -= 1;
					caretLatestPosition.y -= mainWindow->getHeight();//WINDOW_HEIGHT;
					caretLatestPosition.y -= (sentenceWindow->getHeight()-caretHeight)/2;
				}
			}

			//Is this inside the main window?
			RECT desktopW;
			GetWindowRect(GetDesktopWindow(), &desktopW);
			if (   caretLatestPosition.x<0 || caretLatestPosition.x+mainWindow->getWidth()>desktopW.right
				|| caretLatestPosition.y<0 || caretLatestPosition.y+mainWindow->getHeight()>desktopW.bottom) {
				//Better set it to zero, rather than try moving it into range.
				caretLatestPosition.x = 0;
				caretLatestPosition.y = 0;
			}

			//Finally
			AttachThreadInput(caretTrackThreadID, foreID, FALSE);
		}
	}

	return 0;
}



/**
 * This is our threaded locus-of-control, which is woken when a keypress is detected, and
 *   put to sleep when all keys have been released. It is very important that this
 *   thread run very fast; we operate on the assumption that it completes in less
 *   than 1 Thread cycle, including synchronization.
 * This thread is not activated if the "highlight keys" flag is off. This is intended to
 *   allow increased performance on slow systems, decreased annoyance for advanced users,
 *   and an ultimate fall-back if the thread is shown to deadlock or stall with continuous usage.
 * @args = always null.
 * @returns = 0 for success (never really returns)
 */
DWORD WINAPI TrackHotkeyReleases(LPVOID args)
{
	//Loop forever
	for (;;) {
		//Check every key. Since we generally are tracking a very small number of keys, it makes
		//  sense to put this into its own critical section

		//CRITICAL SECTION
		{
			EnterCriticalSection(&threadCriticalSec);

			/*if (isLogging)
				fprintf(logFile, "Thread:\n");*/

			//Loop through our list
			for (std::list< std::pair<unsigned int, VirtKey> >::iterator keyItr = hotkeysDown.begin(); keyItr != hotkeysDown.end();) {
				//Create a version of this vkey in the current keyboard layout
				VirtKey translated(keyItr->second);
				translated.considerLocale();

				//Get the state of this key
				SHORT keyState = GetAsyncKeyState(helpKeyboard->getVirtualKeyID(translated.vkCode, translated.alphanum, translated.modShift)); //We need to use CAPITAL letters for virtual keys. Gah!

				if ((keyState & 0x8000)==0) {
					//Send a hotkey_up event to our window (mimic the wparam used by WM_HOTKEY)
					/*if (isLogging)
						fprintf(logFile, "  Key up: %c  (%x)\n", *keyItr, keyState);*/

					//Send an en_US-style HOTKEY_UP message to the main Window. Do NOT check for failure.
					mainWindow->postMessage(UWM_HOTKEY_UP, keyItr->first, keyItr->second.toLParam());
						//SendMessage(mainWindow, UWM_HOTKEY_UP, *keyItr,0); //Send message seems to make no difference
						//MessageBox(NULL, _T("Couldn't post message to Main Window"), _T("Error"), MB_OK);
						//Bad! Don't send Message Boxes from a critical section!


					//Key has been released, stop tracking it
					keyItr = hotkeysDown.erase(keyItr); //Returns the next valid value
				} else {
					//Normal iteration
					keyItr++;
				}
			}

			//Sleep, but for how long?
			if (hotkeysDown.empty()) {
				/*if (isLogging)
					fprintf(logFile, "  Sleep: forever\n");*/

				//Sleep until woken up
				threadIsActive = false;
				LeaveCriticalSection(&threadCriticalSec);
				SuspendThread(keyTrackThread);
			} else {
				/*if (isLogging)
					fprintf(logFile, "  Sleep: 10ms\n");*/

				//Sleep for 10ms, and continue tracking keyboard input
				LeaveCriticalSection(&threadCriticalSec);
				Sleep(10);
			}
		}
	}

	return 0;
}





//General method to check if a new version is available.
DWORD WINAPI CheckForNewVersion(LPVOID args)
{
	try {
		//Nightlies never check this.
		//if (WZ_VERSION_IS_NIGHTLY)
		//	return 0;

		//Wait a bit before checking
		Sleep(10 * 1000);  //10 seconds

		//First, try to download the URL into a local file.
		wstringstream temp;
		temp <<pathLocalLastSavedVersionInfo.c_str();
		if (URLDownloadToFile(NULL, L"http://waitzar.googlecode.com/svn/trunk/win32_source/waitzar_versions.txt", temp.str().c_str(), 0, NULL)!=S_OK)
			return 0;

		//Second, open the file and parse it line-by-line
		std::ifstream txtFile(pathLocalLastSavedVersionInfo.c_str(), std::ios::in|std::ios::binary);
		if (txtFile.fail())
			return 0;

		//Get the size, rewind
		txtFile.seekg(0, std::ios::end);
		int file_size = txtFile.tellg();
		txtFile.seekg(0, std::ios::beg);
		if (file_size==-1)
			return 0;

		//Load the entire file at once to minimize file I/O
		unsigned char* buffer = new unsigned char [file_size];
		txtFile.read((char*)(&buffer[0]), file_size);
		txtFile.close();

		//Now, loop
		vector<string> lines;
		std::stringstream currLine;
		for (size_t i=0; i<(size_t)file_size; i++) {
			//Skip \r, space
			if (buffer[i]=='\r' || buffer[i]==' ')
				continue;

			//If we encounter a '#', skip to the end of the line
			if (buffer[i]=='#') {
				while (buffer[i]!='\n' && i<(size_t)file_size)
					i++;
			}

			//If we're at the end of the line, add an entry
			if (buffer[i]=='\n' || i==((size_t)file_size)-1) {
				if (!currLine.str().empty()) {
					lines.push_back(currLine.str());
					currLine.str("");
				}
			} else {
				//Otherwise, just add the letter
				currLine <<buffer[i];
			}
		}

		//Finally, check. For now, we only fail if our item is in the list but not in the first position.
		//  This makes it easier to whitelist, say, a "long-term" support version later.
		int ourIndex = -1;
		string verString = waitzar::escape_wstr(WZ_VERSION_MAIN, false);
		for (size_t i=0; i<lines.size(); i++) {
			if (lines[i] == verString) {
				ourIndex = i;
				break;
			}
		}
		newVersionAvailable = ourIndex>0;

	} catch (std::exception ex) {
		//Silently fail on exceptions
		newVersionAvailable = false;
	}

	//What to do if there's a new version available
	if (newVersionAvailable) {
		try {
			//Add a balloon tooltip to our systray icon.
			//Note that multiple messages will stack, so we can just add this message and it 
			//   will display after the "Welcome to WaitZar" message.
			NOTIFYICONDATA nid;
			mainWindow->initShellNotifyIconData(nid);
			nid.uID = STATUS_NID;
			nid.uFlags = NIF_INFO; //Only update the balloon-related variables are set (szInfo, szInfoTitle, dwInfoFlags, uTimeout)
			lstrcpy(nid.szInfoTitle, _T("New Version Available"));
			lstrcpy(nid.szInfo, _T("The current version of WaitZar is out of date.\n\nPlease click here to download the latest version."));
			//nid.uTimeout = 30;  //timeout is invalid as of Vista
			nid.uVersion = NOTIFYICON_VERSION;
			nid.dwInfoFlags = NIIF_WARNING; //Can we switch to NIIF_USER if supported?
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		} catch (std::exception ex) {} // Fail silently.
	}


	return 0;
}




//Borrowed from the code project, modified
//http://www.codeproject.com/KB/graphics/creategrayscaleicon.aspx?msg=163218#xx163218xx
//TODO: Speed this up using array access, removing floatin point.
//TODO: We might consider caching grayscale bitmaps instead.
HBITMAP CreateGrayscaleBitmap(HBITMAP hBmp)
{
    HBITMAP     hGrayBmp = NULL;
    HDC         hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
    BITMAP      bmp;
    HBITMAP     hOldBmp1 = NULL, hOldBmp2 = NULL;
    //ICONINFO    csII, csGrayII;
    //BOOL        bRetValue = FALSE;

    //bRetValue = ::GetIconInfo(hIcon, &csII);
    //if (bRetValue == FALSE) return NULL;

    hMainDC = ::GetDC(NULL);
    hMemDC1 = ::CreateCompatibleDC(hMainDC);
    hMemDC2 = ::CreateCompatibleDC(hMainDC);
    if (hMainDC == NULL || hMemDC1 == NULL || hMemDC2 == NULL) return NULL;

    if (GetObject(hBmp, sizeof(BITMAP), &bmp))
    {
        //DWORD   dwWidth = bmp. csII.xHotspot*2;
        //DWORD   dwHeight = csII.yHotspot*2;

		hGrayBmp = ::CreateBitmap(bmp.bmWidth, bmp.bmHeight, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
        if (true /*csGrayII.hbmColor*/)
        {
            hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, hBmp/*csII.hbmColor*/);
            hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, hGrayBmp/*csGrayII.hbmColor*/);

            //::BitBlt(hMemDC2, 0, 0, dwWidth, dwHeight, hMemDC1, 0, 0,

            //         SRCCOPY);


            LONG    dwLoopY = 0, dwLoopX = 0;
            COLORREF crPixel = 0;
            BYTE     byNewPixel = 0;

            for (dwLoopY = 0; dwLoopY < bmp.bmHeight; dwLoopY++)
            {
                for (dwLoopX = 0; dwLoopX < bmp.bmWidth; dwLoopX++)
                {
                    crPixel = ::GetPixel(hMemDC1, dwLoopX, dwLoopY);

                    byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) +
                                        (GetGValue(crPixel) * 0.587) +
                                        (GetBValue(crPixel) * 0.114));
                    if (crPixel) ::SetPixel(hMemDC2, dwLoopX, dwLoopY,
                                            RGB(byNewPixel, byNewPixel,
                                            byNewPixel));
                } // for

            } // for


            ::SelectObject(hMemDC1, hOldBmp1);
            ::SelectObject(hMemDC2, hOldBmp2);

            //csGrayII.hbmMask = csII.hbmMask;

            //csGrayII.fIcon = TRUE;
            //hGrayIcon = ::CreateIconIndirect(&csGrayII);
        } // if


        //::DeleteObject(csGrayII.hbmColor);
        //::DeleteObject(csGrayII.hbmMask);

    } // if


    //::DeleteObject(csII.hbmColor);
    //::DeleteObject(csII.hbmMask);
    ::DeleteDC(hMemDC1);
    ::DeleteDC(hMemDC2);
    ::ReleaseDC(NULL, hMainDC);

    return hGrayBmp;
} // End of CreateGrayscaleIcon



bool FileExists(const wstring& fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA InfoFile;
	return (GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &InfoFile)==TRUE);
}




//DirToCheck should be of searchable form:
//   c:\x...x
//   The \* will be appended
vector<string> GetConfigSubDirs(std::string dirToCheck, std::string configFileName)
{
	//Convert to wstring
	std::wstringstream dir;
	dir << dirToCheck.c_str();
	dir << "\\*";

	//First, just get all sub-directories with non-Unicode names.
	vector<string> allDir;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(dir.str().c_str(), &FindFileData);
	BOOL hFindRes = (hFind!=INVALID_HANDLE_VALUE);
	for (;hFindRes; hFindRes=FindNextFile(hFind, &FindFileData)) {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			try {
				allDir.push_back(waitzar::escape_wstr(wstring(FindFileData.cFileName), true));
			} catch (std::exception()) {}
		}
	}

	//Next, add only the directories (excluding . and ..) that contain configuration files.
	vector<string> resDir;
	for (size_t i=0; i<allDir.size(); i++) {
		//Easy
		string path = allDir[i];
		if (path == "." || path == "..")
			continue;

		//Harder
		std::wstringstream newpath;
		newpath <<dirToCheck.c_str();
		newpath <<"/" <<path.c_str();
		newpath <<"/" <<configFileName.c_str();
		if (!FileExists(newpath.str()))
			continue;

		resDir.push_back(allDir[i]);
	}
	return resDir;
}


//Quickly load and parse the current input method, etc., for all languages
void FlashLoadState(map<wstring, vector<wstring> >& lastUsedSettings)
{
	//Try to open it.
	if (pathLocalTinySave.empty())
		return;
	wstring buff;
	try {
		buff = waitzar::readUTF8File(pathLocalTinySave);
		if (buff.empty())
			return;
	} catch (std::exception ex) {
		return;
	}

	//Read the version number
	size_t i=0;
	int version = 0;
	for (;buff[i]!='\n' && i<buff.size();i++) {
		if (buff[i]>='0' && buff[i]<='1')
			version = version*10 + (buff[i]-'0');
	}
	if (version!=FLASH_SAVE_VERSION_NUMBER)
		return; //Out-of-date.

	//Now, read each line. Break it down by ":" as well.
	wstring currLang = L"";
	wstringstream currOpt;
	for (i++; i<buff.size(); i++) {
		//Add this?
		if (buff[i]!='\r' && buff[i]!='\n' && buff[i]!=':' && buff[i]!='!')
			currOpt <<buff[i];

		//New option?
		if (buff[i]==L':' || buff[i]==L'!' || buff[i]==L'\n' || i==buff.size()-1) {
			if (currLang.empty()) {
				//Set the language
				currLang = currOpt.str();

				//Note if it's the default
				if (buff[i]==L'!') {
					lastUsedSettings[L"language.default"].push_back(currLang);
				}
			} else {
				//Push back the current value
				lastUsedSettings[currLang].push_back(currOpt.str());
			}
			currOpt.str(L"");
		}

		//New line?
		if (buff[i]==L'\n') 
			currLang = L"";
	}
}


//Quickly save the current input method, etc. for all languages.
void FlashSaveState()
{
	//Try to open it.
	if (pathLocalTinySave.empty())
		return;
	ofstream outFile(pathLocalTinySave.c_str(), std::ios::out|std::ios::binary); //binary needed for utf-8
	if (outFile.fail())
		return;


	//Silently fail on any error.
	wstringstream outStr;
	try {
		//Write a version number
		outStr <<FLASH_SAVE_VERSION_NUMBER <<L"\n";
		for (std::set<Language>::const_iterator it=config.getLanguages().begin(); it!=config.getLanguages().end(); it++) {
			if (it->id==config.activeLanguage.id) {
				outStr <<config.activeLanguage.id <<L"!" <<config.activeInputMethod->id <<L":"; //Use a "!" to mean "default"
				outStr <<config.activeOutputEncoding.id <<L":" <<config.activeDisplayMethods[0]->id <<L":";
				outStr <<config.activeDisplayMethods[1]->id <<L"\n";
			} else {
				outStr <<it->id <<L":" <<it->defaultInputMethod <<L":" <<it->defaultOutputEncoding.id <<L":";
				outStr <<it->defaultDisplayMethodReg <<L":" <<it->defaultDisplayMethodSmall <<L"\n";
			}
		}

		//Write a BOM (why, Microsoft, why!)
		//outFile <<((char)0xFE) <<((char)0xFF);

		//Convert to UTF-8 and write it.
		outFile <<waitzar::wcs2mbs(outStr.str());

	} catch (std::exception ex) {}

	//Done
	outFile.close();
}




//False on some error
/*bool testAllWordsByHand()
{
	//First, ensure that the reverse-lookup is ready
	model->reverseLookupWord(0);

	//Time
	GetSystemTimeAsFileTime(&startTime);

	//For each typable word
	string revWord;
	for (unsigned int wordID=0; ; wordID++) {
		//Check
		revWord=model->reverseLookupWord(wordID);
		if (revWord.empty())
			break;

		//Type this
		model->reset(false);
		for (string::iterator rom = revWord.begin(); rom!=revWord.end(); rom++) {
			//Just check that our romanisation is stored properly.
			if (!model->typeLetter(*rom))
				return false;
		}

		//Test "output" it
		std::pair<bool, unsigned int> ret = model->typeSpace(-1, false);
		if (!ret.first)
			return false;
		model->getWordKeyStrokes(ret.second);
	}


	//Done, display a message box
	GetSystemTimeAsFileTime(&endTime);
	DWORD timeMS = getTimeDifferenceMS(startTime, endTime);

	wchar_t msg[500];
	swprintf(msg, L"Type All total time:   %dms", timeMS);
	MessageBox(NULL, msg, L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);
	return true;
}*/







/**
 * Create our inner-used Zawgyi-One fonts.
 */
void makeFont()
{
	//Load our help window font: Keys
	{
		//First the resource
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(IDR_HELP_KEY_FONT), _T("COREFONT"));
		if (!fontRes) {
			std::wstringstream msg;
			msg <<L"Couldn't find IDR_HELP_KEY_FONT: " <<IDR_HELP_KEY_FONT << " -> " <<GetLastError();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_HELP_KEY_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		helpFntKeys = new PulpCoreFont();
		try {
			helpWindow->initPulpCoreImage(helpFntKeys, fontRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Help Font (keys) didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Tint to default
		helpFntKeys->tintSelf(COLOR_HELPFNT_KEYS);
	}

	//Load our help window font: Foreground
	{
		//First the resource
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(IDR_HELP_FORE_FONT), _T("COREFONT"));
		if (!fontRes) {
			MessageBox(NULL, _T("Couldn't find IDR_HELP_FORE_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_HELP_FORE_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		helpFntFore = new PulpCoreFont();
		try {
			helpWindow->initPulpCoreImage(helpFntFore, fontRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Help Font (foreground) didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Tint to default
		helpFntFore->tintSelf(COLOR_HELPFNT_FORE);
	}

	//Load our help window font: Background
	{
		//First the resource
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(IDR_HELP_BACK_FONT), _T("COREFONT"));
		if (!fontRes) {
			MessageBox(NULL, _T("Couldn't find IDR_HELP_BACK_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_HELP_BACK_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		helpFntBack = new PulpCoreFont();
		try {
			helpWindow->initPulpCoreImage(helpFntBack, fontRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Help Font (background) didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Tint to default
		helpFntBack->tintSelf(COLOR_HELPFNT_BACK);

		//Unlock this resource for later use.
		//UnlockResource(res_handle);
	}


	//Load our help menu's corner image (used for keyboard keys)
	{
		//First the resource
		HRSRC imgRes = FindResource(hInst, MAKEINTRESOURCE(IDR_HELP_CORNER_IMG), _T("COREFONT"));
		if (!imgRes) {
			MessageBox(NULL, _T("Couldn't find IDR_HELP_CORNER_IMG"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, imgRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_HELP_CORNER_IMG"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		helpCornerImg = new PulpCoreImage();
		try {
			helpWindow->initPulpCoreImage(helpCornerImg, imgRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Corner Image File didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Unlock this resource for later use.
		//UnlockResource(res_handle);
	}


	//Load our help menu's "close" image (used because I can't find an anti-aliasing algorithm I like)
	{
		//First the resource
		HRSRC imgRes = FindResource(hInst, MAKEINTRESOURCE(IDR_HELP_CLOSE_IMG), _T("COREFONT"));
		if (!imgRes) {
			MessageBox(NULL, _T("Couldn't find IDR_HELP_CLOSE_IMG"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, imgRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_HELP_CLOSE_IMG"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		helpCloseImg = new PulpCoreImage();
		try {
			helpWindow->initPulpCoreImage(helpCloseImg, imgRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Close Image File didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Unlock this resource for later use.
		//UnlockResource(res_handle);
	}


	//Load our page down (color) image, generate the rest.
	{
		HRSRC imgRes = FindResource(hInst, MAKEINTRESOURCE(IDR_PGDOWN_COLOR), _T("COREFONT"));
		if (!imgRes) {
			MessageBox(NULL, _T("Couldn't find IDR_PGDOWN_COLOR"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, imgRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on IDR_PGDOWN_COLOR"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		pageImages[0] = new PulpCoreImage();
		try {
			mainWindow->initPulpCoreImage(pageImages[0], imgRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"IDR_PGDOWN_COLOR image didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}
	}

	//Initialize the other 3.
	for (size_t i=1; i<=3; i++)
		pageImages[i] = new PulpCoreImage();


	//Copy image[1], PGUP_COLOR, and flip image[0] to create it
	mainWindow->initPulpCoreImage(pageImages[1], pageImages[0]);
	pageImages[1]->flipSelfVertical();

	//Copy image[2], IDR_PGDOWN_SEPIA, and sepia-ize image[0] to create it.
	mainWindow->initPulpCoreImage(pageImages[2], pageImages[0]);
	pageImages[2]->sepiaizeSelf();

	//Copy image[3], IDR_PGUP_SEPIA, and sepia-ize image[1] to create it.
	mainWindow->initPulpCoreImage(pageImages[3], pageImages[1]);
	pageImages[3]->sepiaizeSelf();





	//Load our page down/up images
	/*for (int i=0; i<4; i++) {
		//First the resource
		int PG_RES_ID = i==0?IDR_PGDOWN_COLOR:i==1?IDR_PGUP_COLOR:i==2?IDR_PGDOWN_SEPIA:IDR_PGUP_SEPIA;
		HRSRC imgRes = FindResource(hInst, MAKEINTRESOURCE(PG_RES_ID), _T("COREFONT"));
		if (!imgRes) {
			MessageBox(NULL, _T("Couldn't find PG_RES_ID"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, imgRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on PG_RES_ID"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		pageImages[i] = new PulpCoreImage();
		try {
			mainWindow->initPulpCoreImage(pageImages[i], imgRes, res_handle);
		} catch (std::exception ex) {
			wstringstream msg;
			msg <<"WZ Page Image File didn't load correctly: " <<ex.what();
			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			throw ex;
		}

		//Unlock this resource for later use.
		//UnlockResource(res_handle);
	}*/
}




BOOL waitzarAlreadyStarted()
{
	//Get all processes
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return FALSE;
	cProcesses = cbNeeded / sizeof(DWORD);

	//Check for "WaitZar"
	TCHAR szProcessName[MAX_PATH];
	for (unsigned int i=0; i<cProcesses; i++ ) {
		if( aProcesses[i] != 0 ) {
			//Open a handle to this process, get its name
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, aProcesses[i]);
			if (hProcess!=NULL) {
				HMODULE hMod;
				DWORD cbNeeded;
				lstrcpy(szProcessName, _T("<unknown>"));
				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
					GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
					if (lstrcmp(szProcessName, _T("WaitZar.exe"))==0)
						return TRUE;
				}
			}
        }
	}

	return FALSE;
}


/*void setEncoding(ENCODING encoding)
{
	if (encoding==ENCODING_WININNWA)
		currEncStr = L"WI";
	else if (encoding==ENCODING_ZAWGYI)
		currEncStr = L"ZG";
	else if (encoding==ENCODING_UNICODE)
		currEncStr = L"UNI";

	//Set this encoding, and save its value (in case we haven't loaded the model yet)
	mostRecentEncoding = encoding;
	if (model!=NULL)
		model->setOutputEncoding(encoding);
}*/


void loadConfigOptions()
{
	//Default keys
	hkString  = L"Ctrl+Shift";
	hkRaw = "^+";

	//Default encoding
	//setEncoding(ENCODING_UNICODE);

	//Default font files
	fontFileRegular = "";
	fontFileSmall = "";

	//Read our config file, if it exists.
	numConfigOptions = -1;
	FILE* configFile = fopen("config.txt", "r");
	if (configFile == NULL)
		return;

	//Get file size
	fseek (configFile, 0, SEEK_END);
	long fileSize = ftell(configFile);
	rewind(configFile);

	//Read it all into an array, close the file.
	char * buffer = (char*) malloc(sizeof(char)*fileSize);
	size_t buff_size = fread(buffer, 1, fileSize, configFile);
	fclose(configFile);


	//Read each line
	numConfigOptions = 0;
	char* name = new char[100];
	char* value = new char[100];
	for (size_t i=0; i<buff_size;) {
		//Read name/value
		readLine(buffer, i, buff_size, true, false, false, false, true, false, true, false, name, value);

		//Are both name and value non-zero?
		if (strlen(name)==0 || strlen(value)==0)
			continue;

		//Deal with our name/value pair.
		if (strcmp(name, "mywordswarning")==0) {
		} else if (strcmp(name, "lockwindows")==0) {
		} else if (strcmp(name, "powertyping")==0) {
		} else if (strcmp(name, "burmesenumerals")==0) {
		} else if (strcmp(name, "ballooononstart")==0) {
		} else if (strcmp(name, "alwayselevate")==0) {
		} else if (strcmp(name, "trackcaret")==0) {
		} else if (strcmp(name, "ignoremodel")==0) {
		} else if (strcmp(name, "silencemywordserrors")==0) {
		} else if (strcmp(name, "charaset")==0) {
		} else if (strcmp(name, "defaultencoding")==0) {
		} else if (strcmp(name, "hotkey")==0) {
			//Set it later
			hkRaw = string(value);
			numConfigOptions++;
		} else if (strcmp(name, "fontfileregular")==0) {
			if (strcmp(value, "embedded")==0 || strcmp(value, "default")==0) {
			} else {
				fontFileRegular = string(value);
			}
			numConfigOptions++;
		} else if (strcmp(name, "fontfilesmall")==0) {
			if (strcmp(value, "embedded")==0 || strcmp(value, "default")==0) {
			} else {
				fontFileSmall = string(value);
			}
			numConfigOptions++;
		}

	}

	//Get rid of our buffer
	free(buffer);
	delete [] name;
	delete [] value;
}



bool registerInitialHotkey()
{
	UINT modifier = 0;
	UINT keycode = 0;

	//Now, set the keycode
	//Additional rule: all keystroke modifiers must also themselves be modifiers
	keycode = hkRaw[hkRaw.length()-1];
	switch(keycode) {
		case '!':
			hkString = L"Alt";
			keycode = VK_MENU; //VK_MENU == VK_ALT
			modifier |= MOD_ALT;
			break;
		case '^':
			hkString = L"Ctrl";
			keycode = VK_CONTROL;
			modifier |= MOD_CONTROL;
			break;
		case '+':
			hkString = L"Shift";
			keycode = VK_SHIFT;
			modifier |= MOD_SHIFT;
			break;
		case '_':
			hkString = L"Space";
			keycode = VK_SPACE;
			break;
		default:
			hkString = L"*";
			hkString[0] = (wchar_t)keycode;
	}

	//Now, set the modifiers
	for (size_t pos=0; pos<hkRaw.length()-1; pos++) {
		switch(hkRaw[pos]) {
			case '!':
				hkString = L"Alt+" + hkString;
				modifier |= MOD_ALT;
				break;
			case '^':
				hkString = L"Ctrl+" + hkString;
				modifier |= MOD_CONTROL;
				break;
			case '+':
				hkString = L"Shift+" + hkString;
				modifier |= MOD_SHIFT;
				break;
		}
	}

	//Additional rule: Capital letters require a shift modifier
	if (keycode>='A' && keycode<='Z') {
		hkString = L"Shift+" + hkString;
		modifier |= MOD_SHIFT;
	}

	//Additional rule: Lowercase letters are coded by their uppercase value
	if (keycode>='a' && keycode<='z') {
		keycode -= 'a'-'A';
	}

	return mainWindow->registerHotKey(LANG_HOTKEY, modifier, keycode);
}


//Re-position this near the caret
void positionAtCaret()
{
	//Default "off" flag
	if (!config.getSettings().trackCaret)
		return;

	//Also skip if one window is already visible
	if (mainWindow->isVisible() || sentenceWindow->isVisible())
		return;

	//Reset parameters for our thread
	//  (We set to a nice default, instead of 0,0, so that our window doesn't get "stuck" somewhere.)
	caretLatestPosition.x = 0;
	caretLatestPosition.y = 0;

	//Create and start our thread for tracking the caret
	caretTrackThread = CreateThread(
		NULL,                //Default security attributes
		0,                   //Default stack size
		UpdateCaretPosition, //Threaded function (name)
		NULL,                //Arguments to threaded function
		0,
		&caretTrackThreadID);//Pointer to return the thread's id into
	if (caretTrackThread==NULL) {
		MessageBox(NULL, _T("WaitZar could not create a helper thread. \nThis will not affect normal operation; however, it means that we can't track the caret."), _T("Warning"), MB_ICONWARNING | MB_OK);
		config.overrideSetting(L"track-caret", false);
	}

	//Wait for it.
	WaitForSingleObject(caretTrackThread, 1000);

	//Close it
	CloseHandle(caretTrackThread);

	//Ready?
	if (caretLatestPosition.x!=0 && caretLatestPosition.y!=0) {
		//Line up our windows
		mainWindow->moveWindow(caretLatestPosition.x, caretLatestPosition.y);
		sentenceWindow->moveWindow(caretLatestPosition.x, caretLatestPosition.y + mainWindow->getHeight());
		//MoveWindow(mainWindow, caretLatestPosition.x, caretLatestPosition.y, WINDOW_WIDTH, WINDOW_HEIGHT, FALSE);
		//MoveWindow(senWindow, caretLatestPosition.x, caretLatestPosition.y+WINDOW_HEIGHT, SUB_WINDOW_WIDTH, SUB_WINDOW_HEIGHT, FALSE);
	} else {
		//At least line them up on the main window itself.
		mainWindow->moveWindow(mainWindow->getXPos(), mainWindow->getYPos());
		sentenceWindow->moveWindow(mainWindow->getXPos(), mainWindow->getYPos() + mainWindow->getHeight());
	}
}



///////////////////////////////////////////
// Hotkey registration/deregistration
///////////////////////////////////////////

bool turnOnHotkeySet(const Hotkey* hkSet, const size_t& size, bool on, bool& flagToSet)
{
	//Do nothing if already on/off
	if (flagToSet==on)
		return true; //Only return false if there's an error.

	//Track failures
	bool retVal = true;
	for (size_t i=0; i<size; i++) {
		Hotkey h = hkSet[i];

		//And this result with the current return value (but don't short-circuit the call).
		if (on)
			retVal = mainWindow->registerHotKey(h.hotkeyID, (h.useShift?MOD_SHIFT:0), h.virtKey) && retVal;
		else
			retVal = mainWindow->unregisterHotKey(h.hotkeyID) && retVal;
	}

	//Set a flag
	flagToSet = on;

	//Done; return true if ALL registers/unregisters passed.
	return retVal;
}


bool turnOnPunctuationkeys(bool on)
{
	//Todo: Use vector initialization later (C++ 0x)
	return turnOnHotkeySet(PunctuationHotkeys, sizeof(PunctuationHotkeys)/sizeof(Hotkey), on, punctuationKeysOn);
}


bool turnOnNumberkeys(bool on)
{
	//Todo: Use vector initialization later (C++ 0x)
	return turnOnHotkeySet(NumberHotkeys, sizeof(NumberHotkeys)/sizeof(Hotkey), on, numberKeysOn);
}


bool turnOnHelpKeys(bool on)
{
	//Todo: Use vector initialization later (C++ 0x)
	return turnOnHotkeySet(HelpHotkeys, sizeof(HelpHotkeys)/sizeof(Hotkey), on, helpKeysOn);
}


bool turnOnExtendedKeys(bool on)
{
	//Todo: Use vector initialization later (C++ 0x)
	return turnOnHotkeySet(ExtendedHotkeys, sizeof(ExtendedHotkeys)/sizeof(Hotkey), on, extendedKeysOn);
}



bool turnOnControlkeys(bool on)
{
	//Todo: Use vector initialization later (C++ 0x)
	return turnOnHotkeySet(ControlHotkeys, sizeof(ControlHotkeys)/sizeof(Hotkey), on, controlKeysOn);
}


//TODO: Find a way of automating this, without writing out each one. Is there some kind of
//      good static initializer or generator?
bool turnOnAlphaHotkeys(bool on, bool affectLowercase, bool affectUppercase)
{
	int low_code;
	int high_code;
	bool retVal = TRUE;

	for (low_code=HOTKEY_A_LOW; low_code<=HOTKEY_Z_LOW; low_code++)
	{
		high_code = low_code - 32;
		if (on)  {
			//Register this as an uppercase/lowercase letter
			if (affectUppercase) {
				if (!mainWindow->registerHotKey(high_code, MOD_SHIFT, high_code))
					retVal = false;
			}
			if (affectLowercase) {
				if (!mainWindow->registerHotKey(low_code, NULL, high_code))
					retVal = false;
			}
		} else {
			//De-register this as an uppercase/lowercase letter
			if (affectUppercase) {
				if (!mainWindow->unregisterHotKey(high_code))
					retVal = false;
			}
			if (affectLowercase) {
				if (!mainWindow->unregisterHotKey(low_code))
					retVal = false;
			}
		}
	}

	return retVal;
}


///////////////////////////////////////////
// End hotkey functions
///////////////////////////////////////////




//Show our simple settings dialog.
void showSettingsMenu(HWND hwnd)
{
	//Properly handle hotkeys
	bool refreshControl = controlKeysOn;
	if  (refreshControl)
		turnOnControlkeys(false);


	//MessageBox(hwnd, temp, _T("About"), MB_ICONINFORMATION | MB_OK);
	if (!showingSettingsPopup) {
		showingSettingsPopup = true;

		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_BLANK), hwnd, SettingsDlgProc);
		//MessageBox(NULL, L"Settings", L"Settings", MB_ICONINFORMATION|MB_OK);

		showingSettingsPopup = false;
	}

	//Hotkeys again
	if  (refreshControl)
		turnOnControlkeys(true);
}


//Prototype needed by switchToLanguage
void toggleHelpMode(bool toggleTo);

void switchToLanguage(bool toMM) {
	//Don't do anything if we are switching to the SAME language.
	if (toMM == mmOn)
		return;

	//Ok, switch
	bool res = turnOnAlphaHotkeys(toMM, true, true);
	res = turnOnPunctuationkeys(toMM) && res;
	res = turnOnNumberkeys(toMM) && res;
	res = turnOnExtendedKeys(toMM) && res;

	//If switching to English, turn off all remaining hotkeys. Else, reset the model
	if (!toMM) {
		res = turnOnControlkeys(toMM) && res;
		res = turnOnExtendedKeys(toMM) && res;
	} else {
		//res = turnOnControlkeys(toMM) && res; //Turn ON control keys? Ugh, we need to check...
		currInput->reset(true, true, true, true);
	}

	//Turn on/of our main Help hotkey
	if (toMM)
		res = mainWindow->registerHotKey(HOTKEY_HELP, NULL, VK_F1) && res;
	else {
		mainWindow->unregisterHotKey(HOTKEY_HELP);
	}

	//Turn off "help mode" if we're in it
	if (!toMM)
		toggleHelpMode(false);

	//Any errors?
	if (!res)
		MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

	//Switch to our target language.
	mmOn = toMM;

	//Change icon in the tray
	NOTIFYICONDATA nid;
	mainWindow->initShellNotifyIconData(nid);
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.uVersion = NOTIFYICON_VERSION; //Win2000+ messages
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	if (mmOn)
		nid.hIcon = mmIcon;
	else
		nid.hIcon = engIcon;

	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE) {
		wstringstream msg;
		msg <<"Can't switch icon.\nError code: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Warning"), MB_ICONERROR | MB_OK);
	}


	//Hide all windows
	if (!mmOn) {
		mainWindow->showWindow(false);
		sentenceWindow->showWindow(false);
		helpKeyboard->turnOnHelpMode(false, suppressHelpWindow, suppressMemoryWindow);
	}
}


void reBlit()
{
	//Bit blit our back buffer to the front (should prevent flickering)
	mainWindow->repaintWindow();
	sentenceWindow->repaintWindow();
}


//Can't just blit it; we have to use updatelayeredwindow
void reBlitHelp()
{
	//Help Window
	if (!helpWindow->repaintWindow()) {
		wstringstream msg;
		msg <<"Help window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
	}

	//Memory Window
	if (!memoryWindow->repaintWindow()) {
		wstringstream msg;
		msg <<"Memory window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
	}
}


//Only blit part of the area
void reBlit(RECT blitArea)
{
	//Bit blit our back buffer to the front (should prevent flickering)
	mainWindow->repaintWindow(blitArea);
	sentenceWindow->repaintWindow(blitArea);
}


//Can't just blit it; we have to use updatelayeredwindow
//NOTE: We have to track when this is called, instead of just repainting the entire window.
//NOTE: Seems like it's never called.
void reBlitHelp(RECT blitArea)
{
	//Help Window
	if (!helpWindow->repaintWindow(blitArea)) {
		wstringstream msg;
		msg <<"Help window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
	}

	//Memory Window
	if (!memoryWindow->repaintWindow(blitArea)) {
		wstringstream msg;
		msg <<"Memory window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
	}
}


/**
 * Initialize our on-screen keyboard
 */
void initCalculateHelp()
{
	//Initialize our keyboard
	//TODO: Is there a better way?
	if (helpKeyboard!=NULL)
		delete helpKeyboard;

	//Copy this font for use in the memory box
	//TODO: Something else.
	helpFntMemory = (PulpCoreFont*)WZFactory<WordBuilder>::getZawgyiPngDisplay(L"myanmar", L"zawgibmpsmall", IDR_SMALL_FONT);
	DisplayMethod* zgSmall = WZFactory<WordBuilder>::getZawgyiPngDisplay(L"myanmar", L"zawgibmpsmall", IDR_SMALL_FONT);
	//= (PulpCoreFont*)mmFontSmall;//new PulpCoreFont();

	//Make
	helpKeyboard = new OnscreenKeyboard(zgSmall, helpFntKeys, helpFntFore, helpFntBack, helpFntMemory, helpCornerImg, helpCloseImg);

}



void initCalculate()
{
	//Figure out how big each of our areas is, and where they start
	//TODO: For now, "NULL" is ok, since we're using a bitmapped font. But
	//      we'll need a valid DC (which hasn't been created yet) to pass in if we load
	//      user TTF Fonts. Possible solution: create a dummy window and just copy its DC?
	spaceWidth = mmFont->getStringWidth(_T(" "), NULL);
	firstLineStart = borderWidth;
	secondLineStart = firstLineStart + mmFont->getHeight(NULL) + spaceWidth + borderWidth;
	thirdLineStart = secondLineStart + mmFont->getHeight(NULL) + spaceWidth + borderWidth;
	fourthLineStart = thirdLineStart + (mmFont->getHeight(NULL)*8)/13 + borderWidth;
	int senWinHeight = mmFontSmall->getHeight(NULL) + borderWidth*3;

	//Now, set the windows' default heights
	mainWindow->setDefaultSize(mainWindow->getDefaultWidth(), fourthLineStart);
	sentenceWindow->setDefaultSize(sentenceWindow->getDefaultWidth(), senWinHeight);

	//Initialize the main window
	mainWindow->resizeWindow(mainWindow->getDefaultWidth(), mainWindow->getDefaultHeight(), false);
	mainWindow->createDoubleBufferedSurface();

	//Initialize the sentence window
	sentenceWindow->resizeWindow(sentenceWindow->getDefaultWidth(), sentenceWindow->getDefaultHeight(), false);
	sentenceWindow->createDoubleBufferedSurface();

	//Initialize the helper window
	helpWindow->resizeWindow(helpWindow->getDefaultWidth(), helpWindow->getDefaultHeight(), false);
	helpWindow->createDoubleBufferedSurface();

	//Initialize the memory window
	memoryWindow->resizeWindow(memoryWindow->getDefaultWidth(), memoryWindow->getDefaultHeight(), false);
	memoryWindow->createDoubleBufferedSurface();

	//Re-build the help keyboard
	initCalculateHelp();
}



/**
 * Re-figure the layout of our drawing area, resize if necessary, and
 * draw onto the back buffer. Finally, blit to the front buffer.
 */
void recalculate()
{
	//Convert the current input string to the internal encoding, and then convert it to the display encoding.
	//  We can short-circuit this if the output and display encodings are the same.
	bool noEncChange = (mmFont->encoding==currInput->encoding);
	std::wstring dispRomanStr = currInput->getTypedRomanString(false);
	if (!noEncChange) {
		input2Uni->convertInPlace(dispRomanStr);
		uni2Disp->convertInPlace(dispRomanStr);
	}

	//TODO: The typed sentence string might have a highlight, which changes things slightly.
	vector<wstring> dispSentenceStr;
	{
		vector<wstring> inputSentenceStr = currInput->getTypedSentenceStrings();
		for (vector<wstring>::iterator i=inputSentenceStr.begin(); i!=inputSentenceStr.end(); i++) {
			wstring candidate = *i;
			if (!noEncChange) {
				input2Uni->convertInPlace(candidate);
				uni2Disp->convertInPlace(candidate);
			}
			dispSentenceStr.push_back(candidate);
		}
	}

	//Candidate strings are slightly more complex; have the convert the entire array
	std::vector< std::pair<std::wstring, unsigned int> > dispCandidateStrs = currInput->getTypedCandidateStrings();
	if (!noEncChange) {
		for (size_t i=0; i<dispCandidateStrs.size(); i++) {
			if (!noEncChange) {
				input2Uni->convertInPlace(dispCandidateStrs[i].first);
				uni2Disp->convertInPlace(dispCandidateStrs[i].first);
			}
		}
	}

	//First things first: can we fit this in the current background?
	// (Includes pat-sint strings)
	int cumulativeWidth = (borderWidth+1)*2;
	for (size_t i=0; i<10; i++) {
		unsigned int id = i + currInput->getPagingInfo().first * 10;
		if (id>=dispCandidateStrs.size())
			break;
		cumulativeWidth += mainWindow->getStringWidth(mmFont, dispCandidateStrs[id].first);
		cumulativeWidth += spaceWidth;
	}

	//Add the up/down arrows, if we have too many candidates
	int triangleBaseWidth = pageImages[0]->getWidth() - 1;
	int triangleStartX = 0;
	int pagerWidth = 0;
	if (dispCandidateStrs.size()>10) {
		pagerWidth = 2*borderWidth + borderWidth + triangleBaseWidth;
		cumulativeWidth += pagerWidth;
	}

	//Consider a separate width for the "help string". We should probably eventually call this "third line width" or something...
	int helpStringWidth = 0;
	if (currInput->isHelpInput())
		helpStringWidth = (borderWidth+1)*2 + spaceWidth + mainWindow->getStringWidth(mmFontSmall, ROMAN_INPUT_PROMPT) + 1;

	//If not, resize. Also, keep the size small when possible.
	// Note: Re-sizing to the same size won't trigger a window update, so we can just all expandWindow()
	//       without worrying about performance.
	int newWidth = max(max(mainWindow->getDefaultWidth(), cumulativeWidth), helpStringWidth);
	if (dispCandidateStrs.size()>10)
		triangleStartX = newWidth - pagerWidth;
	mainWindow->expandWindow(newWidth, mainWindow->getClientHeight());

	//Background - Main Window
	mainWindow->selectObject(g_BlackPen);
	mainWindow->selectObject(g_DarkGrayBkgrd);
	mainWindow->drawRectangle(0, 0, mainWindow->getClientWidth(), mainWindow->getClientHeight());

	//Background - Sentence Window
	sentenceWindow->selectObject(g_BlackPen);
	sentenceWindow->selectObject(g_DarkGrayBkgrd);
	sentenceWindow->drawRectangle(0, 0, sentenceWindow->getClientWidth(), sentenceWindow->getClientHeight());

	//Draw each string, highlight the previous word if it's a pat-sint candidate.
	int currPosX = borderWidth + 1;
	mmFontSmall->setColor(0xFF, 0xFF, 0xFF);
	sentenceWindow->drawString(mmFontSmall, dispSentenceStr[0], currPosX, borderWidth+1);
	if (!dispSentenceStr[0].empty())
		currPosX += mainWindow->getStringWidth(mmFontSmall, dispSentenceStr[0]) + 1;
	mmFontSmall->setColor(0xFF, 0x00, 0x00);
	sentenceWindow->drawString(mmFontSmall, dispSentenceStr[1], currPosX, borderWidth+1);
	mmFontSmall->setColor(0xFF, 0xFF, 0xFF);
	if (!dispSentenceStr[1].empty())
		currPosX += mainWindow->getStringWidth(mmFontSmall, dispSentenceStr[1]) + 1;
	int cursorPosX = currPosX++;  //+1 for the cursor
	sentenceWindow->drawString(mmFontSmall, dispSentenceStr[2], currPosX, borderWidth+1);

	//Draw the cursor
	sentenceWindow->moveTo(cursorPosX-1, borderWidth+1);
	sentenceWindow->drawLineTo(cursorPosX-1, sentenceWindow->getClientHeight()-borderWidth-1);

	//Update the sentence window's clickable region for the current encoding rectangle
	wstring currEncStr = config.activeOutputEncoding.initial;
	int encStrWidth = mainWindow->getStringWidth(mmFontSmall, currEncStr);
	RECT rNew;
	rNew.left = sentenceWindow->getClientWidth()-encStrWidth-3;
	rNew.top = 0;
	rNew.right = sentenceWindow->getClientWidth();
	rNew.bottom = sentenceWindow->getClientHeight();
	sentenceWindow->updateRect(changeEncRegionHandle, rNew);

	//Draw the current encoding
	sentenceWindow->selectObject(g_BlackPen);
	sentenceWindow->selectObject(g_GreenBkgrd);
	sentenceWindow->drawRectangle(rNew.left, rNew.top, rNew.right, rNew.bottom);
	sentenceWindow->drawString(mmFontSmall, currEncStr, sentenceWindow->getClientWidth()-encStrWidth-2, sentenceWindow->getClientHeight()/2-mmFontSmall->getHeight(NULL)/2);

	//White overlays
	mainWindow->selectObject(g_EmptyPen);
	mainWindow->selectObject(g_WhiteBkgrd);
	mainWindow->drawRectangle(borderWidth+1, firstLineStart+1, mainWindow->getClientWidth()-borderWidth-pagerWidth-1, secondLineStart-borderWidth);
	mainWindow->drawRectangle(borderWidth+1, secondLineStart, mainWindow->getClientWidth()-borderWidth-pagerWidth-1, thirdLineStart-borderWidth);
	mainWindow->drawRectangle(borderWidth+1, thirdLineStart, mainWindow->getClientWidth()-borderWidth-pagerWidth-1, fourthLineStart-borderWidth-1);

	//Now, draw the strings....
	//PulpCoreFont* mmFont = mmFontBlack;
	int xOffset = 0;
	wstring extendedWordString;

	//Before we do this, draw the help text if applicable
	if (currInput->isHelpInput() && !dispCandidateStrs.empty() && !dispCandidateStrs[0].first.empty()) {
		mmFontSmall->setColor(0x33, 0x33, 0x33); //Gray
		mainWindow->drawString(mmFontSmall, ROMAN_INPUT_PROMPT, borderWidth+1+spaceWidth/2, thirdLineStart-spaceWidth/2);
		mmFontSmall->setColor(0xFF, 0xFF, 0xFF);
	}

	//Now, draw the candiate strings and their backgrounds
	int currLabelID = 1;
	for (size_t it=0; it<10; it++) {
		//Measure the string
		unsigned int id = it + (currInput->getPagingInfo().first * 10);
		if (id>=dispCandidateStrs.size())
			break;
		int thisStrWidth = mainWindow->getStringWidth(mmFont, dispCandidateStrs[id].first);

		//Select fonts, and draw a box under highlighted words
		//mmFont = mmFontBlack;
		if (dispCandidateStrs[id].second & HF_CURRSELECTION) {
			mmFont->setColor(0x00, 0x80, 0x00);
			mainWindow->selectObject(g_YellowBkgrd);
			mainWindow->selectObject(g_GreenPen);
			mainWindow->drawRectangle(borderWidth+xOffset+1, secondLineStart, borderWidth+1+xOffset+thisStrWidth+spaceWidth, secondLineStart+mmFont->getHeight(NULL)+spaceWidth-1);
		} else if (dispCandidateStrs[id].second & HF_PATSINT) {
			mmFont->setColor(0xFF, 0x00, 0x00);
		} else
			mmFont->setColor(0x00, 0x00, 0x00);

		//Draw the string (foreground)
		mainWindow->drawString(mmFont, dispCandidateStrs[id].first, borderWidth+1+spaceWidth/2 + xOffset, secondLineStart+spaceWidth/2);

		//Draw its numbered identifier, or '`' if it's a red-highlighted word
		if (!currInput->isHelpInput()) {
			std::wstringstream digit;
			if (dispCandidateStrs[id].second & HF_LABELTILDE) {
				digit <<L"`";
			} else {
				digit <<currLabelID++;
				if (currLabelID==10)
					currLabelID = 0; //Just renumber for now; we never have more than 10 anyway.
			}
			int digitWidth = mainWindow->getStringWidth(mmFont, digit.str());
			mainWindow->drawString(mmFont, digit.str(), borderWidth+1+spaceWidth/2 + xOffset + thisStrWidth/2 -digitWidth/2, thirdLineStart-spaceWidth/2-1);
		}

		//Increment
		xOffset += thisStrWidth + spaceWidth;
	}

	//Draw the current romanized string
	mmFont->setColor(0x00, 0x00, 0x00);
	mainWindow->drawString(mmFont, dispRomanStr, borderWidth+1+spaceWidth/2, firstLineStart+spaceWidth/2+1);

	//Draw the cursor page up/down box if required
	int pageDownStart = secondLineStart-borderWidth-2;
	int pageDownHalf = pageDownStart + (fourthLineStart-pageDownStart)/2;
	if (dispCandidateStrs.size()>10) {
		//Draw the background of the box.
		mainWindow->selectObject(g_EmptyPen);
		mainWindow->selectObject(g_BlackBkgrd);
		mainWindow->drawRectangle(triangleStartX-1, 0, triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth, fourthLineStart);
		mainWindow->selectObject(g_WhiteBkgrd);
		mainWindow->drawRectangle(triangleStartX-1+borderWidth, pageDownStart+borderWidth, triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2, fourthLineStart-borderWidth/2);

		//Draw the current page
		std::pair<int, int> pgInfo = currInput->getPagingInfo();
		std::wstringstream num;
		num <<pgInfo.first+1;
		int strWidth = mainWindow->getStringWidth(mmFontSmall, num.str());
		mainWindow->drawString(mmFontSmall, num.str(), triangleStartX-1 + (triangleBaseWidth+borderWidth*2+borderWidth)/2 - strWidth/2, pageDownStart-borderWidth-mmFontSmall->getHeight(NULL) + 6);

		//Draw a separator line for the box, half-shaded.
		//Black center line
		mainWindow->selectObject(g_BlackPen);
		mainWindow->moveTo(triangleStartX-1+borderWidth+2, pageDownHalf);
		mainWindow->drawLineTo(triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2-3, pageDownHalf);
		//Gray fringes
		mainWindow->selectObject(g_MediumGrayPen);
		mainWindow->moveTo(triangleStartX-1+borderWidth+1, pageDownHalf);
		mainWindow->drawLineTo(triangleStartX-1+borderWidth+2, pageDownHalf);
		mainWindow->moveTo(triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2-3, pageDownHalf);
		mainWindow->drawLineTo(triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2-2, pageDownHalf);
		//Gray top/bottom lines
		mainWindow->moveTo(triangleStartX-1+borderWidth+2, pageDownHalf-1);
		mainWindow->drawLineTo(triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2-3, pageDownHalf-1);
		mainWindow->moveTo(triangleStartX-1+borderWidth+2, pageDownHalf+1);
		mainWindow->drawLineTo(triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2-3, pageDownHalf+1);

		//Draw the triangles which specify whether or not we have more entries above/below.
		int availWidth = (triangleStartX+triangleBaseWidth+borderWidth*2+borderWidth-borderWidth/2) - (triangleStartX-1+borderWidth);
		int availHeight = (pageDownHalf-1) - (pageDownStart+borderWidth);
		POINT arrowPts[2] = {
			{triangleStartX-1+borderWidth + (availWidth-pageImages[0]->getWidth())/2-1 + 1, pageDownStart+borderWidth + (availHeight-pageImages[0]->getHeight()) - 1},
			{triangleStartX-1+borderWidth + (availWidth-pageImages[0]->getWidth())/2-1 + 1, pageDownHalf+1 + (availHeight-pageImages[0]->getHeight()) - 1}
		};
		for (int i=0; i<2; i++) {
			PulpCoreImage* pgImg = i==0
				?(pgInfo.first==0?pageImages[PGUPSEPIA_ID]: pageImages[PGUPCOLOR_ID])
				:(pgInfo.first>=pgInfo.second-1?pageImages[PGDOWNSEPIA_ID]: pageImages[PGDOWNCOLOR_ID]);
			mainWindow->drawImage(pgImg, arrowPts[i].x, arrowPts[i].y);
		}
	}

	//Paint it all to the screen
	reBlit();
}




void typeCurrentPhrase(const wstring& stringToType)
{
	//Send key presses to the top-level program.
	HWND fore = GetForegroundWindow();
	SetActiveWindow(fore); //This probably won't do anything, since we're not attached to this window's message queue.

	//Convert to the right encoding
	bool noEncChange = (uni2Output->toEncoding==currInput->encoding);
	wstring keyStrokes = stringToType;
	if (!noEncChange) {
		input2Uni->convertInPlace(keyStrokes);
		uni2Output->convertInPlace(keyStrokes);
	}


	//Use SendInput instead of SendMessage, since SendMessage requires the actual
	//  sub-window (component) to recieve the message, whereas SendInput only
	//  requires the top-level window. We could probably hack in SendMessage now that
	//  we're not becoming the active window, but for now I'd rather have a stable
	//  system than one that works on Windows 98.
	//Buffer each key-stroke
	size_t number_of_key_events = 0;
	for (size_t i=0; i<keyStrokes.size(); i++) {
		//Send keydown
		keyInputPrototype.wScan = (WORD)keyStrokes[i];
		keyInputPrototype.dwFlags = KEYEVENTF_UNICODE;
		inputItems[number_of_key_events++].ki = keyInputPrototype;

		keyInputPrototype.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;
		inputItems[number_of_key_events++].ki = keyInputPrototype;
	}


	//Send all the keystrokes at once to avoid a weird bug with single-letter repetitions.
	UINT numSent = SendInput(number_of_key_events,inputItems,sizeof(INPUT));
	if(numSent!=number_of_key_events || number_of_key_events==0) {
		wstringstream msg;
		msg <<"Couldn't send input, only sent " <<numSent <<" of " <<number_of_key_events <<" events. Error code: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_OK|MB_ICONERROR);
	}



	//Now, reset...
	currInput->reset(true, true, true, true); //TODO: Is this necessary?


	//Technically, this can be called with JUST a stopChar, which implies
	//  that the window isn't visible. So check this.
	if (controlKeysOn) {
		//Turn off control keys
		turnOnControlkeys(false);

		//Hide the window(s)
		mainWindow->showWindow(false);
		sentenceWindow->showWindow(false);
	}
}




//Compute x and y co-ordinates based on some criteria.
//Note: hPlus is the amount to increase height by
void LayoutDialogControls(vector<WControl>& pendingItems, HDC currDC, size_t startX, size_t startY, size_t fHeight)
{
	size_t accX = startX;
	size_t accY = startY;
	for (vector<WControl>::iterator it=pendingItems.begin(); it!=pendingItems.end(); it++) {
		//Set x/y, update accX/accY (artificial)
		accX = it->x = (it->x==0 ? accX : it->x);
		accY = it->y = (it->y==0 ? accY : it->y);
		accY += it->hPlus;
		it->y += it->yPlus; //yPlus doesn't affect the Y iterator.

		//Set w/h, update accX/accY (natural)
		// Don't update for empty controls.
		if (it->type!=L"COMBOBOX" && it->type!=L"SysTabControl32" && !it->text.empty()) {
			it->w = LOWORD(GetTabbedTextExtent(currDC, it->text.c_str(), it->text.length(), 0, NULL));
			it->h = (waitzar::count_letter(it->text, L'\n')+1)*fHeight;
			accX += it->w;
		}
		//accY += it->h;
	}
}

void CreateDialogControls(vector<WControl>& pendingItems, HWND hwnd, HFONT dlgFont)
{
	unsigned int flags = 0;
	for (vector<WControl>::iterator it=pendingItems.begin(); it!=pendingItems.end(); it++) {
		//Set flags
		unsigned int visFlag = !it->hidden ? WS_VISIBLE : 0;
		if (it->type==L"STATIC") {//Optionally SS_ICON to auto-resize.
			unsigned int flags3 = (it->iconID==IDC_SETTINGS_FAKEICONID?(SS_BITMAP|SS_NOTIFY):it->iconID==OBM_RGARROW?SS_BITMAP:SS_ICON); //TODO: Fix hack. x2
			flags = WS_CHILD | visFlag | (it->iconID!=0 ? flags3 : 0) | (it->id==IDC_SETTINGS_HELPPNLBORDER?WS_BORDER:0); //TODO: Fix hacks here too...
		} else if (it->type==L"BUTTON") {
			flags = WS_CHILD | visFlag;
			if (it->ownerDrawnBtn)
				flags |= BS_OWNERDRAW;
			else
				flags |= WS_BORDER | (it->id==IDOK ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
		} else if (it->type==L"COMBOBOX")
			flags = WS_CHILD|visFlag|WS_TABSTOP|CBS_DROPDOWNLIST;
		else if (it->type==L"SysTabControl32")
			flags = WS_CHILD | visFlag;
		else
			throw std::exception(waitzar::glue(L"Unknown control type: ", it->type).c_str());

		//Create the control, set the font
		HWND ctl = CreateWindow(it->type.c_str(), it->text.c_str(), flags,
				 it->x, it->y, it->w, it->h,
                 hwnd, NULL, hInst, NULL );
		SendMessage(ctl, WM_SETFONT, (WPARAM)dlgFont, MAKELPARAM(FALSE, 0));
		SetWindowLongPtr(ctl, GWL_ID, it->id);
		if (it->iconID!=0 && it->iconID!=IDC_SETTINGS_FAKEICONID) {
			//Send an update_icon method. I tried SS_ICON before, but it didn't work.
			bool isSysIcon = it->iconID==OIC_QUES||it->iconID==OBM_RGARROW;  //TODO: Add more later.
			//LPCTSTR iconName = it->iconID==OIC_QUES ? IDI_QUESTION : MAKEINTRESOURCE(it->iconID);
			unsigned int loadFlags = (/*it->blWh ? LR_MONOCHROME :*/ LR_DEFAULTCOLOR) | (isSysIcon ? LR_SHARED : 0);
			unsigned int flags2 = (it->iconID==IDI_HELPICON_COLOR||it->iconID==OBM_RGARROW?IMAGE_BITMAP:IMAGE_ICON); //TODO: Fix hack.

			//TEMP
			if (it->iconID==OBM_RGARROW)
				it->iconID=OBM_ZOOM;
			//END TEMP


			HANDLE hicon = (HANDLE)LoadImage((isSysIcon?NULL:hInst), MAKEINTRESOURCE(it->iconID), flags2, it->w, it->h, loadFlags);
			if (it->blWh) {
				HANDLE hiconOld = hicon;
				hicon = CreateGrayscaleBitmap((HBITMAP)hiconOld);
				DeleteObject(hiconOld);
			}
			unsigned int errCode = GetLastError();
			SendMessage(ctl, STM_SETIMAGE, (WPARAM)flags2, (LPARAM)hicon);

		}

		//Turn it into a hyperlink?
		if (it->convertToHyperlink && it->type==L"STATIC")
			ConvertStaticToHyperlink(hwnd, it->id);
		//UpdateWindow(ctl); //Might not be needed, but keep here for now.
	}
}


//Message handling for our about/help dialog box
BOOL CALLBACK HelpDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_INITDIALOG:
		{
			//Resize the help dialog; keep it at the same position.
			//Also, set its caption.
			RECT wndR;
			size_t wndWidth = 473;
			size_t wndHeight = 262;
			GetWindowRect(hwnd, &wndR);
			MoveWindow(hwnd, wndR.left, wndR.top, wndWidth, wndHeight, TRUE);
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)L"About");


			//Get our dialog's default font, since the DS_SETFONT style doesn't apply outside the
			//  resource editor. Load it into the DC, so that we can measure text.
			//HFONT hFont = GetWindowFont(hwnd);
			HFONT dlgFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
			HDC currDC = GetDC(hwnd);
			SelectObject(currDC, dlgFont);

			//General incremental values
			const size_t fHeight = HIWORD(GetTabbedTextExtent(currDC, L"[Wp]", 4, 0, NULL));
			const size_t buff = 3;


			////////////////////////////////////////////////
			// Step 1: Popuplate a vector of structs
			//         with window IDs, positioning, labels.
			////////////////////////////////////////////////
			vector<WControl> pendingItems;

			//Determine our new client area, set our starting Y co-ordinate.
			RECT txtR;
			txtR.left = fHeight*3 + 20 + fHeight/2 - fHeight%2; //The icon is roughly 20px
			txtR.top = fHeight*2 + buff;
			size_t bkgrdH = 21 + (fHeight-1)*2;
			GetClientRect(hwnd, &wndR);
			txtR.right = wndR.right-txtR.left - fHeight*2 - fHeight/2;
			txtR.bottom = wndR.bottom-txtR.top - fHeight - bkgrdH;

			//For each of these controls, if x or y is set, keep that value.
			// Else, keep adding components to the right (x + width of prev. component).
			//If h is set, add that to y after adding the component.
			{
				//Line 1
				wstringstream txtS;
				txtS << "WaitZar version " <<WZ_VERSION_FULL <<" - for the latest news, visit ";
				pendingItems.push_back(WControl(IDC_HELP_L1, txtS.str(), L"STATIC", false, txtR.left));
				pendingItems.push_back(WControl(IDC_HELP_H1, L"WaitZar.com", L"STATIC", true));
				pendingItems[pendingItems.size()-1].hPlus = fHeight*2;

				//Line 2
				txtS.str(L"");
				txtS <<hkString <<" - Switch between Myanmar and English\nType Burmese words like they sound, and press \"space\".";
				pendingItems.push_back(WControl(IDC_HELP_L2, txtS.str(), L"STATIC", false, txtR.left));
				pendingItems[pendingItems.size()-1].hPlus = fHeight*3;

				//Line 3
				pendingItems.push_back(WControl(IDC_HELP_L4, L"WaitZar users should have the relevant fonts installed, if they want to see \nwhat they type after it's chosen.", L"STATIC", false, txtR.left));
				pendingItems[pendingItems.size()-1].hPlus = fHeight*2;

				//Line 4
				pendingItems.push_back(WControl(IDC_HELP_L5A, L"Feel free to ", L"STATIC", false, txtR.left));
				pendingItems.push_back(WControl(IDC_HELP_H5, L"contact us", L"STATIC", true));
				pendingItems.push_back(WControl(IDC_HELP_L5B, L" if you have any questions.", L"STATIC"));
				pendingItems[pendingItems.size()-1].hPlus = fHeight*2;

				//Line 5
				pendingItems.push_back(WControl(IDC_HELP_L6, L"For more information, please read the ", L"STATIC", false, txtR.left));
				pendingItems.push_back(WControl(IDC_HELP_H6, L"WaitZar User's Guide", L"STATIC", true));
			}

			//Now adjust
			LayoutDialogControls(pendingItems, currDC, txtR.left, txtR.top, fHeight);


			//A few more controls that don't fit this pattern.
			//Ok button, background label, icon
			pendingItems.push_back(WControl(IDC_HELP_BKGRD, L"", L"STATIC", false, 0, wndR.bottom-bkgrdH, wndWidth, bkgrdH));
			pendingItems.push_back(WControl(IDC_HELP_ICON, L"", L"STATIC", false, fHeight*2, fHeight*2));
			pendingItems[pendingItems.size()-1].iconID = IDI_WAITZAR;
			size_t w = 75;
			size_t h = 23;
			pendingItems.push_back(WControl(IDOK, L"&Ok", L"BUTTON", false, wndR.right-fHeight*2-11-w, wndR.bottom+1-bkgrdH+h/2, w, h));


			//DC's no longer needed.
			ReleaseDC(hwnd, currDC);


			//Start adding controls automatically.
			CreateDialogControls(pendingItems, hwnd, dlgFont);

			return TRUE;
		}
		case WM_CTLCOLORDLG:
			return (BOOL)g_DlgHelpBkgrd;
		case WM_CTLCOLORSTATIC:
		{
			int ctlID = GetDlgCtrlID((HWND)lParam);
			if (ctlID==IDC_HELP_BKGRD) {
				//Set the background color of our static item
				return (INT_PTR)CreateSolidBrush(RGB(0xBB, 0xFF, 0xCC));
				//return (BOOL)g_DlgHelpSlash; //The system deletes the brush automatically.
			}
			if (ctlID==IDC_HELP_H1 || ctlID==IDC_HELP_H5 || ctlID==IDC_HELP_H6) {
				//Make it blue
				SetTextColor((HDC)wParam, RGB(255, 0, 0));
			}

			//Transparent? Ugh.
			SetBkColor((HDC)wParam, RGB(0xEE, 0xFF, 0xEE));
			return (INT_PTR)CreateSolidBrush(RGB(0xEE, 0xFF, 0xEE));
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
				case IDC_HELP_H1:
					//Load the WaitZar web site.
					ShellExecute(hwnd, L"open", L"http://www.waitzar.com", NULL, NULL, SW_SHOWNORMAL);
					EndDialog(hwnd, IDOK);
					break;
				case IDC_HELP_H5:
					//Load the feedback form
					ShellExecute(hwnd, L"open", L"http://www.waitzar.com/contactus.py", NULL, NULL, SW_SHOWNORMAL);
					EndDialog(hwnd, IDOK);
					break;
				case IDC_HELP_H6:
					//Slightly more complex: try to load the User's Guide locally if it exists; if not,
					// then simply open the user's browser to the latest guide.
					int retVal = (int)ShellExecute(hwnd, L"open", L"WaitZar User's Guide.doc", NULL, NULL, SW_SHOWNORMAL);
					if (retVal<=32) {
						ShellExecute(hwnd, L"open", L"http://waitzar.googlecode.com/svn/trunk/WaitZar%20User%27s%20Guide.doc", NULL, NULL, SW_SHOWNORMAL);
					}
					EndDialog(hwnd, IDOK);
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hwnd, IDCANCEL);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}


//Cached
HBITMAP helpIconColor = NULL;
HBITMAP helpIconGray = NULL;
vector<unsigned int> helpIconDlgIDs;
unsigned int lastHelpDlgID = 0;


//Also used for the settings dialog
vector<wstring> settingsLangIDs;
void UpdateSettingsTab(HWND dlgHwnd, int tabID)
{
	//Remove all entries.
	HWND ctlA = GetDlgItem(dlgHwnd, IDC_SETTINGS_IMCOMBO);
	HWND ctlB = GetDlgItem(dlgHwnd, IDC_SETTINGS_OUTCOMBO);
	SendMessage(ctlA, CB_RESETCONTENT, 0, 0);
	SendMessage(ctlB, CB_RESETCONTENT, 0, 0);

	//Invalid language?
	if (tabID==-1)
		return;

	//Get the language for this panel.
	const wstring langID = settingsLangIDs[tabID];
	const Language& lng = *FindKeyInSet(config.getLanguages(), langID);

	//Add all combo item entries.
	SendMessage(ctlA, CB_ADDSTRING, 0, (LPARAM)L"N/A");
	SendMessage(ctlB, CB_ADDSTRING, 0, (LPARAM)L"N/A");
	SendMessage(ctlA, CB_ADDSTRING, 0, (LPARAM)L"(Auto)");
	SendMessage(ctlB, CB_ADDSTRING, 0, (LPARAM)L"(Auto)");
	for (std::set<InputMethod*>::const_iterator it2=lng.inputMethods.begin(); it2!=lng.inputMethods.end(); it2++) {
		SendMessage(ctlA, CB_ADDSTRING, 0, (LPARAM)(*it2)->displayName.c_str());
	}
	for (std::set<Encoding>::const_iterator it2=lng.encodings.begin(); it2!=lng.encodings.end(); it2++) {
		if (it2->canUseAsOutput)
			SendMessage(ctlB, CB_ADDSTRING, 0, (LPARAM)it2->displayName.c_str());
	}
	SendMessage(ctlA, CB_SETCURSEL, 0, 0); //Choose the first item, for now.
	SendMessage(ctlB, CB_SETCURSEL, 0, 0); //Choose the first item, for now.
}


vector< pair<unsigned int, POINT> > controlsToMove; //"ID", "initialX", where initialX of 0 means "show/hide"
vector< pair<wstring, wstring> > helpIconDlgTextStrings;
bool helpBoxIsVisible = false;
void MakeHelpBoxVisible(HWND dlgHwnd, bool show, unsigned int helpIconID, pair<size_t, size_t> wndWidthMinPlus)
{
	//Always update the help icons' colors
	pair<wstring, wstring> panelText;
	for (size_t i=0; i<helpIconDlgIDs.size(); i++) {
		unsigned int ctlID = helpIconDlgIDs[i];
		bool matchCtl = (ctlID==helpIconID);
		HWND ctl = GetDlgItem(dlgHwnd, ctlID);
		SendMessage(ctl, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM) ((matchCtl&&show)?helpIconColor:helpIconGray));
		if (matchCtl && show) {
			panelText = helpIconDlgTextStrings[i];
		}
	}
	//Update the help icon text (& title)
	if (show) {
		//NOTE: We need to update all windows in z-order, lest they occlude the others.
		HWND ctl = GetDlgItem(dlgHwnd, IDC_SETTINGS_HELPPNLTXT);
		SetWindowText(ctl, panelText.second.c_str());
		ctl = GetDlgItem(dlgHwnd, IDC_SETTINGS_HELPTPNLTITLETXT);
		SetWindowText(ctl, panelText.first.c_str());
		//ctl = GetDlgItem(dlgHwnd, IDC_SETTINGS_HELPCLOSEBTN);
		//SetWindowText(ctl, L"X"); //We need to set this, or it will be overshadowed by the
	}

	//Don't do anything else if we're not performing a full change.
	if (show==helpBoxIsVisible)
		return;

	//Resize our dialog box.
	RECT wndR;
	GetWindowRect(dlgHwnd, &wndR);
	int wndOffset = show?wndWidthMinPlus.second:0;
	int wndNewX = wndR.left + (show?-1:1)*wndWidthMinPlus.second;
	int wndNewWidth = wndWidthMinPlus.first + wndOffset;
	if (show)
		MoveWindow(dlgHwnd, wndNewX, wndR.top, wndNewWidth, wndR.bottom-wndR.top, TRUE);

	//Move our components
	for (size_t i=0; i<controlsToMove.size(); i++) {
		HWND ctl = GetDlgItem(dlgHwnd, controlsToMove[i].first);
		long initialX = controlsToMove[i].second.x;
		long initialY = controlsToMove[i].second.y;
		if (initialX==0 && initialY==0) {
			//if (show)
			//	UpdateWindow(ctl);
			ShowWindow(ctl, show?SW_SHOW:SW_HIDE);
		} else {
			SetWindowPos(ctl, NULL, initialX+wndOffset, initialY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		}
	}

	if (!show)
		MoveWindow(dlgHwnd, wndNewX, wndR.top, wndNewWidth, wndR.bottom-wndR.top, TRUE);

	//Done
	helpBoxIsVisible = show;
}


//Message handling for our settings dialog box
BOOL CALLBACK SettingsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Useful window size constants
	const size_t wndWidth = 320;
	const size_t wndHeight = 320;
	const size_t wndHelpAreaWidth = 180;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			//Init cached help icons
			const size_t iconSz = 16;
			if (helpIconColor==NULL)
				helpIconColor = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDI_HELPICON_COLOR), IMAGE_BITMAP, iconSz, iconSz, LR_DEFAULTCOLOR);
			if (helpIconGray==NULL)
				helpIconGray = CreateGrayscaleBitmap(helpIconColor);
			if (helpIconColor==NULL || helpIconGray==NULL)
				throw std::exception("Couldn't load color/grayscale help icon from memory");
			helpBoxIsVisible = false;

			//Save strings
			if (helpIconDlgTextStrings.empty()) {
				helpIconDlgTextStrings.push_back(pair<wstring, wstring>(L"Language Hotkey", L"The key combination that switches between English and Burmese typing.\n\nWhen in English mode, this is the only hotkey registered with Windows, so you should generally choose an uncomon key sequence to avoid accidentally activating WaitZar.\n\nDefaults to: Ctrl+Shift"));
				helpIconDlgTextStrings.push_back(pair<wstring, wstring>(L"Default Language", L"The language which WaitZar starts in.\n\nSet to \"N/A\" for the default. Set to \"Auto\" to start in the language you last typed in.\n\nDefaults to: Myanmar"));
				helpIconDlgTextStrings.push_back(pair<wstring, wstring>(L"Input Method", L"The input method WaitZar activates initially.\n\nYou should set this to your favorite Roman (WaitZar, Burglish) or Keyboard (Zawgyi-One, myWin) input method.\n\nDefault values:\n  Myanmar: WaitZar\n  Shan: Test Keyboard"));
				helpIconDlgTextStrings.push_back(pair<wstring, wstring>(L"Output Encoding", L"The output encoding that WaitZar activates by default. (Encodings are called \"fonts\" on some web sites.)\n\nIt is recommended to keep this at its default value.\n\nDefaults to: Unicode"));
			}

			//Resize the settings dialog; keep it at the same position.
			//Also, set its caption.
			RECT wndR;
			GetWindowRect(hwnd, &wndR);
			MoveWindow(hwnd, std::max<long>(wndR.left, wndHelpAreaWidth), wndR.top, wndWidth, wndHeight, TRUE);
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)L"WaitZar's Settings");
			//EnableThemeDialogTexture(hwnd, ETDT_ENABLE);
			GetClientRect(hwnd, &wndR);

			//Get dialog font, the myanmar dialog font
			HFONT dlgFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
			HFONT mmdlgFont = menuFont->getInternalHFont();

			//Get font stats
			HDC currDC = GetDC(hwnd);
			SelectObject(currDC, dlgFont);
			const size_t fHeight = HIWORD(GetTabbedTextExtent(currDC, L"[Wp]", 4, 0, NULL));
			const size_t fHeight2 = fHeight + 5;
			const size_t bkgrdH = 21 + (fHeight-1)*2;

			//Make a bold/underlined version of the dialog font.
			LOGFONT lf;
			if (GetObject(dlgFont, sizeof(lf), &lf)==0)
				throw std::exception("Could not retrieve handle to dialog font.");
			lf.lfWeight = FW_BOLD;
			lf.lfUnderline = TRUE;
			HFONT dlgFontBold = CreateFontIndirect(&lf);


			//Create elements and store in a vector.
			// We won't use any of the auto-layout features of LayoutDialogControls. (Maybe...)
			// However, we will probably use auto-sizing of static elements.
			vector<WControl> pendingItems;
			const size_t lblWidth = 140;
			const size_t comboWidth = 110;
			const size_t comboShortWidth = 80;
			const size_t comboShortWidth2 = 60;
			const size_t comboHeight = 150;
			const size_t cHPlus = 22;


			//"Settings" and the language hotkey controls
			size_t lbl1VecID = pendingItems.size();
			pendingItems.push_back(WControl(IDC_SETTINGS_SETOPTLBL, L"Settings:", L"STATIC", false, 15, 15));
			pendingItems[pendingItems.size()-1].hPlus = fHeight2;
			pendingItems.push_back(WControl(IDC_SETTINGS_HKHELP, L"", L"STATIC", false, 15, 0, iconSz, iconSz));
			pendingItems[pendingItems.size()-1].iconID = IDC_SETTINGS_FAKEICONID;
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus - iconSz)/2 - 1;
			pendingItems[pendingItems.size()-1].blWh = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_HKLBL, L"Language Hotkey", L"STATIC", false, 37));
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus-fHeight) - 5;
			pendingItems.push_back(WControl(IDC_SETTINGS_HKCOMBO1, L"", L"COMBOBOX", false, lblWidth, 0, comboShortWidth, comboHeight));
			pendingItems.push_back(WControl(IDC_SETTINGS_HKPLUSLBL, L" + ", L"STATIC", false, lblWidth+comboShortWidth));
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus-fHeight) - 5;
			pendingItems.push_back(WControl(IDC_SETTINGS_HKCOMBO2, L"", L"COMBOBOX", false, 0, 0, comboShortWidth2, comboHeight));
			pendingItems[pendingItems.size()-1].hPlus = cHPlus;
			pendingItems.push_back(WControl(IDC_SETTINGS_LANGHELP, L"", L"STATIC", false, 15, 0, iconSz, iconSz));
			pendingItems[pendingItems.size()-1].iconID = IDC_SETTINGS_FAKEICONID;
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus - iconSz)/2 - 1;
			pendingItems.push_back(WControl(IDC_SETTINGS_LANGLBL, L"Default Language", L"STATIC", false, 37));
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus-fHeight) - 5;
			pendingItems.push_back(WControl(IDC_SETTINGS_LANGCOMBO, L"", L"COMBOBOX", false, lblWidth, 0, comboWidth, comboHeight));
			pendingItems[pendingItems.size()-1].hPlus = cHPlus + fHeight*2;


			//"Language Options" and the various tabbed items
			size_t lbl2VecID = pendingItems.size();
			pendingItems.push_back(WControl(IDC_SETTINGS_LANGOPTLBL, L"Language Options:", L"STATIC", false, 15));
			pendingItems[pendingItems.size()-1].hPlus = fHeight2;
			size_t tabCtlVecID = pendingItems.size();
			pendingItems.push_back(WControl(IDC_SETTINGS_MAINTAB, L"", L"SysTabControl32", false, 15)); //We'll have to fix w/h later.
			pendingItems[pendingItems.size()-1].hPlus = fHeight2*2;
			pendingItems.push_back(WControl(IDC_SETTINGS_IMHELP, L"", L"STATIC", false, 15+6, 0, iconSz, iconSz));
			pendingItems[pendingItems.size()-1].iconID = IDC_SETTINGS_FAKEICONID;
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus - iconSz)/2 - 1;
			pendingItems[pendingItems.size()-1].blWh = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_IMLBL, L"Default Input Method", L"STATIC", false, 37+6));
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus-fHeight) - 5;
			pendingItems.push_back(WControl(IDC_SETTINGS_IMCOMBO, L"", L"COMBOBOX", false, lblWidth+6+20, 0, comboWidth, comboHeight));
			pendingItems[pendingItems.size()-1].hPlus = cHPlus;
			pendingItems.push_back(WControl(IDC_SETTINGS_OUTENCHELP, L"", L"STATIC", false, 15+6, 0, iconSz, iconSz));
			pendingItems[pendingItems.size()-1].iconID = IDC_SETTINGS_FAKEICONID;
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus - iconSz)/2 - 1;
			pendingItems[pendingItems.size()-1].blWh = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_OUTLBL, L"Default Output Encoding", L"STATIC", false, 37+6));
			pendingItems[pendingItems.size()-1].yPlus = (cHPlus-fHeight) - 5;
			pendingItems.push_back(WControl(IDC_SETTINGS_OUTCOMBO, L"", L"COMBOBOX", false, lblWidth+6+20, 0, comboWidth, comboHeight));

			//Save a few ID values
			helpIconDlgIDs.push_back(IDC_SETTINGS_HKHELP);
			helpIconDlgIDs.push_back(IDC_SETTINGS_LANGHELP);
			helpIconDlgIDs.push_back(IDC_SETTINGS_IMHELP);
			helpIconDlgIDs.push_back(IDC_SETTINGS_OUTENCHELP);

			//Layout
			LayoutDialogControls(pendingItems, currDC, 15, 15, fHeight);

			//Fix the widths of the bold/underline labels
			SelectObject(currDC, dlgFontBold);
			pendingItems[lbl1VecID].w = LOWORD(GetTabbedTextExtent(currDC, pendingItems[lbl1VecID].text.c_str(), pendingItems[lbl1VecID].text.length(), 0, NULL)) + 1;
			pendingItems[lbl2VecID].w = LOWORD(GetTabbedTextExtent(currDC, pendingItems[lbl2VecID].text.c_str(), pendingItems[lbl2VecID].text.length(), 0, NULL)) + 1;
			SelectObject(currDC, dlgFont);

			//Fix icon width/heights
			//pendingItems[lblIcon1].w = pendingItems[lblIcon1].h = 16;
			//pendingItems[lblIcon2].w = pendingItems[lblIcon2].h = 16;

			//Re-size the tab control based on tabCtlVecID
			pendingItems[tabCtlVecID].w = wndWidth - 40;
			pendingItems[tabCtlVecID].h = wndHeight - pendingItems[tabCtlVecID].y - bkgrdH - 40;

			//More x/y
			//pendingItems[lblIcon1].y += (cHPlus-pendingItems[lblIcon1].h)/2 - 1;
			//pendingItems[lblIcon2].y += (cHPlus-pendingItems[lblIcon2].h)/2 - 1;
			//pendingItems[lblSet1VecID].y += (cHPlus-fHeight) - 5;
			//pendingItems[lblSet2VecID].y += (cHPlus-fHeight) - 5;

			//Some elements that don't fit so easily into our metric:
			//  Static background, ok button, cancel button
			size_t w = 75;
			size_t h = 23;
			pendingItems.push_back(WControl(IDC_SETTINGS_BKGRD, L"", L"STATIC", false, 0, wndR.bottom-bkgrdH, wndWidth+wndHelpAreaWidth, bkgrdH));
			pendingItems.push_back(WControl(IDOK, L"&Ok", L"BUTTON", false, wndR.right-fHeight*2-11-w - fHeight/2 - w, wndR.bottom+1-bkgrdH+h/2, w, h));
			pendingItems.push_back(WControl(IDCANCEL, L"&Cancel", L"BUTTON", false, wndR.right-fHeight*2-11-w, wndR.bottom+1-bkgrdH+h/2, w, h));
			pendingItems.push_back(WControl(IDC_SETTINGS_HELPPNLBORDER, L"", L"STATIC", false, 15, 15, wndHelpAreaWidth-15, wndR.bottom-wndR.top-bkgrdH-30+3));
			pendingItems[pendingItems.size()-1].hidden = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_HELPPNLTXT, L"Body Txt", L"STATIC", false, 15+5, 15+24+5, wndHelpAreaWidth-15-10, wndR.bottom-wndR.top-bkgrdH-30+3-10-24));
			pendingItems[pendingItems.size()-1].hidden = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_HELPTPNLTITLETXT, L"Title Text", L"STATIC", false, 15+fHeight/2, 15+fHeight/2-2, wndHelpAreaWidth-24-15-2-6, fHeight+4));
			pendingItems[pendingItems.size()-1].hidden = true;
			pendingItems.push_back(WControl(IDC_SETTINGS_HELPCLOSEBTN, L"X", L"BUTTON", false, wndHelpAreaWidth-24, 15, 24, 24));
			pendingItems[pendingItems.size()-1].hidden = true;
			pendingItems[pendingItems.size()-1].ownerDrawnBtn = true;
			//pendingItems.push_back(WControl(IDC_SETTINGS_GENERICARROW, L"", L"STATIC", false, 30, 30, 16, 16));
			//pendingItems[pendingItems.size()-1].iconID = OBM_RGARROW;

			//DC's no longer needed
			ReleaseDC(hwnd, currDC);

			//Save items and their starting locations
			for (vector<WControl>::iterator it=pendingItems.begin(); it!=pendingItems.end(); it++) {
				if (it->id==IDC_SETTINGS_BKGRD)
					continue;

				POINT pt = {it->x, it->y};
				if (it->id==IDC_SETTINGS_HELPPNLBORDER || it->id==IDC_SETTINGS_HELPPNLTXT || it->id==IDC_SETTINGS_HELPCLOSEBTN || it->id==IDC_SETTINGS_HELPTPNLTITLETXT)
					pt.x = pt.y = 0;
				controlsToMove.push_back(pair<unsigned int, POINT>(it->id, pt));
			}

			//Create items
			CreateDialogControls(pendingItems, hwnd, dlgFont);

			//Set fonts for the main labels & tab dialog
			HWND hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_SETOPTLBL);
			SendMessage(hwLbl, WM_SETFONT, (WPARAM)dlgFontBold, MAKELPARAM(FALSE, 0));
			hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_LANGOPTLBL);
			SendMessage(hwLbl, WM_SETFONT, (WPARAM)dlgFontBold, MAKELPARAM(FALSE, 0));
			hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_MAINTAB);
			SendMessage(hwLbl, WM_SETFONT, (WPARAM)mmdlgFont, MAKELPARAM(FALSE, 0));
			hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_HELPTPNLTITLETXT);
			SendMessage(hwLbl, WM_SETFONT, (WPARAM)mmdlgFont, MAKELPARAM(FALSE, 0)); //Looks nice.
			hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_LANGCOMBO);
			SendMessage(hwLbl, WM_SETFONT, (WPARAM)mmdlgFont, MAKELPARAM(FALSE, 0)); //Needed for names.

			//Set "bold" font for close label
			//hwLbl = GetDlgItem(hwnd, IDC_SETTINGS_HELPPNL);
			//SendMessage(hwLbl, WM_SETFONT, (WPARAM)dlgFontBold, MAKELPARAM(FALSE, 0));

			//For each language, add a panel in the tab control
			TCITEM tci;
			tci.mask = TCIF_TEXT;
			size_t defTabID = 0;
			settingsLangIDs.clear();
			HWND hwTabMain = GetDlgItem(hwnd, IDC_SETTINGS_MAINTAB);
			for (std::set<Language>::const_iterator it=config.getLanguages().begin(); it!=config.getLanguages().end(); it++) {
				wchar_t name[512];
				wcscpy(name, it->displayName.c_str());
				tci.pszText = name;
				int tabPageID = TabCtrl_InsertItem(hwTabMain, TabCtrl_GetItemCount(hwTabMain), &tci);
				if (it->id == config.activeLanguage.id)
					defTabID = tabPageID;

				//Save its id, in case the Set somehow re-orders them.
				settingsLangIDs.push_back(it->id);
			}

			//Set color/grayscale Help icons
			for (vector<unsigned int>::iterator it=helpIconDlgIDs.begin(); it!=helpIconDlgIDs.end(); it++) {
				HWND ctl = GetDlgItem(hwnd, *it);
				SendMessage(ctl, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)helpIconGray);
			}

			//Add a few...
			HWND ctl = GetDlgItem(hwnd, IDC_SETTINGS_HKCOMBO1);
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"(None)");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Ctrl");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Alt");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Shift");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Ctrl+Shift");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Ctrl+Alt");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Alt+Shift");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Ctrl+Alt+Shift");
			SendMessage(ctl, CB_SETCURSEL, 1, 0); //Just pick one for now

			//Some more...
			ctl = GetDlgItem(hwnd, IDC_SETTINGS_HKCOMBO2);
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"(None)");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"Shift");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"A");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"B");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"C");
			SendMessage(ctl, CB_SETCURSEL, 1, 0); //Just pick one for now

			//And a few more...
			ctl = GetDlgItem(hwnd, IDC_SETTINGS_LANGCOMBO);
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"N/A");
			SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)L"(Auto)");
			for (std::set<Language>::const_iterator it2=config.getLanguages().begin(); it2!=config.getLanguages().end(); it2++) {
				SendMessage(ctl, CB_ADDSTRING, 0, (LPARAM)it2->displayName.c_str());
			}
			SendMessage(ctl, CB_SETCURSEL, 0, 0); //Choose the first item, for now.

			//Set the main visible tab to the default language.
			// (Does not fire a TCN_SELCHANGE message)
			TabCtrl_SetCurSel(hwTabMain, defTabID);
			UpdateSettingsTab(hwnd, defTabID);

			break;
		}
		case WM_DRAWITEM:
		{
			//Draw our owner-drawn button?
			LPDRAWITEMSTRUCT drawInfo = (LPDRAWITEMSTRUCT)lParam;
			if (wParam!=IDC_SETTINGS_HELPCLOSEBTN)
				break;

			//Retrieve the DC
			HDC& currDC = drawInfo->hDC; //We need the direct DC to draw properly
			RECT bounds = drawInfo->rcItem; //Draw within these bounds

			//Should draw the button with three different states: highlighted,
			//  depressed, neither. But for now, it's not important.

			//Draw the border
			size_t mg = (bounds.right-bounds.left)/4;
			HPEN oldPen = (HPEN)SelectObject(currDC, g_BlackPen);
			HBRUSH oldBrush = (HBRUSH)SelectObject(currDC, g_RedBkgrd);
			Rectangle(currDC, bounds.left, bounds.top, bounds.right, bounds.bottom);
			SelectObject(currDC, g_WhiteThickPen);
			MoveToEx(currDC, bounds.left+mg, bounds.top+mg, NULL);
			LineTo(currDC, bounds.right-mg-1, bounds.bottom-mg-1);
			MoveToEx(currDC, bounds.left+mg, bounds.bottom-mg-1, NULL);
			LineTo(currDC, bounds.right-mg-1, bounds.top+mg);

			//Revert (?)
			SelectObject(currDC, oldPen);
			SelectObject(currDC, oldBrush);

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR info = (LPNMHDR)lParam;
			if (info->idFrom==IDC_SETTINGS_MAINTAB) {
				if (info->code==TCN_SELCHANGE) {
					//Get the ID of the current tab:
					int id = TabCtrl_GetCurSel(info->hwndFrom);

					//Update all
					UpdateSettingsTab(GetParent(info->hwndFrom), id);
				}
			}
			break;
		}
		case WM_CTLCOLORDLG:
			return (BOOL)g_DlgHelpBkgrd;
		/*case WM_CTLCOLORBTN: //Useless message for non-owner-drawn buttons!
		{
			int ctlID = GetDlgCtrlID((HWND)lParam);
			if (ctlID==IDC_SETTINGS_HELPCLOSEBTN) {
				//Set the background color of our static item
				SetTextColor((HDC)wParam, RGB(255, 255, 255)); //Set white text.
				SetBkColor((HDC)wParam, RGB(128, 0, 0));
				return (BOOL)g_RedBkgrd;
			}

			break;
		}*/
		case WM_CTLCOLORSTATIC:
		{
			int ctlID = GetDlgCtrlID((HWND)lParam);
			if (ctlID==IDC_SETTINGS_BKGRD) {
				//Set the background color of our static item
				//return (INT_PTR)CreateSolidBrush(RGB(0,0,0));
				return (INT_PTR)CreateSolidBrush(RGB(0xBB, 0xFF, 0xCC));

				//return (BOOL)g_DlgHelpSlash;
			} else if (ctlID==IDC_SETTINGS_IMLBL || ctlID==IDC_SETTINGS_OUTLBL) {
				return FALSE; //Let the system set itself
			}
			/*if (ctlID==IDC_SETTINGS_HELPPNL) {
				SetTextColor((HDC)wParam, RGB(0x00, 0x00, 0x00));
				SetBkColor((HDC)wParam, RGB(128, 0, 0));
				return (BOOL)g_GreenBkgrd;
			}*/

			//Transparent? Ugh.
			SetBkColor((HDC)wParam, RGB(0xEE, 0xFF, 0xEE));
			return (INT_PTR)CreateSolidBrush(RGB(0xEE, 0xFF, 0xEE));
			//return (BOOL)g_DlgHelpBkgrd;
			break;
		}
		case WM_COMMAND:
		{
			unsigned int ctlID = LOWORD(wParam);
			switch(ctlID)
			{
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					break;

				case IDC_SETTINGS_HKHELP:
				case IDC_SETTINGS_LANGHELP:
				case IDC_SETTINGS_IMHELP:
				case IDC_SETTINGS_OUTENCHELP:
					if (HIWORD(wParam)==STN_CLICKED) {
						bool show = ctlID!=lastHelpDlgID;
						MakeHelpBoxVisible(hwnd, show, ctlID, pair<size_t, size_t>(wndWidth, wndHelpAreaWidth));
						lastHelpDlgID = show ? ctlID : 0;
					}

					break;

				case IDC_SETTINGS_HELPCLOSEBTN:
					if (HIWORD(wParam)==BN_CLICKED) {
						//Close the window
						MakeHelpBoxVisible(hwnd, FALSE, 0, pair<size_t, size_t>(wndWidth, wndHelpAreaWidth));
						lastHelpDlgID = 0;
					}
					break;
			}
			break;
		}
		case WM_CLOSE:
			EndDialog(hwnd, IDCANCEL);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}




void onAllWindowsCreated()
{
	//Create our font
	try {
		makeFont();
	} catch (std::exception ex) {
		//Will generate a WM_DESTROY message
		delete mainWindow;
		return;
	}

	//Perform some basic default stuff (to get a valid DC)
	mainWindow->resizeWindow(mainWindow->getDefaultWidth(), mainWindow->getDefaultHeight(), false);
	mainWindow->createDoubleBufferedSurface();
}



//TEMP
struct pairmatches : public std::binary_function<pair<unsigned int, VirtKey>, unsigned int, bool>
{
	bool operator() (pair<unsigned int, VirtKey>& pr, unsigned int hkid) const { return pr.first == hkid; }
};
/*bool pairmatches(const pair<unsigned int, VirtKey>& pr, unsigned int hkid) {
	return pr.first==hkid;
}*/


//NOTE: We should avoid using hotkeyCode when possible, since it doesn't account for locale-specific information
void handleNewHighlights(unsigned int hotkeyCode, VirtKey& vkey)
{
	//NOTE: HOTKEY_VIRT_LSHIFT is not being triggered here.
	//      It seems that "GetKeyState()" isn't working for some reason.
	/*if (hotkeyCode==HOTKEY_VIRT_LSHIFT||hotkeyCode==HOTKEY_VIRT_RSHIFT) {
		Logger::writeLogLine('L');
	}*/

	//NOTE: This is happening because GetKeyState() returns the state of the keyboard
	//      at the time the last message was parsed.
	// We might try to hack around this with GetKeyAsyncState(), but it is prbably a better
	//      idea to see how the main loop differs when switching directly from another language.
	// NOTE: This might be related to the bug of the first key not being pumped correctly!

	//If this is a shifted key, get which key is shifted: left or right
	if (hotkeyCode==HOTKEY_SHIFT) {
		//Well, I like posting fake messages. :D
		// Note that (lParam>>16)&VK_LSHIFT doesn't work here
		SHORT shiftL = GetAsyncKeyState(VK_LSHIFT);
		SHORT shiftR = GetAsyncKeyState(VK_RSHIFT);
		if ((shiftL&0x8000)!=0)
			mainWindow->postMessage(WM_HOTKEY, HOTKEY_VIRT_LSHIFT, MAKELPARAM(MOD_SHIFT, VK_LSHIFT));
		if ((shiftR&0x8000)!=0)
			mainWindow->postMessage(WM_HOTKEY, HOTKEY_VIRT_RSHIFT, MAKELPARAM(MOD_SHIFT, VK_RSHIFT));
	} else {
		//Is this a valid key? If so, highlight it and repaint the help window
		if (helpKeyboard->highlightKey(vkey.vkCode, vkey.alphanum, vkey.modShift, true)) {
			reBlitHelp();

			//CRITICAL SECTION
			{
				EnterCriticalSection(&threadCriticalSec);

				/*if (isLogging)
					fprintf(logFile, "  Key down: %c\n", keyCode);*/

				//Manage our thread's list of currently pressed hotkeys
				//Remove any previous reference to this hotkey and add it to the beginning of the list.
				//NOTE: We don't need "erase" too, since hotkeysDown is a list (and it CAN shuffle memory inside remove_if)
				hotkeysDown.remove_if(std::bind2nd(pairmatches(), hotkeyCode));
				hotkeysDown.push_front(pair<unsigned int, VirtKey>(hotkeyCode, vkey));
				//hotkeysDown.remove(hotkeyCode);
				//hotkeysDown.push_front(hotkeyCode);

				//Do we need to start our thread?
				if (!threadIsActive) {
					threadIsActive = true;

					ResumeThread(keyTrackThread);

					//GetSystemTimeAsFileTime(&res);
					//int diff = (res.dwLowDateTime-rL)/10000L; //hectonanoseconds div 10,000 to get ms
				}

				LeaveCriticalSection(&threadCriticalSec);
			}
		}
	}
}



//Help functions & pointers
//NOTE: For "help", this is a bit of a misnomer, as it also handles
//      the regular help keyboard keys.
void OnHelpTitleBtnClick(unsigned int btnID)
{
	//Catch this key press; disable the window for the remainder of the session.
	if (helpKeyboard->closeHelpWindow(btnID)) {
		suppressHelpWindow = true;
	}

	//Alternatively, minimize it
	helpKeyboard->minmaxHelpWindow(btnID);

	//We might also try "clicking" on a button by ID
	helpKeyboard->clickButton(btnID);

	//Feed this virtual key through the current keyboard
	//For now, we skip non-alpha keys
	VirtKey vk = VirtKey(helpKeyboard->getLastClickedVKey());
	if (vk.vkCode != 0) {
		if (vk.vkCode==VK_BACK) {
			currInput->handleBackspace(vk);
		} else if (vk.vkCode!=VK_LSHIFT && vk.vkCode!=VK_RSHIFT) { //Skip Shift, but still update the keyboard
			//We need to anticipate that currInput will try to strip the encoding
			vk.considerLocale();
			currInput->handleKeyPress(vk);
		}
		checkAllHotkeysAndWindows();
		recalculate();
	}
}
void OnHelpTitleBtnOver(unsigned int btnID)
{
	//Regular
	helpKeyboard->highlightHelpTitleBtn(btnID, true);

	//Other
	helpKeyboard->highlightVirtKey(btnID, true);
}
void OnHelpTitleBtnOut(unsigned int btnID)
{
	//Regular
	helpKeyboard->highlightHelpTitleBtn(btnID, false);

	//Other
	helpKeyboard->highlightVirtKey(btnID, false);
}


//Memory functions and pointers (copied)
void OnMemoryTitleBtnClick(unsigned int btnID)
{
	//Catch this key press; disable the window for the remainder of the session.
	if (helpKeyboard->closeMemoryWindow(btnID)) {
		suppressMemoryWindow = true;
	}

	//Alternatively, minimize it
	helpKeyboard->minmaxMemoryWindow(btnID);
}
void OnMemoryTitleBtnOver(unsigned int btnID)
{
	helpKeyboard->highlightMemoryTitleBtn(btnID, true);
}
void OnMemoryTitleBtnOut(unsigned int btnID)
{
	helpKeyboard->highlightMemoryTitleBtn(btnID, false);
}


void checkAndInitHelpWindow()
{
	//Has initialization been performed?
	if (helpIsCached)
		return;

	//Time to re-size our help window. Might as well center it now, too
	HWND taskBar = FindWindowW(_T("Shell_TrayWnd"), _T(""));
	RECT r;
	GetClientRect(GetDesktopWindow(), &r);
	int newX = (r.right-r.left)/2-helpKeyboard->getWidth()/2;
	int newY = (r.bottom-r.top)-helpKeyboard->getHeight();
	if (taskBar != NULL) {
		GetClientRect(taskBar, &r);
		newY -= (r.bottom-r.top);
	}
	helpWindow->expandWindow(newX, newY, helpKeyboard->getWidth(), helpKeyboard->getHeight(), false);

	//Move the memory window, too
	memoryWindow->expandWindow(newX+helpKeyboard->getWidth(), newY, helpKeyboard->getMemoryWidth(), helpKeyboard->getMemoryHeight(), false);

	//Might as well build the reverse lookup
	//model->reverseLookupWord(0);

	//...and now we can properly initialize its drawing surface
	helpKeyboard->init(helpWindow, memoryWindow, OnHelpTitleBtnClick, OnHelpTitleBtnOver, OnHelpTitleBtnOut, OnMemoryTitleBtnClick, OnMemoryTitleBtnOver, OnMemoryTitleBtnOut);

	//WORKAROUND - Fixes an issue where WZ won't highlight the first key press (unless it's Shift)
	//CRITICAL SECTION
	/*{
		EnterCriticalSection(&threadCriticalSec);

		//NOTE: This is the workaround: just process a dummy event.
		//      GetKeyState() is failing for some unknown reason on the first press.
		//                    All attempts to "update" it somehow have failed.
		hotkeysDown.push_front(pair<unsigned int, VirtKey>(HOTKEY_U_LOW, VirtKey('u', 'u', false, false, false))); //Comment this line to re-trigger the bug.
		//hotkeysDown.push_front(117); //Comment this line to re-trigger the bug.
		if (!threadIsActive) {
			threadIsActive = true;
			ResumeThread(keyTrackThread);
		}

		LeaveCriticalSection(&threadCriticalSec);
	}*/
	//END WORKAROUND

	//Only needs to be performed once.
	helpIsCached = true;
}


//"Toggle" functions control turning certain things on/off.
//All of these take a boolean value: what are we toggling TO?
void toggleHelpMode(bool toggleTo)
{
	//Do nothing if called in error.
	if (toggleTo == helpKeyboard->isHelpEnabled())
		return;

	//Are we turning the window "on" or "off"
	if (toggleTo) {
		//First, initialize the help window
		checkAndInitHelpWindow();

		//Register all hotkeys relevant for the help window
		bool res = true;
		//if (!controlKeysOn) //We'll need these too.
		//	res = turnOnControlkeys(true);  //Elsewhere...
		if (!turnOnHelpKeys(true) || !res)
			mainWindow->showMessageBox(L"Could not turn on the shift/control hotkeys.", L"Error", MB_ICONERROR | MB_OK);

		//Reset our input2uni transformer (others shouldn't need changing).
		input2Uni = config.getTransformation(config.activeLanguage, currHelpInput->encoding, config.unicodeEncoding);

		//Get an encoding switcher for the reverse-roman lookup
		//const Transformation* uni2Roman = config.getTransformation(config.activeLanguage, config.unicodeEncoding, currTypeInput->encoding);

		//Switch inputs, set as helper
		currHelpInput->treatAsHelpKeyboard(currTypeInput, config.unicodeEncoding, ConfigGetTransformation);
		currInput = currHelpInput;

		//Clear our current word (not the sentence, though, and keep the trigrams)
		//Also reset the helper keyboard.
		currTypeInput->reset(true, true, false, false);
		currHelpInput->reset(true, true, true, true);

		//Show the help window
		helpKeyboard->turnOnHelpMode(true, suppressHelpWindow, suppressMemoryWindow);

		//TODO: Automate repainting the help window...
		reBlitHelp();

		//Show the main/sentence windows if they're not already visible
		//TODO: Automate somehow
		mainWindow->showWindow(true);
		sentenceWindow->showWindow(true);
	} else {
		//Change to the typed input
		currInput = currTypeInput;

		//Did we just add an entry?
		pair<string, wstring> checkEntry = currInput->getAndClearMostRecentRomanizationCheck();
		if (!checkEntry.first.empty() && !checkEntry.second.empty())
			helpKeyboard->addMemoryEntry(checkEntry.second, checkEntry.first);

		//Reset our input2uni transformer (others shouldn't need changing).
		input2Uni = config.getTransformation(config.activeLanguage, currInput->encoding, config.unicodeEncoding);

		//Turn off help keys
		turnOnHelpKeys(false);

		//Hide the mainWindow and (optionally) sentence window)
		mainWindow->showWindow(false);
		if (currInput->getTypedSentenceStrings()[3].empty())
			sentenceWindow->showWindow(false);

		//Hide windows
		helpKeyboard->turnOnHelpMode(false, suppressHelpWindow, suppressMemoryWindow);
	}


	//Redraw all
	//currInput->forceViewChanged(); //Won't work... yet
	recalculate(); //Shouldn't cause any problems...
}


void checkAllHotkeysAndWindows()
{
	//Should the main window be visible?
	if (!currInput->getTypedRomanString(false).empty() || currInput->isHelpInput()) {
		mainWindow->showWindow(true);
	} else {
		mainWindow->showWindow(false);

		//Partial reset
		currInput->reset(true, true, false, false);
	}

	//Should the sentence window be visible?
	if (!currInput->getTypedSentenceStrings()[3].empty() || currInput->isHelpInput() || mainWindow->isVisible()) {
		sentenceWindow->showWindow(true);
		turnOnControlkeys(true);

		//Force repaint
		currInput->forceViewChanged(); //TODO: Is this needed here?
	} else {
		sentenceWindow->showWindow(false);
		turnOnControlkeys(false);

		//Full reset
		currInput->reset(true, true, true, true);
	}
}



//Change our model; reset as necessary depending on what changed
bool logLangChange = false; //Only set once.
void ChangeLangInputOutput(wstring langid, wstring inputid, wstring outputid)
{
	//Step 1: Set
	if (!langid.empty()) {
		//Changing the language changes just about everything.
		config.activeLanguage = *(FindKeyInSet(config.getLanguages(), langid));
		config.activeDisplayMethods.clear();
		config.activeDisplayMethods.push_back(*(FindKeyInSet(config.getDisplayMethods(), config.activeLanguage.defaultDisplayMethodReg)));
		config.activeDisplayMethods.push_back(*(FindKeyInSet(config.getDisplayMethods(), config.activeLanguage.defaultDisplayMethodSmall)));
		config.activeInputMethod = *(FindKeyInSet(config.getInputMethods(), config.activeLanguage.defaultInputMethod));
		config.activeOutputEncoding = config.activeLanguage.defaultOutputEncoding;
	}
	if (!inputid.empty())
		config.activeInputMethod = *(FindKeyInSet(config.getInputMethods(), inputid));
	if (!outputid.empty())
		config.activeOutputEncoding = *(FindKeyInSet(config.getEncodings(), outputid));
	if (logLangChange)
		Logger::markLogTime('L', L"LangInOut is set");

	//Step 2: Read
	currInput = config.activeInputMethod;
	currTypeInput = currInput;
	currHelpInput = NULL;
	mmFont = config.activeDisplayMethods[0];
	mmFontSmall = config.activeDisplayMethods[1];
	input2Uni = config.getTransformation(config.activeLanguage, config.activeInputMethod->encoding, config.unicodeEncoding);
	uni2Output = config.getTransformation(config.activeLanguage, config.unicodeEncoding, config.activeOutputEncoding);
	uni2Disp = config.getTransformation(config.activeLanguage, config.unicodeEncoding, config.activeDisplayMethods[0]->encoding);
	if (logLangChange)
		Logger::markLogTime('L', L"Cached entries saved");


	//TEMP: Enable myWin2.2 for Roman Input Methods
	bool isRoman = false;
	//bool isPulpFontDisplay = (mmFontSmall->type==DISPM_PNG||mmFontSmall->type==BUILTIN);
	try {
		//Intentionally try to cause an exception
		currInput->treatAsHelpKeyboard(NULL, config.unicodeEncoding, NULL);
	} catch (std::exception ex) {
		isRoman = true;
	}
	if (isRoman /*&& isPulpFontDisplay*/) {
		currHelpInput = *(FindKeyInSet(config.getInputMethods(), ConfigManager::sanitize_id(L"mywin")));
	}
	if (logLangChange)
		Logger::markLogTime('L', L"Help keyboard hack enabled.");
	//END TEMP HACKERY... ugh.


	//Now, reset?
	if (!langid.empty() || !inputid.empty()) {
		//Input has changed; reset
		currTypeInput->reset(true, true, true, true);
		if (currHelpInput!=NULL)
			currHelpInput->reset(true, true, true, true);

		//And repaint, just in case
		checkAllHotkeysAndWindows();

		if (logLangChange)
			Logger::markLogTime('L', L"Input reset");
	}
	if (!langid.empty()) { //Rebuild the menus, or build for the first time

		//Reclaim, reset
		if (contextMenu!=NULL) {
			DestroyMenu(contextMenu);
			customMenuItemsLookup.clear();
			if (customMenuItems!=NULL) {
				delete [] customMenuItems;
				customMenuItems = NULL;
			}
			totalMenuItems = 0;
		}

		//Reubild
		if (logLangChange)
			Logger::startLogTimer('L', L"Creating the context menu");
		createContextMenu();
		if (logLangChange)
			Logger::endLogTimer('L');

		//Record (kidding... "log")
		if (logLangChange)
			Logger::markLogTime('L', L"Context menus rebuilt");

		//We'll have to re-size the windows, in case a different font is used
		initCalculate();

		if (logLangChange)
			Logger::markLogTime('L', L"Windows resized");
	}

	//Just to be safe
	recalculate();

	if (logLangChange)
		Logger::markLogTime('L', L"Windows re-rasterized");

	//Finally, save data
	FlashSaveState();
	if (logLangChange)
		Logger::markLogTime('L', L"Current LangInOut flashed to disk");
}




bool handleMetaHotkeys(WPARAM hotkeyCode, VirtKey& vkey)
{
	switch (hotkeyCode) {
		case LANG_HOTKEY:
			//Switch language
			switchToLanguage(!mmOn);
			return true;

		case HOTKEY_HELP:
			//What to do if our user hits "F1".
			if (/*!allowNonBurmeseLetters &&*/ currHelpInput!=NULL) {
				toggleHelpMode(!helpKeyboard->isHelpEnabled());
				checkAllHotkeysAndWindows();
			}
			return true;

		default:
			if (helpKeyboard->isHelpEnabled() && highlightKeys) {
				//Convert:
				VirtKey vk2(vkey);
				vk2.stripLocale();

				//Highlight our virtual keyboard
				handleNewHighlights(hotkeyCode, vk2);

				//Doesn't consume a keypress
				return false;
			}
	}

	//Else, not processed
	return false;
}


bool handleUserHotkeys(WPARAM hotkeyCode, VirtKey& vkey)
{
	//First, is this an upper-case letter?
	//bool isUpper = (wParam>='A' && wParam<='Z');

	//Handle user input; anything that updates a non-specific "model".
	//  TODO: Put code for "help" keyboard functionality HERE; DON'T put it into
	//        LetterInputMethod.h
	switch (hotkeyCode) {
		case HOTKEY_ESC:
			//Close the window, exit help mode
			currInput->handleEsc();
			return true;

		case HOTKEY_BACK:
			//Back up
			currInput->handleBackspace(vkey);
			return true;

		case HOTKEY_DELETE:
			//Delete a phrase
			currInput->handleDelete();
			return true;

		case HOTKEY_RIGHT:
			//Advance the cursor, pick a word
			currInput->handleLeftRight(true, false);
			return true;

		case HOTKEY_LEFT:
			//Move the cursor back, pick a word
			currInput->handleLeftRight(false, false);
			return true;

		case HOTKEY_DOWN:
		case HOTKEY_PAGEDOWN:
			//Page
			currInput->handleUpDown(true);
			return true;

		case HOTKEY_UP:
		case HOTKEY_PAGEUP:
			//Page
			currInput->handleUpDown(false);
			return true;

		case HOTKEY_COMMA: case HOTKEY_PERIOD:
			//Handle stops
			currInput->handleStop(hotkeyCode==HOTKEY_PERIOD, vkey);
			return true;

		case HOTKEY_ENTER: case HOTKEY_SHIFT_ENTER:
			//Handle word selection
			currInput->handleCommit(true);
			return true;

		case HOTKEY_TAB: case HOTKEY_SHIFT_TAB:
			//Tab generally behaves as a right-arow.
			currInput->handleTab();
			return true;

		case HOTKEY_SPACE: case HOTKEY_SHIFT_SPACE:
			//Handle word selection, cursor advancing
			currInput->handleCommit(false);
			return true;

		case HOTKEY_0: case HOTKEY_NUM0:
		case HOTKEY_1: case HOTKEY_NUM1:
		case HOTKEY_2: case HOTKEY_NUM2:
		case HOTKEY_3: case HOTKEY_NUM3:
		case HOTKEY_4: case HOTKEY_NUM4:
		case HOTKEY_5: case HOTKEY_NUM5:
		case HOTKEY_6: case HOTKEY_NUM6:
		case HOTKEY_7: case HOTKEY_NUM7:
		case HOTKEY_8: case HOTKEY_NUM8:
		case HOTKEY_9: case HOTKEY_NUM9:
		case HOTKEY_COMBINE: case HOTKEY_SHIFT_COMBINE:
		{
			//Handle numbers and combiners; pick a word, type a letter, combine/stack letters.
			//numCode = 0 through 9 for numbers, undefined for combiners
			//int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : HOTKEY_NUM0;
			//int numberValue = vkey.alphanum - '0';

			//Handle key press; letter-based keyboard should just pass this on through
			bool typeNumerals = true; //TODO: put this SOMEWHERE in the config file.
			currInput->handleNumber(vkey, typeNumerals);
			return true;
		}

		default:
			//Tricky here: we need to put the "system key" nonsense into the "handleKeyPress"  function
			// otherwise numbers won't work.
			currInput->handleKeyPress(vkey);
			return true;
	}

	//Else, not processed
	return false;
}


void createMyanmarMenuFont()
{
	menuFont = (TtfDisplay*)WZFactory<WordBuilder>::getPadaukZawgyiTtfDisplay(L"myanmar", L"pdkzgwz");
}



//Rebuild the context menu --any time
void createContextMenu()
{
	//Generate an empty menu
	contextMenu = CreateMenu();
	contextMenuPopup = CreateMenu();
	AppendMenu(contextMenu, MF_POPUP|MF_STRING, (UINT_PTR)contextMenuPopup, L"Main");
	if (logLangChange)
		Logger::markLogTime('L', L"Created top-level menu");

	//Add normal entries, including "Typing", but no sub-entries.
	typingMenu = CreateMenu();
	//if (newVersionAvailable)  //Looks terrible in the menu.
	//	AppendMenu(contextMenuPopup, MF_STRING, IDM_NEWVERSION, L"New Version Available!");
	AppendMenu(contextMenuPopup, MF_STRING, IDM_HELP, L"&Help/About");
	AppendMenu(contextMenuPopup, MF_STRING, IDM_SETTINGS, L"&Settings");
	AppendMenu(contextMenuPopup, MF_STRING, IDM_EXIT, L"E&xit");
	AppendMenu(contextMenuPopup, MF_SEPARATOR, NULL, NULL);
	AppendMenu(contextMenuPopup, MF_STRING|MF_CHECKED, IDM_ENGLISH, L"&English (Alt+Shift)");
	AppendMenu(contextMenuPopup, MF_STRING, IDM_MYANMAR, L"&Myanmar (Alt+Shift)");
	AppendMenu(contextMenuPopup, MF_SEPARATOR, NULL, NULL);
	AppendMenu(contextMenuPopup, MF_POPUP|MF_STRING, (UINT_PTR)typingMenu, L"&Typing");
	AppendMenu(contextMenuPopup, MF_STRING, IDM_LOOKUP, L"&Look Up Word (F1)");
	if (logLangChange)
		Logger::markLogTime('L', L"Appended top-level menu items");


	//Build each sub-menu if none have been created yet. (In case we ever recycle this menu).
	if (totalMenuItems == 0) {
		//Give each menu item a unique ID
		unsigned int currDynamicCmd = IDM_TYPING_SUBMENU_START;

		//Init the cache; store in a vector for now.
		WZMenuItem sep = WZMenuItem(0, WZMI_SEP, L"", L"");
		vector<WZMenuItem> myMenuItems;

		//Add the "Language" section
		myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_HEADER, L"", WND_TITLE_LANGUAGE));
		myMenuItems.push_back(sep);
		currDynamicCmd++; //Maintain easy access

		//Add each language as a MI
		for (std::set<Language>::const_iterator it = config.getLanguages().begin(); it!=config.getLanguages().end(); it++)
			myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_LANG, it->id, it->displayName));

		if (logLangChange)
			Logger::markLogTime('L', L"Re-built sub-menu: languages");

		//Add the "Input Methods" section
		myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_HEADER, L"", WND_TITLE_INPUT));
		myMenuItems.push_back(sep);
		currDynamicCmd++; //Maintain easy access

		//Add each input method as an MI
		for (std::set<InputMethod*>::const_iterator it = config.getInputMethods().begin(); it!=config.getInputMethods().end(); it++)
			myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_INPUT, (*it)->id, (*it)->displayName));

		if (logLangChange)
			Logger::markLogTime('L', L"Re-built sub-menu: input methods");

		//Add the "Output Encodings" section
		myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_HEADER, L"", WND_TITLE_OUTPUT));
		myMenuItems.push_back(sep);
		currDynamicCmd++; //Maintain easy access

		//Add each encoding as an MI
		for (std::set<Encoding>::const_iterator it = config.getEncodings().begin(); it!=config.getEncodings().end(); it++) {
			if (!it->canUseAsOutput)
				continue;
			myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_OUTPUT, it->id, it->displayName));
		}

		if (logLangChange)
			Logger::markLogTime('L', L"Re-built sub-menu: encodings");

		//Copy over to allocated memory --we need a constant pointer to each menu item, and vectors are too risky for this.
		customMenuItems = new WZMenuItem[myMenuItems.size()];
		for (size_t i=0; i<myMenuItems.size(); i++) {
			customMenuItems[totalMenuItems] = myMenuItems[i];
			customMenuItemsLookup[customMenuItems[totalMenuItems].menuID] = &customMenuItems[totalMenuItems];
			totalMenuItems++;
		}

		if (logLangChange)
			Logger::markLogTime('L', L"Copied built sub-menus to pointer-controlled memory");
	}


	//Add each menu in our cache
	for (size_t i=0; i<totalMenuItems; i++) {
		if (customMenuItems[i].type==WZMI_SEP)
			AppendMenu(typingMenu, MF_SEPARATOR, 0, NULL);
		else {
			unsigned int flag = (i>0&&customMenuItems[i].type==WZMI_HEADER) ? MF_MENUBARBREAK : 0;
			AppendMenu(typingMenu, MF_OWNERDRAW|flag, customMenuItems[i].menuID, (LPTSTR)&(customMenuItems[i]));
		}
	}

	if (logLangChange)
		Logger::markLogTime('L', L"Added each cached sub-menu.");
}



//Build the context menu --first time
void initContextMenu()
{
	//Make the font
	createMyanmarMenuFont();
	Logger::markLogTime('L', L"Myanmar font created");

	//Make the menu
	//createContextMenu();
	//Logger::markLogTime('L', L"Context menu created");

	//NOTE: This is a bit of a hack; it forces the context menu
	//      to be rebuilt when the language changes.
	//      Later, we can just make it happen all the time.
	//contextMenu = CreateMenu();
	//customMenuItems = new WZMenuItem[1];
	//Logger::markLogTime('L', L"Created dummy context menu");
}


void updateContextMenuState()
{
	//Hotkey string, check mark for the current language.
	std::wstringstream txt;
	txt <<L"English (" <<hkString <<")";
	ModifyMenu(contextMenu, IDM_ENGLISH, MF_BYCOMMAND|(mmOn?0:MF_CHECKED), IDM_ENGLISH, txt.str().c_str());
	txt.str(L"");
	txt <<L"Myanmar (" <<hkString <<")";
	ModifyMenu(contextMenu, IDM_MYANMAR, MF_BYCOMMAND|(mmOn?MF_CHECKED:0), IDM_MYANMAR, txt.str().c_str());

	//Set a check for the "Look Up Word" function
	//  Also remove the "F1" if not applicable.
	UINT flagL = helpKeyboard->isHelpEnabled() ? MF_CHECKED : 0;
	const wstring& POPUP_LOOKUP = mmOn ? POPUP_LOOKUP_MM : POPUP_LOOKUP_EN;
	ModifyMenu(contextMenu, IDM_LOOKUP, MF_BYCOMMAND|flagL, IDM_LOOKUP, POPUP_LOOKUP.c_str());

	//Set checks for the current language, input method, and output encoding
	for (size_t i=0; i<totalMenuItems; i++) {
		//What are we checking against?
		wstring idToCheck;
		if (customMenuItems[i].type==WZMI_LANG)
			idToCheck = config.activeLanguage.id;
		else if (customMenuItems[i].type==WZMI_INPUT)
			idToCheck = config.activeInputMethod->id;
		else if (customMenuItems[i].type==WZMI_OUTPUT)
			idToCheck = config.activeOutputEncoding.id;
		else
			continue;

		//Check/uncheck manually
		unsigned int checkFlag = (customMenuItems[i].id==idToCheck) ? MF_CHECKED : MF_UNCHECKED;
		CheckMenuItem(typingMenu, IDM_TYPING_SUBMENU_START+i, MF_BYCOMMAND|checkFlag);
	}
}



//If there's an exception, show an error message and disable that input in the future.
void disableCurrentInput(HWND currHwnd, const std::exception& ex)
{
	//First, display our warning message.
	{
	std::wstringstream msg;
	msg << "WaitZar has encountered an error with the current Input Method.\nThe input method \"";
	msg <<config.activeInputMethod->displayName;
	msg <<"\" has been disabled; you must switch the Language or restart WaitZar to re-enable it.\n\nDetails:\n";
	msg << ex.what();
	MessageBox((mainWindow->isVisible()||sentenceWindow->isVisible())?currHwnd:NULL, msg.str().c_str(), L"WaitZar IM Runtime Error", MB_ICONWARNING | MB_OK);
	}


	//Next, disable the active input method (temporarily)
	for (size_t i=0; i<totalMenuItems; i++) {
		if (customMenuItems[i].type!=WZMI_INPUT)
			continue;
		if (customMenuItems[i].id != config.activeInputMethod->id)
			continue;
		customMenuItems[i].disabled = true;
	}

	//Finally, switch the input to WaitZar.
	if (FindKeyInSet(config.activeLanguage.inputMethods, L"waitzar")==config.activeLanguage.inputMethods.end())
		ChangeLangInputOutput(L"myanmar", L"waitzar", L""); //Removes our blacklisted item, but at least we're back in MM
	else
		ChangeLangInputOutput(L"", L"waitzar", L"");
}



//Change the encoding by clicking on the SentenceWindow icon
void OnEncodingChangeClick(unsigned int regionID)
{
	//Retrieve the current language, increment by one
	size_t customMenuStartID = 2 + config.getLanguages().size() + 2 + config.getInputMethods().size() + 2;
	for (size_t i=customMenuStartID; i<totalMenuItems; i++) {
		WZMenuItem currItem = customMenuItems[i];
		if (currItem.id == config.activeOutputEncoding.id) {
			//Increment
			size_t newID = i+1;
			if (newID >= totalMenuItems)
				newID = customMenuStartID;

			//Set
			ChangeLangInputOutput(L"", L"", customMenuItems[newID].id);
			break;
		}
	}
}




/**
 * Message-handling code.
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Handle callback
	switch(msg) {
		case UWM_HOTKEY_UP: //HOTKEY_UP is defined by us, it is just like HOTKEY_DOWN except it doesn't use the lparam
		{
			//Turn this lparam into a virtual key
			VirtKey vk(lParam);
			//vk.stripLocale(); We don't need to strip the locale, since the ACTUAL en_US id was sent.

			//Update our virtual keyboard
			if (helpKeyboard->highlightKey(vk.vkCode, vk.alphanum, vk.modShift, false))
				reBlitHelp();

			break;
		}

		/**
		 * Regarding hotkeys: The Main message loop (here) handles the following:
		 *   1) All hotkey registration/unregistration and, by extension, the language hotkey.
		 *   2) Showing/hiding of windows.
		 *   3) Sending output to Windows
		 *   4) Repainting and recalculating of window content.
		 * Thus, we expect InputManagers to only handle updates to their internal models, which is still
		 *  fairly complex. InputManagers can indicate that they require repainting, or that they've
		 *  just invalidated their sentence window, but the main loop is where this happens.
		 */
		case WM_HOTKEY:
		{
			//We don't handle hotkeys if a higher-level dialog is showing
			if (showingHelpPopup || showingKeyInputPopup || showingSettingsPopup)
				break;

			//Turn this hotkey into a virtual key, to make things simpler.
			VirtKey vk(lParam);

			//NOTE: We have to mangle this a bit (with shift) if the virtual
			//      keyboard is showing and Shift has been pressed.
			if (helpKeyboard->isHelpEnabled() && helpKeyboard->isShiftLocked()) {
				vk = VirtKey(vk.vkCode, true, vk.modAlt, vk.modCtrl);
			}


			//First, handle all "system" or "meta" level commands, like switching the language,
			// switching into help mode, etc.
			//Then, handle all "dynamic" commands; those which change depending on the
			// current IM or mode.
			if (handleMetaHotkeys(wParam, vk)) {
				//Make sure our hotkeys are set
				//TODO: Remove specific hotkey calls from toggleHelp() and other meta functions...
				//checkAllHotkeysAndWindows();
			} else {
				//Set flags for the current state of the Input Manager. We will
				// check these against the exit state to see what has changed,
				// and thus what needs to be updated.
				bool wasProvidingHelp = currInput->isHelpInput();
				bool wasEmptySentence = currInput->getTypedSentenceStrings()[3].empty();
				bool wasEmptyRoman = currInput->getTypedRomanString(false).empty();

				//Process the message
				try {
					handleUserHotkeys(wParam, vk);
				} catch (std::exception ex) {
					if (!showingKeyInputPopup) {
						//Properly handle hotkeys
						//NOTE: We need this here so that "Enter" is passed to the window.
						bool refreshControl = controlKeysOn;
						if  (refreshControl==true)
							turnOnControlkeys(false);

						//Show
						showingKeyInputPopup = true;
						disableCurrentInput(hwnd, ex);
						showingKeyInputPopup = false;

						//Control keys again
						if  (refreshControl==true)
							turnOnControlkeys(true);
					}
					break;
				}

				//Save the "typed string" for later
				wstring stringToType = currInput->getTypedSentenceStrings()[3];

				//Check 1: Did we just switch out of help mode?
				if (wasProvidingHelp != currInput->isHelpInput())
					toggleHelpMode(!wasProvidingHelp);

				//Check 2: Did SOMETHING change? (Entering/exiting sentence mode, just type the first
				//         word in the candidate list or finish it, or enter/exit help mode?) This will
				//         perform unnecessary calculations, but it's not too wasteful, and makes up for it
				//         by cleaning up the code sufficiently.
				if (    (wasEmptySentence != currInput->getTypedSentenceStrings()[3].empty())
					||  (wasEmptyRoman != currInput->getTypedRomanString(false).empty())
					||  (wasProvidingHelp != currInput->isHelpInput()))
					checkAllHotkeysAndWindows();

				//Feedback Check 1: Do we need to type the current sentence?
				if (currInput->getAndClearRequestToTypeSentence())
					typeCurrentPhrase(stringToType);

				//Feedback Check 2: Do we need to repaint the window?
				if (currInput->getAndClearViewChanged())
					recalculate();
			}
			break;
		}


		///////TEMP   --- we should manage this in MyWin32Window
		/*case WM_MOUSEMOVE: //Change the cursor
		{
			//Only react if no control keys are pressed
			if ((wParam&(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON))==0) {
				size_t clientX = GET_X_LPARAM(lParam);
				size_t clientY = GET_Y_LPARAM(lParam);

				//This seems to work. It flickers a tiny bit, but is easier
				// than manually setting the class cursor, for now.
				if (clientX<20 && clientY<20)
					SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			}

			break;
		}*/
		/////////END TEMP



		case WM_MEASUREITEM:  //Measure our custom menus.
		{
			//Retrieve our custom data structure
			LPMEASUREITEMSTRUCT measureItem = (LPMEASUREITEMSTRUCT) lParam;
			WZMenuItem* item = (WZMenuItem*)measureItem->itemData;
			HDC currDC = GetDC(hwnd); //We need the direct DC to draw properly

			//First time?
			if (sysFont==NULL) {
				sysFont = new TtfDisplay();
				//sysFont->init(sysFontRes); //Init to nothing
			}

			//Get the right width
			measureItem->itemWidth = 3;
			if (item->containsMM)
				measureItem->itemWidth += menuFont->getStringWidth(item->title, currDC);
			else
				measureItem->itemWidth += sysFont->getStringWidth(item->title, currDC);

			//And height
			measureItem->itemHeight = ((menuFont->getHeight(currDC)>sysFont->getHeight(currDC)) ? menuFont->getHeight(currDC) : sysFont->getHeight(currDC)) + 3;
			break;
		}

		case WM_DRAWITEM: //Draw custom menu items
		{
			//Are we receiving draw events for a menu?
			if (wParam!=0)
				break;

			//Retrieve our custom data structure.
			LPDRAWITEMSTRUCT drawInfo = (LPDRAWITEMSTRUCT)lParam;
            WZMenuItem* item = (WZMenuItem*)drawInfo->itemData;
			HDC& currDC = drawInfo->hDC; //We need the direct DC to draw properly

			//Check the type, again
			if (drawInfo->CtlType != ODT_MENU)
				break;

			//Get our font
			TtfDisplay* myFont = sysFont;
			if (item->containsMM)
				myFont = menuFont;

			//Set the background and save the old colors (we always set it to something, even if it's no change)
			COLORREF oldBkgrd;
			if (item->type==WZMI_HEADER) {
				oldBkgrd = SetBkColor(currDC, cr_Black);
				myFont->setColor(GetRValue(cr_White), GetGValue(cr_White), GetBValue(cr_White));
			} else if ((drawInfo->itemState&ODS_HOTLIGHT)||(drawInfo->itemState&ODS_SELECTED)) {
				oldBkgrd = SetBkColor(currDC, cr_MenuItemBkgrd);
				if (item->disabled)
					myFont->setColor(GetRValue(cr_MenuItemDisabledText), GetGValue(cr_MenuItemDisabledText), GetBValue(cr_MenuItemDisabledText));
				else
					myFont->setColor(GetRValue(cr_MenuItemText), GetGValue(cr_MenuItemText), GetBValue(cr_MenuItemText));
			} else {
				oldBkgrd = SetBkColor(currDC, cr_MenuDefaultBkgrd);
				if (item->disabled)
					myFont->setColor(GetRValue(cr_MenuDefaultDisabledText), GetGValue(cr_MenuDefaultDisabledText), GetBValue(cr_MenuDefaultDisabledText));
				else
					myFont->setColor(GetRValue(cr_MenuDefaultText), GetGValue(cr_MenuDefaultText), GetBValue(cr_MenuDefaultText));
			}

			//Conditionally set the font
			unsigned int yOffset = 2;
			if (item->containsMM)
				yOffset = 0;

			//Fill the background (border first)
			HBRUSH oldBrush;
			HPEN oldPen = (HPEN)SelectObject(currDC, g_EmptyPen);
			if (item->type==WZMI_HEADER) {
				oldBrush = (HBRUSH)SelectObject(currDC, g_BlackBkgrd);
				Rectangle(currDC, drawInfo->rcItem.left, drawInfo->rcItem.top, drawInfo->rcItem.right, drawInfo->rcItem.bottom);
			} else if ((drawInfo->itemState&ODS_HOTLIGHT)||(drawInfo->itemState&ODS_SELECTED)) {
				oldBrush = (HBRUSH)SelectObject(currDC, g_MenuItemHilite);
				Rectangle(currDC, drawInfo->rcItem.left, drawInfo->rcItem.top, drawInfo->rcItem.right, drawInfo->rcItem.bottom);
				SelectObject(currDC, g_MenuItemBkgrd);
				Rectangle(currDC, drawInfo->rcItem.left+1, drawInfo->rcItem.top+1, drawInfo->rcItem.right-1, drawInfo->rcItem.bottom-1);
			} else {
				oldBrush = (HBRUSH)SelectObject(currDC, g_MenuDefaultBkgrd);
				Rectangle(currDC, drawInfo->rcItem.left, drawInfo->rcItem.top, drawInfo->rcItem.right, drawInfo->rcItem.bottom);
			}
			SelectObject(currDC, oldPen);
			SelectObject(currDC, oldBrush);


			//Leave space for the check-mark bitmap
			int checkTop = (drawInfo->rcItem.bottom-drawInfo->rcItem.top)/2-GetSystemMetrics(SM_CYMENUCHECK)/2;
			RECT checkRect = {drawInfo->rcItem.left, drawInfo->rcItem.top+checkTop, drawInfo->rcItem.left+GetSystemMetrics(SM_CXMENUCHECK), drawInfo->rcItem.top+checkTop+GetSystemMetrics(SM_CYMENUCHECK)};
			int startX = checkRect.right + 1;
            int startY = drawInfo->rcItem.top + yOffset + 1;


			//Draw the check mark?
			if (drawInfo->itemState&ODS_CHECKED) {
				if ((drawInfo->itemState&ODS_HOTLIGHT)||(drawInfo->itemState&ODS_SELECTED)) {
					oldBrush = (HBRUSH)SelectObject(currDC, g_DotHiliteBkgrd);
					oldPen = (HPEN)SelectObject(currDC, g_DotHilitePen);
				} else {
					oldBrush = (HBRUSH)SelectObject(currDC, g_BlackBkgrd);
					oldPen = (HPEN)SelectObject(currDC, g_MediumGrayPen);
				}
				unsigned int offset = 2;
				Ellipse(currDC, checkRect.left+1+offset, checkRect.top-1+offset, checkRect.right-offset, checkRect.bottom-1-offset);
				SelectObject(currDC, oldPen);
				SelectObject(currDC, oldBrush);
			}

			//Draw the text to the DC
			myFont->drawString(currDC, item->title.c_str(), startX, startY);

			//Reset the DC to its original values.
			SetBkColor(currDC, oldBkgrd);
			break;
		}


		case UWM_SYSTRAY: //Custom callback for our system tray icon
		{
			if (lParam == NIN_BALLOONUSERCLICK) {
				//A bit of a hack; it's not obvious how to check WHICH baloon message is set
				if (newVersionAvailable) {
					//Go to the WaitZar web page.
					ShellExecute(NULL, L"open", L"http://www.waitzar.com/download.py", NULL, NULL, SW_SHOWNORMAL);
				}
			} else if (lParam==WM_RBUTTONUP || lParam==WM_LBUTTONUP) {
				//Get the mouse's position; we'll put the menu there
				POINT pt;
				GetCursorPos(&pt);

				//Update the menu
				updateContextMenuState();

				//Cause our popup to appear in front of any other window.
				SetForegroundWindow(hwnd);

				//Force a track on this menu.
				int retVal = TrackPopupMenu(contextMenuPopup, //Which menu to track
                                 TPM_RETURNCMD |    //This code specifies that we return the ID of the selected menu item.
                                 TPM_RIGHTBUTTON,   //Track right mouse button
                                 pt.x, pt.y,        //Specifies the menu's location.
                                 0,                 //Reserved (MSDN: _Must_ be zero)
                                 hwnd,              //Owner
                                 NULL);            //MSDN: Ignored
				if (retVal == IDM_HELP) {
					//Properly handle hotkeys
					bool refreshControl = controlKeysOn;
					if  (refreshControl==true)
						turnOnControlkeys(false);


					//MessageBox(hwnd, temp, _T("About"), MB_ICONINFORMATION | MB_OK);
					if (!showingHelpPopup) {
						showingHelpPopup = true;
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_BLANK), hwnd, HelpDlgProc);
						showingHelpPopup = false;
					}

					//Hotkeys again
					if  (refreshControl)
						turnOnControlkeys(true);
				} else if (retVal == IDM_SETTINGS) {
					showSettingsMenu(hwnd);
				} else if (retVal == IDM_ENGLISH) {
					switchToLanguage(false);

					//Reset the model
					currInput->reset(true, true, true, true);
				} else if (retVal == IDM_MYANMAR) {
					switchToLanguage(true);

					//Reset the model
					currInput->reset(true, true, true, true);
				} else if (retVal == IDM_LOOKUP) {
					//Manage our help window
					if (currHelpInput!=NULL) {
						if (!mmOn)
							switchToLanguage(true);
						toggleHelpMode(!currInput->isHelpInput()); //TODO: Check this works!
						checkAllHotkeysAndWindows();
					}
				} else if (retVal == IDM_EXIT) {
					//Will generate a WM_DESTROY message
					delete mainWindow;
				} else if (retVal >= IDM_TYPING_SUBMENU_START) {
					//Switch the language, input manager, or output manager.
					if (customMenuItemsLookup.count(retVal)==0)
						throw std::exception("Bad menu item");
					WZMenuItem* currItem = customMenuItemsLookup[retVal];
					if (!currItem->disabled) {
						//Reset the help keyboard
						if ((currItem->type==WZMI_LANG) || (currItem->type==WZMI_INPUT))
							helpKeyboard->clearAllMemoryEntries();

						//First, disable the Help Keyboard if it's showing.
						if (currHelpInput!=NULL && helpKeyboard->isHelpEnabled()) {
							toggleHelpMode(false);
							checkAllHotkeysAndWindows();
						}

						if (currItem->type==WZMI_LANG)
							ChangeLangInputOutput(currItem->id, L"", L"");
						else if (currItem->type==WZMI_INPUT)
							ChangeLangInputOutput(L"", currItem->id, L"");
						else if (currItem->type==WZMI_OUTPUT)
							ChangeLangInputOutput(L"", L"", currItem->id);
					}
				}

				//Fixes a bug re: MSKB article: Q135788
				PostMessage(hwnd, 0, 0, 0);
			}

			break;
		}
		case WM_DESTROY:
		{
			//Cleanup
			if (!mainWindow->unregisterHotKey(LANG_HOTKEY))
				MessageBox(NULL, _T("Main Hotkey remains..."), _T("Warning"), MB_ICONERROR | MB_OK);
			if (mmOn) {
				if (!turnOnAlphaHotkeys(false, true, true))
					MessageBox(NULL, _T("Some hotkeys remain..."), _T("Warning"), MB_ICONERROR | MB_OK);
			}

			//Remove systray icon
			NOTIFYICONDATA nid;
			mainWindow->initShellNotifyIconData(nid);
			nid.uID = STATUS_NID;
			nid.uFlags = NIF_TIP; //??? Needed ???
			Shell_NotifyIcon(NIM_DELETE, &nid);


			//Delete our custom menu
			if (contextMenu!=NULL)
				DestroyMenu(contextMenu);

			//Close our thread, delete our critical section
			if (highlightKeys) {
				DeleteCriticalSection(&threadCriticalSec);
				CloseHandle(keyTrackThread);

				//This should already be closed; closing it twice is an error.
				//CloseHandle(caretTrackThread);  //Leave commented...
			}

			break;
		}
		default:
			return 1; //1 ==> not done
	}

	return 0;
}


void buildFilePathNames()
{
	//Compounded
	pathMainConfig = cfgDir + fs + cfgFile;
	//TODO: Find a nice way to move more of the next function up here.

	//Find the local config path & folder; even if the folder might not exist yet.
	wchar_t localAppPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, localAppPath))) {
		//Try to create the folder if it doesn't exist
		try {
			pathLocalFolder = waitzar::escape_wstr(wstring(localAppPath), true) + fs + "WaitZar";
			pathLocalConfig = pathLocalFolder + fs + "config.override.json.txt";
			pathLocalTinySave = pathLocalFolder + fs + "tinysave.txt";
			pathLocalLastSavedVersionInfo = pathLocalFolder + fs + "waitzar_version.txt";
		} catch (std::exception ex) {}
	}

	//Find the user config path
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, localAppPath))) {
		//Create the path
		try {
			pathUserConfig = waitzar::escape_wstr(localAppPath, true) + fs + "waitzar.config.json.txt";
		} catch (std::exception ex) {}
	}
}



bool findAndLoadAllConfigFiles()
{
	//Find all config files
	bool suppressThisException = false;
	map<wstring, vector<wstring> > lastUsedSettings;
	try {
		//Build up known path names, save globally for future use.
		buildFilePathNames();

		//First, load the flash-save and parse it.
		try {
			//Get the file.
			FlashLoadState(lastUsedSettings);
		} catch (std::exception ex) {
			//Doesn't much matter if this fails; just pass an empty map.
			lastUsedSettings.clear();
		}

		//Set the main config file
		config.initMainConfig(pathMainConfig);

		//Browse for all language directories, add them
		vector<string> langFolderNames = GetConfigSubDirs(cfgDir, cfgFile);
		for (vector<string>::iterator fold = langFolderNames.begin(); fold!=langFolderNames.end(); fold++) {
			//Get the main language config file
			string langCfgDir;
			string langCfgFile;
			vector<string> langModuleCfgFiles;
			std::stringstream errorMsg;
			try {
				langCfgDir = cfgDir + fs + *fold;
				langCfgFile = langCfgDir + fs + cfgFile;
			} catch (std::exception ex) {
				//NOTE: This catch statement won't catch non-unicode names anymore; we just silently ignore them.
				//      We should find a nicer way of re-enabling this.
				errorMsg << "Error loading config file for language: " <<*fold;
				errorMsg << std::endl << "Details: " << std::endl << ex.what();
			}

			//Now, get the sub-config files
			vector<string> modFolders = GetConfigSubDirs(langCfgDir, cfgFile);
			for (vector<string>::iterator mod = modFolders.begin(); mod!=modFolders.end(); mod++) {
				try {
					string modCfgFile = langCfgDir + fs + *mod + fs + cfgFile;
					langModuleCfgFiles.push_back(modCfgFile);
				} catch (std::exception ex) {
					//NOTE: This statement also won't catch...
					errorMsg << "Error loading config file for language: " <<*fold;
					errorMsg << std::endl << "and module: " <<*mod;
					errorMsg << std::endl << "Details: " << std::endl << ex.what();
					break;
				}
			}

			//Handle errors:
			//TODO: Better. (Check, e.g., Unicode directory names.)
			//if (inError)
			//	throw std::exception(errorMsg.str().c_str());

			config.initAddLanguage(langCfgFile, langModuleCfgFiles);
		}



		//TODO: Add SHGetKnownFolderPath() if on Vista, keep SHGetFolderPath if on XP or less.



		//Now, load the local config file
		if (!pathLocalFolder.empty() && !pathLocalConfig.empty()) {
			//Try to create the folder if it doesn't exist
			std::wstringstream temp;
			temp << pathLocalFolder.c_str();
			if (!FileExists(temp.str()))
				CreateDirectory(temp.str().c_str(), NULL);

			//Does the config FILE exist?
			temp.str(L"");
			temp << pathLocalConfig.c_str();
			if (FileExists(temp.str()))
				config.initLocalConfig(pathLocalConfig);
			else {
				//Create the file
				std::ofstream emptyConfig;
				emptyConfig.open(temp.str().c_str());

				//Add an empty set of parameters "{}" and some comments.
				emptyConfig << "# This file contains application-specific overrides for the WaitZar" <<std::endl
					<< "# typing program. Please do not edit this file. To provide user" <<std::endl
					<< "# overrides, edit \"waitzar.config.json.txt\", located in your \"Documents\"" <<std::endl
					<< "# or \"My Documents\" folder." <<std::endl
					<< "{" <<std::endl
					<< std::endl
					<< "}" <<std::endl;

				//Save
				emptyConfig.flush();
				emptyConfig.close();
			}
		}


		//And finally, the user config file.
		if (!pathUserConfig.empty()) {
			//Does it exist?
			std::wstringstream temp;
			temp << pathUserConfig.c_str();
			if (FileExists(temp.str()))
				config.initUserConfig(pathUserConfig);
			else {
				//Create the file
				std::ofstream emptyConfig;
				emptyConfig.open(temp.str().c_str());

				//Add an empty set of parameters "{}" and some comments.
				emptyConfig << "# This file contains user-specific settings for the WaitZar" <<std::endl
					<< "# typing program. These are the highest-level overrides possible;" <<std::endl
					<< "# to set an option, provide its full path and value. Separate" <<std::endl
					<< "# different settings with commas." <<std::endl
					<< "{" <<std::endl
					<< "  # Example: un-comment the following two lines (remove the \"#\")" <<std::endl
					<< "  #  to activate these settings:" <<std::endl
					<< std::endl
					<< "  #\"settings.track-caret\" : \"no\"," <<std::endl
					<< "  #\"settings.hotkey\" : \"Ctrl + Space\"" <<std::endl
					<< "}" <<std::endl;

				//Save
				emptyConfig.flush();
				emptyConfig.close();
			}
		}

		Logger::markLogTime('L', L"Config files loaded");

		//First test: does "config" not exist at all? If so, throw a special exception,
		//  and avoid the warning message box.
		std::wstringstream temp;
		temp << cfgDir.c_str();
		if (!FileExists(temp.str())) {
			suppressThisException = true;
			throw std::exception("No config directory");
		}

		//Final test: make sure all config files work
		Logger::startLogTimer('L', L"Reading & validating config files");
		config.validate(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard, lastUsedSettings);
		Logger::endLogTimer('L');
		Logger::markLogTime('L', L"Config files validated");
	} catch (std::exception& ex) {
		//In case of errors, just reset & use the embedded file
		config = ConfigManager(getMD5Hash);

		//Inform the user, UNLESS they set the magic number...
		if (!suppressThisException) {
			std::wstringstream msg;
			msg << "Error loading one of your config files.\nWaitZar will use the default configuration.\n\nDetails:\n";
			msg << ex.what();
			MessageBox(NULL, msg.str().c_str(), L"Config File Error", MB_ICONWARNING | MB_OK);
		}


		//Try one more time, this time with the default config file.
		try {
			//Load the resource as a byte array and get its size, etc.
			HRSRC res = FindResource(hInst, MAKEINTRESOURCE(IDR_DEFAULT_CONFIG), _T("Model"));
			if (!res)
				throw std::exception("Couldn't find resource WZ_DEFAULT_CFG.");
			HGLOBAL res_handle = LoadResource(NULL, res);
			if (!res_handle)
				throw std::exception("Couldn't get a handle on WZ_DEFAULT_CFG.");
			char* res_data = (char*)LockResource(res_handle);
			DWORD res_size = SizeofResource(NULL, res);

			//Convert the byte array to unicode
			wchar_t *uniData = new wchar_t[res_size];
			if (mymbstowcs(uniData, res_data, res_size)==0)
				throw std::exception("Invalid unicode character in WZ_DEFAULT_CFG.");

			//Set the config file
			config.initMainConfig(wstring(uniData));

			//Reclaim memory and system resources.
			//delete [] res_data;
			delete [] uniData;
			//UnlockResource(res_handle);
			Logger::markLogTime('L', L"Config files loaded: DEFAULT is taking over");

			//One more test.
			config.validate(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard, lastUsedSettings);
			Logger::markLogTime('L', L"Config files validated: DEFAULT is taking over");
		} catch (std::exception& ex2) {
			std::wstringstream msg2;
			msg2 << "Error loading default config file.\nWaitZar will not be able to function, and is shutting down.\n\nDetails:\n";
			msg2 << ex2.what();
			MessageBox(NULL, msg2.str().c_str(), L"Default Config Error", MB_ICONERROR | MB_OK);
			return false;
		}
	}

	return true;
}



void createPaintResources()
{
	cr_MenuItemBkgrd = RGB(0x8F, 0xA1, 0xF8);
	cr_MenuDefaultBkgrd = GetSysColor(COLOR_MENU);
	cr_MenuDefaultText = GetSysColor(COLOR_MENUTEXT);
	cr_MenuDefaultDisabledText = RGB(0x6D, 0x6D, 0x6D);
	cr_MenuItemText = RGB(0x20, 0x31, 0x89);
	cr_MenuItemDisabledText = RGB(0x4D, 0x4D, 0xBD);
	cr_Black = RGB(0x00, 0x00, 0x00);
	cr_White = RGB(0xFF, 0xFF, 0xFF);
	g_WhiteBkgrd = CreateSolidBrush(RGB(255, 255, 255));
	g_DarkGrayBkgrd = CreateSolidBrush(RGB(128, 128, 128));
	g_BlackBkgrd = CreateSolidBrush(RGB(0, 0, 0));
	g_YellowBkgrd = CreateSolidBrush(RGB(255, 255, 0));
	g_GreenBkgrd = CreateSolidBrush(RGB(0, 128, 0));
	g_RedBkgrd = CreateSolidBrush(RGB(128, 0, 0));
	g_DlgHelpBkgrd = CreateSolidBrush(RGB(0xEE, 0xFF, 0xEE));
	g_MenuItemBkgrd = CreateSolidBrush(cr_MenuItemBkgrd);
	g_MenuDefaultBkgrd = CreateSolidBrush(cr_MenuDefaultBkgrd);
	g_MenuItemHilite = CreateSolidBrush(RGB(0x07, 0x2B, 0xE4));
	g_DlgHelpSlash = CreateSolidBrush(RGB(0xBB, 0xFF, 0xCC));
	g_DotHiliteBkgrd = CreateSolidBrush(RGB(0x00, 0x1A, 0x7A));
	g_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
	g_BlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	g_MediumGrayPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
	g_WhiteThickPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	g_DotHilitePen = CreatePen(PS_SOLID, 1, RGB(0x42, 0x5D, 0xBC));
	g_EmptyPen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));
}


//Temp code; borrowed for now:
//TODO: Test & replace with waitzar::wcs2mbs()
std::string to_utf8(const wchar_t* buffer, int len) {
  int nChars = ::WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
  if (nChars == 0) return "";
  string newbuffer;
  newbuffer.resize(nChars) ;
  WideCharToMultiByte(CP_UTF8, 0, buffer, len, const_cast< char* >(newbuffer.c_str()), nChars, NULL, NULL);
  return newbuffer;
}
std::string to_utf8(const std::wstring& str) { return to_utf8(str.c_str(), (int)str.size()); }



bool checkUserSpecifiedRegressionTests(wstring testFileName)
{
	//Anything to process?
	if (testFileName.empty())
		return false;

	//Main processing loop
	wstring outFileName;
	ofstream outFile;  //Don't open unless we need it.
	unsigned int numErrors = 0;
	unsigned int numTests = 0;
	try {
		//Open our test file, read it into a Key Magic Keyboard
		KeyMagicInputMethod testFile;
		testFile.loadTextRulesFile(waitzar::escape_wstr(testFileName, false));

		//Now, check for various options
		const std::wstring& language = testFile.getOption(L"language");
		const std::wstring& inputMethod = testFile.getOption(ConfigManager::sanitize_id(L"input-method"));
		const std::wstring& outEncoding = testFile.getOption(ConfigManager::sanitize_id(L"output-encoding"));
		if (language.empty())
			throw std::exception("Invalid test file: missing the \"language\" option.");
		if (inputMethod.empty())
			throw std::exception("Invalid test file: missing the \"input-method\" option.");
		if (outEncoding.empty())
			throw std::exception("Invalid test file: missing the \"output-encoding\" option.");

		//Convert all Rules to "single string" format; this will test them, too.
		vector< pair<wstring, wstring> > testPairs = testFile.convertToRulePairs();

		//Set the language, input method, and output encoding.
		//First check if they exist, though.
		if (FindKeyInSet(config.getLanguages(), language)==config.getLanguages().end())
			throw std::exception(waitzar::glue(L"Unknown language: \"", language, L"\"").c_str());
		if (FindKeyInSet(config.getInputMethods(), inputMethod)==config.getInputMethods().end())
			throw std::exception(waitzar::glue(L"Unknown input method: \"", inputMethod, L"\"").c_str());
		if (FindKeyInSet(config.getEncodings(), outEncoding)==config.getEncodings().end())
			throw std::exception(waitzar::glue(L"Unknown output encoding: \"", outEncoding, L"\"").c_str());
		ChangeLangInputOutput(language, inputMethod, outEncoding);

		//Construct the output file name
		size_t lastSlash = string::npos;
		{
			size_t lastSlash1 = testFileName.rfind(L"/");
			size_t lastSlash2 = testFileName.rfind(L"\\");
			if (lastSlash1!=string::npos && lastSlash2!=string::npos)
				lastSlash = std::max<size_t>(lastSlash1, lastSlash2);
			else
				lastSlash = std::min<size_t>(lastSlash1, lastSlash2);
		}
		size_t lastDot = testFileName.rfind(L".");
		if (lastDot<lastSlash || lastDot==string::npos)
			outFileName = testFileName + L".results";
		else
			outFileName = testFileName.substr(0, lastDot) + L".results" + testFileName.substr(lastDot, testFileName.size());


		//Delete the old error file, if it exists
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		std::wstringstream temp;
		temp <<outFileName.c_str();
		if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==TRUE)
			remove(waitzar::escape_wstr(outFileName, false).c_str()); //Will only fail if the file doesn't exist anyway.


		//Now, let's track our errors!
		for (vector< pair<wstring, wstring> >::iterator currTest = testPairs.begin(); currTest!=testPairs.end(); currTest++) {
			//Reset the input method
			currInput->reset(true, true, true, true);

			//Now, type each letter in the test sequence
			for (size_t i=0; i<currTest->first.length(); i++) {
				//Construct a VirtKey. Do some extra checking to avoid silent errors.
				wchar_t wc = currTest->first[i];
				if (wc >= 0x7F)
					throw std::exception("Error: Cannot type unicode sequences as input.");
				char c = (wc&0x7F);
				VirtKey vk(c);
				if (vk.vkCode==0)
					throw std::exception(waitzar::glue("Error: Unknown input letter: ", string(1, c)).c_str());
				currInput->handleKeyPress(vk);
			}

			//Retrieve the output, in the correct encoding.
			bool noEncChange = (uni2Output->toEncoding==currInput->encoding);
			wstring resOut = currInput->getTypedSentenceStrings()[3];
			if (!noEncChange) {
				input2Uni->convertInPlace(resOut);
				uni2Output->convertInPlace(resOut);
			}

			//Now, compare
			numTests++;
			if (currTest->second != resOut) {
				//First time? Then init our file
				if (numErrors++ == 0) {
					outFile.open(outFileName.c_str(), std::ios::out|std::ios::binary); //binary needed for utf-8
					if (outFile.fail())
						throw std::exception(waitzar::glue(L"Cannot open output file: ", outFileName).c_str());
					outFile <<"Test results\n-----------\n";
					outFile.flush();
				}

				//Print our test results
				outFile <<to_utf8(currTest->first)
					<<" => " <<to_utf8(currTest->second)
					<<std::endl
					<<"   Actual result: "
					<<to_utf8(resOut)
					<<std::endl;
				outFile.flush();
			}
		}


		//Close output file
		if (outFile.is_open())
			outFile.close();
	} catch (std::exception ex) {
		wstringstream msg;
		msg <<L"Error running test file: \"" <<testFileName <<L"\"" <<std::endl;
		msg <<L"WaitZar will now exit.\n\nDetails: " <<ex.what() <<std::endl;
		MessageBox(NULL, msg.str().c_str(), L"Testing Error", MB_ICONERROR | MB_OK);

		//Close output file
		if (outFile.is_open())
			outFile.close();
		return true;
	}

	//Prepare verbs, etc.
	wstring werewas = numErrors==1 ? L"was" : L"were";
	wstring plural = numErrors==1 ? L"" : L"s";

	wstringstream msg;
	msg <<"Test complete.";
	if (numErrors==0)
		msg <<"\n\n   No errors detected." <<std::endl;
	else {
		msg <<"\n\nThere " <<werewas <<" " <<numErrors <<" error" <<plural <<".\n";
		msg <<"Total test" <<plural <<": " <<numTests <<"\n";
		msg <<"Log saved to: \n\n   " <<outFileName <<std::endl;
	}
	MessageBox(NULL, msg.str().c_str(), L"Test Results", MB_ICONINFORMATION | MB_OK);

	return true;
}



/**
 * Main method for Windows applications
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Init log:
	Logger::resetLogFile('L');
	Logger::startLogTimer('L', L"Starting WaitZar {");
	Logger::startLogTimer('L', L"Trace");

	//First and foremost
	helpIsCached = false;
	isDragging = false;
	helpKeyboard = NULL;
	contextMenu = NULL;
	customMenuItems = NULL;
	menuFont = NULL;
	sysFont = NULL;

	//Save for later; if we try retrieving it, we'll just get a bunch of conversion
	//  warnings. Plus, the hInstance should never change.
	hInst = hInstance;


	//Parse the command line
	wstring testFileName;
	{
		std::wstringstream tempW;
		tempW <<lpCmdLine;
		wstring cmdLine = tempW.str();

		//Find the parameter -t or -test, followed by a space
		size_t cmdID = std::min<size_t>(cmdLine.find(L"-t "), cmdLine.find(L"-test "));
		if (cmdID != std::string::npos) {
			cmdID = cmdLine.find(L" ", cmdID) + 1;

			//Build it
			wstringstream val;
			bool inQuote = false;
			for (size_t i=cmdID; i<cmdLine.length(); i++) {
				if (cmdLine[i]==L' ' && !inQuote)
					break;
				if (cmdLine[i]==L'"')
					inQuote = !inQuote;
				else
					val <<cmdLine[i];
			}
			testFileName = val.str();
		}
	}


	Logger::markLogTime('L', L"Command Line Parsed");

////////////////
//DEBUG
////////////////
//testFileName = L"../test_cases/ayar_tests.txt";
////////////////
////////////////


	//Create our brushes and pens
	createPaintResources();


	//Create our windows so that they exist in memory for config.validate()
	mainWindow = new MyWin32Window(L"waitZarMainWindow");
	sentenceWindow = new MyWin32Window(L"waitZarSentenceWindow");
	helpWindow = new MyWin32Window(L"waitZarHelpWindow");
	memoryWindow = new MyWin32Window(L"waitZarMemoryWindow");


	//Load our configuration file now; save some headaches later
	//NOTE: These are the OLD config settings; we should be able to remove them eventually.
	loadConfigOptions();


	//Give this process a low background priority
	//  NOTE: We need to balance this eventually.
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);


	//Initialize all our windows.
	try {
		//Init
		mainWindow->init(L"WaitZar", WndProc, g_DarkGrayBkgrd, hInst, 100, 100, 240, 120, positionAtCaret, onAllWindowsCreated, false);
		sentenceWindow->init(L"WaitZar", NULL, g_DarkGrayBkgrd, hInst, 100, 100+mainWindow->getDefaultHeight(), 300, 26, positionAtCaret, onAllWindowsCreated, false);
		helpWindow->init(L"WaitZar", NULL, g_GreenBkgrd, hInst, 400, 300, 200, 200, NULL, onAllWindowsCreated, true);
		memoryWindow->init(L"WaitZar", NULL, g_GreenBkgrd, hInst, 400, 300, 200, 200, NULL, onAllWindowsCreated, true);
	} catch (std::exception err) {
		std::wstringstream msg;
		msg << "Error creating WaitZar's windows.\nWaitZar will now exit.\n\nDetails:\n";
		msg << err.what();
		MessageBox(NULL, msg.str().c_str(), L"CreateWindow() Error", MB_ICONERROR | MB_OK);
		return 0;
	}

	//"And another thing!"
	RECT r = {1,1,2,2}; //Just an empty rectangle for now; we'll update it as necessary
	changeEncRegionHandle = sentenceWindow->subscribeRect(r, OnEncodingChangeClick);

	Logger::markLogTime('L', L"Windows Initialized");

	//Find all config files, load.
	Logger::startLogTimer('L', L"Detecting & loading config files");
	if (!findAndLoadAllConfigFiles())
		return 0;
	Logger::endLogTimer('L');

	Logger::markLogTime('L', L"Config files located, loaded, and validated.");


	//Link windows if necessary.
	if (config.getSettings().lockWindows)
		mainWindow->linkToWindow(sentenceWindow, SOUTH);


	//Should we run a UAC test on startup?
	if (config.getSettings().alwaysElevate) {
		//Will elevating help?
		if (IsVistaOrMore() && !IsAdmin()) {
			TCHAR szCurFileName[1024];
            GetModuleFileName(GetModuleHandle(NULL), szCurFileName, 1023);
			if (!elevateWaitZar(szCurFileName))
				MessageBox(NULL, _T("Could not elevate WaitZar. Program will now exit."), _T("Error!"), MB_ICONERROR | MB_OK);

			//Return either way; the current thread must exit.
			return 0;
		}

		Logger::markLogTime('L', L"UAC managed");
	}



	//Create our context menu
	try {
		Logger::startLogTimer('L', L"Generating context menu font");
		initContextMenu();
		Logger::endLogTimer('L');
	} catch (std::exception ex) {
		wstringstream msg;
		msg <<ex.what();
		mainWindow->showMessageBox(msg.str(), L"Context Menu Error", MB_ICONERROR|MB_OK);
		return 0;
	}

	Logger::markLogTime('L', L"Context menu font created");

	//Load some icons...
	mmIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MYANMAR), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	engIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ENGLISH), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16

	//Make our "notify icon" data structure
	NOTIFYICONDATA nid;
	mainWindow->initShellNotifyIconData(nid);
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.uVersion = NOTIFYICON_VERSION; //Win2000+ style notifications
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_LOADING), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...

	//Error checking..
	if (mmIcon == NULL || engIcon==NULL)
		MessageBox(NULL, _T("Unable to load Icon!"), _T("Warning"), MB_ICONWARNING | MB_OK);

	Logger::markLogTime('L', L"Icon resources loaded");

	//Set our hotkey
	if(!registerInitialHotkey()) {
		//Check if we're running Wait Zar already
		if (waitzarAlreadyStarted()==TRUE) {
			MessageBox(NULL, _T("Wait Zar is already running. \n\nYou should see an \"ENG\" icon in your system tray; click on that to change the language. \n\nPlease see the Wait Zar User's Guide if you have any questions.  \n\n(If you are certain WaitZar is not actually running, please wait several minutes and then re-start the program.)"), _T("Wait Zar already running..."), MB_ICONINFORMATION | MB_OK);
			return 0;
		}
		MessageBox(NULL, _T("The main language shortcut could not be set up.\nWait Zar will not function properly."), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
	}
	mmOn = false;

	Logger::markLogTime('L', L"Initial hotkey registered");

	//Support for balloon tooltips
	if (config.getSettings().balloonStart) {
		nid.uFlags |= NIF_INFO;
		lstrcpy(nid.szInfoTitle, _T("Welcome to WaitZar"));
		if (testFileName.empty())
			swprintf(nid.szInfo, _T("Hit %ls to switch to Myanmar.\n\nClick this icon for more options."), hkString.c_str());
		else
			swprintf(nid.szInfo, _T("WaitZar is running regression tests. \n\nPlease wait for these to finish."));
		//nid.uTimeout = 20; //Timeout is invalid as of vista
		nid.uVersion = NOTIFYICON_VERSION;
		nid.dwInfoFlags = NIIF_INFO; //Can we switch to NIIF_USER if supported?
	}

	//Add our icon to the tray
	Shell_NotifyIcon(NIM_ADD, &nid);

	Logger::markLogTime('L', L"Tray icon added");

	//Initialize our keyboard input structures
	for (int i=0; i<1000; i++) {
		//We expect an input of type "keyboard"
		inputItems[i].type = INPUT_KEYBOARD;

		//Specify unicode (wVk MUST be zero)
		keyInputPrototype.dwFlags=KEYEVENTF_UNICODE;
		keyInputPrototype.wVk=0;

		//Have the system provide its own timestamp
		keyInputPrototype.time=0;

		//No extra info
		keyInputPrototype.dwExtraInfo=0;
	}

	Logger::markLogTime('L', L"Keyboard input struct initialized (is this needed?)");

	//Success?
	if(mainWindow->isInvalid() || sentenceWindow->isInvalid() || helpWindow->isInvalid() || memoryWindow->isInvalid()) {
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Now that everything has been loaded, run any user-driven tests
	if (checkUserSpecifiedRegressionTests(testFileName)) {
		//Cleanly exit. (Sort of... this might be bad since no messages are being pumped).
		Logger::markLogTime('L', L"User regression tests run.");
		delete mainWindow;
	}


	//Check if the current version is up-to-date.
	//TODO: Thread this call.
	//CheckCurrentVersion();



	//Set the locale to the current system locale.
	//VirtKey::SetCurrLocale(LOWORD(GetKeyboardLayout(0)));
	//TODO: Check IsCurrLocaleInsufficient() and show a balloon (or something) if this is the case.

	//Set defaults
	Logger::startLogTimer('L', L"Starting default language");
	logLangChange = true;
	ChangeLangInputOutput(config.activeLanguage.id, config.activeInputMethod->id, config.activeOutputEncoding.id);
	logLangChange = false;
	Logger::endLogTimer('L');
	Logger::markLogTime('L', L"Default language set");


	//Testing mywords?
	/*if (currTest == mywords)
		GetSystemTimeAsFileTime(&startTime);*/


	//Logging mywords?
	if (currTest == mywords) {
		/*GetSystemTimeAsFileTime(&endTime);*/
		DWORD timeMS = 0; /*getTimeDifferenceMS(startTime, endTime);*/


		wchar_t msg[500];
		swprintf(msg, L"Mywords total time:   %dms", timeMS);
		MessageBox(NULL, msg, L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);

		return 0;
	}



	//Create (but don't start) our thread tracker
	if (highlightKeys) {
		//Initialize our critical section
		InitializeCriticalSection(&threadCriticalSec);
		threadIsActive = false;

		keyTrackThread = CreateThread(
			NULL,                //Default security attributes
			0,                   //Default stack size
			TrackHotkeyReleases, //Threaded function (name)
			NULL,                //Arguments to threaded function
			CREATE_SUSPENDED,    //Don't start this thread when it's created
			&keyTrackThreadID);  //Pointer to return the thread's id into
		if (keyTrackThread==NULL) {
			MessageBox(NULL, _T("WaitZar could not create a helper thread. \nThis will not affect normal operation; however, it means that WaitZar will not be able to highlight keys as you press them, which is a useful benefit for beginners."), _T("Warning"), MB_ICONWARNING | MB_OK);
			highlightKeys = false;
		}

		Logger::markLogTime('L', L"Key highlight thread created");
	}


	//Create (but don't start) our version checking thread
	if (checkLatestVersion) {
		checkVersionThread = CreateThread(
			NULL,                //Default security attributes
			0,                   //Default stack size
			CheckForNewVersion,  //Threaded function (name)
			NULL,                //Arguments to threaded function
			CREATE_SUSPENDED,    //Don't start this thread when it's created
			&checkVersionThreadID);  //Pointer to return the thread's id into
		if (checkVersionThread==NULL) {
			MessageBox(NULL, _T("WaitZar could not create a helper thread. \nThis will not affect normal operation; however, it will prevent WaitZar from checking if there are new updates to the software."), _T("Warning"), MB_ICONWARNING | MB_OK);
			checkLatestVersion = false;
		}

		Logger::markLogTime('L', L"Version checking thread created.");
	}

	//Check for a new version of WZ.
	// (This should trigger the thread to sleep?)
	ResumeThread(checkVersionThread);


	//Potential debug loop (useful)
	if (currTest == model_print) {
		//logFile = fopen("wz_log.txt", "w");

		//model->debugOut(logFile);

		MessageBox(NULL, L"Model saved to output.", _T("Notice"), MB_ICONERROR | MB_OK);
		return 1;
	}



	//Show it's ready by changing the shell icon
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.uVersion = NOTIFYICON_VERSION; //Win2000+ version notifications
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	nid.hIcon = engIcon;
	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE) {
		TCHAR eTemp[200];
		swprintf(eTemp, _T("Can't load initial icon.\nError code: %x"), GetLastError());
		MessageBox(NULL, eTemp, _T("Warning"), MB_ICONERROR | MB_OK);
	}

	Logger::markLogTime('L', L"System tray icon set to \"ready\"");


	//Logging time to start?
	if (currTest == start_up) {
		//GetSystemTimeAsFileTime(&endTime);
		DWORD timeMS = 0; /*getTimeDifferenceMS(startTime, endTime);*/

		wchar_t msg[500];
		swprintf(msg, L"Time to start up:   %dms", timeMS);
		MessageBox(NULL, msg, L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);

		return 0;
	}


	//Logging total time to type all words?
	if (currTest == type_all) {
		//if (!testAllWordsByHand())
		MessageBox(NULL, L"Error running type_all check: currently disabled!", L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);
		return 0;
	}

	//Done
	Logger::endLogTimer('L');
	Logger::markLogTime('L', L"WaitZar is now running");
	Logger::endLogTimer('L', L"}");


	//Main message handling loop
	MSG Msg;
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	//Done
	return (int)Msg.wParam;
}


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
