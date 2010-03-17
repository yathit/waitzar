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

//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
//#define _CRT_NON_CONFORMING_SWPRINTFS

//Define to require a specific version of Windows.
#define _WIN32_WINNT 0x0500 //Run on Windows 2000, XP, and Vista (haven't tested NT or the "server"s yet)
#define _WIN32_IE 0x0500    //I don't understand why I need this, but the speech balloons won't compile unless I have it.
//#define _WIN32_WINNT 0x0410 //Run on Windows 98+, fails for KEYBOARD_INPUT

//Slim-down our list of definitions. Would you believe that this causes NO
//  noticeable size reduction on Windows XP, VS2003? Perhaps it helps
//  on Vista....
//Anyway, if you add a new function and get an "undefined" error, comment
//  the relevant #define out.
#define NOGDICAPMASKS       //- CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOKEYSTATES         //- MK_*
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
#define NOWINOFFSETS        //- GWL_*, GCL_*, associated routines
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
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <list>
#include <limits>
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

//VS Includes
#include "resource.h"


using namespace waitzar;
using std::string;
using std::wstring;
using std::vector;
using std::list;
using std::pair;
using std::wstringstream;


//Current version
const wstring WAIT_ZAR_VERSION = L"1.7";

//Menu item texts
const wstring POPUP_UNI = L"Unicode 5.1";
const wstring POPUP_ZG = L"Zawgyi-One";
const wstring POPUP_WIN = L"Win Innwa";
const wstring POPUP_LOOKUP_MM = L"&Look Up Word (F1)";
const wstring POPUP_LOOKUP_EN = L"&Look Up Word";

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
const unsigned int DYNAMIC_CMD_START = 50000;
const wstring WND_TITLE_LANGUAGE = L"\u1018\u102C\u101E\u102C\u1005\u1000\u102C\u1038";
const wstring WND_TITLE_INPUT = L"Input Method";
const wstring WND_TITLE_OUTPUT = L"Encoding";

//Font conversion
//wstring currEncStr;
ENCODING mostRecentEncoding = ENCODING_UNICODE;

//Brushes & Pens
HBRUSH g_WhiteBkgrd;
HBRUSH g_DarkGrayBkgrd;
HBRUSH g_BlackBkgrd;
HBRUSH g_YellowBkgrd;
HBRUSH g_GreenBkgrd;
HBRUSH g_DlgHelpBkgrd;
HBRUSH g_DlgHelpSlash;
HBRUSH g_MenuItemBkgrd;
HBRUSH g_MenuItemHilite;
HBRUSH g_MenuDefaultBkgrd;
HBRUSH g_DotHiliteBkgrd;
HPEN g_GreenPen;
HPEN g_BlackPen;
HPEN g_MediumGrayPen;
HPEN g_EmptyPen;
HPEN g_DotHilitePen;

//Colors
COLORREF cr_MenuItemBkgrd;
COLORREF cr_MenuItemText;
COLORREF cr_MenuDefaultBkgrd;
COLORREF cr_MenuDefaultText;
COLORREF cr_White;
COLORREF cr_Black;

//Global Variables
HINSTANCE hInst;
HICON mmIcon;
HICON engIcon;
//WordBuilder *model;

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



//TODO: There are still some places where we use a font without coloring it.
//      For reference, here are all the old colors:
/*
mmFontGreen->tintSelf(0x008000);
mmFontBlack->tintSelf(0x000000);
mmFontRed->tintSelf(0xFF0000);
mmFontSmallWhite->tintSelf(0xFFFFFF);
mmFontSmallGray->tintSelf(0x333333);
mmFontSmallRed->tintSelf(0xFF0000);
*/



PAINTSTRUCT Ps;
WORD stopChar;
int numConfigOptions;
int numCustomWords;
INPUT inputItems[2000];
KEYBDINPUT keyInputPrototype;
bool helpIsCached;
//string mywordsFileName = "mywords.txt";


//User-drawn menu data structures
enum WZMI_TYPES {WZMI_SEP, WZMI_HEADER, WZMI_LANG, WZMI_INPUT, WZMI_OUTPUT};
struct WZMenuItem {
	unsigned int menuID;
	WZMI_TYPES type;
	wstring id;
	wstring title;
	bool containsMM;

	WZMenuItem(){}

