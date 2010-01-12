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
#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
#define _CRT_NON_CONFORMING_SWPRINTFS

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
#define NOCOLOR             //- Screen colors
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

//Prototypes
bool turnOnHotkeys(bool on, bool affectLowercase, bool affectUppercase);
bool turnOnControlkeys(bool on);
bool turnOnNumberkeys(bool on);
bool turnOnPunctuationkeys(bool on);
bool turnOnExtendedKeys(bool on); //"Help keys"
bool turnOnHelpKeys(bool on); //Reduced to contain only the shift key
bool loadModel(HINSTANCE hInst);

//Better support for dragging
bool isDragging;
POINT dragFrom;

//Unique IDs
const unsigned int LANG_HOTKEY = 142;
const unsigned int STATUS_NID = 144;

//Custom message IDs
const unsigned int UWM_SYSTRAY = WM_USER+1;
const unsigned int UWM_HOTKEY_UP = WM_USER+2;

//Grr... notepad...
const unsigned int UNICOD_BOM = 0xFEFF;
const unsigned int BACKWARDS_BOM = 0xFFFE;

//Font conversion
wstring currEncStr;
ENCODING mostRecentEncoding = ENCODING_UNICODE;

//Brushes & Pens
HBRUSH g_WhiteBkgrd;
HBRUSH g_DarkGrayBkgrd;
HBRUSH g_YellowBkgrd;
HBRUSH g_GreenBkgrd;
HBRUSH g_DlgHelpBkgrd;
HBRUSH g_DlgHelpSlash;
HPEN g_GreenPen;
HPEN g_BlackPen;
HPEN g_EmptyPen;

//Global Variables
HINSTANCE hInst;
HICON mmIcon;
HICON engIcon;
WordBuilder model;

//These need to be pointers, for now. Reference semantics are just too complex.
PulpCoreFont *mmFontBlack;
PulpCoreFont *mmFontGreen;
PulpCoreFont *mmFontRed;
PulpCoreFont *mmFontSmallWhite;
PulpCoreFont *mmFontSmallGray;
PulpCoreFont *mmFontSmallRed;

PAINTSTRUCT Ps;
WORD stopChar;
int numConfigOptions;
int numCustomWords;
INPUT inputItems[2000];
KEYBDINPUT keyInputPrototype;
bool helpIsCached;
//wstring returnVal;
string mywordsFileName = "mywords.txt";

//For now, we track the shortcut pat-sint keys directly. Later, we'll integrate this into the model (if people like it)
int patSintIDModifier = 0;

//Help Window resources
//Leave as pointers for now.
PulpCoreFont *helpFntKeys;
PulpCoreFont *helpFntFore;
PulpCoreFont *helpFntBack;
PulpCoreFont *helpFntMemory;
PulpCoreImage *helpCornerImg;
OnscreenKeyboard *helpKeyboard;

//BLENDFUNCTION BLEND_FULL = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA }; //NOTE: This requires premultiplied pixel values
//POINT PT_ORIGIN;
HANDLE keyTrackThread;   //Handle to our thread
DWORD  keyTrackThreadID; //Its unique ID (never zero)
CRITICAL_SECTION threadCriticalSec; //Global critical section object
list<unsigned int> hotkeysDown; //If a wparam is in this list, it is being tracked
bool threadIsActive; //If "false", this thread must be woken to do anything useful
vector<wstring> userDefinedWords; //Words the user types in. Stored with a negative +1 index
vector<wstring> userDefinedWordsZg; //Cache of the Zawgyi version of the word typed
wstring userKeystrokeVector;
const string systemDefinedWords = "`~!@#$%^&*()-_=+[{]}\\|;:'\"<>/? 1234567890"; //Special "words" used in our keyboard, like "(" and "`"
vector< pair <int, unsigned short> > systemWordLookup;

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
BOOL customDictWarning = FALSE;

//These two will take some serious fixing later.
TCHAR langHotkeyString[100];
char langHotkeyRaw[100];

bool typePhrases = true;
bool dragBothWindowsTogether = true;
bool typeBurmeseNumbers = true;
bool showBalloonOnStart = true;
bool alwaysRunElevated = false;
bool highlightKeys = true;
bool experimentalTextCursorTracking = true;
bool dontLoadModel = false;
bool allowNonBurmeseLetters = false;
bool ignoreMywordsWarnings = false;
//unsigned int maxDictionaryEntries = 0;
//unsigned int maxNexusEntries = 0;
//unsigned int maxPrefixEntries = 0;
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


//Init properly
bool mainInitDone;
bool sentenceInitDone;
bool helpInitDone;
bool memoryInitDone;

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
SentenceList sentence;
int prevProcessID;
bool showingHelpPopup = false;


//Width/height of client area
//int C_WIDTH;
//int C_HEIGHT;
//int SUB_C_WIDTH;
//int SUB_C_HEIGHT;

//Try it differently for the help menu
//SIZE HELP_CLIENT_SIZE;
//SIZE MEMORY_CLIENT_SIZE;

//Calculate's integers
int firstLineStart;
int secondLineStart;
int thirdLineStart;
int fourthLineStart;
int borderWidth = 2;
int spaceWidth;

//Avoid crashing if explorer is running slowly
//bool mainWindow->isVisible();
//bool sentenceWindow->isVisible();
//bool helpWindow->isVisible();
//bool memoryWindow->isVisible();

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
bool testAllWordsByHand()
{
	//First, ensure that the reverse-lookup is ready
	model.reverseLookupWord(0);

	//Time
	GetSystemTimeAsFileTime(&startTime);

	//For each typable word
	string revWord;
	for (unsigned int wordID=0; ; wordID++) {
		//Check
		revWord=model.reverseLookupWord(wordID);
		if (revWord.empty())
			break;

		//Type this
		model.reset(false);
		for (string::iterator rom = revWord.begin(); rom!=revWord.end(); rom++) {
			//Just check that our romanisation is stored properly.
			if (!model.typeLetter(*rom))
				return false;
		}

		//Test "output" it
		std::pair<bool, unsigned int> ret = model.typeSpace(-1);
		if (!ret.first)
			return false;
		model.getWordKeyStrokes(ret.second);
	}


	//Done, display a message box
	GetSystemTimeAsFileTime(&endTime);
	DWORD timeMS = getTimeDifferenceMS(startTime, endTime);

	wchar_t msg[500];
	swprintf(msg, L"Type All total time:   %dms", timeMS);
	MessageBox(NULL, msg, L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);
	return true;
}



void buildSystemWordLookup()
{
	for (size_t i=0; i<systemDefinedWords.size(); i++) {
		char c = systemDefinedWords[i];
		int hotkey_id = 0;
		switch (c) {
			case '`':
				hotkey_id = HOTKEY_COMBINE;
				break;
			case '~':
				hotkey_id = HOTKEY_SHIFT_COMBINE;
				break;
			case '!':
				hotkey_id = HOTKEY_SHIFT_1;
				break;
			case '@':
				hotkey_id = HOTKEY_SHIFT_2;
				break;
			case '#':
				hotkey_id = HOTKEY_SHIFT_3;
				break;
			case '$':
				hotkey_id = HOTKEY_SHIFT_4;
				break;
			case '%':
				hotkey_id = HOTKEY_SHIFT_5;
				break;
			case '^':
				hotkey_id = HOTKEY_SHIFT_6;
				break;
			case '&':
				hotkey_id = HOTKEY_SHIFT_7;
				break;
			case '*':
				hotkey_id = HOTKEY_SHIFT_8;
				break;
			case '(':
				hotkey_id = HOTKEY_SHIFT_9;
				break;
			case ')':
				hotkey_id = HOTKEY_SHIFT_0;
				break;
			case '-':
				hotkey_id = HOTKEY_MINUS;
				break;
			case '_':
				hotkey_id = HOTKEY_SHIFT_MINUS;
				break;
			case '=':
				hotkey_id = HOTKEY_EQUALS;
				break;
			case '+':
				hotkey_id = HOTKEY_SHIFT_EQUALS;
				break;
			case '[':
				hotkey_id = HOTKEY_LEFT_BRACKET;
				break;
			case '{':
				hotkey_id = HOTKEY_SHIFT_LEFT_BRACKET;
				break;
			case ']':
				hotkey_id = HOTKEY_RIGHT_BRACKET;
				break;
			case '}':
				hotkey_id = HOTKEY_SHIFT_RIGHT_BRACKET;
				break;
			case ';':
				hotkey_id = HOTKEY_SEMICOLON;
				break;
			case ':':
				hotkey_id = HOTKEY_SHIFT_SEMICOLON;
				break;
			case '\'':
				hotkey_id = HOTKEY_APOSTROPHE;
				break;
			case '"':
				hotkey_id = HOTKEY_SHIFT_APOSTROPHE;
				break;
			case '\\':
				hotkey_id = HOTKEY_BACKSLASH;
				break;
			case '|':
				hotkey_id = HOTKEY_SHIFT_BACKSLASH;
				break;
			case '<':
				hotkey_id = HOTKEY_SHIFT_COMMA;
				break;
			case '>':
				hotkey_id = HOTKEY_SHIFT_PERIOD;
				break;
			case '/':
				hotkey_id = HOTKEY_FORWARDSLASH;
				break;
			case '?':
				hotkey_id = HOTKEY_SHIFT_FORWARDSLASH;
				break;
			case ' ':
				hotkey_id = HOTKEY_SHIFT_SPACE;
				break;
			case '1':
				hotkey_id = HOTKEY_1;
				break;
			case '2':
				hotkey_id = HOTKEY_2;
				break;
			case '3':
				hotkey_id = HOTKEY_3;
				break;
			case '4':
				hotkey_id = HOTKEY_4;
				break;
			case '5':
				hotkey_id = HOTKEY_5;
				break;
			case '6':
				hotkey_id = HOTKEY_6;
				break;
			case '7':
				hotkey_id = HOTKEY_7;
				break;
			case '8':
				hotkey_id = HOTKEY_8;
				break;
			case '9':
				hotkey_id = HOTKEY_9;
				break;
			case '0':
				hotkey_id = HOTKEY_0;
				break;
		}

		systemWordLookup.push_back(pair<int, unsigned short>(hotkey_id, i));
	}
}



//myWin 2.1 rules for stacking
bool canStack(wchar_t letter)
{
	return (letter>=0x1000 && letter<=0x1003)
		|| (letter>=0x1005 && letter<=0x1021);
}






/**
 * Create our inner-used Zawgyi-One fonts.
 */
