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

//Define to require a specific version of Windows.
// Maybe this is what Vista isn't liking?
#define _WIN32_WINNT 0x0500 //Run on Windows 2000+
//#define _WIN32_WINNT 0x0410 //Run on Windows 98+, fails for KEYBOARD_INPUT

//System includes
#include <windows.h>
#include <windowsx.h> //For GET_X_LPARAM
#include <psapi.h> //For getting a list of currently running processes
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <list>

//Our includes
#include "WordBuilder.h"
#include "PulpCoreFont.h"
#include "resource.h"
#include "Hotkeys.h"

//Current version
#define WAIT_ZAR_VERSION _T("1.5+ Beta")

//Menu item texts
TCHAR* POPUP_UNI = _T("Unicode 5.1");
TCHAR* POPUP_ZG = _T("Zawgyi-One");
TCHAR* POPUP_WIN = _T("Win Innwa");

//Prototypes
BOOL turnOnHotkeys(BOOL on);
BOOL turnOnControlkeys(BOOL on);
BOOL turnOnNumberkeys(BOOL on);
BOOL turnOnPunctuationkeys(BOOL on);
void switchToLanguage(BOOL toMM);
BOOL loadModel(HINSTANCE hInst);

//Unique IDs
#define LANG_HOTKEY 142
#define STATUS_NID 144

//Custom message IDs
#define UWM_SYSTRAY (WM_USER + 1)

//Grr... notepad...
#define UNICOD_BOM 0xFEFF
#define BACKWARDS_BOM 0xFFFE

//Font conversion
int cachedEncoding;
TCHAR currEncStr[10];

//Brushes & Pens
HBRUSH g_WhiteBkgrd;
HBRUSH g_DarkGrayBkgrd;
HBRUSH g_YellowBkgrd;
HBRUSH g_GreenBkgrd;
HPEN g_GreenPen;
HPEN g_BlackPen;
HPEN g_EmptyPen;

//Global Variables
HINSTANCE hInst;
INPUT inputItem;
KEYBDINPUT keyInput;
HICON mmIcon;
HICON engIcon;
WordBuilder *model;
PulpCoreFont *mmFontBlack;
PulpCoreFont *mmFontGreen;
PulpCoreFont *mmFontSmallBlack;
PAINTSTRUCT Ps;
WORD stopChar;
int numConfigOptions;
int numCustomWords;

//Configuration variables.
BOOL customDictWarning = FALSE;
TCHAR langHotkeyString[100];
char langHotkeyRaw[100];
BOOL typePhrases = TRUE;
BOOL dragBothWindowsTogether = TRUE;
BOOL typeBurmeseNumbers = TRUE;

//Double-buffering stuff - mainWindow
HWND mainWindow;
HDC mainDC;
HDC mainUnderDC;
HBITMAP mainBitmap;
BOOL mainWindowSkipMove = FALSE;

//Double-buffering stuff - secondaryWindow
HWND senWindow;
HDC senDC;
HDC senUnderDC;
HBITMAP senBitmap;
BOOL senWindowSkipMove = FALSE;

//Record-keeping
TCHAR currStr[50];
TCHAR currPhrase[500];
BOOL mmOn;
BOOL controlKeysOn = FALSE;
BOOL numberKeysOn = FALSE;
BOOL punctuationKeysOn = FALSE;
std::list<int> *prevTypedWords;
int cursorAfterIndex;
int prevProcessID;

//Default client sizes for our windows
int WINDOW_WIDTH = 240;
int WINDOW_HEIGHT = 120;
int SUB_WINDOW_WIDTH = 300;
int SUB_WINDOW_HEIGHT = 26;

//Width/height of client area
int C_WIDTH;
int C_HEIGHT;
int SUB_C_WIDTH;
int SUB_C_HEIGHT;

//Calculate's integers
int firstLineStart;
int secondLineStart;
int thirdLineStart;
int fourthLineStart;
int borderWidth = 2;
int spaceWidth;

//Avoid crashing if explorer is running slowly
bool mainWindowIsVisible;
bool subWindowIsVisible;



/**
 * Create our inner-used Zawgyi-One fonts.
 */
void makeFont(HWND currHwnd)
{
	//Load our font resource
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
	mmFontBlack = new PulpCoreFont(fontRes, res_handle, mainDC);
	if (mmFontBlack->isInError()==TRUE) {
		TCHAR errorStr[600];
		swprintf(errorStr, _T("WZ Font didn't load correctly: %s"), mmFontBlack->getErrorMsg());

		MessageBox(NULL, errorStr, _T("Error"), MB_ICONERROR | MB_OK);
		return;
	}

	//Unlock this resource for later use.
	UnlockResource(res_handle);

	//Copy-construct a new font
	mmFontGreen = new PulpCoreFont(mmFontBlack, mainDC);

	//Tint both to their respective colors
	mmFontGreen->tintSelf(0x008000);
	mmFontBlack->tintSelf(0x000000);

	//Save resources if we don't use the second window
	if (typePhrases==FALSE)
		return;

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

	mmFontSmallBlack = new PulpCoreFont(fontRes2, res_handle_2, senDC);
	if (mmFontSmallBlack->isInError()==TRUE) {
		TCHAR errorStr[600];
		swprintf(errorStr, _T("WZ Small Font didn't load correctly: %s"), mmFontSmallBlack->getErrorMsg());

		MessageBox(NULL, errorStr, _T("Error"), MB_ICONERROR | MB_OK);
		return;
	}

	//Unlock this resource for later use.
	UnlockResource(res_handle_2);

	//Tint
	mmFontBlack->tintSelf(0x000000);
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
	numCustomWords = -1;
	FILE* userFile = fopen("mywords.txt", "r");
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
			readLine(uniBuffer, currPosition, numUniChars, true, true, false, true, false, false, name, value);

			//Make sure both name and value are non-empty
			if (strlen(value)==0 || lstrlen(name)==0)
				continue;

			//Add this romanization
			if (!model->addRomanization(name, value)) {
				MessageBox(NULL, model->getLastError(), _T("Error adding Romanisation"), MB_ICONERROR | MB_OK);
			}
			numCustomWords++;
		}
		delete [] uniBuffer;
		delete [] name;
		delete [] value;


		if (numCustomWords>0 && customDictWarning==TRUE)
			MessageBox(NULL, _T("Warning! You are using a custom dictionary: \"mywords.txt\".\nThis feature of Wait Zar is EXPERIMENTAL; WaitZar.exe may crash.\n(You may disable this warning by setting mywordswarning = no in config.txt).\n\nPlease report any crashes at the issues page: \nhttp://code.google.com/p/waitzar/issues/list\n\nPress \"Ok\" to continue using Wait Zar."), _T("Warning..."), MB_ICONWARNING | MB_OK);

	}
}