	WZMenuItem(unsigned int menuID, WZMI_TYPES type, const wstring&id, const wstring&title) {
		this->menuID = menuID;
		this->type = type;
		this->id = id;
		this->title = title;
		this->containsMM = false;
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
//Leave as pointers for now.
PulpCoreFont *helpFntKeys;
PulpCoreFont *helpFntFore;
PulpCoreFont *helpFntBack;
PulpCoreFont *helpFntMemory;
PulpCoreImage *helpCornerImg;
OnscreenKeyboard* helpKeyboard;

//Page down/up images
PulpCoreImage* pageImages[4] = {NULL, NULL, NULL, NULL};
enum { PGDOWNCOLOR_ID=0, PGUPCOLOR_ID=1, PGDOWNSEPIA_ID=2, PGUPSEPIA_ID=3 };

//BLENDFUNCTION BLEND_FULL = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA }; //NOTE: This requires premultiplied pixel values
//POINT PT_ORIGIN;
HANDLE keyTrackThread;   //Handle to our thread
DWORD  keyTrackThreadID; //Its unique ID (never zero)
CRITICAL_SECTION threadCriticalSec; //Global critical section object
list<unsigned int> hotkeysDown; //If a wparam is in this list, it is being tracked
bool threadIsActive; //If "false", this thread must be woken to do anything useful


//NOTE: A sep. Zawgyi list is not needed; all words are only stored in one encoding.
//vector<wstring> userDefinedWords; //Words the user types in. Stored with a negative +1 index
//vector<wstring> userDefinedWordsZg; //Cache of the Zawgyi version of the word typed


//User keystrokes
wstring userKeystrokeVector;

//Special resources for tracking the caret
//Note: This is run synchronously; it's spawned into its own thread just so we can
//      call "AttachThreadInput()"
HANDLE caretTrackThread;
DWORD caretTrackThreadID;
POINT caretLatestPosition;

//Help window colors
#define COLOR_HELPFNT_KEYS        0x606060
#define COLOR_HELPFNT_FORE        0x000000
#define COLOR_HELPFNT_BACK        0x0019FF

//Our configuration
ConfigManager config;

//Configuration variables.
//BOOL customDictWarning = FALSE;

//These two will take some serious fixing later.
TCHAR langHotkeyString[100];
char langHotkeyRaw[100];

//bool typePhrases = true;
bool dragBothWindowsTogether = true;
bool showBalloonOnStart = true;
bool alwaysRunElevated = false;
bool highlightKeys = true;
bool experimentalTextCursorTracking = true;
//bool dontLoadModel = false;
//bool allowNonBurmeseLetters = false;
//bool ignoreMywordsWarnings = false;
string fontFileRegular;
string fontFileSmall;


//Partially-managed windows
MyWin32Window* mainWindow = NULL;
MyWin32Window* sentenceWindow = NULL;
MyWin32Window* helpWindow = NULL;
MyWin32Window* memoryWindow = NULL;

//Avoid cyclical messaging:
bool mainWindowSkipMove = false;
bool senWindowSkipMove = false;


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
bool showingHelpPopup = false;


//Calculate's integers
int firstLineStart;
int secondLineStart;
int thirdLineStart;
int fourthLineStart;
int borderWidth = 2;
int spaceWidth;


//Log file, since the debugger doesn't like multi-process threads
bool isLogging = false;
FILE *logFile;

//For testing
FILETIME startTime;
FILETIME endTime;
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



unsigned long getTimeDifferenceMS(const FILETIME &st, const FILETIME &end)
{
	if (st.dwHighDateTime != end.dwHighDateTime)
		return std::numeric_limits<DWORD>::max();
	return (end.dwLowDateTime - st.dwLowDateTime)/10000L;
}



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
			for (std::list<unsigned int>::iterator keyItr = hotkeysDown.begin(); keyItr != hotkeysDown.end();) {
				//Get the state of this key
				SHORT keyState = GetKeyState(helpKeyboard->getVirtualKeyID(*keyItr)); //We need to use CAPITAL letters for virtual keys. Gah!

				if ((keyState & 0x8000)==0) {
					//Send a hotkey_up event to our window (mimic the wparam used by WM_HOTKEY)
					/*if (isLogging)
						fprintf(logFile, "  Key up: %c  (%x)\n", *keyItr, keyState);*/

					if (!mainWindow->postMessage(UWM_HOTKEY_UP, *keyItr, 0)) {
						//SendMessage(mainWindow, UWM_HOTKEY_UP, *keyItr,0); //Send message seems to make no difference
						MessageBox(NULL, _T("Couldn't post message to Main Window"), _T("Error"), MB_OK);
					}

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



//DirToCheck should be of searchable form:
//   c:\x...x
//   The \* will be appended
vector<wstring> GetConfigSubDirs(std::string dirToCheck, std::string configFileName)
{
	//Convert to wstring
	std::wstringstream dir;
	dir << dirToCheck.c_str();
	dir << "\\*";

	//First, just get all sub-directories
	vector<wstring> allDir;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(dir.str().c_str(), &FindFileData);
	if(hFind != INVALID_HANDLE_VALUE) {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			allDir.push_back(FindFileData.cFileName);
		while(FindNextFile(hFind, &FindFileData)) {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				allDir.push_back(FindFileData.cFileName);
		}
	}

	//Next, add only the directories (excluding . and ..) that contain configuration files.
	vector<wstring> resDir;
	for (size_t i=0; i<allDir.size(); i++) {
		//Easy
		wstring path = allDir[i];
		if (path == L"." || path == L"..")
			continue;

		//Harder
		std::wstringstream newpath;
		newpath << dirToCheck.c_str();
		newpath << "/" << path;
		newpath << "/" << configFileName.c_str();
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		if (GetFileAttributesEx(newpath.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
			continue;

		resDir.push_back(allDir[i]);
	}
	return resDir;
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
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_HELP_KEY_FONT), _T("COREFONT"));
		if (!fontRes) {
			MessageBox(NULL, _T("Couldn't find WZ_HELP_KEY_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on WZ_HELP_KEY_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
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
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_HELP_FORE_FONT), _T("COREFONT"));
		if (!fontRes) {
			MessageBox(NULL, _T("Couldn't find WZ_HELP_FORE_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on WZ_HELP_FORE_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
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
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_HELP_BACK_FONT), _T("COREFONT"));
		if (!fontRes) {
			MessageBox(NULL, _T("Couldn't find WZ_HELP_BACK_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on WZ_HELP_BACK_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
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
		HRSRC imgRes = FindResource(hInst, MAKEINTRESOURCE(WZ_HELP_CORNER), _T("COREFONT"));
		if (!imgRes) {
			MessageBox(NULL, _T("Couldn't find WZ_HELP_CORNER"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, imgRes);
		if (!res_handle) {
			MessageBox(NULL, _T("Couldn't get a handle on WZ_HELP_CORNER"), _T("Error"), MB_ICONERROR | MB_OK);
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

	//Load our page down/up images
	for (int i=0; i<4; i++) {
		//First the resource
		int PG_RES_ID = i==0?WZ_PGDOWN_COLOR:i==1?WZ_PGUP_COLOR:i==2?WZ_PGDOWN_SEPIA:WZ_PGUP_SEPIA;
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
	}
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
	lstrcpy(langHotkeyString, _T("Ctrl+Shift"));
	strcpy(langHotkeyRaw, "^+");

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
			numConfigOptions++;
			/*if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				customDictWarning = TRUE;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				customDictWarning = FALSE;
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "lockwindows")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				dragBothWindowsTogether = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				dragBothWindowsTogether = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "powertyping")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typePhrases = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typePhrases = false;
			else
				numConfigOptions--;*/
		} else if (strcmp(name, "burmesenumerals")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typeBurmeseNumbers = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typeBurmeseNumbers = false;
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "ballooononstart")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				showBalloonOnStart = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				showBalloonOnStart = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "alwayselevate")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				alwaysRunElevated = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				alwaysRunElevated = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "trackcaret")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				experimentalTextCursorTracking = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				experimentalTextCursorTracking = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "ignoremodel")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				dontLoadModel = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				dontLoadModel = false;
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "silencemywordserrors")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				ignoreMywordsWarnings = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				ignoreMywordsWarnings = false;
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "charaset")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "any")==0)
				allowNonBurmeseLetters = true;
			else if (strcmp(value, "burmese")==0 || strcmp(value, "myanmar")==0)
				allowNonBurmeseLetters = false;
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "defaultencoding")==0) {
			numConfigOptions++;
			/*if (strcmp(value, "wininnwa")==0)
				setEncoding(ENCODING_WININNWA);
			else if (strcmp(value, "zawgyi")==0)
				setEncoding(ENCODING_ZAWGYI);
			else if (strcmp(value, "unicode")==0 || strcmp(value, "parabaik")==0 || strcmp(value, "padauk")==0 || strcmp(value, "myanmar3")==0)
				setEncoding(ENCODING_UNICODE);
			else*/
				numConfigOptions--;
		} else if (strcmp(name, "hotkey")==0) {
			//Set it later
			strcpy(langHotkeyRaw, value);
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
	size_t str_len = strlen(langHotkeyRaw);

	//Now, set the keycode
	//Additional rule: all keystroke modifiers must also themselves be modifiers
	keycode = langHotkeyRaw[str_len-1];
	switch(keycode) {
		case '!':
			lstrcpy(langHotkeyString, _T("Alt"));
			keycode = VK_MENU; //VK_MENU == VK_ALT
			modifier |= MOD_ALT;
			break;
		case '^':
			lstrcpy(langHotkeyString, _T("Ctrl"));
			keycode = VK_CONTROL;
			modifier |= MOD_CONTROL;
			break;
		case '+':
			lstrcpy(langHotkeyString, _T("Shift"));
			keycode = VK_SHIFT;
			modifier |= MOD_SHIFT;
			break;
		case '_':
			lstrcpy(langHotkeyString, _T("Space"));
			keycode = VK_SPACE;
			break;
		default:
			swprintf(langHotkeyString, _T("%C"), keycode);
	}

	//Now, set the modifiers
	TCHAR* temp = new TCHAR[100];
	for (size_t pos=0; pos<str_len-1; pos++) {
		switch(langHotkeyRaw[pos]) {
			case '!':
				swprintf(temp, _T("Alt+%s"), langHotkeyString);
				lstrcpy(langHotkeyString, temp);
				modifier |= MOD_ALT;
				break;
			case '^':
				swprintf(temp, _T("Ctrl+%s"), langHotkeyString);
				lstrcpy(langHotkeyString, temp);
				modifier |= MOD_CONTROL;
				break;
			case '+':
				swprintf(temp, _T("Shift+%s"), langHotkeyString);
				lstrcpy(langHotkeyString, temp);
				modifier |= MOD_SHIFT;
				break;
		}
	}

	//Additional rule: Capital letters require a shift modifier
	if (keycode>='A' && keycode<='Z') {
		swprintf(temp, _T("Shift+%s"), langHotkeyString);
		lstrcpy(langHotkeyString, temp);
		modifier |= MOD_SHIFT;
	}

	//Additional rule: Lowercase letters are coded by their uppercase value
	if (keycode>='a' && keycode<='z') {
		keycode -= 'a'-'A';
	}

	//Reclaim memory
	delete [] temp;

	return mainWindow->registerHotKey(LANG_HOTKEY, modifier, keycode);
}


//Re-position this near the caret
void positionAtCaret()
{
	//Default "off" flag
	if (!experimentalTextCursorTracking)
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
		experimentalTextCursorTracking = false;
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
		currInput->reset(true, true, true, true);
	}

	//Turn on/of our main Help hotkey
	if (toMM)
		res = mainWindow->registerHotKey(HOTKEY_HELP, NULL, VK_F1) && res;
	else
		mainWindow->unregisterHotKey(HOTKEY_HELP);

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
		helpWindow->showWindow(false);
		memoryWindow->showWindow(false);
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
	helpFntMemory = (PulpCoreFont*)mmFontSmall;//new PulpCoreFont();

	//Make
	helpKeyboard = new OnscreenKeyboard((PulpCoreFont*)mmFontSmall, helpFntKeys, helpFntFore, helpFntBack, helpFntMemory, helpCornerImg);
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

	//Now, set the window's default height
	mainWindow->setDefaultSize(mainWindow->getDefaultWidth(), fourthLineStart);

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
	//TODO: Again, the "NULL"s will eventually have to go.
	int cumulativeWidth = (borderWidth+1)*2;
	for (size_t i=0; i<10; i++) {
		unsigned int id = i + currInput->getPagingInfo().first * 10;
		if (id>=dispCandidateStrs.size())
			break;
		cumulativeWidth += mmFont->getStringWidth(dispCandidateStrs[id].first, NULL);
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

	//If not, resize. Also, keep the size small when possible.
	// Note: Re-sizing to the same size won't trigger a window update, so we can just all expandWindow()
	//       without worrying about performance.
	int newWidth = max(mainWindow->getDefaultWidth(), cumulativeWidth);
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
		currPosX += mmFontSmall->getStringWidth(dispSentenceStr[0], NULL) + 1;
	mmFontSmall->setColor(0xFF, 0x00, 0x00);
	sentenceWindow->drawString(mmFontSmall, dispSentenceStr[1], currPosX, borderWidth+1);
	mmFontSmall->setColor(0xFF, 0xFF, 0xFF);
	if (!dispSentenceStr[1].empty())
		currPosX += mmFontSmall->getStringWidth(dispSentenceStr[1], NULL) + 1;
	int cursorPosX = currPosX++;  //+1 for the cursor
	sentenceWindow->drawString(mmFontSmall, dispSentenceStr[2], currPosX, borderWidth+1);

	//Draw the cursor
	sentenceWindow->moveTo(cursorPosX-1, borderWidth+1);
	sentenceWindow->drawLineTo(cursorPosX-1, sentenceWindow->getClientHeight()-borderWidth-1);

	//Draw the current encoding
	wstring currEncStr = config.activeOutputEncoding.initial;
	int encStrWidth = mmFontSmall->getStringWidth(currEncStr, NULL);
	sentenceWindow->selectObject(g_BlackPen);
	sentenceWindow->selectObject(g_GreenBkgrd);
	sentenceWindow->drawRectangle(sentenceWindow->getClientWidth()-encStrWidth-3, 0, sentenceWindow->getClientWidth(), sentenceWindow->getClientHeight());
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
	if (currInput->isHelpInput() && !dispCandidateStrs.empty() && !dispCandidateStrs[0].first.empty())
		mainWindow->drawString(mmFontSmall, L"(Press \"Space\" to type this word)", borderWidth+1+spaceWidth/2, thirdLineStart-spaceWidth/2);

	//Now, draw the candiate strings and their backgrounds
	int currLabelID = 1;
	for (size_t it=0; it<10; it++) {
		//Measure the string
		unsigned int id = it + (currInput->getPagingInfo().first * 10);
		if (id>=dispCandidateStrs.size())
			break;
		int thisStrWidth = mmFont->getStringWidth(dispCandidateStrs[id].first, NULL);

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
		std::wstringstream digit;
		if (dispCandidateStrs[id].second & HF_LABELTILDE) {
			digit <<L"`";
		} else {
			digit <<currLabelID++;
			if (currLabelID==10)
				currLabelID = 0; //Just renumber for now; we never have more than 10 anyway.
		}
		int digitWidth = mmFont->getStringWidth(digit.str(), NULL);
		mainWindow->drawString(mmFont, digit.str(), borderWidth+1+spaceWidth/2 + xOffset + thisStrWidth/2 -digitWidth/2, thirdLineStart-spaceWidth/2-1);

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
		int strWidth = mmFontSmall->getStringWidth(num.str(), NULL);
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




void typeCurrentPhrase()
{
	//Send key presses to the top-level program.
	HWND fore = GetForegroundWindow();
	SetActiveWindow(fore); //This probably won't do anything, since we're not attached to this window's message queue.

	//Convert to the right encoding
	bool noEncChange = (uni2Output->toEncoding==currInput->encoding);
	wstring keyStrokes = currInput->getTypedSentenceStrings()[3];
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




//Message handling for our dialog box
BOOL CALLBACK HelpDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_INITDIALOG:
		{
			//Resize the help dialog; keep it at the same position.
			//helpWindow->resizeWindow(473, 262);  //NOOO!
			RECT r;
			GetWindowRect(hwnd, &r);
			MoveWindow(hwnd, r.left, r.top, 473, 262, TRUE);

			//Change the text of our dialog box
			wstringstream txt;
			txt << "WaitZar version " <<WAIT_ZAR_VERSION <<" - for the latest news, visit ";
			SetWindowText(GetDlgItem(hwnd, ID_HELP_L1), txt.str().c_str());

			txt.str(L"");
			txt <<langHotkeyString <<" - Switch between Myanmar and English";
			SetWindowText(GetDlgItem(hwnd, ID_HELP_L2), txt.str().c_str());

			//Convert hyperlinks
			ConvertStaticToHyperlink(hwnd, ID_HELP_H1);
			ConvertStaticToHyperlink(hwnd, ID_HELP_H5);
			ConvertStaticToHyperlink(hwnd, ID_HELP_H6);

			//Get font size...
			HFONT hFont = GetWindowFont(hwnd);
			HDC currDC = GetDC(hwnd);
			SelectObject(currDC, hFont);
			int fHeight = HIWORD(GetTabbedTextExtent(currDC, L"[Wp]", 4, 0, NULL));
			int buff = 3;

			//Move our Ok Button
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_OK);
			GetClientRect(hw, &r);
			MoveWindow(hw, 430-r.right, 224-r.bottom, r.right, r.bottom, TRUE);
			}

			//Move our background label into position, and resize it
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_BKGRD);
			int newH = r.bottom+(fHeight-1)*2;
			MoveWindow(hw, 0, 236-newH, 467, newH, TRUE);
			}

			//Move our icon into position
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_IC);
			GetClientRect(hw, &r);
			MoveWindow(hw, fHeight*2, fHeight*2, r.right, r.bottom, TRUE);
			}