void makeFont()
{
	//Load our font resource (main fonts)
	{
		//Try to load our user-specified font image.
		if (!fontFileRegular.empty()) {
			size_t fLen = fontFileRegular.length();
			bool validFont = true;

			//Is the file a PNG file by name?
			if (fLen<5 || fontFileRegular[fLen-4]!='.' || fontFileRegular[fLen-3]!='p' || fontFileRegular[fLen-2]!='n' || fontFileRegular[fLen-1]!='g') {
				validFont = false;
			} else {
				//Does the file exist?
				FILE* fontFile = fopen(fontFileRegular.c_str(), "rb");
				if (fontFile == NULL)
					validFont = false;
				else {
					//Try to read its data into a char[] array; get the size, too
					fseek (fontFile, 0, SEEK_END);
					long fileSize = ftell(fontFile);
					rewind(fontFile);
					char * file_buff = new char[fileSize];
					size_t file_buff_size = fread(file_buff, 1, fileSize, fontFile);
					fclose(fontFile);

					if (file_buff_size!=fileSize)
						validFont = false;
					else {
						//Ok, load our font
						mmFontBlack = new PulpCoreFont();
						mainWindow->initPulpCoreImage(mmFontBlack, file_buff, file_buff_size);

						//Is our font in error? If so, load the embedded font
						if (mmFontBlack->isInError()==TRUE) {
							wstringstream msg;
							msg <<"Custom font didn't load correctly: " <<mmFontBlack->getErrorMsg();
							MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);

							validFont = false;
							delete mmFontBlack;
						}
					}
				}
			}

			//Did we make it?
			if (!validFont) {
				fontFileRegular = "";
			}
		}


		//Do we need to load the internal font?
		if (fontFileRegular.empty()) {
			HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_FONT), _T("COREFONT"));
			if (!fontRes) {
				MessageBox(NULL, _T("Couldn't find WZ_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
				return;
			}

			//Get a handle from this resource.
			HGLOBAL res_handle = LoadResource(NULL, fontRes);
			if (!res_handle) {
				MessageBox(NULL, _T("Couldn't get a handle on WZ_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
				return;
			}

			//Create our PulpCoreFont (it's white when we load it, not black, by the way)
			mmFontBlack = new PulpCoreFont();
			//mmFontBlack->init(fontRes, res_handle, mainWindow->WARNINGgetUnderDC());
			mainWindow->initPulpCoreImage(mmFontBlack, fontRes, res_handle);

			//Unlock this resource for later use.
			UnlockResource(res_handle);
		}

		//Is our embedded font in error?
		if (mmFontBlack->isInError()==TRUE) {
			wstringstream msg;
			msg <<"WZ Font didn't load correctly: " <<mmFontBlack->getErrorMsg();

			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Copy-construct a new font
		mmFontGreen = new PulpCoreFont();
		mainWindow->initPulpCoreImage(mmFontGreen, mmFontBlack);

		mmFontRed = new PulpCoreFont();
		mainWindow->initPulpCoreImage(mmFontRed, mmFontBlack);

		//Tint both to their respective colors
		mmFontGreen->tintSelf(0x008000);
		mmFontBlack->tintSelf(0x000000);
		mmFontRed->tintSelf(0xFF0000);
	}


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
		helpWindow->initPulpCoreImage(helpFntKeys, fontRes, res_handle);
		if (helpFntKeys->isInError()==TRUE) {
			wstringstream msg;
			msg <<"WZ Help Font (keys) didn't load correctly: " <<helpFntKeys->getErrorMsg();

			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Tint to default
		helpFntKeys->tintSelf(COLOR_HELPFNT_KEYS);

		//Unlock this resource for later use.
		UnlockResource(res_handle);
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
		helpWindow->initPulpCoreImage(helpFntFore, fontRes, res_handle);
		if (helpFntFore->isInError()==TRUE) {
			wstringstream msg;
			msg <<"WZ Help Font (foreground) didn't load correctly: " <<helpFntFore->getErrorMsg();

			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Tint to default
		helpFntFore->tintSelf(COLOR_HELPFNT_FORE);

		//Unlock this resource for later use.
		UnlockResource(res_handle);
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
		helpWindow->initPulpCoreImage(helpFntBack, fontRes, res_handle);
		if (helpFntBack->isInError()==TRUE) {
			wstringstream msg;
			msg <<"WZ Help Font (background) didn't load correctly: " <<helpFntBack->getErrorMsg();

			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Tint to default
		helpFntBack->tintSelf(COLOR_HELPFNT_BACK);

		//Unlock this resource for later use.
		UnlockResource(res_handle);
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
		helpWindow->initPulpCoreImage(helpCornerImg, imgRes, res_handle);
		if (helpCornerImg->isInError()==TRUE) {
			wstringstream msg;
			msg <<"WZ Corner Image File didn't load correctly: " <<helpCornerImg->getErrorMsg();

			MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Unlock this resource for later use.
		UnlockResource(res_handle);
	}


	//Save resources if we don't use the second window
	/*if (typePhrases==FALSE)
		return;*/


	//Try to load our user-specified font image.
	if (!fontFileSmall.empty()) {
		size_t fLen = fontFileSmall.length();
		bool validFont = true;

		//Is the file a PNG file by name?
		if (fLen<5 || fontFileSmall[fLen-4]!='.' || fontFileSmall[fLen-3]!='p' || fontFileSmall[fLen-2]!='n' || fontFileSmall[fLen-1]!='g') {
			validFont = false;
		} else {
			//Does the file exist?
			FILE* fontFile = fopen(fontFileSmall.c_str(), "rb");
			if (fontFile == NULL)
				validFont = false;
			else {
				//Try to read its data into a char[] array; get the size, too
				fseek (fontFile, 0, SEEK_END);
				long fileSize = ftell(fontFile);
				rewind(fontFile);
				char * file_buff = new char[fileSize];
				size_t file_buff_size = fread(file_buff, 1, fileSize, fontFile);
				fclose(fontFile);

				if (file_buff_size!=fileSize)
					validFont = false;
				else {
					//Ok, load our font
					mmFontSmallWhite = new PulpCoreFont();
					sentenceWindow->initPulpCoreImage(mmFontSmallWhite, file_buff, file_buff_size);

					//Is our font in error? If so, load the embedded font
					if (mmFontSmallWhite->isInError()==TRUE) {
						wstringstream msg;
						msg <<"Custom (small) font didn't load correctly: " <<mmFontSmallWhite->getErrorMsg();
						MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);

						validFont = false;
						delete mmFontSmallWhite;
					}
				}
			}
		}

		//Did we make it?
		if (!validFont) {
			fontFileSmall = "";
		}
	}


	//Do we need to load the embedded font as backup?
	if (fontFileSmall.empty()) {
		//Now, our small font (resource first!)
		HRSRC fontRes2 = FindResource(hInst, MAKEINTRESOURCE(WZ_SMALL_FONT), _T("COREFONT"));
		if (!fontRes2) {
			MessageBox(NULL, _T("Couldn't find WZ_SMALL_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		//Get a handle from this resource.
		HGLOBAL res_handle_2 = LoadResource(NULL, fontRes2);
		if (!res_handle_2) {
			MessageBox(NULL, _T("Couldn't get a handle on WZ_SMALL_FONT"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		mmFontSmallWhite = new PulpCoreFont();
		sentenceWindow->initPulpCoreImage(mmFontSmallWhite, fontRes2, res_handle_2);

		//Unlock this resource for later use.
		UnlockResource(res_handle_2);
	}


	//Is our embedded font in error?
	if (mmFontSmallWhite->isInError()==TRUE) {
		wstringstream msg;
		msg <<"WZ Small Font didn't load correctly: " <<mmFontSmallWhite->getErrorMsg();

		MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
		return;
	}


	//Copy this font for use in the memory box
	helpFntMemory = new PulpCoreFont();
	mainWindow->initPulpCoreImage(helpFntMemory, mmFontSmallWhite);


	//Tint
	mmFontSmallWhite->tintSelf(0xFFFFFF);

	//New copy
	mmFontSmallGray = new PulpCoreFont();
	mainWindow->initPulpCoreImage(mmFontSmallGray, mmFontSmallWhite);
	mmFontSmallGray->tintSelf(0x333333);

	//New copy
	mmFontSmallRed = new PulpCoreFont();
	mainWindow->initPulpCoreImage(mmFontSmallRed, mmFontSmallWhite);
	mmFontSmallRed->tintSelf(0xFF0000);

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



void readUserWords() {
	//Read our words file, if it exists.
	if (currTest != model_print) {
		numCustomWords = -1;
		FILE* userFile = fopen(mywordsFileName.c_str(), "r");
		if (userFile != NULL) {
			//Get file size
			fseek (userFile, 0, SEEK_END);
			long fileSize = ftell(userFile);
			rewind(userFile);

			//Read it all into an array, close the file.
			char * buffer = (char*) malloc(sizeof(char)*fileSize);
			size_t buff_size = fread(buffer, 1, fileSize, userFile);
			fclose(userFile);

			numCustomWords = 0;
			if (buff_size==0) {
				return; //Empty file.
			}

			//Finally, convert this array to unicode
			TCHAR * uniBuffer;
			size_t numUniChars = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, NULL, 0);
			uniBuffer = (TCHAR*) malloc(sizeof(TCHAR)*numUniChars);
			if (!MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, uniBuffer, (int)numUniChars)) {
				MessageBox(NULL, _T("mywords.txt contains invalid UTF-8 characters.\n\nWait Zar will still function properly; however, your custom dictionary will be ignored."), _T("Warning"), MB_ICONWARNING | MB_OK);
				return;
			}
			delete [] buffer;

			//Skip the BOM, if it exists
			size_t currPosition = 0;
			if (uniBuffer[currPosition] == UNICOD_BOM)
				currPosition++;
			else if (uniBuffer[currPosition] == BACKWARDS_BOM) {
				MessageBox(NULL, _T("mywords.txt appears to be backwards. You should fix the Unicode encoding using Notepad or another Windows-based text utility.\n\nWait Zar will still function properly; however, your custom dictionary will be ignored."), _T("Warning"), MB_ICONWARNING | MB_OK);
				return;
			}

			//Read each line
			TCHAR* name = new TCHAR[100];
			char* value = new char[100];
			while (currPosition<numUniChars) {
				//Get the name/value pair using our nifty template function....
				readLine(uniBuffer, currPosition, numUniChars, true, true, false, allowNonBurmeseLetters, true, false, false, false, name, value);

				//Make sure both name and value are non-empty
				if (strlen(value)==0 || lstrlen(name)==0)
					continue;

				//Add this romanization
				if (!model.addRomanization(name, value) && !ignoreMywordsWarnings) {
					MessageBox(NULL, model.getLastError().c_str(), _T("Error adding Romanisation"), MB_ICONERROR | MB_OK);
				}
				numCustomWords++;
			}
			delete [] uniBuffer;
			delete [] name;
			delete [] value;


			if (numCustomWords>0 && customDictWarning==TRUE)
				MessageBox(NULL, _T("Warning! You are using a custom dictionary: \"mywords.txt\".\nThis feature of Wait Zar is EXPERIMENTAL; WaitZar.exe may crash.\n(You may disable this warning by setting mywordswarning = no in config.txt).\n\nPlease report any crashes at the issues page: \nhttp://code.google.com/p/waitzar/issues/list\n\nPress \"Ok\" to continue using Wait Zar."), _T("Warning..."), MB_ICONWARNING | MB_OK);

		} else {
			//Special case if testing
			if (currTest == mywords) {
				MessageBox(NULL, _T("Error! Custom mywords file does not exist!"), _T("Test-Related Error"), MB_ICONWARNING | MB_OK);
			}
		}
	}
}


void setEncoding(ENCODING encoding)
{
	if (encoding==ENCODING_WININNWA)
		currEncStr = L"WI";
	else if (encoding==ENCODING_ZAWGYI)
		currEncStr = L"ZG";
	else if (encoding==ENCODING_UNICODE)
		currEncStr = L"UNI";

	//Set this encoding, and save its value (in case we haven't loaded the model yet)
	mostRecentEncoding = encoding;
	model.setOutputEncoding(encoding);
}


void loadConfigOptions()
{
	//Default keys
	lstrcpy(langHotkeyString, _T("Ctrl+Shift"));
	strcpy(langHotkeyRaw, "^+");

	//Default encoding
	setEncoding(ENCODING_UNICODE);

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
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				customDictWarning = TRUE;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				customDictWarning = FALSE;
			else
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
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typePhrases = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typePhrases = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "burmesenumerals")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typeBurmeseNumbers = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typeBurmeseNumbers = false;
			else
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
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				dontLoadModel = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				dontLoadModel = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "silencemywordserrors")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				ignoreMywordsWarnings = true;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				ignoreMywordsWarnings = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "charaset")==0) {
			numConfigOptions++;
			if (strcmp(value, "any")==0)
				allowNonBurmeseLetters = true;
			else if (strcmp(value, "burmese")==0 || strcmp(value, "myanmar")==0)
				allowNonBurmeseLetters = false;
			else
				numConfigOptions--;
		} else if (strcmp(name, "defaultencoding")==0) {
			numConfigOptions++;
			if (strcmp(value, "wininnwa")==0)
				setEncoding(ENCODING_WININNWA);
			else if (strcmp(value, "zawgyi")==0)
				setEncoding(ENCODING_ZAWGYI);
			else if (strcmp(value, "unicode")==0 || strcmp(value, "parabaik")==0 || strcmp(value, "padauk")==0 || strcmp(value, "myanmar3")==0)
				setEncoding(ENCODING_UNICODE);
			else
				numConfigOptions--;
		} else if (strcmp(name, "hotkey")==0) {
			//Set it later
			strcpy(langHotkeyRaw, value);
			numConfigOptions++;
		/*} else if (strcmp(name, "dictionarysize")==0) {
			long val = atol(value);
			if (val>=0 && val<UINT_MAX) {
				maxDictionaryEntries = (unsigned int)val;
				numConfigOptions++;
			}
		} else if (strcmp(name, "nexussize")==0) {
			long val = atol(value);
			if (val>=0 && val<UINT_MAX) {
				maxNexusEntries = (unsigned int)val;
				numConfigOptions++;
			}
		} else if (strcmp(name, "prefixsize")==0) {
			long val = atol(value);
			if (val>=0 && val<UINT_MAX) {
				maxPrefixEntries = (unsigned int)val;
				numConfigOptions++;
			}*/
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



/**
 * Load the Wait Zar language model.
 */
bool loadModel() {
	//Load our embedded resource, the WaitZar model
	HGLOBAL     res_handle = NULL;
	HRSRC       res;
    char *      res_data;
    DWORD       res_size;

	//Previous versions of Wait Zar used the Google sparse hash library; however, even with
	//  its small footprint, this method required too much memory. So, we'll just allocate
	//  a jagged array.

	//Special...
	int numberCheck = 0;

	if (dontLoadModel) {
		//For any of the "size" values that are set to "default", let's see
		//   if we can come up with sensible defaults
		/*if (maxDictionaryEntries==0)
			maxDictionaryEntries = 3000; //WaitZar has only 2,400 words
		if (maxNexusEntries==0)
			maxNexusEntries = maxDictionaryEntries*2-maxDictionaryEntries/3; //Assume an even distribution of 3000 words of 5 letters each on 26 keys, then take 20% of this
		if (maxPrefixEntries==0)
			maxPrefixEntries = maxDictionaryEntries+maxDictionaryEntries/3; //Assume the same, and cut it by 5*/

		//Create our data structures
		//In total, this uses 41KB of raw memory just for storing our skeleton, so
		//  I estimate about 1MB of memory for actually storing the data.
		//  That's a lot, but it's worth it so that people's custom mywords files don't crash randomly.
		vector<wstring> dictionary;
		vector< vector<unsigned int> > nexus;
		vector< vector<unsigned int> >  prefix;

		//Of all these, only nexus is assumed to have anything in it
		nexus.push_back(vector<unsigned int>());

		//This should totally work :P (yes, I tested it rigorously)
		model = WordBuilder(dictionary, nexus, prefix);
	} else {
		{
			//Load the resource as a byte array and get its size, etc.
			res = FindResource(hInst, MAKEINTRESOURCE(WZ_MODEL), _T("Model"));
			if (!res) {
				MessageBox(NULL, _T("Couldn't find WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
				return false;
			}
			res_handle = LoadResource(NULL, res);
			if (!res_handle) {
				MessageBox(NULL, _T("Couldn't get a handle on WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
				return false;
			}
			res_data = (char*)LockResource(res_handle);
			res_size = SizeofResource(NULL, res);

			//Save our "model"
			model = WordBuilder(res_data, res_size, allowNonBurmeseLetters);

			//Done - This shouldn't matter, though, since the process only
			//       accesses it once and, fortunately, this is not an external file.
			UnlockResource(res_handle);
		}


		//We also need to load our easy pat-sint combinations
		if (currTest != model_print) {
			//Load the resource as a byte array and get its size, etc.
			res = FindResource(hInst, MAKEINTRESOURCE(WZ_EASYPS), _T("Model"));
			if (!res) {
				MessageBox(NULL, _T("Couldn't find WZ_EASYPS"), _T("Error"), MB_ICONERROR | MB_OK);
				return false;
			}
			res_handle = LoadResource(NULL, res);
			if (!res_handle) {
				MessageBox(NULL, _T("Couldn't get a handle on WZ_EASYPS"), _T("Error"), MB_ICONERROR | MB_OK);
				return false;
			}
			res_data = (char*)LockResource(res_handle);
			res_size = SizeofResource(NULL, res);

			//We, unfortunately, have to convert this to unicode now...
			wchar_t *uniData = new wchar_t[res_size];
			mymbstowcs(uniData, res_data, res_size);
			DWORD uniSize = wcslen(uniData);

			//Now, read through each line and add it to the external words list.
			wchar_t pre[200];
			wchar_t curr[200];
			wchar_t post[200];
			size_t index = 0;

			//Skip the BOM
			//if (res_data[index] == 0xFE && res_data[index+1]==0xFF)
			//	index += 2;

			bool inError = false;
			for (;index<uniSize;) {
				//Left-trim
				while (uniData[index] == ' ')
					index++;

				//Comment? Empty line? If so, skip...
				if (uniData[index]=='#' || uniData[index]=='\n') {
					while (uniData[index] != '\n')
						index++;
					index++;
					continue;
				}

				//Init
				pre[0] = 0x0000;
				int pre_pos = 0;
				bool pre_done = false;
				curr[0] = 0x0000;
				int curr_pos = 0;
				bool curr_done = false;
				post[0] = 0x0000;
				int post_pos = 0;

				//Ok, look for pre + curr = post
				while (index<uniSize) {
					if (uniData[index] == '\n') {
						index++;
						break;
					} else if (uniData[index] == '+') {
						//Switch modes
						pre_done = true;
						index++;
					} else if (uniData[index] == '=') {
						//Switch modes
						pre_done = true;
						curr_done = true;
						index++;
					} else if (uniData[index] >= 0x1000 && uniData[index] <= 0x109F) {
						//Add this to the current string
						if (curr_done) {
							post[post_pos++] = uniData[index++];
						} else if (pre_done) {
							curr[curr_pos++] = uniData[index++];
						} else {
							pre[pre_pos++] = uniData[index++];
						}
					} else {
						//Ignore it; avoid weird errors
						index++;
					}
				}

				//Ok, seal the strings
				post[post_pos++] = 0x0000;
				curr[curr_pos++] = 0x0000;
				pre[pre_pos++] = 0x0000;

				//Do we have anything?
				if (wcslen(post)!=0 && wcslen(curr)!=0 && wcslen(pre)!=0) {
					//Ok, process these strings and store them
					if (!model.addShortcut(pre, curr, post)) {
						if (!inError)
							MessageBox(NULL, model.getLastError().c_str(), _T("Error"), MB_ICONERROR | MB_OK);
						inError = true;

						if (isLogging) {
							for (size_t q=0; q<model.getLastError().size(); q++)
								fprintf(logFile, "%c", model.getLastError()[q]);
							fprintf(logFile, "\n  pre: ");
							for (unsigned int x=0; x<wcslen(pre); x++)
								fprintf(logFile, "U+%x ", pre[x]);
							fprintf(logFile, "\n");

							fprintf(logFile, "  curr: ");
							for (unsigned int x=0; x<wcslen(curr); x++)
								fprintf(logFile, "U+%x ", curr[x]);
							fprintf(logFile, "\n");

							fprintf(logFile, "  post: ");
							for (unsigned int x=0; x<wcslen(post); x++)
								fprintf(logFile, "U+%x ", post[x]);
							fprintf(logFile, "\n\n");
						}
					}
				}
			}

			if (inError)
				return false;

			//Free memory
			delete [] uniData;

			//Done - This shouldn't matter, though, since the process only
			//       accesses it once and, fortunately, this is not an external file.
			UnlockResource(res_handle);
		}
	}

	//One final check
	if (model.isInError())
		return false;

	return true;
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
	}
}


//Keeps all variables in sync, and allows repositioning
/*void ShowAWindow(const HWND &windowToShow, bool &flagToSet, int cmdShow, bool doReposition)
{
	//Avoid duplicate commands
	bool show = (cmdShow==SW_SHOW);
	if (flagToSet==show)
		return;

	//Re-position?
	if (!flagToSet && doReposition)
		positionAtCaret();

	//Set flags, perform move
	ShowWindow(windowToShow, cmdShow);
	flagToSet = show;
}*/



//Wrapper for MainWindow
/*void ShowMainWindow(int cmdShow)
{
	ShowAWindow(mainWindow, mainWindow->isVisible(), cmdShow, true);
}

//Wrapper for SenWindow
void ShowSubWindow(int cmdShow)
{
	ShowAWindow(senWindow, sentenceWindow->isVisible(), cmdShow, true);
}

//Wrapper for HelpWindow
void ShowHelpWindow(int cmdShow)
{
	ShowAWindow(helpWindow, helpWindow->isVisible(), cmdShow, false);
	ShowAWindow(memoryWindow, memoryWindow->isVisible(), cmdShow, false);
}*/



//Helpful wrapper
void ShowBothWindows(int cmdShow)
{
	bool show = (cmdShow==SW_SHOW);
	mainWindow->showWindow(show);

	if (typePhrases)
		sentenceWindow->showWindow(show);
}



void switchToLanguage(bool toMM) {
	//Don't do anything if we are switching to the SAME language.
	if (toMM == mmOn)
		return;

	//Ok, switch
	bool res;
	if (toMM) {
		res = turnOnHotkeys(true, true, true) && turnOnPunctuationkeys(true);
		//if (typeBurmeseNumbers==TRUE)
		res = turnOnNumberkeys(true) && res; //JUST numbers, not control.

		//Turno on our extended key set, too, to capture things like "("
		res = turnOnExtendedKeys(true) && res;

		//Register our help key too
		if (!mainWindow->registerHotKey(HOTKEY_HELP, NULL, VK_F1))
			res = false;
	} else {
		res = turnOnHotkeys(false, true, true);

		//It's possible we still have some hotkeys left on...
		if (controlKeysOn)
			turnOnControlkeys(false);
		if (numberKeysOn)
			turnOnNumberkeys(false);
		if (punctuationKeysOn)
			turnOnPunctuationkeys(false);
		if (extendedKeysOn)
			turnOnExtendedKeys(false);
		if (helpKeysOn)
			turnOnHelpKeys(false);

		//Turn off our help key
		if (!mainWindow->unregisterHotKey(HOTKEY_HELP))
			res = false;
	}

	//Any errors?
	if (!res)
		MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

	//Switch to our target language.
	mmOn = toMM;

	//Change icon in the tray
	// NOTE: Error is somewhere HERE (in creation, not ShellNotifyIcon)
	NOTIFYICONDATA nid;
	mainWindow->initShellNotifyIconData(nid);
	//nid.cbSize = sizeof(NOTIFYICONDATA);
	//nid.hWnd = mainWindow;
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

	//Any windows left?
	if (!mmOn) {
		ShowBothWindows(SW_HIDE);

		if (helpWindow->isVisible()) {
			helpWindow->showWindow(false);
			//ShowHelpWindow(SW_HIDE);
		}
	}
}


void reBlit()
{
	//Bit blit our back buffer to the front (should prevent flickering)
	mainWindow->repaintWindow();
	//BitBlt(mainDC,0,0,C_WIDTH,C_HEIGHT,mainUnderDC,0,0,SRCCOPY);
	if (typePhrases)
		sentenceWindow->repaintWindow();
		//BitBlt(senDC,0,0,SUB_C_WIDTH,SUB_C_HEIGHT,senUnderDC,0,0,SRCCOPY);
}


//Can't just blit it; we have to use updatelayeredwindow
void reBlitHelp()
{
	//Help Window
	//if (UpdateLayeredWindow(helpWindow, GetDC(NULL), NULL, &HELP_CLIENT_SIZE, helpUnderDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==FALSE) {
	if (!helpWindow->repaintWindow()) {
		wstringstream msg;
		msg <<"Help window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
		//DestroyWindow(mainWindow);
	}

	//Memory Window
	//if (UpdateLayeredWindow(memoryWindow, GetDC(NULL), NULL, &MEMORY_CLIENT_SIZE, memoryUnderDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==FALSE) {
	if (!memoryWindow->repaintWindow()) {
		wstringstream msg;
		msg <<"Memory window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
		//DestroyWindow(mainWindow);
	}
}


//Only blit part of the area
void reBlit(RECT blitArea)
{
	//Bit blit our back buffer to the front (should prevent flickering)
	mainWindow->repaintWindow(blitArea);
	//BitBlt(mainDC,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,mainUnderDC,blitArea.left,blitArea.top,SRCCOPY);
	if (typePhrases)
		sentenceWindow->repaintWindow(blitArea);
		//BitBlt(senDC,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,senUnderDC,blitArea.left,blitArea.top,SRCCOPY);
}


//Can't just blit it; we have to use updatelayeredwindow
//NOTE: We have to track when this is called, instead of just repainting the entire window.
//NOTE: Seems like it's never called.
void reBlitHelp(RECT blitArea)
{
	//Help Window
	//if (UpdateLayeredWindow(helpWindow, GetDC(NULL), NULL, &HELP_CLIENT_SIZE, helpUnderDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==FALSE) {
	if (!helpWindow->repaintWindow(blitArea)) {
		wstringstream msg;
		msg <<"Help window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
		//DestroyWindow(mainWindow);
	}

	//Memory Window
	//if (UpdateLayeredWindow(memoryWindow, GetDC(NULL), NULL, &MEMORY_CLIENT_SIZE, memoryUnderDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==FALSE) {
	if (!memoryWindow->repaintWindow(blitArea)) {
		wstringstream msg;
		msg <<"Memory window failed to update: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		delete mainWindow;
		//DestroyWindow(mainWindow);
	}
}


void initCalculate()
{
	//Figure out how big each of our areas is, and where they start
	spaceWidth = mmFontBlack->getStringWidth(_T(" "));
	firstLineStart = borderWidth;
	secondLineStart = firstLineStart + mmFontBlack->getHeight() + spaceWidth + borderWidth;
	thirdLineStart = secondLineStart + mmFontBlack->getHeight() + spaceWidth + borderWidth;
	fourthLineStart = thirdLineStart + (mmFontBlack->getHeight()*8)/13 + borderWidth;

	//Now, set the window's default height
	mainWindow->setDefaultSize(mainWindow->getDefaultWidth(), fourthLineStart);
	//WINDOW_HEIGHT = fourthLineStart;
}


/*void expandHWND(HWND hwnd, HDC &dc, HDC &underDC, HBITMAP &bmp, int newX, int newY, bool noMove, int newWidth, int newHeight, int &SAVED_CLIENT_WIDTH, int &SAVED_CLIENT_HEIGHT)
{
	//Resize the current window; use SetWindowPos() since it's easier...
	int flags = SWP_NOZORDER | SWP_NOACTIVATE;
	if (noMove)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd, NULL, newX, newY, newWidth, newHeight, flags );
	RECT r;
	GetClientRect(hwnd, &r);
	SAVED_CLIENT_WIDTH = r.right;
	SAVED_CLIENT_HEIGHT = newHeight;

	//We also have to set our graphics contexts correctly. Also, throw out the old ones.
	DeleteDC(underDC);
	DeleteObject(bmp);
	dc = GetDC(hwnd);
	underDC = CreateCompatibleDC(dc);
	bmp = CreateCompatibleBitmap(dc, SAVED_CLIENT_WIDTH, SAVED_CLIENT_HEIGHT);
	SelectObject(underDC, bmp);
}*/


/*void expandHWND(HWND hwnd, HDC &dc, HDC &underDC, HBITMAP &bmp, int newWidth, int newHeight, int &SAVED_CLIENT_WIDTH, int &SAVED_CLIENT_HEIGHT)
{
	expandHWND(hwnd, dc, underDC, bmp, 0, 0, true, newWidth, newHeight, SAVED_CLIENT_WIDTH, SAVED_CLIENT_HEIGHT);
}*/



/**
 * Initialize our on-screen keyboard
 */
void initCalculateHelp()
{
	//Initialize our keyboard
	helpKeyboard = new OnscreenKeyboard(mmFontSmallWhite, helpFntKeys, helpFntFore, helpFntBack, helpFntMemory, helpCornerImg);
}


/*wchar_t* makeStringFromKeystrokes(std::vector<unsigned short> keystrokes)
{
	for (unsigned int i=0; i<keystrokes.size(); i++) {
		returnVal[i] = keystrokes[i];
	}
	returnVal[keystrokes.size()] = 0x0000;

	return returnVal;
}*/



/**
 * Re-figure the layout of our drawing area, resize if necessary, and
 * draw onto the back buffer. Finally, blit to the front buffer.
 */
void recalculate()
{
	//First things first: can we fit this in the current background?
	int cumulativeWidth = (borderWidth+1)*2;
	std::vector<UINT32> words =  model.getPossibleWords();
	for (size_t i=0; i<words.size(); i++) {
		cumulativeWidth += mmFontBlack->getStringWidth(model.getWordString(words[i]));
		cumulativeWidth += spaceWidth;
	}

	//Extra width for pat-sint suggestion?
	if (model.hasPostStr()) {
		cumulativeWidth += mmFontBlack->getStringWidth(model.getPostString());
		cumulativeWidth += spaceWidth;
	}

	//If not, resize. Also, keep the size small when possible.
	if (cumulativeWidth>mainWindow->getClientWidth())
		mainWindow->expandWindow(cumulativeWidth, mainWindow->getClientHeight());
		//expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, cumulativeWidth, C_HEIGHT, C_WIDTH, C_HEIGHT);
	else if (cumulativeWidth<mainWindow->getDefaultWidth() && mainWindow->getClientWidth()>mainWindow->getDefaultWidth())
		mainWindow->expandWindow(mainWindow->getDefaultWidth(), mainWindow->getClientHeight());
		//expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, WINDOW_WIDTH, C_HEIGHT, C_WIDTH, C_HEIGHT);
	else if (cumulativeWidth>mainWindow->getDefaultWidth() && cumulativeWidth<mainWindow->getClientWidth())
		mainWindow->expandWindow(cumulativeWidth, mainWindow->getClientHeight());
		//expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, cumulativeWidth, C_HEIGHT, C_WIDTH, C_HEIGHT);

	//Background
	mainWindow->selectObject(g_BlackPen);
	mainWindow->selectObject(g_DarkGrayBkgrd);
	mainWindow->drawRectangle(0, 0, mainWindow->getClientWidth(), mainWindow->getClientHeight());

	//Background -second window
	if (typePhrases) {
		//Draw the background
		sentenceWindow->selectObject(g_BlackPen);
		sentenceWindow->selectObject(g_DarkGrayBkgrd);
		sentenceWindow->drawRectangle(0, 0, sentenceWindow->getClientWidth(), sentenceWindow->getClientHeight());

		//Draw each string
		std::list<int>::const_iterator printIT = sentence.begin();
		int currentPosX = borderWidth + 1;
		int cursorPosX = currentPosX;
		int counterCursorID=0;
		int countup = 0;
		for (;printIT != sentence.end(); printIT++) {
			//Append this string
			wstring strToDraw;
			PulpCoreFont* colorFont = mmFontSmallWhite;
			if (*printIT>=0)
				strToDraw = model.getWordString(*printIT);
			else {
				int numSystemWords = systemDefinedWords.size();
				int id = -(*printIT)-1;
				if (id<numSystemWords) {
					strToDraw = systemDefinedWords[id];
				} else
					strToDraw = userDefinedWordsZg[id-numSystemWords];
			}
			if (countup++ == sentence.getCursorIndex() && model.hasPostStr() && patSintIDModifier==-1) {
				colorFont = mmFontSmallRed;
				if (patSintIDModifier==-1)
					strToDraw = model.getPostString();
			}
			sentenceWindow->drawString(colorFont, strToDraw, currentPosX, borderWidth+1);
			currentPosX += (mmFontSmallWhite->getStringWidth(strToDraw)+1);

			//Line? (don't print now; we also want to draw it at cursorIndex==-1)
			if (counterCursorID == sentence.getCursorIndex())
				cursorPosX = currentPosX;

			//Increment
			counterCursorID++;
		}

		//Draw the cursor
		sentenceWindow->moveTo(cursorPosX-1, borderWidth+1);
		sentenceWindow->drawLineTo(cursorPosX-1, sentenceWindow->getClientHeight()-borderWidth-1);

		//Draw the current encoding
		int encStrWidth = mmFontSmallWhite->getStringWidth(currEncStr);
		sentenceWindow->selectObject(g_BlackPen);
		sentenceWindow->selectObject(g_GreenBkgrd);
		sentenceWindow->drawRectangle(sentenceWindow->getClientWidth()-encStrWidth-3, 0, sentenceWindow->getClientWidth(), sentenceWindow->getClientHeight());
		sentenceWindow->drawString(mmFontSmallWhite, currEncStr, sentenceWindow->getClientWidth()-encStrWidth-2, sentenceWindow->getClientHeight()/2-mmFontSmallWhite->getHeight()/2);
	}

	//White overlays
	mainWindow->selectObject(g_EmptyPen);
	mainWindow->selectObject(g_WhiteBkgrd);
	mainWindow->drawRectangle(borderWidth+1, firstLineStart+1, mainWindow->getClientWidth()-borderWidth-1, secondLineStart-borderWidth);
	mainWindow->drawRectangle(borderWidth+1, secondLineStart, mainWindow->getClientWidth()-borderWidth-1, thirdLineStart-borderWidth);
	mainWindow->drawRectangle(borderWidth+1, thirdLineStart, mainWindow->getClientWidth()-borderWidth-1, fourthLineStart-borderWidth-1);

	//Now, draw the strings....
	PulpCoreFont* mmFont = mmFontBlack;
	int xOffset = 0;
	TCHAR digit[5];
	wstring extendedWordString;
	if (helpWindow->isVisible()) {
		//Prepare the extended word string a bit early
		currStrZg = waitzar::sortMyanmarString(currStr);
		currStrZg = wstring(waitzar::renderAsZawgyi(currStrZg.c_str()));
		extendedWordString = currStrZg;

		//We only have one word: the guessed romanisation
		currStrDictID = -1;
		for (unsigned int i=0; i<model.getTotalDefinedWords(); i++) {
			//Does this word match?
			wstring currWord = model.getWordString(i);
			if (currWord == extendedWordString) {
				currLetterSt = currWord;
				currStrDictID = i;
				break;
			}
		}

		//Try a less-strict filtering
		// We leave this out... it shouldn't be necessary if we're filtering our strings.
		/*if (currMatchID==-1) {
			unsigned int currEnc = model->getOutputEncoding();
			model->setOutputEncoding(ENCODING_UNICODE);
			for (unsigned int i=0; i<model->getTotalDefinedWords(); i++) {
				wchar_t *currUni = makeStringFromKeystrokes(model->getWordKeyStrokes(i));
				if (wcscmp(currUni, currLetterSt)==0) {
					wcscpy(currLetterSt, currUni);
					currMatchID = i;
					break;
				}
			}
			model->setOutputEncoding(currEnc);
		}*/

		//Any match at all?
		if (currStrDictID!=-1) {
			string romanWord = model.reverseLookupWord(currStrDictID);

			currLetterSt = L'(';
			for (size_t q=0; q<romanWord.size(); q++)
				currLetterSt += romanWord[q];
			currLetterSt += L')';

			mainWindow->drawString(mmFontGreen, currLetterSt, borderWidth+1+spaceWidth/2, secondLineStart+spaceWidth/2);
		}

		//Helper text
		if (currStrZg.length()>0)
			mainWindow->drawString(mmFontSmallGray, L"(Press \"Space\" to type this word)", borderWidth+1+spaceWidth/2, thirdLineStart-spaceWidth/2);
	} else if (mainWindow->isVisible()) { //Crashes otherwise
		//Add the post-processed word, if it exists...
		int mySelectedID = ((int)model.getCurrSelectedID()) + patSintIDModifier;
		if (model.hasPostStr()) {
			words.insert(words.begin(), model.getPostID());
			mySelectedID++;
		}

		for (size_t i=0; i<words.size(); i++) {
			//If this is the currently-selected word, draw a box under it.
			//int x = words[i];
			int thisStrWidth = mmFont->getStringWidth(model.getWordString(words[i]));
			if (i!=mySelectedID)
				mmFont = mmFontBlack;
			else {
				mmFont = mmFontGreen;

				mainWindow->selectObject(g_YellowBkgrd);
				mainWindow->selectObject(g_GreenPen);
				mainWindow->drawRectangle(borderWidth+xOffset+1, secondLineStart, borderWidth+1+xOffset+thisStrWidth+spaceWidth, secondLineStart+mmFont->getHeight()+spaceWidth-1);
			}

			//Fix the pen if this is a post word
			// Also, fix the ID
			int wordID = (int)i;
			if (model.hasPostStr()) {
				wordID--;
				if (i==0)
					mmFont = mmFontRed;
			}

			//NOTE: Something is wrong with mmFont(Green); it's not getting initialized properly...
			mainWindow->drawString(mmFont, model.getWordString(words[i]), borderWidth+1+spaceWidth/2 + xOffset, secondLineStart+spaceWidth/2);

			if (wordID<10) {
				if (wordID>=0)
					swprintf(digit, _T("%i"), ((wordID+1)%10));
				else {
					digit[0] = '`';
					digit[1] = 0x0000;
					if (i!=mySelectedID)
						mmFont = mmFontBlack;
					else
						mmFont = mmFontGreen;
				}

				int digitWidth = mmFont->getStringWidth(digit);

				mainWindow->drawString(mmFont, digit, borderWidth+1+spaceWidth/2 + xOffset + thisStrWidth/2 -digitWidth/2, thirdLineStart-spaceWidth/2-1);
			}

			xOffset += thisStrWidth + spaceWidth;
		}
	}

	if (!helpWindow->isVisible()) {
		wstring parenStr = model.getParenString();
		wstringstream line;
		line <<currStr;
		if (!parenStr.empty()) {
			line <<" (" <<parenStr <<")";
		}
		extendedWordString = line.str();
	}

	mainWindow->drawString(mmFontBlack, extendedWordString, borderWidth+1+spaceWidth/2, firstLineStart+spaceWidth/2+1);

	//Paint
	reBlit();
}



wstring getUserWordKeyStrokes(unsigned int id, unsigned int encoding)
{
	//Get the string
	wstring typedStr;
	unsigned int numSystemDefWords = systemDefinedWords.size();
	if (id<numSystemDefWords) {
		typedStr = systemDefinedWords[id];
	} else {
		id -= numSystemDefWords;
		if (encoding==ENCODING_UNICODE)
			typedStr = userDefinedWords[id];
		else if (encoding==ENCODING_ZAWGYI)
			typedStr = userDefinedWordsZg[id];
		else if (encoding==ENCODING_WININNWA) {
			wstring srcStr = userDefinedWords[id];
			wchar_t destStr[200];
			convertFont(destStr, srcStr.c_str(), Myanmar3, WinInnwa);
			typedStr = destStr;
		} else
			typedStr = L"";
	}

	//Convert
	size_t length = typedStr.length();
	userKeystrokeVector.clear();
	for (size_t i=0; i<length; i++)
		userKeystrokeVector.push_back((unsigned short) typedStr[i]);

	return userKeystrokeVector;
}



void typeCurrentPhrase()
{
	//Send key presses to the top-level program.
	HWND fore = GetForegroundWindow();
	SetActiveWindow(fore); //This probably won't do anything, since we're not attached to this window's message queue.


	//Use SendInput instead of SendMessage, since SendMessage requires the actual
	//  sub-window (component) to recieve the message, whereas SendInput only
	//  requires the top-level window. We could probably hack in SendMessage now that
	//  we're not becoming the active window, but for now I'd rather have a stable
	//  system than one that works on Windows 98.
	std::list<int>::const_iterator printIT = sentence.begin();
	wstring keyStrokes;
	int number_of_key_events = 0;
	for (;printIT!=sentence.end() || stopChar!=0;) {
		//We may or may not have a half/full stop at the end.
		if (printIT!=sentence.end()) {
			if (*printIT>=0)
				keyStrokes = model.getWordKeyStrokes(*printIT);
			else {
				keyStrokes = getUserWordKeyStrokes(-(*printIT)-1, model.getOutputEncoding());
			}
		} else {
			keyStrokes.clear();
			keyStrokes.push_back(stopChar);
		}

		//Buffer each key-stroke
		for (size_t i=0; i<keyStrokes.size(); i++) {
			//Send keydown
			keyInputPrototype.wScan = (WORD)keyStrokes[i];
			keyInputPrototype.dwFlags = KEYEVENTF_UNICODE;
			inputItems[number_of_key_events++].ki = keyInputPrototype;

			keyInputPrototype.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;
			inputItems[number_of_key_events++].ki = keyInputPrototype;
		}

		//Increment
		if (printIT!=sentence.end())
			printIT++;
		else
			stopChar = 0;
	}


	//Send all the keystrokes at once to avoid a weird bug with single-letter repetitions.
	UINT numSent = SendInput(number_of_key_events,inputItems,sizeof(INPUT));
	if(numSent!=number_of_key_events || number_of_key_events==0) {
		wstringstream msg;
		msg <<"Couldn't send input, only sent " <<numSent <<" of " <<number_of_key_events <<" events. Error code: " <<GetLastError();
		MessageBox(NULL, msg.str().c_str(), _T("Error"), MB_OK|MB_ICONERROR);
	}



	//Now, reset...
	patSintIDModifier = 0;
	model.reset(true);
	sentence.clear();
	/*for (unsigned int i=0; i<userDefinedWords.size(); i++) {
		delete [] userDefinedWords[i];
		delete [] userDefinedWordsZg[i];
	}*/
	userDefinedWords.clear();
	userDefinedWordsZg.clear();


	//Technically, this can be called with JUST a stopChar, which implies
	//  that the window isn't visible. So check this.
	if (controlKeysOn) {
		//Turn off control keys
		turnOnControlkeys(false);

		//Hide the window(s)
		ShowBothWindows(SW_HIDE);
	}
}



BOOL selectWord(int id, bool indexNegativeEntries)
{
	//Are there any words to use?
	int wordID = id;
	if (!indexNegativeEntries) {
		//One last check: are we doing a pat-sint shortcut?
		if (patSintIDModifier==-1) {
			if (!model.hasPostStr())
				return FALSE;
			wordID = model.getPostID();
		} else {
			//Ok, look it up in the model as usual
			std::pair<BOOL, UINT32> typedVal = model.typeSpace(id);
			if (typedVal.first == FALSE)
				return FALSE;
			wordID = typedVal.second;
		}
	}

	//Optionally turn off numerals
	//if (numberKeysOn==TRUE && typeBurmeseNumbers==FALSE)
	//	turnOnNumberkeys(FALSE);
	if (!typePhrases) {
		//Simple Case
		sentence.clear();
		sentence.insert(wordID);
		typeCurrentPhrase();
	} else {
		//Pat-sint clears the previous word
		if (patSintIDModifier==-1)
			sentence.deletePrev(model);

		//Advanced Case - Insert
		sentence.insert(wordID);
	}

	return TRUE;
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
	//Only perform ONCE
	if (!mainInitDone || !sentenceInitDone || !helpInitDone || !memoryInitDone)
		return;

	//Create our font
	makeFont();

	//Perform initial calculations if this worked
	if (mmFontBlack->isInError()) {
		//Will generate a WM_DESTROY message
		delete mainWindow;
		return;
	}
	initCalculate();

	//Initialize the main window
	mainWindow->resizeWindow(mainWindow->getDefaultWidth(), mainWindow->getDefaultHeight());
	mainWindow->createDoubleBufferedSurface();

	//Initialize the sentence window
	sentenceWindow->resizeWindow(sentenceWindow->getDefaultWidth(), sentenceWindow->getDefaultHeight());
	sentenceWindow->createDoubleBufferedSurface();

	//Initialize the helper window
	helpWindow->resizeWindow(helpWindow->getDefaultWidth(), helpWindow->getDefaultHeight());
	helpWindow->createDoubleBufferedSurface();

	//Initialize the memory window
	memoryWindow->resizeWindow(memoryWindow->getDefaultWidth(), memoryWindow->getDefaultHeight());
	memoryWindow->createDoubleBufferedSurface();

	//Initialize the calculationf for the help window
	initCalculateHelp();
}



//Message handling for our help window
LRESULT CALLBACK HelpWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CREATE:
		{
			//Save our window, init done
			helpWindow->saveHwnd(hwnd);
			helpInitDone = true;

			//Performa general initialization
			onAllWindowsCreated();

			break;
		}
		/*case WM_NCHITTEST: //Allow dragging of the client area...
		{
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT) {
				return HTCAPTION;
			} else
				return uHitTest;
			break;
		}*/
		case WM_LBUTTONDOWN:
		{
			//Thanks to dr. Carbon for suggesting this method.
			if (SetCapture(hwnd)!=NULL)
				break;

			//Drag the mosue
			isDragging = true;
			GetCursorPos(&dragFrom);
			break;
		}
		case WM_MOUSEMOVE:
		{ //Allow dragging of the mouse by its client area. Reportedly more accurate than NCHIT_TEST
			if (isDragging) {
				RECT rect;
				POINT dragTo;
				GetWindowRect(hwnd, &rect);
				GetCursorPos(&dragTo);

				//Constantly update its position
				MoveWindow(hwnd, (dragTo.x - dragFrom.x) + rect.left,
					(dragTo.y - dragFrom.y) + rect.top,
					rect.right - rect.left, rect.bottom - rect.top, FALSE);

				dragFrom = dragTo;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (isDragging) {
				isDragging = false;
				ReleaseCapture();
			}
			break;
		}
		case WM_MOVE:
		{
			//Move the main window?
			/*if (senWindowSkipMove==FALSE && (mainWindow->isVisible() || sentenceWindow->isVisible()) && dragBothWindowsTogether==TRUE) {
				RECT r;
				GetWindowRect(hwnd, &r);
				RECT r2;
				GetWindowRect(GetDesktopWindow(), &r2);
				mainWindowSkipMove = TRUE;
				SetWindowPos(mainWindow, HWND_TOPMOST, min(max(r.left, 0), r2.right-C_WIDTH), max(r.top-C_HEIGHT, 0), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			senWindowSkipMove = FALSE;*/
			break;
		}
		case WM_PAINT:
		{
			//Update only if there's an area which needs updating (e.g., a higher-level
			//  window has dragged over this one's client area... it can happen only with popups,
			//  but let's do it just to be safe.
			RECT updateRect;
			if (GetUpdateRect(hwnd, &updateRect, FALSE) != 0)
			{
				//Blitting every tick will slow us down... we should validate the
				//  rectangle after drawing it.
				reBlitHelp(updateRect);

				//Validate the client area
				ValidateRect(hwnd, NULL);
			}

			break;
		}
		case WM_CLOSE:
			//Will generate a WM_DESTROY message.
			delete helpWindow;
			//DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}


//Message handling for our memory window
LRESULT CALLBACK MemoryWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CREATE:
		{
			//Save our window
			memoryWindow->saveHwnd(hwnd);
			memoryInitDone = true;

			//Performa general initialization
			onAllWindowsCreated();

			break;
		}
		case WM_LBUTTONDOWN:
		{
			//Thanks to dr. Carbon for suggesting this method.
			if (SetCapture(hwnd)!=NULL)
				break;

			//Drag the mosue
			isDragging = true;
			GetCursorPos(&dragFrom);
			break;
		}
		case WM_MOUSEMOVE:
		{ //Allow dragging of the mouse by its client area. Reportedly more accurate than NCHIT_TEST
			if (isDragging) {
				RECT rect;
				POINT dragTo;
				GetWindowRect(hwnd, &rect);
				GetCursorPos(&dragTo);

				//Constantly update its position
				MoveWindow(hwnd, (dragTo.x - dragFrom.x) + rect.left,
					(dragTo.y - dragFrom.y) + rect.top,
					rect.right - rect.left, rect.bottom - rect.top, FALSE);

				dragFrom = dragTo;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (isDragging) {
				isDragging = false;
				ReleaseCapture();
			}
			break;
		}
		case WM_PAINT:
		{
			//Update only if there's an area which needs updating (e.g., a higher-level
			//  window has dragged over this one's client area... it can happen only with popups,
			//  but let's do it just to be safe.
			RECT updateRect;
			if (GetUpdateRect(hwnd, &updateRect, FALSE) != 0)
			{
				//Blitting every tick will slow us down... we should validate the
				//  rectangle after drawing it.
				reBlitHelp(updateRect);

				//Validate the client area
				ValidateRect(hwnd, NULL);
			}

			break;
		}
		case WM_CLOSE:
			//Will generate a WM_DESTROY message.
			delete memoryWindow;
			//DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}




//Message handling for our secondary window
LRESULT CALLBACK SubWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CREATE:
		{
			//Save our window
			sentenceWindow->saveHwnd(hwnd);
			sentenceInitDone = true;

			//Performa general initialization
			onAllWindowsCreated();

			break;
		}
		/*case WM_NCHITTEST: //Allow dragging of the client area...
		{
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT) {
				return HTCAPTION;
			} else
				return uHitTest;
			break;
		}*/
		case WM_LBUTTONDOWN:
		{
			//Thanks to dr. Carbon for suggesting this method.
			if (SetCapture(hwnd)!=NULL)
				break;

			//Drag the mosue
			isDragging = true;
			GetCursorPos(&dragFrom);
			break;
		}
		case WM_MOUSEMOVE:
		{ //Allow dragging of the mouse by its client area. Reportedly more accurate than NCHIT_TEST
			if (isDragging) {
				RECT rect;
				POINT dragTo;
				GetWindowRect(hwnd, &rect);
				GetCursorPos(&dragTo);

				//Constantly update its position
				MoveWindow(hwnd, (dragTo.x - dragFrom.x) + rect.left,
					(dragTo.y - dragFrom.y) + rect.top,
					rect.right - rect.left, rect.bottom - rect.top, FALSE);

				dragFrom = dragTo;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (isDragging) {
				isDragging = false;
				ReleaseCapture();
			}
			break;
		}
		case WM_MOVE:
		{
			//Move the main window?
			if (!senWindowSkipMove && (mainWindow->isVisible() || sentenceWindow->isVisible()) && dragBothWindowsTogether) {
				RECT r;
				GetWindowRect(hwnd, &r);
				RECT r2;
				GetWindowRect(GetDesktopWindow(), &r2);
				mainWindowSkipMove = true;
				mainWindow->setWindowPosition(min(max(r.left, 0), r2.right-mainWindow->getClientWidth()), max(r.top-mainWindow->getClientHeight(), 0), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			senWindowSkipMove = false;
			break;
		}
		case WM_PAINT:
		{
			//Update only if there's an area which needs updating (e.g., a higher-level
			//  window has dragged over this one's client area... it can happen only with popups,
			//  but let's do it just to be safe.
			RECT updateRect;
			if (GetUpdateRect(hwnd, &updateRect, FALSE) != 0)
			{
				//Blitting every tick will slow us down... we should validate the
				//  rectangle after drawing it.
				reBlit(updateRect);

				//Validate the client area
				ValidateRect(hwnd, NULL);
			}

			break;
		}
		case WM_CLOSE:
			//Will generate a WM_DESTROY message.
			delete sentenceWindow;
			//DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}



void updateHelpWindow()
{
	if (!helpWindow->isVisible()) {
		//Did we even initialize the help window?
		if (!helpIsCached) {
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
			//HELP_CLIENT_SIZE.cx = newW;
			//HELP_CLIENT_SIZE.cy = newH;

			//Move the memory window, too
			memoryWindow->expandWindow(newX+helpKeyboard->getWidth(), newY, helpKeyboard->getMemoryWidth(), helpKeyboard->getMemoryHeight(), false);
			//MEMORY_CLIENT_SIZE.cx = newWMem;
			//MEMORY_CLIENT_SIZE.cy = newHMem;

			//Might as well build the reverse lookup
			model.reverseLookupWord(0);

			//...and now we can properly initialize its drawing surface
			helpKeyboard->init(helpWindow);
			helpKeyboard->initMemory(memoryWindow);

			helpIsCached = true;
		}


		//Register all hotkeys relevant for the help window
		bool res = true;
		if (!controlKeysOn) //We'll need these too.
			res = turnOnControlkeys(true);
		if (!turnOnHelpKeys(true) || !res)
			mainWindow->showMessageBox(L"Could not turn on the shift/control hotkeys.", L"Error", MB_ICONERROR | MB_OK);


		//Clear our current word (not the sentence, though, and keep the trigrams)
		patSintIDModifier = 0;
		currStr.clear();
		model.reset(false);
		recalculate();

		//Show the help window
		helpWindow->showWindow(true);
		//ShowHelpWindow(SW_SHOW);
		reBlitHelp();

		//Show the main/sentence windows; this is just good practice.
		if (!mainWindow->isVisible()) {
			//Show it.
			mainWindow->showWindow(true);
			//ShowMainWindow(SW_SHOW);
		}
		if (!sentenceWindow->isVisible()) {
			sentenceWindow->showWindow(true);
			//ShowSubWindow(SW_SHOW);
		}
	} else {
		//Clear our word string
		currStr.clear();

		turnOnHelpKeys(false);
		helpWindow->showWindow(false);
		//ShowHelpWindow(SW_HIDE);

		//Hide the main window, too, and possibly the secondary window
		mainWindow->showWindow(false);
		//ShowMainWindow(SW_HIDE);
		if (sentence.size()==0) {
			sentenceWindow->showWindow(false);
			//ShowSubWindow(SW_HIDE);
		}

		recalculate();
	}
}



/**
 * Message-handling code.
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Handle callback
	switch(msg) {
		case WM_CREATE:
		{
			//Save our window
			mainWindow->saveHwnd(hwnd);
			mainInitDone = true;

			//Performa general initialization
			onAllWindowsCreated();

			break;
		}
		case WM_MOVE:
		{
			//Move the sentence window?
			if (typePhrases && !mainWindowSkipMove && sentenceWindow!=NULL && sentenceWindow->isVisible() && dragBothWindowsTogether) {
				RECT r;
				GetWindowRect(hwnd, &r);
				RECT r2;
				GetWindowRect(GetDesktopWindow(), &r2);
				senWindowSkipMove = true;
				sentenceWindow->setWindowPosition(min(max(r.left, 0), r2.right-sentenceWindow->getClientWidth()), min(r.top+mainWindow->getClientHeight(), r2.bottom-sentenceWindow->getClientHeight()), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				//SetWindowPos(senWindow, HWND_TOPMOST, min(max(r.left, 0), r2.right-SUB_C_WIDTH), min(r.top+C_HEIGHT, r2.bottom-SUB_C_HEIGHT), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			mainWindowSkipMove = false;
			break;
		}
		case UWM_HOTKEY_UP: //HOTKEY_UP is defined by us, it is just like HOTKEY_DOWN except it doesn't use the lparam
		{
			//Update our virtual keyboard
			if (helpKeyboard->highlightKey(wParam, false))
				reBlitHelp();

			break;
		}
		case WM_HOTKEY:
		{
			//Added to help us avoid multiple reactions, if necessary
			// (probably only necessary for the number/myanmar numbers distinction)
			bool keyWasUsed = false;

			//Handle our main language hotkey
			if(wParam == LANG_HOTKEY) {
				//Switch language
				switchToLanguage(!mmOn);

				//Reset the model
				sentence.clear();
				patSintIDModifier = 0;
				model.reset(true);

				keyWasUsed = true;
			}


			//Should we update the virtual keyboard? This is done independently
			//  of actually handling the keypress itself
			if (helpWindow->isVisible() && highlightKeys) {
				//If this is a shifted key, get which key is shifted: left or right
				unsigned int keyCode = wParam;
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

				//Doesn't consume a keypress
				//keyWasUsed = keyWasUsed;
			}


			//What to do if our user hits "F1".
			if (wParam == HOTKEY_HELP && !allowNonBurmeseLetters) {
				updateHelpWindow();

				keyWasUsed = true;
			}


			//Close the window?
			if (wParam == HOTKEY_ESC) {
				if (helpWindow->isVisible()) {
					//Clear our word string
					currStr.clear();

					turnOnHelpKeys(false);
					helpWindow->showWindow(false);
					//ShowHelpWindow(SW_HIDE);

					//Hide the main window, too, and possibly the secondary window
					mainWindow->showWindow(false);
					//ShowMainWindow(SW_HIDE);
					if (sentence.size()==0)
						sentenceWindow->showWindow(false);
						//ShowSubWindow(SW_HIDE);

					recalculate();
				} else {
					if (!mainWindow->isVisible()) {
						//Kill the entire sentence.
						sentence.clear();
						patSintIDModifier = 0;
						model.reset(true);
						turnOnControlkeys(false);
						ShowBothWindows(SW_HIDE);
					} else {
						patSintIDModifier = 0;
						model.reset(false);

						//No more numbers
						//if (typeBurmeseNumbers==FALSE)
						//	turnOnNumberkeys(FALSE);

						//Are we using advanced input?
						if (!typePhrases) {
							//Turn off control keys
							turnOnControlkeys(false);
							ShowBothWindows(SW_HIDE);
						} else {
							//Just hide the typing window for now.
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							if (sentence.size()==0) {
								//Kill the entire sentence.
								sentence.clear();
								ShowBothWindows(SW_HIDE);
								turnOnControlkeys(false);
							} else
								recalculate();
						}
					}
				}

				keyWasUsed = true;
			}


			//Delete: Phrases only
			if (wParam == HOTKEY_DELETE) {
				if (helpWindow->isVisible()) {
					//Delete the letter in front of you (encoding-wise, not visibly)
					//ADD LATER

				} else {
					if (!mainWindow->isVisible()) {
						//Delete the next word
						if (sentence.deleteNext())
							recalculate();
						if (sentence.size()==0) {
							//Kill the entire sentence.
							sentence.clear();
							turnOnControlkeys(false);
							ShowBothWindows(SW_HIDE);
						}
					}
				}

				keyWasUsed = true;
			}


			//Back up
			if (wParam == HOTKEY_BACK) {
				if (helpWindow->isVisible()) {
					//Delete the letter in back of you (encoding-wise, not visibly)
					if (!currStr.empty())
						currStr.erase(currStr.length()-1);
					recalculate();
				} else {
					if (!mainWindow->isVisible()) {
						//Delete the previous word
						if (sentence.deletePrev(model))
							recalculate();
						if (sentence.size()==0) {
							//Kill the entire sentence.
							sentence.clear();
							turnOnControlkeys(false);
							ShowBothWindows(SW_HIDE);
						}
					} else {
						if (model.backspace()) {
							//Truncate...
							currStr.erase(currStr.length()-1);
							recalculate();
						} else {
							//No more numerals.
							//if (typeBurmeseNumbers==FALSE)
							//	turnOnNumberkeys(FALSE);

							//Are we using advanced input?
							if (!typePhrases) {
								//Turn off control keys
								turnOnControlkeys(false);

								ShowBothWindows(SW_HIDE);
							} else {
								//Just hide the typing window for now.
								mainWindow->showWindow(false);
								//ShowMainWindow(SW_HIDE);

								if (sentence.size()==0) {
									//Kill the entire sentence.
									sentence.clear();
									turnOnControlkeys(false);

									sentenceWindow->showWindow(false);
									//ShowSubWindow(SW_HIDE);
								}
							}
						}
					}
				}

				keyWasUsed = true;
			}


			//Handle control hotkeys
			if (wParam == HOTKEY_RIGHT) {
				if (helpWindow->isVisible()) {
					//Move the letter cursor one to the right
					//ADD LATER

				} else {
					if (mainWindow->isVisible()) {
						//Move right/left within the current word.
						if (patSintIDModifier==-1) {
							patSintIDModifier = 0;
							recalculate();
						} else if (model.moveRight(1) == TRUE)
							recalculate();
					} else {
						//Move right/left within the current phrase.
						if (sentence.moveCursorRight(1, model))
							recalculate();
					}
				}

				keyWasUsed = true;
			} else if (wParam == HOTKEY_LEFT) {
				if (helpWindow->isVisible()) {
					//Move the letter cursor one to the left
					//ADD LATER

				} else {
					if (mainWindow->isVisible()) {
						if (model.moveRight(-1) == TRUE)
							recalculate();
						else if (model.hasPostStr() && patSintIDModifier==0) {
							//Move left to our "pat-sint shortcut"
							patSintIDModifier = -1;
							recalculate();
						}
					} else {
						//Move right/left within the current phrase.
						if (sentence.moveCursorRight(-1, model))
							recalculate();
					}
				}

				keyWasUsed = true;
			}


			//Determine what number, if any, was pressed
			int numCode = -1;
			if (wParam>=HOTKEY_0 && wParam<=HOTKEY_9)
				numCode = (int)wParam - HOTKEY_0;
			if (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9)
				numCode = (int)wParam - HOTKEY_NUM0;

			//Handle numbers
			if (!helpWindow->isVisible()) {
				stopChar=0;
				if (numCode>-1 || wParam==HOTKEY_COMBINE || (wParam==HOTKEY_SHIFT_COMBINE&&mainWindow->isVisible())) {
					if (mainWindow->isVisible()) {
						//Convert 1..0 to 0..9
						if (--numCode<0)
							numCode = 9;

						//Mangle as usual...
						if (wParam==HOTKEY_COMBINE || wParam==HOTKEY_SHIFT_COMBINE) {
							numCode = -1;
							patSintIDModifier = -1;
						} else
							patSintIDModifier = 0;

						//The model is visible: select that word
						BOOL typed = selectWord(numCode, helpWindow->isVisible());
						if (typed==TRUE && typePhrases) {
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							currStr.clear();
							patSintIDModifier = 0;
							model.reset(false);
							recalculate();
						} else
							patSintIDModifier = 0;

						keyWasUsed = true;
					} else if (typeBurmeseNumbers) {
						if (!typePhrases) {
							sentence.clear();
							sentence.insert(numCode);
							typeCurrentPhrase();
						} else {
							//Just type that number directly.
							sentence.insert(numCode);
							sentence.moveCursorRight(0, true, model);

							//Is our window even visible?
							if (!sentenceWindow->isVisible()) {
								turnOnControlkeys(true);

								sentenceWindow->showWindow(true);
								//ShowSubWindow(SW_SHOW);
							}

							recalculate();
						}

						keyWasUsed = true;
					}
				}
			}


			//Handle Half-stop/Full-stop
			if (wParam==HOTKEY_COMMA || wParam==HOTKEY_PERIOD) {
				stopChar = model.getStopCharacter((wParam==HOTKEY_PERIOD));
				if (helpWindow->isVisible()) {
					//Possibly do nothing...
					//ADD LATER

				} else {
					if (!mainWindow->isVisible()) {
						if (!sentenceWindow->isVisible()) {
							//This should be cleared already, but let's be safe...
							sentence.clear();
						}
						//Otherwise, we perform the normal "enter" routine.
						typeCurrentPhrase();
					}
				}

				keyWasUsed = true;
			}


			//Handle Enter
			if (wParam==HOTKEY_ENTER || wParam==HOTKEY_SHIFT_ENTER) {
				if (helpWindow->isVisible()) {
					//Select our word, add it to the dictionary temporarily.
					// Flag the new entry so it can be cleared later when the sentence is selected
					if (currStrZg.length()>0) {
						if (currStrDictID==-1) {
							wstring tempStr = waitzar::sortMyanmarString(currStr);
							userDefinedWords.push_back(tempStr);
							userDefinedWordsZg.push_back(currStrZg);
							currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());

							//Add it to the memory list
							helpKeyboard->addMemoryEntry(currStrZg.c_str(), "<no entry>");
						} else {
							//Add it to the memory list
							string revWord = model.reverseLookupWord(currStrDictID);
							helpKeyboard->addMemoryEntry(currStrZg.c_str(), revWord.c_str());
						}

						//Hide the help window
						turnOnHelpKeys(false);
						helpWindow->showWindow(false);
						//ShowHelpWindow(SW_HIDE);

						//Try to type this word
						BOOL typed = selectWord(currStrDictID, true);
						if (typed==TRUE && typePhrases) {
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							patSintIDModifier = 0;
							model.reset(false);
							currStr.clear();
							recalculate();
						}

						//We need to reset the trigrams here...
						sentence.updateTrigrams(model);
					}
				} else {
					stopChar = 0;
					if (mainWindow->isVisible()) {
						//The model is visible: select that word
						BOOL typed = selectWord(-1, helpWindow->isVisible());
						if (typed==TRUE && typePhrases) {
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							currStr.clear();
							patSintIDModifier = 0;
							model.reset(false);
							recalculate();
						}
					} else {
						//Type the entire sentence
						typeCurrentPhrase();
					}
				}

				keyWasUsed = true;
			}

			//Handle Space Bar
			if (wParam==HOTKEY_SPACE || wParam==HOTKEY_SHIFT_SPACE) {
				if (helpWindow->isVisible()) {
					//Select our word, add it to the dictionary temporarily.
					// Flag the new entry so it can be cleared later when the sentence is selected
					if (currStrZg.length()>0) {
						if (currStrDictID==-1) {
							wstring tempStr = waitzar::sortMyanmarString(currStr);
							userDefinedWords.push_back(tempStr);
							userDefinedWordsZg.push_back(currStrZg);
							currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());

							//Add it to the memory list
							helpKeyboard->addMemoryEntry(currStrZg.c_str(), "<no entry>");
						} else {
							//Add it to the memory list
							string revWord = model.reverseLookupWord(currStrDictID);
							helpKeyboard->addMemoryEntry(currStrZg.c_str(), revWord.c_str());
						}

						//Hide the help window
						turnOnHelpKeys(false);
						helpWindow->showWindow(false);
						//ShowHelpWindow(SW_HIDE);

						//Try to type this word
						BOOL typed = selectWord(currStrDictID, true);
						if (typed==TRUE && typePhrases) {
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							patSintIDModifier = 0;
							model.reset(false);
							currStr.clear();
							recalculate();
						}

						//We need to reset the trigrams here...
						sentence.updateTrigrams(model);

						keyWasUsed = true;
					}
				} else {
					stopChar = 0;
					if (mainWindow->isVisible()) {
						//The model is visible: select that word
						BOOL typed = selectWord(-1, helpWindow->isVisible());
						if (typed==TRUE && typePhrases) {
							mainWindow->showWindow(false);
							//ShowMainWindow(SW_HIDE);

							patSintIDModifier = 0;
							model.reset(false);
							currStr.clear();
							recalculate();
						}

						keyWasUsed = true;
					} else {
						//A bit tricky here. If the cursor's at the end, we'll
						//  do HOTKEY_ENTER. But if not, we'll just advance the cursor.
						//Hopefully this won't confuse users so much.
						if (wParam==HOTKEY_SPACE) {
							if (sentence.getCursorIndex()==-1 || sentence.getCursorIndex()<((int)sentence.size()-1)) {
								sentence.moveCursorRight(1, model);
								recalculate();
							} else {
								//Type the entire sentence
								typeCurrentPhrase();
							}

							keyWasUsed = true;
						}
					}
				}
			}

			//Handle our individual letter presses as hotkeys
			if (helpWindow->isVisible()) {
				//Handle our help menu
				wstring nextBit = helpKeyboard->typeLetter(wParam);
				if (!nextBit.empty()) {
					//Valid letter
					currStr += nextBit;
					size_t len = currStr.length();

					//Special cases
					if (nextBit.length()==1 && nextBit[0]==L'\u1039') {
						//Combiner functions in reverse
						if (len>1 && canStack(currStr[len-2])) {
							currStr[len-1] = currStr[len-2];
							currStr[len-2] = nextBit[0];
						} else {
							currStr.erase(currStr.length()-1); //Not standard behavior, but let's avoid bad combinations.
						}
					} else if (nextBit == wstring(L"\u1004\u103A\u1039")) {
						//Kinzi can be typed after the consonant instead of before it.
						//For now, we only cover the general case of typing "kinzi" directly after a consonant
						if (len>3 && canStack(currStr[len-4])) {
							currStr[len-1] = currStr[len-4];
							currStr[len-4] = nextBit[0];
							currStr[len-3] = nextBit[1];
							currStr[len-2] = nextBit[2];
						}
					}


					//Pre-sort unicode strings (should be helpful)
					recalculate();

					//Is the main window visible?
					if (!mainWindow->isVisible()) {
						//Show it
						if (!typePhrases || !sentenceWindow->isVisible()) {
							ShowBothWindows(SW_SHOW);
						} else {
							mainWindow->showWindow(true);
							//ShowMainWindow(SW_SHOW);
						}
					}

					keyWasUsed = true;
				}
			} else {
				if (!keyWasUsed) {
					//Reset pat-sint choice
					patSintIDModifier = 0;

					//Handle regular letter-presses
					int keyCode = (int)wParam;
					if (wParam >= HOTKEY_A && wParam <= HOTKEY_Z) //Seems like we should be doing with this Shift modifiers..
						keyCode += 32;
					if (keyCode >= HOTKEY_A_LOW && keyCode <= HOTKEY_Z_LOW)
					{
						//Run this keypress into the model. Accomplish anything?
						if (!model.typeLetter(keyCode))
							break;

						//Is this the first keypress of a romanized word? If so, the window is not visible...
						if (!mainWindow->isVisible())
						{
							//Reset it...
							currStr.clear();
							//recalculate();

							//Optionally turn on numerals
							if (!numberKeysOn)
								turnOnNumberkeys(true);

							//Show it
							if (!typePhrases || !sentenceWindow->isVisible()) {
								//Turn on control keys
								turnOnControlkeys(true);
								ShowBothWindows(SW_SHOW);
							} else {
								mainWindow->showWindow(true);
								//ShowMainWindow(SW_SHOW);
							}
						}

						//Now, handle the keypress as per the usual...
						wstringstream msg;
						msg <<currStr <<(char)keyCode;
						currStr = msg.str();
						recalculate();

						keyWasUsed = true;
					}
				}
			}



			//If this letter/number/etc. wasn't processed, see if we can type any of our
			// system-defined keys
			if (numCode==-1)
				numCode = wParam;
			else
				numCode = HOTKEY_0 + numCode;
			if (!helpWindow->isVisible() && !mainWindow->isVisible() && !keyWasUsed) {
				int newID = -1;
				for (size_t i=0; i<systemWordLookup.size(); i++) {
					if (systemWordLookup[i].first==numCode) {
						newID = i;
						break;
					}
				}

				//Did we get anything?
				if (newID!=-1) {
					newID = -1-newID;

					//Try to type this word
					BOOL typed = selectWord(newID, true);
					if (typed==TRUE && typePhrases) {
						if (!sentenceWindow->isVisible()) {
							turnOnControlkeys(true);

							sentenceWindow->showWindow(true);
							//ShowSubWindow(SW_SHOW);
						}

						recalculate();
					}

					//We need to reset the trigrams here...
					sentence.updateTrigrams(model);
				}
			}

			break;
		}
		case WM_PAINT:
		{
			//Update only if there's an area which needs updating (e.g., a higher-level
			//  window has dragged over this one's client area... it can happen only with popups,
			//  but let's do it just to be safe.
			RECT updateRect;
			if (GetUpdateRect(hwnd, &updateRect, FALSE) != 0)
			{
				//Blitting every tick will slow us down... we should validate the
				//  rectangle after drawing it.
				reBlit(updateRect);

				//Validate the client area
				ValidateRect(hwnd, NULL);
			}

			break;
		}
		case WM_LBUTTONDOWN:
		{
			//Thanks to dr. Carbon for suggesting this method.
			if (SetCapture(hwnd)!=NULL)
				break;

			//Drag the mosue
			isDragging = true;
			GetCursorPos(&dragFrom);
			break;
		}
		case WM_MOUSEMOVE:
		{
			//Allow dragging of the mouse by its client area. Reportedly more accurate than NCHIT_TEST
			if (isDragging) {
				RECT rect;
				POINT dragTo;
				GetWindowRect(hwnd, &rect);
				GetCursorPos(&dragTo);

				//Constantly update its position
				MoveWindow(hwnd, (dragTo.x - dragFrom.x) + rect.left,
					(dragTo.y - dragFrom.y) + rect.top,
					rect.right - rect.left, rect.bottom - rect.top, FALSE);

				dragFrom = dragTo;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (isDragging) {
				isDragging = false;
				ReleaseCapture();
			}
			break;
		}
		case UWM_SYSTRAY: //Custom callback for our system tray icon
		{
			POINT pt;
			HMENU hmenu, hpopup;

			if (lParam==WM_RBUTTONUP || lParam==WM_LBUTTONUP) {
				//Make a popup menu
				GetCursorPos(&pt);
				hmenu = LoadMenu(hInst, MAKEINTRESOURCE(WZ_MENU));
				hpopup = GetSubMenu(hmenu, 0);


				//Note: set our text apropriately:
				TCHAR temp[200];
				UINT flagE = mmOn ? 0 : MF_CHECKED;
				UINT flagM = mmOn ? MF_CHECKED : 0;
				swprintf(temp, _T("English (%s)"), langHotkeyString);
				ModifyMenu(hmenu, IDM_ENGLISH, MF_BYCOMMAND|flagE, IDM_ENGLISH, temp);
				swprintf(temp, _T("Myanmar (%s)"), langHotkeyString);
				ModifyMenu(hmenu, IDM_MYANMAR, MF_BYCOMMAND|flagM, IDM_MYANMAR, temp);

				//Set checks for our sub-menus:
				UINT flagU = model.getOutputEncoding()==ENCODING_UNICODE ? MF_CHECKED : 0;
				UINT flagZ = model.getOutputEncoding()==ENCODING_ZAWGYI ? MF_CHECKED : 0;
				UINT flagW = model.getOutputEncoding()==ENCODING_WININNWA ? MF_CHECKED : 0;
				ModifyMenu(hmenu, ID_ENCODING_UNICODE5, MF_BYCOMMAND|flagU, ID_ENCODING_UNICODE5, POPUP_UNI.c_str());
				ModifyMenu(hmenu, ID_ENCODING_ZAWGYI, MF_BYCOMMAND|flagZ, ID_ENCODING_ZAWGYI, POPUP_ZG.c_str());
				ModifyMenu(hmenu, ID_ENCODING_WININNWA, MF_BYCOMMAND|flagW, ID_ENCODING_WININNWA, POPUP_WIN.c_str());

				//Set a check for the "Look Up Word" function
				//  Also remove the "F1" if not applicable.
				UINT flagL = helpWindow->isVisible() ? MF_CHECKED : 0;
				const wstring & POPUP_LOOKUP = mmOn ? POPUP_LOOKUP_MM : POPUP_LOOKUP_EN;
				ModifyMenu(hmenu, IDM_LOOKUP, MF_BYCOMMAND|flagL, IDM_LOOKUP, POPUP_LOOKUP.c_str());


				//Cause our popup to appear in front of any other window.
				SetForegroundWindow(hwnd);

				//Force a track on this menu.
				int retVal = TrackPopupMenu(hpopup, //Which menu to track
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
					sentence.clear();
					patSintIDModifier = 0;
					model.reset(true);
				} else if (retVal == IDM_MYANMAR) {
					switchToLanguage(true);

					//Reset the model
					sentence.clear();
					patSintIDModifier = 0;
					model.reset(true);
				} else if (retVal == IDM_LOOKUP) {
					//Manage our help window
					if (!mmOn)
						switchToLanguage(true);
					updateHelpWindow();
				} else if (retVal == IDM_EXIT) {
					//Will generate a WM_DESTROY message
					delete mainWindow;
					//DestroyWindow(hwnd);
				} else if (retVal == ID_ENCODING_UNICODE5) {
					setEncoding(ENCODING_UNICODE);
					recalculate();
				} else if (retVal == ID_ENCODING_ZAWGYI) {
					setEncoding(ENCODING_ZAWGYI);
					recalculate();
				} else if (retVal == ID_ENCODING_WININNWA) {
					setEncoding(ENCODING_WININNWA);
					recalculate();
				}

				//Fixes a bug re: MSKB article: Q135788
				PostMessage(hwnd, 0, 0, 0);

				//Reclaim resources
				DestroyMenu(hmenu);
			}

			break;
		}
		case WM_CTLCOLORDLG:
			//Our dialog has no background color.
			// (this might not actually have any effect)
			return NULL_BRUSH;
			break;
		case WM_CLOSE:
			//Will generate a WM_DESTROY message
			delete mainWindow;
			//DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
		{
			//Cleanup
			if (!mainWindow->unregisterHotKey(LANG_HOTKEY))
				MessageBox(NULL, _T("Main Hotkey remains..."), _T("Warning"), MB_ICONERROR | MB_OK);
			if (mmOn) {
				if (!turnOnHotkeys(false, true, true))
					MessageBox(NULL, _T("Some hotkeys remain..."), _T("Warning"), MB_ICONERROR | MB_OK);
			}

			//Remove systray icon
			NOTIFYICONDATA nid;
			mainWindow->initShellNotifyIconData(nid);
			//nid.cbSize = sizeof(NOTIFYICONDATA);
			//nid.hWnd = hwnd;
			nid.uID = STATUS_NID;
			nid.uFlags = NIF_TIP; //??? Needed ???
			Shell_NotifyIcon(NIM_DELETE, &nid);

			//Close our thread, delete our critical section
			if (highlightKeys) {
				DeleteCriticalSection(&threadCriticalSec);
				CloseHandle(keyTrackThread);
				//CloseHandle(caretTrackThread);  //This should already be closed; closing
				//                                  it twice is an error. 
			}

			//Log?
			if (isLogging) {
				fprintf(logFile, "WaitZar closed\n");
				fclose(logFile);
			}

			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}


bool turnOnHotkeys(bool on, bool affectLowercase, bool affectUppercase)
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



bool turnOnPunctuationkeys(bool on)
{
	bool retVal = true;

	if (on==TRUE) {
		//Punctuation keys
		if (!mainWindow->registerHotKey(HOTKEY_COMMA, NULL, VK_OEM_COMMA))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_PERIOD, NULL, VK_OEM_PERIOD))
			retVal = false;
	} else {
		//Additional punctuation keys
		if (!mainWindow->unregisterHotKey(HOTKEY_COMMA))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_PERIOD))
			retVal = false;
	}

	//Return
	punctuationKeysOn = on;
	return retVal;
}


bool turnOnNumberkeys(bool on)
{
	bool retVal = true;

	//Register numbers
	if (on) {
		//Special case: combiner key
		if (!mainWindow->registerHotKey(HOTKEY_COMBINE, 0, VK_OEM_3))
			retVal = false;

		//Numbers are no longer control keys.
		if (!mainWindow->registerHotKey(HOTKEY_NUM0, NULL, VK_NUMPAD0))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM1, NULL, VK_NUMPAD1))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM2, NULL, VK_NUMPAD2))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM3, NULL, VK_NUMPAD3))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM4, NULL, VK_NUMPAD4))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM5, NULL, VK_NUMPAD5))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM6, NULL, VK_NUMPAD6))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM7, NULL, VK_NUMPAD7))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM8, NULL, VK_NUMPAD8))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_NUM9, NULL, VK_NUMPAD9))
			retVal = false;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (!mainWindow->registerHotKey(i, NULL, i))
				retVal = false;
		}
	} else {
		//Combiner
		if (!mainWindow->unregisterHotKey(HOTKEY_COMBINE))
			retVal = false;

		//Numbers
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM0))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM1))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM2))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM3))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM4))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM5))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM6))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM7))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM8))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_NUM9))
			retVal = false;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (!mainWindow->unregisterHotKey(i))
				retVal = false;
		}
	}

	numberKeysOn = on;
	return retVal;
}


bool turnOnHelpKeys(bool on)
{
	bool retVal = true;

	if (on) {
		//We'll keep our shifted hotkeys, but also add a hotkey for shift itself.
		//  We need to disambiguate the left and right shift keys later, since
		//  registering VK_LSHIFT and VK_RSHIFT doesn't seem to work
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT, MOD_SHIFT, VK_SHIFT))
			retVal = false;
	} else {
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT))
			retVal = false;
	}

	helpKeysOn = on;
	return retVal;
}


bool turnOnExtendedKeys(bool on)
{
	bool retVal = true;

	//Register help keys
	if (on) {
		//Our combiner key (register shifted, too, to prevent errors)
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_COMBINE, MOD_SHIFT, VK_OEM_3))
			retVal = false;

		//Various additional keyboard keys
		if (!mainWindow->registerHotKey(HOTKEY_LEFT_BRACKET, 0, VK_OEM_4))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_LEFT_BRACKET, MOD_SHIFT, VK_OEM_4))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_RIGHT_BRACKET, 0, VK_OEM_6))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_RIGHT_BRACKET, MOD_SHIFT, VK_OEM_6))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_BACKSLASH, 0, VK_OEM_5))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_BACKSLASH, MOD_SHIFT, VK_OEM_5))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SEMICOLON, 0, VK_OEM_1))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_SEMICOLON, MOD_SHIFT, VK_OEM_1))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_APOSTROPHE, 0, VK_OEM_7))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_APOSTROPHE, MOD_SHIFT, VK_OEM_7))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_FORWARDSLASH, 0, VK_OEM_2))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_FORWARDSLASH, MOD_SHIFT, VK_OEM_2))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_COMMA, MOD_SHIFT, VK_OEM_COMMA))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_PERIOD, MOD_SHIFT, VK_OEM_PERIOD))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_SPACE, MOD_SHIFT, HOTKEY_SPACE))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_ENTER, MOD_SHIFT, VK_RETURN))
			retVal = false;

		//Even though we won't use them, we should track them in our virtual keyboard
		if (!mainWindow->registerHotKey(HOTKEY_MINUS, 0, VK_OEM_MINUS))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_MINUS, MOD_SHIFT, VK_OEM_MINUS))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_EQUALS, 0, VK_OEM_PLUS))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_SHIFT_EQUALS, MOD_SHIFT, VK_OEM_PLUS))
			retVal = false;


		//Number keys shifted
		for (int i=HOTKEY_SHIFT_0; i<=HOTKEY_SHIFT_9; i++) {
			if (!mainWindow->registerHotKey(i, MOD_SHIFT, (i-HOTKEY_SHIFT_0)+HOTKEY_0))
				retVal = false;
		}
	} else {
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_COMBINE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_LEFT_BRACKET))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_RIGHT_BRACKET))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_BACKSLASH))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_LEFT_BRACKET))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_RIGHT_BRACKET))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_BACKSLASH))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SEMICOLON))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_SEMICOLON))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_APOSTROPHE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_APOSTROPHE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_FORWARDSLASH))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_FORWARDSLASH))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_COMMA))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_PERIOD))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_MINUS))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_MINUS))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_EQUALS))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_EQUALS))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_SPACE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_SHIFT_ENTER))
			retVal = false;
		for (int i=HOTKEY_SHIFT_0; i<=HOTKEY_SHIFT_9; i++) {
			if (!mainWindow->unregisterHotKey(i))
				retVal = false;
		}
	}

	extendedKeysOn = on;
	return retVal;
}



bool turnOnControlkeys(bool on)
{
	bool retVal = true;

	//Register control keys
	if (on) {
		if (!mainWindow->registerHotKey(HOTKEY_SPACE, NULL, HOTKEY_SPACE))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_ENTER, NULL, VK_RETURN))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_LEFT, NULL, VK_LEFT))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_ESC, NULL, VK_ESCAPE))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_BACK, NULL, VK_BACK))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_DELETE, NULL, VK_DELETE))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_RIGHT, NULL, VK_RIGHT))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_UP, NULL, VK_UP))
			retVal = false;
		if (!mainWindow->registerHotKey(HOTKEY_DOWN, NULL, VK_DOWN))
			retVal = false;
	} else {
		if (!mainWindow->unregisterHotKey(HOTKEY_SPACE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_ENTER))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_LEFT))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_ESC))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_BACK))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_DELETE))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_RIGHT))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_UP))
			retVal = false;
		if (!mainWindow->unregisterHotKey(HOTKEY_DOWN))
			retVal = false;
	}

	controlKeysOn = on;
	return retVal;
}



void makeMainWindow(LPCWSTR windowClassName)
{
	//Set a window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = g_DarkGrayBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//Create our main window, set default sizes
	int def_width  = 240;
	int def_height = 120;
	mainWindow = new MyWin32Window();
	mainWindow->init(windowClassName, L"WaitZar", hInst, 100, 100, def_width, def_height, positionAtCaret, false);
	mainWindow->setDefaultSize(def_width, def_height);
	/*mainWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);*/
}


void makeSubWindow(LPCWSTR windowClassName)
{
	if (!typePhrases)
		return;

	//Set a window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = SubWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = g_DarkGrayBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Sub-Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//Create a handle to the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	int def_width  = 300;
	int def_height =  26;
	sentenceWindow = new MyWin32Window();
	sentenceWindow->init(windowClassName, L"WaitZar", hInst, 100, 100+mainWindow->getDefaultWidth(), def_width, def_height, positionAtCaret, false);
	sentenceWindow->setDefaultSize(def_width, def_height);
	/*senWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		100, 100+WINDOW_HEIGHT, SUB_WINDOW_WIDTH, SUB_WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);*/
}


void makeHelpWindow(LPCWSTR windowClassName)
{
	//Set a window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = HelpWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = g_GreenBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Help-Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//Create a handle to the window
	// We use LAYERED to allow for alpha blending on a per-pixel basis.
	//The MSDN docs say this might slow the program down, but I'll reserve
	// any optimizations until we have actual reported slowdown.
	int def_width  = 200;
	int def_height = 200;
	helpWindow = new MyWin32Window();
	helpWindow->init(windowClassName, L"WaitZar", hInst, 400, 300, def_width, def_height, NULL, true);
	helpWindow->setDefaultSize(def_width, def_height);
	/*helpWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		400, 300, HELP_WINDOW_WIDTH, HELP_WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);*/
}



void makeMemoryWindow(LPCWSTR windowClassName)
{
	//Set a window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = MemoryWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = g_GreenBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Memory-Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//Create a handle to the window
	// We use LAYERED to allow for alpha blending on a per-pixel basis.
	//The MSDN docs say this might slow the program down, but I'll reserve
	// any optimizations until we have actual reported slowdown.
	int def_width  = 200;
	int def_height = 200;
	memoryWindow = new MyWin32Window();
	memoryWindow->init(windowClassName, L"WaitZar", hInst, 400, 300, def_width, def_height, NULL, true);
	memoryWindow->setDefaultSize(def_width, def_height);
	/*memoryWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		400, 300, MEMORY_WINDOW_WIDTH, MEMORY_WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);*/
}



/**
 * Borrowed from KeyMagic.
 *
 * Note to self: I've tried many times to re-write this function and make it less messy.
 *  The problem is, the code is pretty good (checks all error conditions, handles
 *  the various intricate options of Windows access control settings, etc.), and I
 *  don't think I could clean it up without breaking some of this. Even though the
 *  coding style is vastly different from my own, I think I'll have to just accept
 *  the clutter for the time being --it serves its purpose, and I have more important
 *  code to fix.
 */
BOOL IsAdmin()
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;


   /*
      Determine if the current thread is running as a user that is a member of
      the local admins group.  To do this, create a security descriptor that
      has a DACL which has an ACE that allows only local aministrators access.
      Then, call AccessCheck with the current thread's token and the security
      descriptor.  It will say whether the user could access an object if it
      had that security descriptor.  Note: you do not need to actually create
      the object.  Just checking access against the security descriptor alone
      will be sufficient.
   */
   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   __try
   {

      /*
         AccessCheck() requires an impersonation token.  We first get a primary
         token and then create a duplicate impersonation token.  The
         impersonation token is not actually assigned to the thread, but is
         used in the call to AccessCheck.  Thus, this function itself never
         impersonates, but does use the identity of the thread.  If the thread
         was impersonating already, this function uses that impersonation context.
      */
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY,
		  TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

         if (!OpenProcessToken(GetCurrentProcess(),
			 TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation,
		  &hImpersonationToken))
		  __leave;


      /*
        Create the binary representation of the well-known SID that
        represents the local administrators group.  Then create the security
        descriptor and DACL with an ACE that allows only local admins access.
        After that, perform the access check.  This will determine whether
        the current user is a local admin.
      */
      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin,
		  SECURITY_DESCRIPTOR_REVISION))
         __leave;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask,
		  psidAdmin))
         __leave;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

      /*
         AccessCheck validates a security descriptor somewhat; set the group
         and owner so that enough of the security descriptor is filled out to
         make AccessCheck happy.
      */
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         __leave;

      dwAccessDesired = ACCESS_READ;

      /*
         Initialize GenericMapping structure even though you
         do not use generic rights.
      */
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                       &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                       &fReturn))
      {
         fReturn = FALSE;
         __leave;
      }

   }

   __finally
   {
      // Clean up.
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);
      if (psidAdmin) FreeSid(psidAdmin);
      if (hImpersonationToken) CloseHandle (hImpersonationToken);
      if (hToken) CloseHandle (hToken);
   }

   return fReturn;

}


bool IsVistaOrMore()
{
	OSVERSIONINFO OSversion;
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);
	return (OSversion.dwMajorVersion>=6);
}


/**
 * Elevate and run a new instance of WaitZar
 */
void elevateWaitZar(LPCWSTR wzFileName)
{
	//Define our task
	SHELLEXECUTEINFO wzInfo;
    wzInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	wzInfo.fMask = 0;
	wzInfo.hwnd = NULL;
	wzInfo.lpVerb = _T("runas");
	wzInfo.lpFile = wzFileName;
	wzInfo.lpParameters = _T("runasadmin"); //Is this necessary?
    wzInfo.lpDirectory = NULL;
    wzInfo.nShow = SW_NORMAL;

	//Start the task
	if (ShellExecuteEx(&wzInfo) == FALSE) {
		MessageBox(NULL, _T("Could not elevate WaitZar. Program will now exit."), _T("Error!"), MB_ICONERROR | MB_OK);
	}
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
	mainInitDone = false;
	helpInitDone = false;

	//First and foremost
	helpIsCached = false;
	isDragging = false;

	//Also...
	buildSystemWordLookup();

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
	g_WhiteBkgrd = CreateSolidBrush(RGB(255, 255, 255));
	g_DarkGrayBkgrd = CreateSolidBrush(RGB(128, 128, 128));
	g_YellowBkgrd = CreateSolidBrush(RGB(255, 255, 0));
	g_GreenBkgrd = CreateSolidBrush(RGB(0, 128, 0));
	g_DlgHelpBkgrd = CreateSolidBrush(RGB(0xEE, 0xFF, 0xEE));
	g_DlgHelpSlash = CreateSolidBrush(RGB(0xBB, 0xFF, 0xCC));
	g_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
	g_BlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	g_EmptyPen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));



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
				langCfgDir += fs + config.escape_wstr(*fold, true);
				langCfgFile = langCfgDir + fs + cfgFile;
			} catch (std::exception ex) {
				errorMsg << "Error loading config file for language: " <<config.escape_wstr(*fold, false);
				errorMsg << std::endl << "Details: " << std::endl << ex.what();
				inError = true;
			}

			//Now, get the sub-config files
			if (!inError) {
				vector<wstring> modFolders = GetConfigSubDirs(langCfgDir, cfgFile);
				for (vector<wstring>::iterator mod = modFolders.begin(); mod!=modFolders.end(); mod++) {
					try {
						string modCfgFile = langCfgDir;
						modCfgFile += fs + config.escape_wstr(*mod, true);
						modCfgFile += fs + cfgFile;
						langModuleCfgFiles.push_back(modCfgFile);
					} catch (std::exception ex) {
						errorMsg << "Error loading config file for language: " <<config.escape_wstr(*fold, false);
						errorMsg << std::endl << "and module: " <<config.escape_wstr(*mod, false);
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
			string localConfigDir = config.escape_wstr(localAppPath, true) + fs + "WaitZar";
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
			userConfigFile = config.escape_wstr(localAppPath, true) + fs + "waitzar.config.json.txt";

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
		config.testAllFiles();
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
			UnlockResource(res_handle);

			//One more test.
			config.testAllFiles();
		} catch (std::exception ex2) {
			std::wstringstream msg2;
			msg2 << "Error loading default config file.\nWaitZar will not be able to function, and is shutting down.\n\nDetails:\n";
			msg2 << ex2.what();
			MessageBox(NULL, msg2.str().c_str(), L"Default Config Error", MB_ICONERROR | MB_OK);
			return 0;
		}
	}


	//TEST: did our settings load?
	Settings s = config.getSettings();
	wstringstream msg;
	msg << "Settings" <<std::endl;
	msg << "Always elevate: " <<s.alwaysElevate.get() <<std::endl;
	msg << "Balloon start: " <<s.balloonStart.get() <<std::endl;
	msg << "Lock windows: " <<s.lockWindows.get() <<std::endl;
	msg << "Silence mywords warnings: " <<s.silenceMywordsErrors.get() <<std::endl;
	msg << "Track caret: " <<s.trackCaret.get() <<std::endl;
	msg << "Hotkey: " <<s.hotkey.get() <<std::endl;
	msg << "---------------------" <<std::endl;
	msg << "Languages" <<std::endl;
	std::vector<std::wstring> langs = config.getLanguages();
	for (std::vector<std::wstring>::iterator s=langs.begin(); s!=langs.end(); s++)
		msg <<"   " << *s <<std::endl;
	MessageBox(NULL, msg.str().c_str(), L"Settings", MB_ICONINFORMATION | MB_OK);




	//Load our configuration file now; save some headaches later
	loadConfigOptions();

	//Modify our config options?
	if (currTest == mywords) {
		dontLoadModel = true;
		mywordsFileName = "D:\\Open Source Projects\\Waitzar\\eclipse_project\\MyanmarList_v2.txt";
	}


	//Should we run a UAC test on startup?
	if (alwaysRunElevated) {
		//Will elevating help?
		if (IsVistaOrMore() && !IsAdmin()) {
			TCHAR szCurFileName[1024];
            GetModuleFileName(GetModuleHandle(NULL), szCurFileName, 1023);
			elevateWaitZar(szCurFileName);
			return 0;
		}
	}

	//Give this process a low background priority
	//  NOTE: We need to balance this eventually.
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	//Create our windows.
	makeMainWindow(_T("waitZarMainWindow"));
	makeSubWindow(_T("waitZarSentenceWindow"));
	makeHelpWindow(_T("waitZarHelpWindow"));
	makeMemoryWindow(_T("waitZarMemoryWindow"));

	//Set default sizes
	/*mainWindow->setDefaultSize(240, 120);
	sentenceWindow->setDefaultSize(300, 26);
	helpWindow->setDefaultSize(200, 200);
	memoryWindow->setDefaultSize(200, 200);*/

	//Our vector is used to store typed words for later...
	//sentence = new SentenceList();

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
	//nid.cbSize = sizeof(NOTIFYICONDATA); //natch
	//nid.hWnd = mainWindow; //Cauess OUR window to receive notifications for this icon.
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
	//inputItems = new INPUT[1000];
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
	if(mainWindow->isInvalid() || (typePhrases && sentenceWindow->isInvalid()) || helpWindow->isInvalid() || memoryWindow->isInvalid()) {
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//If we got this far, let's try to load our file.
	if (!loadModel()) {
		delete mainWindow;
		//DestroyWindow(mainWindow);
		if (typePhrases)
			delete sentenceWindow;
			//DestroyWindow(senWindow);
		delete helpWindow;
		delete memoryWindow;
		//DestroyWindow(helpWindow);
		//DestroyWindow(memoryWindow);
		return 1;
	}

	//We might have a cached encoding level set...
	model.setOutputEncoding(mostRecentEncoding);

	//Testing mywords?
	if (currTest == mywords)
		GetSystemTimeAsFileTime(&startTime);

	//Also load user-specific words
	readUserWords();

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
	if (currTest != model_print) {
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
	}


	//Potential debug loop (useful)
	if (currTest == model_print) {
		if (isLogging) {
			fclose(logFile);
			isLogging = false;
		}
		logFile = fopen("wz_log.txt", "w");

		model.debugOut(logFile);

		MessageBox(NULL, L"Model saved to output.", _T("Notice"), MB_ICONERROR | MB_OK);
		return 1;
	}



	//Show it's ready by changing the shell icon
	//nid.cbSize = sizeof(NOTIFYICONDATA);
	//nid.hWnd = mainWindow;
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
		if (!testAllWordsByHand())
			MessageBox(NULL, L"Error running type_all check!", L"WaitZar Testing Mode", MB_ICONERROR | MB_OK);
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