void setEncoding(UINT encoding) 
{
	if (encoding==ENCODING_WININNWA)
		lstrcpy(currEncStr, _T("WI"));
	else if (encoding==ENCODING_ZAWGYI)
		lstrcpy(currEncStr, _T("ZG"));
	else if (encoding==ENCODING_UNICODE)
		lstrcpy(currEncStr, _T("UNI"));

	if (model==NULL)
		cachedEncoding = encoding;
	else {
		model->setOutputEncoding(encoding);
		cachedEncoding = -1;
	}
}


void loadConfigOptions()
{
	//Default keys
	lstrcpy(langHotkeyString, _T("Ctrl+Shift"));
	strcpy(langHotkeyRaw, "^+");

	//Default encoding
	setEncoding(ENCODING_UNICODE);

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
		readLine(buffer, i, buff_size, true, false, false, true, false, true, name, value);

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
				dragBothWindowsTogether = TRUE;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				dragBothWindowsTogether = FALSE;
			else
				numConfigOptions--;
		} else if (strcmp(name, "powertyping")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typePhrases = TRUE;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typePhrases = FALSE;
			else
				numConfigOptions--;
		} else if (strcmp(name, "burmesenumerals")==0) {
			numConfigOptions++;
			if (strcmp(value, "yes")==0 || strcmp(value, "true")==0)
				typeBurmeseNumbers = TRUE;
			else if (strcmp(value, "no")==0 || strcmp(value, "false")==0)
				typeBurmeseNumbers = FALSE;
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
		}

	}

	//Get rid of our buffer
	free(buffer);
	delete [] name;
	delete [] value;
}



BOOL registerInitialHotkey()
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

	return RegisterHotKey(mainWindow, LANG_HOTKEY, modifier, keycode);
}



/**
 * Load the Wait Zar language model.
 */