			//Determine our new client area
			RECT txtR;
			txtR.left = fHeight*2 + r.right + fHeight/2;
			txtR.top = fHeight*2 + buff;
			GetClientRect(hwnd, &r);
			txtR.right = r.right-txtR.left - fHeight*2 - fHeight/2;
			txtR.bottom = r.bottom-txtR.top - fHeight;
			GetClientRect(GetDlgItem(hwnd, ID_HELP_BKGRD), &r);
			txtR.bottom -= r.bottom;
			int accY = txtR.top;

			//Move the first line
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L1);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight, TRUE);
			GetClientRect(hw, &r);
			}

			//Move the first line's link.
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_H1);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left+r.right, accY, width, fHeight, TRUE);
			accY += fHeight + fHeight;
			}

			//Move the second line's text
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L2);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight, TRUE);
			accY += fHeight;
			}

			//Move the third line's text
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L3);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight, TRUE);
			accY += fHeight*2;
			}

			//Move the fourth line's text
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L4);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight*2, TRUE);
			GetClientRect(hw, &r);
			accY += r.bottom;
			}

			//Move the fifth line's text, part 1
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L5A);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight, TRUE);
			GetClientRect(hw, &r);
			}

			//Move the fifth line's link.
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_H5);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left+r.right, accY, width, fHeight, TRUE);
			int oldW = r.right;
			GetClientRect(hw, &r);
			r.right += oldW;
			}

			//Move the fifth line's text, part 2
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L5B);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left+r.right, accY, width, fHeight, TRUE);
			GetClientRect(hw, &r);
			accY += fHeight*2;
			}

			//Move the sixth line's text
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_L6);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left, accY, width, fHeight, TRUE);
			GetClientRect(hw, &r);
			}

			//Move the sixth line's link.
			{
			HWND hw = GetDlgItem(hwnd, ID_HELP_H6);
			TCHAR temp[200];
			GetWindowText(hw, temp, 200);
			int width = LOWORD(GetTabbedTextExtent(currDC, temp, wcslen(temp), 0, NULL));
			MoveWindow(hw, txtR.left+r.right, accY, width, fHeight, TRUE);
			//accY += fHeight + fHeight;
			}




			return TRUE;
		}
		case WM_CTLCOLORDLG:
			return (BOOL)g_DlgHelpBkgrd;
		case WM_CTLCOLORSTATIC:
		{
			if (GetDlgCtrlID((HWND)lParam)==ID_HELP_BKGRD) {
				//Set the background color of our static item
				return (BOOL)g_DlgHelpSlash;
			}
			if (GetDlgCtrlID((HWND)lParam)==ID_HELP_H1) {
				//Make it blue
				SetTextColor((HDC)wParam, RGB(255, 0, 0));
			}

			//Transparent? Ugh.
			SetBkColor((HDC)wParam, RGB(0xEE, 0xFF, 0xEE));
			return (BOOL)g_DlgHelpBkgrd;
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_HELP_OK:
					EndDialog(hwnd, IDOK);
					break;
				case ID_HELP_H1:
					//Load the WaitZar web site.
					ShellExecute(hwnd, L"open", L"http://www.waitzar.com", NULL, NULL, SW_SHOWNORMAL);
					EndDialog(hwnd, IDOK);
					break;
				case ID_HELP_H5:
					//Load the feedback form
					ShellExecute(hwnd, L"open", L"http://www.waitzar.com/contactus.py", NULL, NULL, SW_SHOWNORMAL);
					EndDialog(hwnd, IDOK);
					break;
				case ID_HELP_H6:
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

	//Initialize the calculationf for the help window
	//initCalculateHelp();
}


void handleNewHighlights(unsigned int keyCode)
{
	//If this is a shifted key, get which key is shifted: left or right
	if (keyCode==HOTKEY_SHIFT) {
		//Well, I like posting fake messages. :D
		// Note that (lParam>>16)&VK_LSHIFT doesn't work here
		if ((GetKeyState(VK_LSHIFT)&0x8000)!=0)
			mainWindow->postMessage(WM_HOTKEY, HOTKEY_VIRT_LSHIFT, MOD_SHIFT);
		if ((GetKeyState(VK_RSHIFT)&0x8000)!=0)
			mainWindow->postMessage(WM_HOTKEY, HOTKEY_VIRT_RSHIFT, MOD_SHIFT);
	} else {
		//Is this a valid key? If so, highlight it and repaint the help window
		if (helpKeyboard->highlightKey(keyCode, true)) {
			reBlitHelp();

			//CRITICAL SECTION
			{
				EnterCriticalSection(&threadCriticalSec);

				/*if (isLogging)
					fprintf(logFile, "  Key down: %c\n", keyCode);*/

				//Manage our thread's list of currently pressed hotkeys
				hotkeysDown.remove(keyCode);
				hotkeysDown.push_front(keyCode);

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
	helpKeyboard->init(helpWindow, memoryWindow);

	//WORKAROUND - Fixes an issue where WZ won't highlight the first key press (unless it's Shift)
	//CRITICAL SECTION
	{
		EnterCriticalSection(&threadCriticalSec);

		//NOTE: This is the workaround: just process a dummy event.
		//      GetKeyState() is failing for some unknown reason on the first press.
		//                    All attempts to "update" it somehow have failed.
		hotkeysDown.push_front(117); //Comment this line to re-trigger the bug.
		if (!threadIsActive) {
			threadIsActive = true;
			ResumeThread(keyTrackThread);
		}

		LeaveCriticalSection(&threadCriticalSec);
	}
	//END WORKAROUND

	//Only needs to be performed once.
	helpIsCached = true;
}


//"Toggle" functions control turning certain things on/off.
//All of these take a boolean value: what are we toggling TO?
void toggleHelpMode(bool toggleTo)
{
	//Do nothing if called in error.
	if (toggleTo == helpWindow->isVisible())
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

		//Switch inputs, set as helper
		currTypeInput->treatAsHelpKeyboard(currHelpInput);
		currInput = currHelpInput;

		//Clear our current word (not the sentence, though, and keep the trigrams)
		//Also reset the helper keyboard. 
		currTypeInput->reset(true, true, false, false);
		currHelpInput->reset(true, true, true, true); 

		//Show the help window
		helpWindow->showWindow(true);
		memoryWindow->showWindow(true);

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

		//Turn off help keys
		turnOnHelpKeys(false);

		//Hide windows
		helpWindow->showWindow(false);
		memoryWindow->showWindow(false);
	}


	//Redraw all
	currInput->forceViewChanged();
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

	//Step 2: Read
	currInput = config.activeInputMethod;
	currTypeInput = currInput;
	currHelpInput = NULL; 
	mmFont = config.activeDisplayMethods[0];
	mmFontSmall = config.activeDisplayMethods[1];
	input2Uni = config.getTransformation(config.activeLanguage, config.activeInputMethod->encoding, config.unicodeEncoding);
	uni2Output = config.getTransformation(config.activeLanguage, config.unicodeEncoding, config.activeOutputEncoding);
	uni2Disp = config.getTransformation(config.activeLanguage, config.unicodeEncoding, config.activeDisplayMethods[0]->encoding);

	//Now, reset?
	if (!langid.empty() || !inputid.empty()) {
		//Input has changed; reset
		currTypeInput->reset(true, true, true, true);
		if (currHelpInput!=NULL)
			currHelpInput->reset(true, true, true, true);

		//And repaint, just in case
		checkAllHotkeysAndWindows();
		recalculate();
	}
	if (!langid.empty()) {
		//Rebuild the menus?
		if (contextMenu!=NULL) {
			//Reclaim
			DestroyMenu(contextMenu);
			totalMenuItems = 0;
			customMenuItemsLookup.clear();
			delete [] customMenuItems;

			//Reubild
			createContextMenu();
		}

		//We'll have to re-size the windows, in case a different font is used
		initCalculate();
	}
}




bool handleMetaHotkeys(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
		case LANG_HOTKEY:
			//Switch language
			switchToLanguage(!mmOn);
			return true;

		case HOTKEY_HELP:
			//What to do if our user hits "F1".
			if (/*!allowNonBurmeseLetters &&*/ currHelpInput!=NULL)
				toggleHelpMode(true);
			return true;

		default:
			if (helpWindow->isVisible() && highlightKeys) {
				//Highlight our virtual keyboard
				handleNewHighlights(wParam);

				//Doesn't consume a keypress
				return false;
			}
	}

	//Else, not processed
	return false;
}


bool handleUserHotkeys(WPARAM wParam, LPARAM lParam)
{
	//First, is this an upper-case letter?
	bool isUpper = (wParam>='A' && wParam<='Z');

	//Handle user input; anything that updates a non-specific "model".
	//  TODO: Put code for "help" keyboard functionality HERE; DON'T put it into 
	//        LetterInputMethod.h
	switch (wParam) {
		case HOTKEY_ESC:
			//Close the window, exit help mode
			currInput->handleEsc();
			return true;

		case HOTKEY_BACK:
			//Back up
			currInput->handleBackspace();
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
			currInput->handleStop(wParam==HOTKEY_PERIOD);
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
			int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : HOTKEY_NUM0;
			int numCode = (int)wParam - base;

			//Handle key press; letter-based keyboard should just pass this on through
			bool typeNumerals = true; //TODO: put this SOMEWHERE in the config file.
			currInput->handleNumber(numCode, wParam, isUpper, typeNumerals);
			return true;
		}

		default:
			//Tricky here: we need to put the "system key" nonsense into the "handleKeyPress"  function
			// otherwise numbers won't work.
			currInput->handleKeyPress(wParam, isUpper);
			return true;
	}

	//Else, not processed
	return false;
}