BOOL loadModel() {
	//Load our embedded resource, the WaitZar model
	HGLOBAL     res_handle = NULL;
	HRSRC       res;
    char *      res_data;
    DWORD       res_size;

	//Previous versions of Wait Zar used the Google sparse hash library; however, even with
	//  its small footprint, this method required too much memory. So, we'll just allocate
	//  a jagged array.
/*	WORD **dictionary;
	UINT32 **nexus;
	UINT32 **prefix;*/

	//And sizes
/*	int dictMaxID;
	int dictMaxSize;
	int nexusMaxID;
	int nexusMaxSize;
	int prefixMaxID;
	int prefixMaxSize;*/

	//Special...
	int numberCheck = 0;

	//Load the resource as a byte array and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(WZ_MODEL), _T("Model"));
	if (!res) {
		MessageBox(NULL, _T("Couldn't find WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
        return FALSE;
	}
    res_handle = LoadResource(NULL, res);
	if (!res_handle) {
		MessageBox(NULL, _T("Couldn't get a handle on WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
        return FALSE;
	}
    res_data = (char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

/*	//Loop through each line
	DWORD currLineStart = 0;
	char currLetter[] = "1000";
	int count = 0;
	int mode = 0;
	int lastCommentedNumber = 0;
	int currDictionaryID = 0;
	UINT32 newWord[1000];
	int newWordSz;
	while (currLineStart < res_size) {
		//LTrim
		while (res_data[currLineStart] == ' ')
			currLineStart++;
		//Is this an empty line?
		if (res_data[currLineStart] == '\n') {
			currLineStart++;
			continue;
		}
		//Is this a comment
		else if (res_data[currLineStart] == '#') {
			count = 0;
			mode++;

			//Skip to the end of the line
			lastCommentedNumber = 0;
			for (;;) {
				char curr = res_data[currLineStart++];
				if (curr == '\n')
					break;
				else if (curr >= '0' && curr <= '9') {
					lastCommentedNumber *= 10;
					lastCommentedNumber += (curr-'0');
				}
			}
			switch (mode) {
				case 1: //Words
					//Initialize our dictionary
					dictMaxID = lastCommentedNumber;
					dictMaxSize = (dictMaxID*3)/2;
					dictionary = (WORD **)malloc(dictMaxSize * sizeof(WORD *));
					currDictionaryID = 0;
					break;
				case 2: //Nexi
					//Initialize our nexus list
					nexusMaxID = lastCommentedNumber;
					nexusMaxSize = (nexusMaxID*3)/2;
					nexus = (UINT32 **)malloc(nexusMaxSize * sizeof(UINT32 *));
					currDictionaryID = 0;
					break;
				case 3: //Prefixes
					//Initialize our prefixes list
					prefixMaxID = lastCommentedNumber;
					prefixMaxSize = (prefixMaxID*3)/2;
					prefix = (UINT32 **)malloc(prefixMaxSize * sizeof(UINT32 *));
					currDictionaryID = 0;
					break;
			}

			continue;
		}

		switch (mode) {
			case 1: //Words
			{
				//Skip until the first number inside the bracket
				while (res_data[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket.
				//  Each "word" is of the form DD(-DD)*,
				newWordSz = 0;

				for(;;) {
					//Read a "pair"
					currLetter[2] = res_data[currLineStart++];
					currLetter[3] = res_data[currLineStart++];

					//Translate/Add this letter
					newWord[newWordSz++] = (WORD)strtol(currLetter, NULL, 16);

					//Continue?
					char nextChar = res_data[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Double check
						if (numberCheck<10) {
							if (newWordSz!=1 || newWord[0]!=0x1040+numberCheck) {
								TCHAR tempError[400];
								swprintf(tempError, _T("Model MUST begin with numbers 0 through 9 (e.g., 1040 through 1049) for reasons of parsimony.\nFound: [%x] at %i"), newWord[0], newWordSz);
								MessageBox(NULL, tempError, _T("Error"), MB_ICONERROR | MB_OK);
								return FALSE;
							}
							numberCheck++;
						}

						//Finangle & add this word
						dictionary[currDictionaryID] = (WORD *)malloc((newWordSz+1) * sizeof(WORD));
						dictionary[currDictionaryID][0] = (WORD)newWordSz;
						for (int i=0; i<newWordSz; i++) {
							dictionary[currDictionaryID][i+1] = newWord[i];
						}
						currDictionaryID++;

						if (nextChar == ']')
							break;
						else
							newWordSz = 0;
					}
				}
				break;
			}
			case 2: //Mappings (nexi)
			{
				//Skip until the first letter inside the bracket
				while (res_data[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newWordSz=0;
				while (res_data[currLineStart] != '}') {
					//Read a hashed mapping: character
					int nextInt = 0;
					char nextChar = 0;
					while (res_data[currLineStart] != ':')
						nextChar = res_data[currLineStart++];
					currLineStart++;

					//Read a hashed mapping: number
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != '}') {
						nextInt *= 10;
						nextInt += (res_data[currLineStart++] - '0');
					}

					//Add that entry to the hash
					newWord[newWordSz++] = ((nextInt<<8) | (0xFF&nextChar));

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				nexus[currDictionaryID] = (UINT32 *)malloc((newWordSz+1) * sizeof(UINT32));
				nexus[currDictionaryID][0] = (UINT32)newWordSz;
				for (int i=0; i<newWordSz; i++) {
					nexus[currDictionaryID][i+1] = newWord[i];
				}
				currDictionaryID++;

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (res_data[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newWordSz = 0;
				int nextVal;
				while (res_data[currLineStart] != '}') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ':') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}
					currLineStart++;

					//Store: key
					newWord[newWordSz++] = nextVal;

					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}
					//Store: val
					newWord[newWordSz++] = nextVal;

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newWordSz;

				//Skip until the first letter inside the square bracket
				while (res_data[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Add a new vector for these
				while (res_data[currLineStart] != ']') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != ']') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}

					//Add it
					newWord[newWordSz++] = nextVal;

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				prefix[currDictionaryID] = (UINT32 *)malloc((newWordSz+2) * sizeof(UINT32));
				prefix[currDictionaryID][0] = (UINT32)lastCommentedNumber/2;
				prefix[currDictionaryID][1] = (UINT32)(newWordSz - lastCommentedNumber);
				for (int i=0; i<lastCommentedNumber; i++) {
					prefix[currDictionaryID][i+2] = newWord[i];
				}
				for (UINT32 i=0; i<prefix[currDictionaryID][1]; i++) {
					prefix[currDictionaryID][i+lastCommentedNumber+2] = newWord[i+lastCommentedNumber];
				}
				currDictionaryID++;

				break;
			}
			default:
				MessageBox(NULL, _T("Too many comments."), _T("Error"), MB_ICONERROR | MB_OK);
				return FALSE;
		}

		//Assume all processing is done, and read until the end of the line
		while (res_data[currLineStart] != '\n')
			currLineStart++;
		currLineStart++;
	}
*/
	//Save our "model"
	model = new WordBuilder(res_data, res_size);
	//model = new WordBuilder(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);
//	model = new WordBuilder(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);

	//Done - This shouldn't matter, though, since the process only
	//       accesses it once and, fortunately, this is not an external file.
	UnlockResource(res_handle);

	return TRUE;
}


void ShowBothWindows(int cmdShow)
{
	bool show = (cmdShow==SW_SHOW);
	
	ShowWindow(mainWindow, cmdShow);
	mainWindowIsVisible = show;

	if (typePhrases==TRUE) {
		ShowWindow(senWindow, cmdShow);
		subWindowIsVisible = show;
	}
}



void switchToLanguage(BOOL toMM) {
	//Don't do anything if we are switching to the SAME language.
	if (toMM == mmOn)
		return;

	//Ok, switch
	BOOL res;
	if (toMM==TRUE) {
		res = turnOnHotkeys(TRUE) && turnOnPunctuationkeys(TRUE);
		if (typeBurmeseNumbers==TRUE)
			res = res && turnOnNumberkeys(TRUE); //JUST numbers, not control.
	} else {
		res = turnOnHotkeys(FALSE);

		//It's possible we still have some hotkeys left on...
		if (controlKeysOn == TRUE)
			turnOnControlkeys(FALSE);
		if (numberKeysOn == TRUE) 
			turnOnNumberkeys(FALSE);
		if (punctuationKeysOn == TRUE)
			turnOnPunctuationkeys(FALSE);
	}
	if (res==FALSE)
		MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

	//Any windows left?
	if (mmOn==FALSE) {
		ShowBothWindows(SW_HIDE);
	}
}


void reBlit()
{
	//Bit blit our back buffer to the front (should prevent flickering)
	BitBlt(mainDC,0,0,C_WIDTH,C_HEIGHT,mainUnderDC,0,0,SRCCOPY);
	if (typePhrases==TRUE)
		BitBlt(senDC,0,0,SUB_C_WIDTH,SUB_C_HEIGHT,senUnderDC,0,0,SRCCOPY);
}


//Only blit part of the area
void reBlit(RECT blitArea)
{
	//Bit blit our back buffer to the front (should prevent flickering)
	BitBlt(mainDC,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,mainUnderDC,blitArea.left,blitArea.top,SRCCOPY);
	if (typePhrases==TRUE)
		BitBlt(senDC,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,senUnderDC,blitArea.left,blitArea.top,SRCCOPY);
}


void initCalculate()
{
	//Figure out how big each of our areas is, and where they start
	spaceWidth = mmFontBlack->getStringWidth(_T(" "));
	firstLineStart = borderWidth;
	secondLineStart = firstLineStart + mmFontBlack->getHeight() + spaceWidth + borderWidth;
	thirdLineStart = secondLineStart + mmFontBlack->getHeight() + spaceWidth + borderWidth;
	fourthLineStart = thirdLineStart + (mmFontBlack->getHeight()*8)/13 + borderWidth;

	//Now, set the window's height
	WINDOW_HEIGHT = fourthLineStart;
}


void expandHWND(HWND hwnd, HDC &dc, HDC &underDC, HBITMAP &bmp, int newWidth, int newHeight, int &SAVED_CLIENT_WIDTH, int &SAVED_CLIENT_HEIGHT)
{
	//Resize the current window; use SetWindowPos() since it's easier...
	SetWindowPos(hwnd, NULL, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
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
}



/**
 * Re-figure the layout of our drawing area, resize if necessary, and
 * draw onto the back buffer. Finally, blit to the front buffer.
 */
void recalculate()
{
	//First things first: can we fit this in the current background?
	int cumulativeWidth = (borderWidth+1)*2;
	std::vector<UINT32> words =  model->getPossibleWords();
	for (size_t i=0; i<words.size(); i++) {
		cumulativeWidth += mmFontBlack->getStringWidth(model->getWordString(words[i]));
		cumulativeWidth += spaceWidth;
	}

	//If not, resize. Also, keep the size small when possible.
	if (cumulativeWidth>C_WIDTH)
		expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, cumulativeWidth, C_HEIGHT, C_WIDTH, C_HEIGHT);
	else if (cumulativeWidth<WINDOW_WIDTH && C_WIDTH>WINDOW_WIDTH)
		expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, WINDOW_WIDTH, C_HEIGHT, C_WIDTH, C_HEIGHT);
	else if (cumulativeWidth>WINDOW_WIDTH && cumulativeWidth<C_WIDTH)
		expandHWND(mainWindow, mainDC, mainUnderDC, mainBitmap, cumulativeWidth, C_HEIGHT, C_WIDTH, C_HEIGHT);

	//Background
	SelectObject(mainUnderDC, g_BlackPen);
	SelectObject(mainUnderDC, g_DarkGrayBkgrd);
	Rectangle(mainUnderDC, 0, 0, C_WIDTH, C_HEIGHT);

	//Background -second window
	if (typePhrases==TRUE) {
		//Draw the background
		SelectObject(senUnderDC, g_BlackPen);
		SelectObject(senUnderDC, g_DarkGrayBkgrd);
		Rectangle(senUnderDC, 0, 0, SUB_C_WIDTH, SUB_C_HEIGHT);

		//Draw each string
		//TCHAR tempPhrase[500];
		lstrcpy(currPhrase, _T(""));
		std::list<int>::iterator printIT = prevTypedWords->begin();
		int currentPosX = borderWidth + 1;
		int cursorPosX = currentPosX;
		int counterCursorID=0;
		for (;printIT != prevTypedWords->end(); printIT++) {
			//Append this string
			mmFontSmallBlack->drawString(senUnderDC, model->getWordString(*printIT), currentPosX, borderWidth+1);
			currentPosX += (mmFontSmallBlack->getStringWidth(model->getWordString(*printIT))+1);

			//Line? (don't print now; we also want to draw it at cursorAfterIndex==-1)
			if (counterCursorID == cursorAfterIndex)
				cursorPosX = currentPosX;

			//Increment
			counterCursorID++;
		}

		//Draw the cursor
		MoveToEx(senUnderDC, cursorPosX-1, borderWidth+1, NULL);
		LineTo(senUnderDC, cursorPosX-1, SUB_C_HEIGHT-borderWidth-1);
		
		//Draw the current encoding
		int encStrWidth = mmFontSmallBlack->getStringWidth(currEncStr);
		SelectObject(senUnderDC, g_BlackPen);
		SelectObject(senUnderDC, g_GreenBkgrd);
		Rectangle(senUnderDC, SUB_C_WIDTH-encStrWidth-3, 0, SUB_C_WIDTH, SUB_C_HEIGHT);
		mmFontSmallBlack->drawString(senUnderDC, currEncStr, SUB_C_WIDTH-encStrWidth-2, SUB_C_HEIGHT/2-mmFontSmallBlack->getHeight()/2);
	}

	//White overlays
	SelectObject(mainUnderDC, g_EmptyPen);
	SelectObject(mainUnderDC, g_WhiteBkgrd);
	Rectangle(mainUnderDC, borderWidth+1, firstLineStart+1, C_WIDTH-borderWidth-1, secondLineStart-borderWidth);
	Rectangle(mainUnderDC, borderWidth+1, secondLineStart, C_WIDTH-borderWidth-1, thirdLineStart-borderWidth);
	Rectangle(mainUnderDC, borderWidth+1, thirdLineStart, C_WIDTH-borderWidth-1, fourthLineStart-borderWidth-1);

	//Now, draw the strings....
	PulpCoreFont* mmFont = mmFontBlack;
	int xOffset = 0;
	TCHAR digit[5];
	for (size_t i=0; i<words.size(); i++) {
		//If this is the currently-selected word, draw a box under it.
		int thisStrWidth = mmFont->getStringWidth(model->getWordString(words[i]));
		if (i!=model->getCurrSelectedID())
			mmFont = mmFontBlack;
		else {
			mmFont = mmFontGreen;

			SelectObject(mainUnderDC, g_YellowBkgrd);
			SelectObject(mainUnderDC, g_GreenPen);
			Rectangle(mainUnderDC, borderWidth+xOffset+1, secondLineStart, borderWidth+1+xOffset+thisStrWidth+spaceWidth, secondLineStart+mmFont->getHeight()+spaceWidth-1);
		}

		mmFont->drawString(mainUnderDC, model->getWordString(words[i]), borderWidth+1+spaceWidth/2 + xOffset, secondLineStart+spaceWidth/2);

		if (i<10) {
			swprintf(digit, _T("%i"), ((i+1)%10));
			int digitWidth = mmFont->getStringWidth(digit);

			mmFont->drawString(mainUnderDC, digit, borderWidth+1+spaceWidth/2 + xOffset + thisStrWidth/2 -digitWidth/2, thirdLineStart-spaceWidth/2-1);
		}

		xOffset += thisStrWidth + spaceWidth;
	}

	TCHAR extendedWordString[300];
	TCHAR* parenStr = model->getParenString();
	if (parenStr!=NULL && lstrlen(parenStr)>0) {
		swprintf(extendedWordString, _T("%s (%s)"), currStr, parenStr);
	} else {
		lstrcpy(extendedWordString, currStr);
	}

	mmFontBlack->drawString(mainUnderDC, extendedWordString, borderWidth+1+spaceWidth/2, firstLineStart+spaceWidth/2+1);

	//Paint
	reBlit();
}



void typeCurrentPhrase()
{
	//Send key presses to the top-level program.
	HWND fore = GetForegroundWindow();
	SetActiveWindow(fore);

	//Load our font
	/*HFONT zgFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(GetDC(mainWindow), LOGPIXELSY), 72), //Font height in UNITS, gah...
		0, //Width, make best-guess
		0, 0, //Escapement, orientation
		FW_NORMAL, //Font weight
		FALSE, //Not italic
		FALSE, //Not underlined
		FALSE, //Not strikethrough
		ANSI_CHARSET, //???
		OUT_DEFAULT_PRECIS, //Output precision
		CLIP_DEFAULT_PRECIS, //Default clipping, might get us into trouble with Zawgyi
		5, //Necessary, maybe cleartype is needed, too.
		DEFAULT_PITCH | FF_DONTCARE, //Font pitch and family
		_T("Zawgyi-One")
	);
	if (zgFont!=NULL) 
		SendMessage(fore, WM_SETFONT, WPARAM(zgFont), FALSE);*/

	//Use SendInput instead of SendMessage, since SendMessage requires the actual
	//  sub-window (component) to recieve the message, whereas SendInput only
	//  requires the top-level window. We could probably hack in SendMessage now that
	//  we're not becoming the active window, but for now I'd rather have a stable
	//  system than one that works on Windows 98.
	std::list<int>::iterator printIT = prevTypedWords->begin();
	std::vector<WORD> keyStrokes;
	for (;printIT!=prevTypedWords->end() || stopChar!=0;) {
		//We may or may not have a half/full stop at the end.
		if (printIT!=prevTypedWords->end()) {
			keyStrokes = model->getWordKeyStrokes(*printIT);
		} else {
			keyStrokes.clear();
			keyStrokes.push_back(stopChar);
		}

		//Set our data structure's fields.
		inputItem.type=INPUT_KEYBOARD;
		keyInput.wVk=0;
		keyInput.dwFlags=KEYEVENTF_UNICODE;
		keyInput.time=0;
		keyInput.dwExtraInfo=0;

		//Send a whole bunch of these...
		BOOL result = TRUE;
		for (size_t i=0; i<keyStrokes.size(); i++) {
			keyInput.wScan = keyStrokes[i];
			inputItem.ki=keyInput;
			if(!SendInput(1,&inputItem,sizeof(INPUT))) {
				result = FALSE;
				break;
			}
		}
		if (result == FALSE)
			MessageBox(NULL, _T("Couldn't send input"), _T("Error"), MB_OK|MB_ICONERROR);

		//Increment
		if (printIT!=prevTypedWords->end())
			printIT++;
		else
			stopChar = 0;
	}

	//Now, reset...
	model->reset(true);
	prevTypedWords->clear();
	cursorAfterIndex = -1;

	//Technically, this can be called with JUST a stopChar, which implies
	//  that the window isn't visible. So check this.
	if (controlKeysOn) {
		//Turn off control keys
		turnOnControlkeys(FALSE);

		//Hide the window(s)
		ShowBothWindows(SW_HIDE);
	}
}



BOOL selectWord(int id)
{
	//Are there any words to use?
	std::pair<BOOL, UINT32> typedVal = model->typeSpace(id);
	if (typedVal.first == FALSE)
		return FALSE;

	//Optionally turn off numerals
	if (numberKeysOn==TRUE && typeBurmeseNumbers==FALSE)
		turnOnNumberkeys(FALSE);

	if (typePhrases==FALSE) {
		//Simple Case
		prevTypedWords->clear();
		prevTypedWords->push_back(typedVal.second);
		typeCurrentPhrase();
	} else {
		//Advanced Case - Insert
		cursorAfterIndex++;
		if (cursorAfterIndex==prevTypedWords->size())
			prevTypedWords->push_back(typedVal.second);
		else {
			std::list<int>::iterator addIT = prevTypedWords->begin();
			advance(addIT, cursorAfterIndex);
			prevTypedWords->insert(addIT, typedVal.second);
		}
	}

	return TRUE;
}


BOOL moveCursorRight(int amt, BOOL allowSameIndex) 
{
	//Any words?
	if (prevTypedWords->size()==0)
		return FALSE;

	//Any change?
	int newAmt = (int)cursorAfterIndex+amt;
	if (newAmt >= (int)prevTypedWords->size())
		newAmt = (int)prevTypedWords->size()-1;
	else if (newAmt < -1)
		newAmt = -1;
	if (newAmt == cursorAfterIndex && allowSameIndex==FALSE)
		return FALSE;

	//Set the trigram
	if (newAmt>=0) {
		WORD trigram[3];
		int trigram_count;
		std::list<int>::iterator findIT = prevTypedWords->begin();
		advance(findIT, newAmt);
		for (trigram_count=0;trigram_count<3; trigram_count++) {
			if (newAmt-trigram_count<0)
				break;
			trigram[trigram_count] = *findIT;
			if (trigram_count<2 && newAmt-trigram_count-1>=0)
				findIT--;
		}
		model->insertTrigram(trigram, trigram_count);
	}

	//Update our index
	cursorAfterIndex = newAmt;
	return TRUE;
}

BOOL moveCursorRight(int amt) 
{
	return moveCursorRight(amt, FALSE);
}



LRESULT CALLBACK SubWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_CREATE:
		{
			//Resize our window?
			MoveWindow(hwnd, 100, 100+WINDOW_HEIGHT, SUB_WINDOW_WIDTH, SUB_WINDOW_HEIGHT, FALSE);

			//Now, create all our buffering objects
			RECT r;
			GetClientRect(hwnd, &r);
			SUB_C_WIDTH = r.right;
			SUB_C_HEIGHT = r.bottom;

			senDC = GetDC(hwnd);
			senUnderDC = CreateCompatibleDC(senDC);

			senBitmap = CreateCompatibleBitmap(senDC, SUB_WINDOW_WIDTH, SUB_WINDOW_HEIGHT);
			SelectObject(senUnderDC, senBitmap);
			break;
		}
		case WM_NCHITTEST: //Allow dragging of the client area...
		{
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT) {
				return HTCAPTION;
			} else
				return uHitTest;
			break;
		}
		case WM_MOVE: 
		{
			//Move the main window?
			if (senWindowSkipMove==FALSE && (mainWindowIsVisible || subWindowIsVisible) && dragBothWindowsTogether==TRUE) {
				RECT r;
				GetWindowRect(hwnd, &r);
				RECT r2;
				GetWindowRect(GetDesktopWindow(), &r2);
				mainWindowSkipMove = TRUE;
				SetWindowPos(mainWindow, HWND_TOPMOST, min(max(r.left, 0), r2.right-C_WIDTH), max(r.top-C_HEIGHT, 0), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			senWindowSkipMove = FALSE;
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
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
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
			//Make our font?
			makeFont(hwnd);
			initCalculate();

			//Resize our window?
			MoveWindow(hwnd, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, FALSE);

			//Now, create all our buffering objects
			RECT r;
			GetClientRect(hwnd, &r);
			C_WIDTH = r.right;
			C_HEIGHT = r.bottom;

			mainDC = GetDC(hwnd);
			mainUnderDC = CreateCompatibleDC(mainDC);

			mainBitmap = CreateCompatibleBitmap(mainDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			SelectObject(mainUnderDC, mainBitmap);

			break;
		}
		case WM_MOVE: 
		{
			//Move the sentence window?
			if (typePhrases==TRUE && mainWindowSkipMove==FALSE && subWindowIsVisible && dragBothWindowsTogether==TRUE) {
				RECT r;
				GetWindowRect(hwnd, &r);
				RECT r2;
				GetWindowRect(GetDesktopWindow(), &r2);
				senWindowSkipMove = TRUE;
				SetWindowPos(senWindow, HWND_TOPMOST, min(max(r.left, 0), r2.right-SUB_C_WIDTH), min(r.top+C_HEIGHT, r2.bottom-SUB_C_HEIGHT), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			mainWindowSkipMove = FALSE;
			break;
		}
		case WM_HOTKEY:
		{
			//Handle our main language hotkey
			if(wParam == LANG_HOTKEY) {
				//Switch language
				if (mmOn==TRUE)
					switchToLanguage(FALSE);
				else
					switchToLanguage(TRUE);

				//Reset the model
				prevTypedWords->clear();
				cursorAfterIndex = -1;
				model->reset(true);
			}


			//Close the window?
			if (wParam == HOTKEY_ESC) {
				if (!mainWindowIsVisible) {
					//Kill the entire sentence.
					prevTypedWords->clear();
					cursorAfterIndex = -1;
					turnOnControlkeys(FALSE);
					ShowBothWindows(SW_HIDE);
				} else {
					model->reset(false);

					//No more numbers
					if (typeBurmeseNumbers==FALSE)
						turnOnNumberkeys(FALSE);

					//Are we using advanced input?
					if (typePhrases==FALSE) {
						//Turn off control keys
						turnOnControlkeys(FALSE);
						ShowBothWindows(SW_HIDE);
					} else {
						//Just hide the typing window for now.
						ShowWindow(mainWindow, SW_HIDE);
						mainWindowIsVisible = false;

						if (prevTypedWords->size()==0) {
							//Kill the entire sentence.
							prevTypedWords->clear();
							cursorAfterIndex = -1;
							ShowBothWindows(SW_HIDE);
						}
					}
				}
			}


			//Delete: Phrases only
			if (wParam == HOTKEY_DELETE) {
				if (!mainWindowIsVisible) {
					//Delete the next word
					if (cursorAfterIndex>=-1 && cursorAfterIndex<((int)prevTypedWords->size()-1)) {
						std::list<int>::iterator erIT = prevTypedWords->begin();
						advance(erIT, cursorAfterIndex+1);
						prevTypedWords->erase(erIT);
						recalculate();
					}
					if (prevTypedWords->size()==0) {
						//Kill the entire sentence.
						prevTypedWords->clear();
						turnOnControlkeys(FALSE);
						cursorAfterIndex = -1;
						ShowBothWindows(SW_HIDE);
					}
				}
			}


			//Back up
			if (wParam == HOTKEY_BACK) {
				if (!mainWindowIsVisible) {
					//Delete the previous word
					if (cursorAfterIndex>=0 && cursorAfterIndex<(int)prevTypedWords->size()) {
						std::list<int>::iterator erIT = prevTypedWords->begin();
						advance(erIT, cursorAfterIndex);
						prevTypedWords->erase(erIT);
						cursorAfterIndex--;
						recalculate();
					}
					if (prevTypedWords->size()==0) {
						//Kill the entire sentence.
						prevTypedWords->clear();
						cursorAfterIndex = -1;
						turnOnControlkeys(FALSE);
						ShowBothWindows(SW_HIDE);
					}
				} else {
					if (model->backspace()) {
						//Truncate...
						currStr[lstrlen(currStr)-1] = 0;
						recalculate();
					} else {
						//No more numerals.
						if (typeBurmeseNumbers==FALSE)
							turnOnNumberkeys(FALSE);

						//Are we using advanced input?
						if (typePhrases==FALSE) {
							//Turn off control keys
							turnOnControlkeys(FALSE);
							
							ShowBothWindows(SW_HIDE);
						} else {
							//Just hide the typing window for now.
							ShowWindow(mainWindow, SW_HIDE);
							mainWindowIsVisible = false;

							if (prevTypedWords->size()==0) {
								//Kill the entire sentence.
								prevTypedWords->clear();
								cursorAfterIndex = -1;
								turnOnControlkeys(FALSE);

								ShowWindow(senWindow, SW_HIDE);
								subWindowIsVisible = false;
							}
						}
					}
				}
			}


			//Handle control hotkeys
			if (wParam == HOTKEY_RIGHT) {
				if (mainWindowIsVisible) {
					//Move right/left within the current word.
					if (model->moveRight(1) == TRUE)
						recalculate();
				} else {
					//Move right/left within the current phrase.
					if (moveCursorRight(1) == TRUE)
						recalculate();
				}
			} else if (wParam == HOTKEY_LEFT) {
				if (mainWindowIsVisible) {
					if (model->moveRight(-1) == TRUE)
						recalculate();
				} else {
					//Move right/left within the current phrase.
					if (moveCursorRight(-1) == TRUE)
						recalculate();
				}
			}


			//Handle numbers
			int numCode = -1;
			stopChar = 0;
			if (wParam>=HOTKEY_0 && wParam<=HOTKEY_9)
				numCode = (int)wParam - HOTKEY_0;
			if (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9)
				numCode = (int)wParam - HOTKEY_NUM0;
			if (numCode > -1) {
				//Our key code has been properly transformed.
				if (mainWindowIsVisible) {
					//Convert 1..0 to 0..9
					if (--numCode<0)
						numCode = 9;

					//The model is visible: select that word
					BOOL typed = selectWord(numCode);
					if (typed==TRUE && typePhrases==TRUE) {
						ShowWindow(mainWindow, SW_HIDE);
						mainWindowIsVisible = false;

						lstrcpy(currStr, _T(""));
						model->reset(false);
						recalculate();
					}
				} else {
					if (typePhrases==FALSE) {
						prevTypedWords->clear();
						prevTypedWords->push_back(numCode);
						typeCurrentPhrase();
					} else {
						//Just type that number directly. 
						cursorAfterIndex++;
						if (cursorAfterIndex==prevTypedWords->size())
							prevTypedWords->push_back(numCode);
						else {
							std::list<int>::iterator addIT = prevTypedWords->begin();
							advance(addIT, cursorAfterIndex);
							prevTypedWords->insert(addIT, numCode);
						}
						moveCursorRight(0, TRUE);

						//Is our window even visible?
						if (!subWindowIsVisible) {
							turnOnControlkeys(TRUE);
							
							ShowWindow(senWindow, SW_SHOW);
							subWindowIsVisible = true;
						}

						recalculate();
					}
				}
			}


			//Handle Half-stop/Full-stop
			if (wParam==HOTKEY_COMMA || wParam==HOTKEY_PERIOD) {
				stopChar = model->getStopCharacter((wParam==HOTKEY_PERIOD));
				if (!mainWindowIsVisible) {
					if (!subWindowIsVisible) {
						//This should be cleared already, but let's be safe...
						prevTypedWords->clear();	
					}
					//Otherwise, we perform the normal "enter" routine.
					typeCurrentPhrase();
				}
			}


			//Handle Enter
			if (wParam==HOTKEY_ENTER) {
				stopChar = 0;
				if (mainWindowIsVisible) {
					//The model is visible: select that word
					BOOL typed = selectWord(-1);
					if (typed==TRUE && typePhrases==TRUE) {
						ShowWindow(mainWindow, SW_HIDE);
						mainWindowIsVisible = false;

						lstrcpy(currStr, _T(""));
						model->reset(false);
						recalculate();
					}
				} else {
					//Type the entire sentence
					typeCurrentPhrase();
				}
			}

			//Handle Space Bar
			if (wParam==HOTKEY_SPACE) {
				stopChar = 0;
				if (mainWindowIsVisible) {
					//The model is visible: select that word
					BOOL typed = selectWord(-1);
					if (typed==TRUE && typePhrases==TRUE) {
						ShowWindow(mainWindow, SW_HIDE);
						mainWindowIsVisible = false;
						
						model->reset(false);
						lstrcpy(currStr, _T(""));
						recalculate();
					}
				} else {
					//A bit tricky here. If the cursor's at the end, we'll 
					//  do HOTKEY_ENTER. But if not, we'll just advance the cursor.
					//Hopefully this won't confuse users so much.
					if (cursorAfterIndex==-1 || cursorAfterIndex<((int)prevTypedWords->size()-1)) {
						cursorAfterIndex++;
						recalculate();
					} else {
						//Type the entire sentence
						typeCurrentPhrase();
					}
				}
			}

			//Handle our individual keystrokes as hotkeys (less registering that way...)
			int keyCode = (int)wParam;
			if (wParam >= HOTKEY_A && wParam <= HOTKEY_Z)
				keyCode += 32;
			if (wParam >= HOTKEY_A_LOW && wParam <= HOTKEY_Z_LOW)
			{
				//Run this keypress into the model. Accomplish anything?
				if (!model->typeLetter(keyCode))
					break;

				//List all possible words
				recalculate();


				//Is this the first keypress of a romanized word? If so, the window is not visible...
				if (!mainWindowIsVisible)
				{
					//Reset it...
					lstrcpy(currStr, _T(""));
					recalculate();

					//Optionally turn on numerals
					if (numberKeysOn==FALSE)
						turnOnNumberkeys(TRUE);

					//Show it
					if (typePhrases==FALSE || !subWindowIsVisible) {
						//Turn on control keys
						turnOnControlkeys(TRUE);
						ShowBothWindows(SW_SHOW);
					} else {
						ShowWindow(mainWindow, SW_SHOW);
						mainWindowIsVisible = true;
					}
				}

				//Now, handle the keypress as per the usual...
				TCHAR keyStr[50];
				lstrcpy(keyStr, currStr);
				swprintf(currStr, _T("%s%c"), keyStr, keyCode);
				recalculate();
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
		case WM_NCHITTEST: //Allow dragging of the client area...
		{
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT) {
				return HTCAPTION;
			} else
				return uHitTest;
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
				UINT flagU = model->getOutputEncoding()==ENCODING_UNICODE ? MF_CHECKED : 0;
				UINT flagZ = model->getOutputEncoding()==ENCODING_ZAWGYI ? MF_CHECKED : 0;
				UINT flagW = model->getOutputEncoding()==ENCODING_WININNWA ? MF_CHECKED : 0;
				ModifyMenu(hmenu, ID_ENCODING_UNICODE5, MF_BYCOMMAND|flagU, ID_ENCODING_UNICODE5, POPUP_UNI);
				ModifyMenu(hmenu, ID_ENCODING_ZAWGYI, MF_BYCOMMAND|flagZ, ID_ENCODING_ZAWGYI, POPUP_ZG);
				ModifyMenu(hmenu, ID_ENCODING_WININNWA, MF_BYCOMMAND|flagW, ID_ENCODING_WININNWA, POPUP_WIN);


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
					BOOL refreshControl = controlKeysOn;
					if  (refreshControl==TRUE)
						turnOnControlkeys(FALSE);

					//Show our box
					TCHAR temp[550];
					swprintf(temp, _T("WaitZar version %s - for more information, see: http://code.google.com/p/waitzar/\n\n%s - Switch between Myanmar and English\nType Burmese words like they sound, and press \"space\".\n\nWaitZar users should have the relevant fonts installed, if they want to see what they type after it's chosen.\nPlease see the User's Guide for more information."), WAIT_ZAR_VERSION, langHotkeyString);
					MessageBox(hwnd, temp, _T("About"), MB_ICONINFORMATION | MB_OK);

					//Hotkeys again
					if  (refreshControl==TRUE)
						turnOnControlkeys(TRUE);
				} else if (retVal == IDM_ENGLISH) {
					switchToLanguage(FALSE);
					
					//Reset the model
					prevTypedWords->clear();
					cursorAfterIndex = -1;
					model->reset(true);
				} else if (retVal == IDM_MYANMAR) {
					switchToLanguage(TRUE);

					//Reset the model
					prevTypedWords->clear();
					cursorAfterIndex = -1;
					model->reset(true);
				} else if (retVal == IDM_EXIT) {
					DestroyWindow(hwnd);
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
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			//Cleanup
			if (UnregisterHotKey(hwnd, LANG_HOTKEY) == FALSE)
				MessageBox(NULL, _T("Main Hotkey remains..."), _T("Warning"), MB_ICONERROR | MB_OK);
			if (mmOn==TRUE) {
				if (turnOnHotkeys(FALSE) == FALSE)
					MessageBox(NULL, _T("Some hotkeys remain..."), _T("Warning"), MB_ICONERROR | MB_OK);
			}

			//Remove systray icon
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hwnd;
			nid.uID = STATUS_NID;
			nid.uFlags = NIF_TIP; //??? Needed ???
			Shell_NotifyIcon(NIM_DELETE, &nid);

			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}


BOOL turnOnHotkeys(BOOL on)
{
	int low_code;
	int high_code;
	BOOL retVal = TRUE;

	for (low_code=HOTKEY_A_LOW; low_code<=HOTKEY_Z_LOW; low_code++)
	{
		high_code = low_code - 32;
		if (on==TRUE)  {
			//Register this as an uppercase/lowercase letter
			if (RegisterHotKey(mainWindow, high_code, MOD_SHIFT, high_code)==FALSE)
				retVal = FALSE;
			if (RegisterHotKey(mainWindow, low_code, NULL, high_code)==FALSE)
				retVal = FALSE;
		} else {
			//De-register this as an uppercase/lowercase letter
			if (UnregisterHotKey(mainWindow, high_code)==FALSE)
				retVal = FALSE;
			if (UnregisterHotKey(mainWindow, low_code)==FALSE)
				retVal = FALSE;
		}
	}

	//Switch to our target language.
	mmOn = on;

	//Change icon in the tray
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = mainWindow;
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	if (mmOn)
		nid.hIcon = mmIcon;
	else
		nid.hIcon = engIcon;

	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE)
		MessageBox(NULL, _T("Can't switch icon..."), _T("Warning"), MB_ICONERROR | MB_OK);

	return retVal;
}


BOOL turnOnPunctuationkeys(BOOL on)
{
	BOOL retVal = true;

	if (on==TRUE) {
		//Punctuation keys
		if (RegisterHotKey(mainWindow, HOTKEY_COMMA, NULL, VK_OEM_COMMA)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_PERIOD, NULL, VK_OEM_PERIOD)==FALSE)
			retVal = FALSE;
	} else {
		//Additional punctuation keys
		if (UnregisterHotKey(mainWindow, HOTKEY_COMMA)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_PERIOD)==FALSE)
			retVal = FALSE;
	}

	//Return
	punctuationKeysOn = on;
	return retVal;
}


BOOL turnOnNumberkeys(BOOL on)
{
	BOOL retVal = true;

	//Register numbers
	if (on==TRUE) {
		//Numbers are no longer control keys.
		if (RegisterHotKey(mainWindow, HOTKEY_NUM0, NULL, VK_NUMPAD0)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM1, NULL, VK_NUMPAD1)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM2, NULL, VK_NUMPAD2)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM3, NULL, VK_NUMPAD3)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM4, NULL, VK_NUMPAD4)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM5, NULL, VK_NUMPAD5)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM6, NULL, VK_NUMPAD6)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM7, NULL, VK_NUMPAD7)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM8, NULL, VK_NUMPAD8)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_NUM9, NULL, VK_NUMPAD9)==FALSE)
			retVal = FALSE;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (RegisterHotKey(mainWindow, i, NULL, i)==FALSE)
				retVal = FALSE;
		}
	} else {
		//Numbers
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM0)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM1)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM2)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM3)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM4)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM5)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM6)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM7)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM8)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_NUM9)==FALSE)
			retVal = FALSE;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (UnregisterHotKey(mainWindow, i)==FALSE)
				retVal = FALSE;
		}
	}

	numberKeysOn = on;
	return retVal;
}