void createMyanmarMenuFont()
{
	//Get the Padauk embedded resource
	HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_PADAUK_ZG), _T("COREFONT"));
	if (!fontRes)
		throw std::exception("Couldn't find WZ_PADAUK_ZG");
	HGLOBAL res_handle = LoadResource(NULL, fontRes);
	if (!res_handle)
		throw std::exception("Couldn't get a handle on WZ_PADAUK_ZG");

	//Now, create it
	menuFont = new TtfDisplay();
	menuFont->fontFaceName = L"PdkZgWz";
	menuFont->pointSize = 10;
	mainWindow->initTtfMethod(menuFont, fontRes, res_handle, 0x000000);
}



//Rebuild the context menu --any time
void createContextMenu()
{
	//Load the context menu into memory from the resource file.
	contextMenu = LoadMenu(hInst, MAKEINTRESOURCE(WZ_MENU));
	contextMenuPopup = GetSubMenu(contextMenu, 0);

	//Build our complex submenu for Language, Input Method, and Output Encoding
	typingMenu = GetSubMenu(contextMenuPopup, 7); //TODO: Change to 6.... remove "Encoding" menu
	RemoveMenu(typingMenu, ID_DELETE_ME, MF_BYCOMMAND);

	//Build each sub-menu if none have been created yet. (In case we ever recycle this menu).
	if (totalMenuItems == 0) {
		//Give each menu item a unique ID
		unsigned int currDynamicCmd = DYNAMIC_CMD_START;

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

		//Add the "Input Methods" section
		myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_HEADER, L"", WND_TITLE_INPUT));
		myMenuItems.push_back(sep);
		currDynamicCmd++; //Maintain easy access

		//Add each input method as an MI
		for (std::set<InputMethod*>::const_iterator it = config.getInputMethods().begin(); it!=config.getInputMethods().end(); it++) 
			myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_INPUT, (*it)->id, (*it)->displayName));

		//Add the "Output Encodings" section
		myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_HEADER, L"", WND_TITLE_OUTPUT));
		myMenuItems.push_back(sep);
		currDynamicCmd++; //Maintain easy access

		//Add each input method as an MI
		for (std::set<Encoding>::const_iterator it = config.getEncodings().begin(); it!=config.getEncodings().end(); it++) {
			if (!it->canUseAsOutput)
				continue;
			myMenuItems.push_back(WZMenuItem(currDynamicCmd++, WZMI_OUTPUT, it->id, it->displayName));
		}

		//Copy over to allocated memory --we need a constant pointer to each menu item, and vectors are too risky for this.
		customMenuItems = new WZMenuItem[myMenuItems.size()];
		for (size_t i=0; i<myMenuItems.size(); i++) {
			customMenuItems[totalMenuItems] = myMenuItems[i];
			customMenuItemsLookup[customMenuItems[totalMenuItems].menuID] = &customMenuItems[totalMenuItems];
			totalMenuItems++;
		}
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
}



//Build the context menu --first time
void initContextMenu() 
{
	//Make the font
	createMyanmarMenuFont();

	//Make the menu
	createContextMenu();
}


void updateContextMenuState()
{
	//Hotkey string, check mark for the current language.
	std::wstringstream txt;
	txt <<L"English (" <<langHotkeyString <<")";
	ModifyMenu(contextMenu, IDM_ENGLISH, MF_BYCOMMAND|(mmOn?0:MF_CHECKED), IDM_ENGLISH, txt.str().c_str());
	txt.str(L"");
	txt <<L"Myanmar (" <<langHotkeyString <<")";
	ModifyMenu(contextMenu, IDM_MYANMAR, MF_BYCOMMAND|(mmOn?MF_CHECKED:0), IDM_MYANMAR, txt.str().c_str());	

	//Set a check for the "Look Up Word" function
	//  Also remove the "F1" if not applicable.
	UINT flagL = helpWindow->isVisible() ? MF_CHECKED : 0;
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
		CheckMenuItem(typingMenu, DYNAMIC_CMD_START+i, MF_BYCOMMAND|checkFlag);
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
			//Update our virtual keyboard
			if (helpKeyboard->highlightKey(wParam, false))
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
			//First, handle all "system" or "meta" level commands, like switching the language,
			// switching into help mode, etc.
			//Then, handle all "dynamic" commands; those which change depending on the 
			// current IM or mode.
			if (!handleMetaHotkeys(wParam, lParam)) {
				//Set flags for the current state of the Input Manager. We will
				// check these against the exit state to see what has changed,
				// and thus what needs to be updated.
				bool wasProvidingHelp = currInput->isHelpInput();
				bool wasEmptySentence = currInput->getTypedSentenceStrings()[3].empty();
				bool wasEmptyRoman = currInput->getTypedRomanString(false).empty();

				//Process the message
				handleUserHotkeys(wParam, lParam);

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
					typeCurrentPhrase();

				//Feedback Check 2: Do we need to repaint the window?
				if (currInput->getAndClearViewChanged())
					recalculate();
			}
			break;
		}



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
				myFont->setColor(GetRValue(cr_MenuItemText), GetGValue(cr_MenuItemText), GetBValue(cr_MenuItemText));
			} else {
				oldBkgrd = SetBkColor(currDC, cr_MenuDefaultBkgrd);
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
			if (lParam==WM_RBUTTONUP || lParam==WM_LBUTTONUP) {
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
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_WZ_HELP), hwnd, HelpDlgProc);
						showingHelpPopup = false;
					}

					//Hotkeys again
					if  (refreshControl)
						turnOnControlkeys(true);
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
					if (!mmOn)
						switchToLanguage(true);
					toggleHelpMode(!currInput->isHelpInput()); //TODO: Check this works!
				} else if (retVal == IDM_EXIT) {
					//Will generate a WM_DESTROY message
					delete mainWindow;
				} else if (retVal >= DYNAMIC_CMD_START) {
					//Switch the language, input manager, or output manager.
					if (customMenuItemsLookup.count(retVal)==0)
						throw std::exception("Bad menu item");
					WZMenuItem* currItem = customMenuItemsLookup[retVal];
					if (currItem->type==WZMI_LANG)
						ChangeLangInputOutput(currItem->id, L"", L"");
					else if (currItem->type==WZMI_INPUT)
						ChangeLangInputOutput(L"", currItem->id, L"");
					else if (currItem->type==WZMI_OUTPUT)
						ChangeLangInputOutput(L"", L"", currItem->id);
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

			//Log?
			if (isLogging) {
				fprintf(logFile, "WaitZar closed\n");
				fclose(logFile);
			}

			break;
		}
		default:
			return 1; //1 ==> not done
	}

	return 0;
}