BOOL turnOnControlkeys(BOOL on)
{
	BOOL retVal = true;

	//Register control keys
	if (on==TRUE) {
		if (RegisterHotKey(mainWindow, HOTKEY_SPACE, NULL, HOTKEY_SPACE)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_ENTER, NULL, VK_RETURN)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_LEFT, NULL, VK_LEFT)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_ESC, NULL, VK_ESCAPE)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_BACK, NULL, VK_BACK)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_DELETE, NULL, VK_DELETE)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_RIGHT, NULL, VK_RIGHT)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_UP, NULL, VK_UP)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(mainWindow, HOTKEY_DOWN, NULL, VK_DOWN)==FALSE)
			retVal = FALSE;
	} else {
		if (UnregisterHotKey(mainWindow, HOTKEY_SPACE)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_ENTER)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_LEFT)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_ESC)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_BACK)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_DELETE)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_RIGHT)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_UP)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(mainWindow, HOTKEY_DOWN)==FALSE)
			retVal = FALSE;
	}

	controlKeysOn = on;
	return retVal;
}



HWND makeMainWindow(LPCWSTR windowClassName)
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
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = g_DarkGrayBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Create a handle to the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	mainWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);

	return mainWindow;
}


HWND makeSubWindow(LPCWSTR windowClassName)
{
	if (typePhrases==FALSE)
		return NULL;

	//Set a window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = SubWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = g_DarkGrayBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Sub-Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Create a handle to the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	senWindow = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		windowClassName,
		_T("WaitZar"),
		WS_POPUP, //No border or title bar
		100, 100+WINDOW_HEIGHT, SUB_WINDOW_WIDTH, SUB_WINDOW_HEIGHT,
		NULL, NULL, hInst, NULL
	);

	return senWindow;
}