/**
 * Main method for Windows applications
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Logging time to start?
	if (currTest == start_up)
		GetSystemTimeAsFileTime(&startTime);


	//Save for later; if we try retrieving it, we'll just get a bunch of conversion
	//  warnings. Plus, the hInstance should never change.
	hInst = hInstance;

	//First and foremost
	helpIsCached = false;
	isDragging = false;
	helpKeyboard = NULL;
	contextMenu = NULL;
	menuFont = NULL;
	sysFont = NULL;

	//Also...
	try {
		//buildSystemWordLookup();
	} catch (std::exception ex) {
	/*	std::wstringstream msg;
		msg << "Error building system look.\nWaitZar will now terminate.\n\nDetails:\n";
		msg << ex.what();
		MessageBox(NULL, msg.str().c_str(), L"System Hotkeys Error", MB_ICONWARNING | MB_OK);
		return 0;*/
	}

	//Log?
	if (isLogging) {
		logFile = fopen("wz_log.txt", "w");
		if (logFile==NULL) {
			MessageBox(NULL, _T("Unable to open Log file"), _T("Warning"), MB_ICONWARNING | MB_OK);
			isLogging = false;
		} else {
			fprintf(logFile, "WaitZar was opened\n");
		}
	} else
		logFile = NULL;
	waitzar::setLogFile(logFile);


	//Create a white/black brush
	cr_MenuItemBkgrd = RGB(0x8F, 0xA1, 0xF8);
	cr_MenuDefaultBkgrd = GetSysColor(COLOR_MENU);
	cr_MenuDefaultText = GetSysColor(COLOR_MENUTEXT);
	cr_MenuItemText = RGB(0x20, 0x31, 0x89);
	cr_Black = RGB(0x00, 0x00, 0x00);
	cr_White = RGB(0xFF, 0xFF, 0xFF);
	g_WhiteBkgrd = CreateSolidBrush(RGB(255, 255, 255));
	g_DarkGrayBkgrd = CreateSolidBrush(RGB(128, 128, 128));
	g_BlackBkgrd = CreateSolidBrush(RGB(0, 0, 0));
	g_YellowBkgrd = CreateSolidBrush(RGB(255, 255, 0));
	g_GreenBkgrd = CreateSolidBrush(RGB(0, 128, 0));
	g_DlgHelpBkgrd = CreateSolidBrush(RGB(0xEE, 0xFF, 0xEE));
	g_MenuItemBkgrd = CreateSolidBrush(cr_MenuItemBkgrd);
	g_MenuDefaultBkgrd = CreateSolidBrush(cr_MenuDefaultBkgrd);
	g_MenuItemHilite = CreateSolidBrush(RGB(0x07, 0x2B, 0xE4));
	g_DlgHelpSlash = CreateSolidBrush(RGB(0xBB, 0xFF, 0xCC));
	g_DotHiliteBkgrd = CreateSolidBrush(RGB(0x00, 0x1A, 0x7A));
	g_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
	g_BlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	g_MediumGrayPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
	g_DotHilitePen = CreatePen(PS_SOLID, 1, RGB(0x42, 0x5D, 0xBC));
	g_EmptyPen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));


	//Create our windows so that they exist in memory for config.validate()
	mainWindow = new MyWin32Window(L"waitZarMainWindow");
	sentenceWindow = new MyWin32Window(L"waitZarSentenceWindow");
	helpWindow = new MyWin32Window(L"waitZarHelpWindow");
	memoryWindow = new MyWin32Window(L"waitZarMemoryWindow");



	//Find all config files
	try {
		//Useful shorthands
		string cfgDir = "config";
		string fs = "\\";
		string cfgFile = "config.json.txt";

		//Set the main config file
		config.initMainConfig(cfgDir + fs + cfgFile);

		//Browse for all language directories, add them
		vector<wstring> langFolders = GetConfigSubDirs(cfgDir, cfgFile);
		for (vector<wstring>::iterator fold = langFolders.begin(); fold!=langFolders.end(); fold++) {
			//Get the main language config file
			string langCfgDir;
			string langCfgFile;
			vector<string> langModuleCfgFiles;
			std::stringstream errorMsg;
			bool inError = false;
			try {
				langCfgDir = cfgDir;
				langCfgDir += fs + waitzar::escape_wstr(*fold, true);
				langCfgFile = langCfgDir + fs + cfgFile;
			} catch (std::exception ex) {
				errorMsg << "Error loading config file for language: " <<waitzar::escape_wstr(*fold, false);
				errorMsg << std::endl << "Details: " << std::endl << ex.what();
				inError = true;
			}

			//Now, get the sub-config files
			if (!inError) {
				vector<wstring> modFolders = GetConfigSubDirs(langCfgDir, cfgFile);
				for (vector<wstring>::iterator mod = modFolders.begin(); mod!=modFolders.end(); mod++) {
					try {
						string modCfgFile = langCfgDir;
						modCfgFile += fs + waitzar::escape_wstr(*mod, true);
						modCfgFile += fs + cfgFile;
						langModuleCfgFiles.push_back(modCfgFile);
					} catch (std::exception ex) {
						errorMsg << "Error loading config file for language: " <<waitzar::escape_wstr(*fold, false);
						errorMsg << std::endl << "and module: " <<waitzar::escape_wstr(*mod, false);
						errorMsg << std::endl << "Details: " << std::endl << ex.what();
						inError = true;
						break;
					}
				}
			}

			//Handle errors:
			if (inError)
				throw std::exception(errorMsg.str().c_str());

			config.initAddLanguage(langCfgFile, langModuleCfgFiles);
		}



		//TODO: Add SHGetKnownFolderPath() if on Vista, keep SHGetFolderPath if on XP or less.



		//Now, load the local config file
		wchar_t localAppPath[MAX_PATH];
		string localConfigFile;
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, localAppPath))) {
			//Try to create the folder if it doesn't exist
			string localConfigDir = waitzar::escape_wstr(localAppPath, true) + fs + "WaitZar";
			WIN32_FILE_ATTRIBUTE_DATA InfoFile;
			std::wstringstream temp;
			temp << localConfigDir.c_str();
			if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
				CreateDirectory(temp.str().c_str(), NULL);

			//Build the path
			localConfigFile = localConfigDir + fs + "config.override.json.txt";

			//Does it exist?
			temp.str(L"");
			temp << localConfigFile.c_str();
			if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==TRUE)
				config.initLocalConfig(localConfigFile);
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
		string userConfigFile;
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, localAppPath))) {
			//Create the path
			userConfigFile = waitzar::escape_wstr(localAppPath, true) + fs + "waitzar.config.json.txt";

			//Does it exist?
			WIN32_FILE_ATTRIBUTE_DATA InfoFile;
			std::wstringstream temp;
			temp << userConfigFile.c_str();
			if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==TRUE)
				config.initUserConfig(userConfigFile);
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

		//First test: does "config" not exist at all? If so, throw a special exception,
		//  and avoid the warning message box.
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		std::wstringstream temp;
		temp << cfgDir.c_str();
		if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
			throw std::exception("No config directory");

		//Final test: make sure all config files work
		config.validate(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard);
	} catch (std::exception ex) {
		//In case of errors, just reset & use the embedded file
		config = ConfigManager();

		//Inform the user, UNLESS they set the magic number...
		if (strcmp(ex.what(), "No config directory")!=0) {
			std::wstringstream msg;
			msg << "Error loading one of your config files.\nWaitZar will use the default configuration.\n\nDetails:\n";
			msg << ex.what();
			MessageBox(NULL, msg.str().c_str(), L"Config File Error", MB_ICONWARNING | MB_OK);
		}


		//Try one more time, this time with the default config file.
		try {
			//Load the resource as a byte array and get its size, etc.
			HRSRC res = FindResource(hInst, MAKEINTRESOURCE(WZ_DEFAULT_CFG), _T("Model"));
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

			//One more test.
			config.validate(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard);
		} catch (std::exception ex2) {
			std::wstringstream msg2;
			msg2 << "Error loading default config file.\nWaitZar will not be able to function, and is shutting down.\n\nDetails:\n";
			msg2 << ex2.what();
			MessageBox(NULL, msg2.str().c_str(), L"Default Config Error", MB_ICONERROR | MB_OK);
			return 0;
		}
	}


	//TEST: did our settings load?
	/*Settings s = config.getSettings();
	wstringstream msg;
	msg << "Settings" <<std::endl;
	msg << "Always elevate: " <<s.alwaysElevate <<std::endl;
	msg << "Balloon start: " <<s.balloonStart <<std::endl;
	msg << "Lock windows: " <<s.lockWindows <<std::endl;
	msg << "Silence mywords warnings: " <<s.silenceMywordsErrors <<std::endl;
	msg << "Track caret: " <<s.trackCaret <<std::endl;
	msg << "Hotkey: " <<s.hotkey <<std::endl;
	msg << "Default Language: " <<s.defaultLanguage <<std::endl;
	msg << "---------------------" <<std::endl;
	msg << "Languages" <<std::endl;
	std::set<Language> langs = config.getLanguages();
	for (std::set<Language>::iterator s=langs.begin(); s!=langs.end(); s++) {
		msg <<"   " << s->displayName <<std::endl;
		wstring comma = L"";

		//Input Methods
		comma = L"";
		msg <<L"      Input Methods: " <<"[";
		for (std::set<InputMethod*>::const_iterator i=s->inputMethods.begin(); i!=s->inputMethods.end(); i++)  {
			msg <<comma <<(*i)->displayName;
			comma = L", ";
		}
		msg <<"]" <<std::endl;

		//Encodings
		comma = L"";
		msg <<L"      Encodings: " <<"[";
		for (std::set<Encoding>::const_iterator i=s->encodings.begin(); i!=s->encodings.end(); i++)  {
			msg <<comma <<i->displayName <<(i->canUseAsOutput?L"*":L"");
			comma = L", ";
		}
		msg <<"]" <<std::endl;

		//Display Methods
		comma = L"";
		msg <<L"      Display Methods: " <<"[";
		for (std::set<DisplayMethod*>::const_iterator i=s->displayMethods.begin(); i!=s->displayMethods.end(); i++)  {
			msg <<comma <<(*i)->id;
			comma = L", ";
		}
		msg <<"]" <<std::endl;

		//Transformations
		msg <<L"      Transformations: " <<"[{self->self}";
		for (std::set<Transformation*>::const_iterator i=s->transformations.begin(); i!=s->transformations.end(); i++)  {
			msg <<comma <<"{" <<(*i)->fromEncoding.id <<"->" <<(*i)->toEncoding.id <<"}";
			comma = L", ";
		}
		msg <<"]" <<std::endl;

	}
	MessageBox(NULL, msg.str().c_str(), L"Settings", MB_ICONINFORMATION | MB_OK);*/


	//Load our configuration file now; save some headaches later
	loadConfigOptions();

	//Modify our config options?
	if (currTest == mywords) {
		//dontLoadModel = true;
//		mywordsFileName = "D:\\Open Source Projects\\Waitzar\\eclipse_project\\MyanmarList_v2.txt";
	}


	//Should we run a UAC test on startup?
	if (alwaysRunElevated) {
		//Will elevating help?
		if (IsVistaOrMore() && !IsAdmin()) {
			TCHAR szCurFileName[1024];
            GetModuleFileName(GetModuleHandle(NULL), szCurFileName, 1023);
			if (!elevateWaitZar(szCurFileName))
				MessageBox(NULL, _T("Could not elevate WaitZar. Program will now exit."), _T("Error!"), MB_ICONERROR | MB_OK);

			//Return either way; the current thread must exit.
			return 0;
		}
	}

	//Give this process a low background priority
	//  NOTE: We need to balance this eventually.
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);


	//Create our windows.
	try {
		//Init
		mainWindow->init(L"WaitZar", WndProc, g_DarkGrayBkgrd, hInst, 100, 100, 240, 120, positionAtCaret, onAllWindowsCreated, false);
		sentenceWindow->init(L"WaitZar", NULL, g_DarkGrayBkgrd, hInst, 100, 100+mainWindow->getDefaultHeight(), 300, 26, positionAtCaret, onAllWindowsCreated, false);
		helpWindow->init(L"WaitZar", NULL, g_GreenBkgrd, hInst, 400, 300, 200, 200, NULL, onAllWindowsCreated, true);
		memoryWindow->init(L"WaitZar", NULL, g_GreenBkgrd, hInst, 400, 300, 200, 200, NULL, onAllWindowsCreated, true);

		//Then link
		if (dragBothWindowsTogether)
			mainWindow->linkToWindow(sentenceWindow, SOUTH);
	} catch (std::exception err) {
		std::wstringstream msg;
		msg << "Error creating WaitZar's windows.\nWaitZar will now exit.\n\nDetails:\n";
		msg << err.what();
		MessageBox(NULL, msg.str().c_str(), L"CreateWindow() Error", MB_ICONERROR | MB_OK);
		return 0;
	}

	//Create our context menu
	initContextMenu();

	//Load some icons...
	mmIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_MM), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	engIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_ENG), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16

	//Make our "notify icon" data structure
	NOTIFYICONDATA nid;
	mainWindow->initShellNotifyIconData(nid);
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_LOADING), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...

	//Error checking..
	if (mmIcon == NULL || engIcon==NULL)
		MessageBox(NULL, _T("Unable to load Icon!"), _T("Warning"), MB_ICONWARNING | MB_OK);

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

	//Edit: Add support for balloon tooltips
	if (showBalloonOnStart) {
		nid.uFlags |= NIF_INFO;
		lstrcpy(nid.szInfoTitle, _T("Welcome to WaitZar"));
		swprintf(nid.szInfo, _T("Hit %ls to switch to Myanmar.\n\nClick here for more options."), langHotkeyString);
		nid.uTimeout = 20;
		nid.dwInfoFlags = NIIF_INFO; //Can we switch to NIIF_USER if supported?
	}

	//Add our icon to the tray
	Shell_NotifyIcon(NIM_ADD, &nid);


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

	//Success?
	if(mainWindow->isInvalid() || sentenceWindow->isInvalid() || helpWindow->isInvalid() || memoryWindow->isInvalid()) {
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//If we got this far, let's try to load our file.
	/*try {
		//loadModel();
	} catch (std::exception ex) {
		//Prompt user:
		std::wstringstream msg;
		msg << "Couldn't load the model.\nWaitZar will not be able to function, and is shutting down.\n\nDetails:\n";
		msg << ex.what();
		MessageBox(NULL, msg.str().c_str(), L"Model Error", MB_ICONERROR | MB_OK);

		//Remove resources, exit
		delete mainWindow;
		delete sentenceWindow;
		delete helpWindow;
		delete memoryWindow;
		return 1;
	}*/


	//Set defaults
	ChangeLangInputOutput(config.activeLanguage.id, config.activeInputMethod->id, config.activeOutputEncoding.id);


	//Todo... find a better way of setting this (loadModel() and loadConfigOptions() clash)
	//   Actually, this shold be fixed when we switch to the new config files
	//model->setOutputEncoding(mostRecentEncoding);

	//Testing mywords?
	if (currTest == mywords)
		GetSystemTimeAsFileTime(&startTime);


	//Logging mywords?
	if (currTest == mywords) {
		GetSystemTimeAsFileTime(&endTime);
		DWORD timeMS = getTimeDifferenceMS(startTime, endTime);

		wchar_t msg[500];
		swprintf(msg, L"Mywords total time:   %dms", timeMS);
		MessageBox(NULL, msg, L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);

		return 0;
	}

	//Did we get any?
	/*if (currTest != model_print) {
		TCHAR noConfigWarningMsg[1500];
		lstrcpy(noConfigWarningMsg, _T(""));
		if (numConfigOptions==0) {
			lstrcpy(noConfigWarningMsg, _T("config.txt contained no valid configuration options."));
		}
		if (numCustomWords==0) {
			if (lstrlen(noConfigWarningMsg)==0)
				lstrcpy(noConfigWarningMsg, _T("mywords.txt contained no Burmese words."));
			else
				lstrcat(noConfigWarningMsg, _T(" Also, mywords.txt contained no Burmese words."));
		}
		if (lstrlen(noConfigWarningMsg)>0) {
			lstrcat(noConfigWarningMsg, _T("\nThis could be caused by a number of things:\n   + config.txt should be ASCII-encoded. mywords.txt should be UTF-8-encoded. Possibly you used another encoding?"));
			lstrcat(noConfigWarningMsg, _T("\n   + Perhaps you mis-spelled a configuration option. Check the Wait Zar manual to make sure you spelled each configuration option correctly.\n   + Maybe you commented out a line by mistake? The \"#\" key means to ignore a line."));
			lstrcat(noConfigWarningMsg, _T("\n   + Maybe your line-endings are wrong? Wait Zar can handle \\n OR \\r\\l (Windows or Linux) but that's all..."));
			lstrcat(noConfigWarningMsg, _T("\n\nIf you think this was caused by a bug in Wait Zar, please post an issue at http://code.google.com/p/waitzar/issues/list\n\nThis is just a warning --Wait Zar will still work fine without any config.txt or mywords.txt files."));
			MessageBox(NULL, noConfigWarningMsg, _T("Warning"), MB_ICONWARNING | MB_OK);
		}
	}*/

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
	}


	//Potential debug loop (useful)
	if (currTest == model_print) {
		if (isLogging) {
			fclose(logFile);
			isLogging = false;
		}
		logFile = fopen("wz_log.txt", "w");

		//model->debugOut(logFile);

		MessageBox(NULL, L"Model saved to output.", _T("Notice"), MB_ICONERROR | MB_OK);
		return 1;
	}



	//Show it's ready by changing the shell icon
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	nid.hIcon = engIcon;
	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE) {
		TCHAR eTemp[200];
		swprintf(eTemp, _T("Can't load initial icon.\nError code: %x"), GetLastError());
		MessageBox(NULL, eTemp, _T("Warning"), MB_ICONERROR | MB_OK);
	}


	//Logging time to start?
	if (currTest == start_up) {
		GetSystemTimeAsFileTime(&endTime);
		DWORD timeMS = getTimeDifferenceMS(startTime, endTime);

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