/**
 * Main method for Windows applications
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Save for later; if we try retrieving it, we'll just get a bunch of conversion
	//  warnings. Plus, the hInstance should never change.
	hInst = hInstance;
	mainWindowIsVisible = false;
	subWindowIsVisible = false;

	//Load our configuration file now; save some headaches later
	loadConfigOptions();

	//Give this process a low background priority
	//  NOTE: We need to balance this eventually.
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	//Create a white/black brush
	g_WhiteBkgrd = CreateSolidBrush(RGB(255, 255, 255));
	g_DarkGrayBkgrd = CreateSolidBrush(RGB(128, 128, 128));
	g_YellowBkgrd = CreateSolidBrush(RGB(255, 255, 0));
	g_GreenBkgrd = CreateSolidBrush(RGB(0, 128, 0));
	g_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
	g_BlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	g_EmptyPen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));

	//Create our main window.
	HWND mainWindow = makeMainWindow(_T("waitZarMainWindow"));
	HWND senWindow = makeSubWindow(_T("waitZarSentenceWindow"));

	//Our vector is used to store typed words for later...
	prevTypedWords = new std::list<int>();
	cursorAfterIndex = -1;

	//Load some icons...
	mmIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_MM), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	engIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_ENG), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16

	//Make our "notify icon" data structure
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA); //natch
	nid.hWnd = mainWindow; //Cauess OUR window to receive notifications for this icon.
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
	if( registerInitialHotkey()==0 ) {
		//Check if we're running Wait Zar already
		if (waitzarAlreadyStarted()==TRUE) {
			MessageBox(NULL, _T("Wait Zar is already running. \n\nYou should see an \"ENG\" icon in your system tray; click on that to change the language. \n\nPlease see the Wait Zar User's Guide if you have any questions."), _T("Wait Zar already running..."), MB_ICONINFORMATION | MB_OK);
			return 0;
		}
		MessageBox(NULL, _T("The main language shortcut could not be set up.\nWait Zar will not function properly."), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
	}
	mmOn = FALSE;

	//Add our icon to the tray
	Shell_NotifyIcon(NIM_ADD, &nid);


	//Initialize our romanisation string
	lstrcpy(currStr, _T(""));
	lstrcpy(currPhrase, _T(""));

	//Success?
	if(mainWindow==NULL || (typePhrases==TRUE && senWindow==NULL)) {
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//If we got this far, let's try to load our file.
	if (loadModel() == FALSE) {
		DestroyWindow(mainWindow);
		if (typePhrases==TRUE)
			DestroyWindow(senWindow);
		return 1;
	}

	//We might have a cached encoding level set...
	if (cachedEncoding!=-1) {
		model->setOutputEncoding(cachedEncoding);
		cachedEncoding = -1;
	}

	//Also load user-specific words
	readUserWords();

	//Did we get any?
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

	//Show it's ready by changing the shell icon
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = mainWindow;
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	nid.hIcon = engIcon;
	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE)
		MessageBox(NULL, _T("Can't switch icon..."), _T("Warning"), MB_ICONERROR | MB_OK);


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
