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
#include <stdio.h>
#include <tchar.h>
#include <string>

//Our includes
//#include "CStdioFile_UTF8.h"
#include "WordBuilder.h"
#include "PulpCoreFont.h"
#include "resource.h"
#include "Hotkeys.h"

//Prototypes
BOOL turnOnHotkeys(HWND hwnd, BOOL on);
BOOL turnOnControlkeys(HWND hwnd, BOOL on);
void switchToLanguage(HWND hwnd, BOOL toMM);
BOOL loadModel(HINSTANCE hInst);
UINT32 HsiehHash ( std::string *str );

//Unique IDs
#define LANG_HOTKEY 142
#define STATUS_NID 144

//Custom message IDs
#define UWM_SYSTRAY (WM_USER + 1)

//Grr... notepad...
#define UNICOD_BOM 0xFEFF

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
PAINTSTRUCT Ps;

//Double-buffering stuff
HWND hwnd;
HDC gc;
HDC underDC;
HBITMAP bmpDC;

//Record-keeping
TCHAR currStr[50];
BOOL mmOn;
BOOL doneDrag;
BOOL controlKeysOn = FALSE;

//Default client sizes for our windows
int WINDOW_WIDTH = 240;
int WINDOW_HEIGHT = 120;

//Width/height of client area
int C_WIDTH;
int C_HEIGHT;

//Calculate's integers
int firstLineStart;
int secondLineStart;
int thirdLineStart;
int fourthLineStart;
int borderWidth = 2;
int spaceWidth;



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
	mmFontBlack = new PulpCoreFont(fontRes, res_handle, gc);
	if (mmFontBlack->isInError()==TRUE) {
		TCHAR errorStr[600];
		swprintf(errorStr, _T("WZ Font didn't load correctly: %s"), mmFontBlack->getErrorMsg());

		MessageBox(NULL, errorStr, _T("Error"), MB_ICONERROR | MB_OK);
		return;
	}

	//Unlock this resource for later use.
	UnlockResource(res_handle);

	//Copy-construct a new font
	mmFontGreen = new PulpCoreFont(mmFontBlack, gc);

	//Tint both to their respective colors
	mmFontGreen->tintSelf(0x008000);
	mmFontBlack->tintSelf(0x000000);
}



void readUserWords() {
	//Read our words file, if it exists.
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

		//Finally, convert this array to unicode
		TCHAR * uniBuffer;
		size_t numUniChars = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, NULL, 0);
		uniBuffer = (TCHAR*) malloc(sizeof(TCHAR)*numUniChars);
		if (!MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, uniBuffer, (int)numUniChars)) {
			MessageBox(NULL, _T("mywords.txt contains invalid UTF-8 characters"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}
		delete [] buffer;

		//Skip the BOM, if it exists
		int currPosition = 0;
		if (uniBuffer[currPosition] == UNICOD_BOM)
			currPosition++;

		//Read each line
		for (size_t i=currPosition; i<numUniChars; i++) {
			//LTrim
			while (uniBuffer[i] == ' ')
				i++;

			//Comment? If so, skip to the next newline.
			//  Also skip empty lines
			if (uniBuffer[i]=='#' || uniBuffer[i]=='\n') {
				while (uniBuffer[i] != '\n')
					i++;
				continue;
			}

			//Read our MM word
			TCHAR name[50];
			int name_pos = 0;
			lstrcpy(name, _T(""));
			while (uniBuffer[i] != '=') {
				if (uniBuffer[i] == ' ')
					i++;
				else
					name[name_pos++] = uniBuffer[i++];
			}
			name[name_pos] = '\0';
			i++;

			//Read our romanization
			char value[50];
			name_pos = 0;
			strcpy(value, "");
			while (uniBuffer[i] != '\n') {
				char romanChar = (char)uniBuffer[i++];
				if (romanChar>='A' && romanChar<='Z') 
					romanChar += ('a'-'A');
				if (romanChar>='a' && romanChar<='z') 
					value[name_pos++] = romanChar;
			}
			value[name_pos++] = '\0';

			//Valid?
			if (strlen(value)==0 || lstrlen(name)==0)
				continue;

			//Add this romanization
			model->addRomanization(name, value);
		}


		delete [] uniBuffer;
	}
}




BOOL registerInitialHotkey() 
{
	//Default keys
	UINT modifier = MOD_CONTROL | MOD_SHIFT;
	UINT keycode = VK_SHIFT;

	//Read our config file, if it exists. 
	FILE* configFile = fopen("config.txt", "r");
	if (configFile != NULL) {
		//Get file size
		fseek (configFile, 0, SEEK_END);
		long fileSize = ftell(configFile);
		rewind(configFile);

		//Read it all into an array, close the file.
		char * buffer = (char*) malloc(sizeof(char)*fileSize);
		size_t buff_size = fread(buffer, 1, fileSize, configFile);
		fclose(configFile);


		//Read each line
		for (size_t i=0; i<buff_size; i++) {
			//LTrim
			while (buffer[i] == ' ')
				i++;

			//Comment? If so, skip to the next newline.
			//  Also skip empty lines
			if (buffer[i]=='#' || buffer[i]=='\n') {
				while (buffer[i] != '\n')
					i++;
				continue;
			}

			//Read our property's name
			char name[50];
			int name_pos = 0;
			strcpy(name, "");
			while (buffer[i] != '=') {
				if (buffer[i] == ' ')
					i++;
				else
					name[name_pos++] = buffer[i++];
			}
			name[name_pos] = '\0';
			i++;

			//Read our property's value
			char value[50];
			name_pos = 0;
			strcpy(value, "");
			while (buffer[i] != '\n') {
				if (buffer[i] == ' ')
					i++;
				else
					value[name_pos++] = buffer[i++];
			}
			value[name_pos++] = '\0';

			//No possible hotkeys?
			if (name_pos<2) 
				continue;

			//Deal with our name/value pair.
			if (strcmp(name, "hotkey")==0) {
				//It's a hotkey code. First, reset...
				modifier = 0;
				
				//Now, set the keycode
				//Additional rule: all keystroke modifiers must also themselves be modifiers
				keycode = value[name_pos-2];
				switch(keycode) {
					case '!':
						keycode = VK_MENU; //VK_MENU == VK_ALT
						modifier |= MOD_ALT;
						break;
					case '^':
						keycode = VK_CONTROL;
						modifier |= MOD_CONTROL;
						break;
					case '+':
						keycode = VK_SHIFT;
						modifier |= MOD_SHIFT;
						break;
					case '_':
						keycode = VK_SPACE;
						break;
				}
				
				//Now, set the modifiers
				for (int pos=0; pos<name_pos-2; pos++) {
					switch(value[pos]) {
						case '!':
							modifier |= MOD_ALT;
							break;
						case '^':
							modifier |= MOD_CONTROL;
							break;
						case '+':
							modifier |= MOD_SHIFT;
							break;
					}
				}
			
				//Additional rule: Capital letters require a shift modifier
				if (keycode>='A' && keycode<='Z')
					modifier |= MOD_SHIFT;

				//Additional rule: Lowercase letters are coded by their uppercase value
				if (keycode>='a' && keycode<='z')
					keycode -= 'a'-'A';
			}
		}

		//Get rid of our buffer
		free(buffer);
	}


	/*TCHAR temp[150];
	swprintf(temp, _T("mod: %i   key: %c"), modifier, keycode);
	MessageBox(NULL, temp, _T("Hotkey..."), MB_ICONERROR | MB_OK);*/


	return RegisterHotKey(hwnd, LANG_HOTKEY, modifier, keycode);
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
	WORD **dictionary;
	UINT32 **nexus;
	UINT32 **prefix;

	//And sizes
	int dictMaxID;
	int dictMaxSize;
	int nexusMaxID;
	int nexusMaxSize;
	int prefixMaxID;
	int prefixMaxSize;

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

	//Loop through each line
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

	//Save our "model"
	model = new WordBuilder(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);
//	model = new WordBuilder(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);

	//Done - This shouldn't matter, though, since the process only 
	//       accesses it once and, fortunately, this is not an external file.
	UnlockResource(res_handle);

	return TRUE;
}



void switchToLanguage(HWND hwnd, BOOL toMM) {
	//Don't do anything if we are switching to the SAME language.
	if (toMM == mmOn)
		return;

	//Ok, switch
	BOOL res;
	if (toMM==TRUE)
		res = turnOnHotkeys(hwnd, TRUE);
	else {
		res = turnOnHotkeys(hwnd, FALSE);

		//It's possible we still have some hotkeys left on..
		if (controlKeysOn == TRUE)
			turnOnControlkeys(hwnd, FALSE);
	}
	if (res==FALSE)
		MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

	//Any windows left?
	if (mmOn==FALSE)
		ShowWindow(hwnd, SW_HIDE);
}


void reBlit() 
{
	//Bit blit our back buffer to the front (should prevent flickering)
	BitBlt(gc,0,0,C_WIDTH,C_HEIGHT,underDC,0,0,SRCCOPY);
}


//Only blit part of the area
void reBlit(RECT blitArea)
{
	//Bit blit our back buffer to the front (should prevent flickering)
	BitBlt(gc,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,underDC,blitArea.left,blitArea.top,SRCCOPY);
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


void expandHWND(int newWidth)
{
	//Resize the current window; use SetWindowPos() since it's easier...
	SetWindowPos(hwnd, NULL, 0, 0, newWidth, C_HEIGHT, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	RECT r;
	GetClientRect(hwnd, &r);
	C_WIDTH = r.right;

	//We also have to set our graphics contexts correctly. Also, throw out the old ones.
	DeleteDC(underDC);
	DeleteObject(bmpDC);
	gc = GetDC(hwnd);
	underDC = CreateCompatibleDC(gc);
	bmpDC = CreateCompatibleBitmap(gc, C_WIDTH, C_HEIGHT);
	SelectObject(underDC, bmpDC);
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
		expandHWND(cumulativeWidth);
	else if (cumulativeWidth<WINDOW_WIDTH)
		expandHWND(WINDOW_WIDTH);
	else if (cumulativeWidth>WINDOW_WIDTH && cumulativeWidth<C_WIDTH)
		expandHWND(cumulativeWidth);

	//Background
	SelectObject(underDC, g_BlackPen);
	SelectObject(underDC, g_DarkGrayBkgrd);
	Rectangle(underDC, 0, 0, C_WIDTH, C_HEIGHT);

	//White overlays
	SelectObject(underDC, g_EmptyPen);
	SelectObject(underDC, g_WhiteBkgrd);
	Rectangle(underDC, borderWidth+1, firstLineStart+1, C_WIDTH-borderWidth-1, secondLineStart-borderWidth);
	Rectangle(underDC, borderWidth+1, secondLineStart, C_WIDTH-borderWidth-1, thirdLineStart-borderWidth);
	Rectangle(underDC, borderWidth+1, thirdLineStart, C_WIDTH-borderWidth-1, fourthLineStart-borderWidth-1);

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

			SelectObject(underDC, g_YellowBkgrd);
			SelectObject(underDC, g_GreenPen);
			Rectangle(underDC, borderWidth+xOffset+1, secondLineStart, borderWidth+1+xOffset+thisStrWidth+spaceWidth, secondLineStart+mmFont->getHeight()+spaceWidth-1);
		}

		mmFont->drawString(underDC, model->getWordString(words[i]), borderWidth+1+spaceWidth/2 + xOffset, secondLineStart+spaceWidth/2);

		if (i<10) {
			swprintf(digit, _T("%i"), ((i+1)%10));
			int digitWidth = mmFont->getStringWidth(digit);
			
			mmFont->drawString(underDC, digit, borderWidth+1+spaceWidth/2 + xOffset + thisStrWidth/2 -digitWidth/2, thirdLineStart-spaceWidth/2-1);
		}

		xOffset += thisStrWidth + spaceWidth;
	}

	mmFontBlack->drawString(underDC, currStr, borderWidth+1+spaceWidth/2, firstLineStart+spaceWidth/2+1);

	//Paint
	reBlit();
}



void selectWord(int id) 
{
	//Are there any words to use?
	std::pair<BOOL, UINT32> typedVal = model->typeSpace(id);
	if (typedVal.first == FALSE)
		return;

	//Send key presses to the top-level program.
	HWND fore = GetForegroundWindow();
	SetActiveWindow(fore);

	//Use SendInput instead of SendMessage, since SendMessage requires the actual
	//  sub-window (component) to recieve the message, whereas SendInput only
	//  requires the top-level window. We could probably hack in SendMessage now that
	//  we're not becoming the active window, but for now I'd rather have a stable
	//  system than one that works on Windows 98.
	std::vector<WORD> keyStrokes = model->getWordKeyStrokes(typedVal.second);
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

	//Turn off control keys
	turnOnControlkeys(hwnd, FALSE);

	//Hide the window
	ShowWindow(hwnd, SW_HIDE);
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
			doneDrag = TRUE;
			
			//Now, create all our buffering objects
			RECT r;
			GetClientRect(hwnd, &r);
			C_WIDTH = r.right;
			C_HEIGHT = r.bottom;

			gc = GetDC(hwnd);
			underDC = CreateCompatibleDC(gc);

			bmpDC = CreateCompatibleBitmap(gc, WINDOW_WIDTH, WINDOW_HEIGHT);
			SelectObject(underDC, bmpDC);
			
			break;
		}
		case WM_HOTKEY:
		{
			//Handle our main language hotkey
			if(wParam == LANG_HOTKEY) {
				//Switch language
				if (mmOn==TRUE)
					switchToLanguage(hwnd, FALSE);
				else
					switchToLanguage(hwnd, TRUE);
				
				//Reset the model
				model->reset(true);
			}


			//Close the window?
			if (wParam == HOTKEY_ESC) {
				model->reset(false);

				//Turn off control keys
				turnOnControlkeys(hwnd, FALSE);

				ShowWindow(hwnd, SW_HIDE);
			}


			//Back up
			if (wParam == HOTKEY_BACK) {
				if (model->backspace()) {
					//Truncate...
					currStr[lstrlen(currStr)-1] = 0;
					recalculate();
				} else {
					//Turn off control keys
					turnOnControlkeys(hwnd, FALSE);

					ShowWindow(hwnd, SW_HIDE);
				}
			}


			//Handle control hotkeys
			if (wParam == HOTKEY_RIGHT) {
				if (model->moveRight(1) == TRUE)
					recalculate();
			} else if (wParam == HOTKEY_LEFT) {
				if (model->moveRight(-1) == TRUE)
					recalculate();
			}


			//Handle numbers
			int numCode = -1;
			if (wParam>=HOTKEY_0 && wParam<=HOTKEY_9)
				numCode = (int)wParam - HOTKEY_0;
			if (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9)
				numCode = (int)wParam - HOTKEY_NUM0;
			if (numCode > -1) {
				//Convert 1..0 to 0..9
				if (--numCode<0)
					numCode = 9;

				//Now, select that word
				selectWord(numCode);
			}

			//Handle space bar
			if (wParam==HOTKEY_SPACE || wParam==HOTKEY_ENTER) {
				selectWord(-1);
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
				if (IsWindowVisible(hwnd) == FALSE)
				{
					//Reset it...
					lstrcpy(currStr, _T(""));
					recalculate();

					//Turn on control keys
					turnOnControlkeys(hwnd, TRUE);

					//Show it
					ShowWindow(hwnd, SW_SHOW);
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
					MessageBox(hwnd, _T("WaitZar version 1.2 - for more information, see: http://code.google.com/p/waitzar/\n\nAlt+Shift - Switch between Myanmar and English\nType Burmese words like they sound, and press \"space\".\n\nWaitZar users should have Zawgyi-One installed, if they want to see what they type after it's chosen."), _T("About"), MB_ICONINFORMATION | MB_OK);
				} else if (retVal == IDM_ENGLISH) {
					switchToLanguage(hwnd, FALSE);
				} else if (retVal == IDM_MYANMAR) {
					switchToLanguage(hwnd, TRUE);
				} else if (retVal == IDM_EXIT) {
					DestroyWindow(hwnd);
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
				if (turnOnHotkeys(hwnd, FALSE) == FALSE)
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


BOOL turnOnHotkeys(HWND hwnd, BOOL on) 
{
	int low_code;
	int high_code;
	BOOL retVal = TRUE;

	for (low_code=HOTKEY_A_LOW; low_code<=HOTKEY_Z_LOW; low_code++)  
	{
		high_code = low_code - 32;
		if (on==TRUE)  {
			//Register this as an uppercase/lowercase letter
			if (RegisterHotKey(hwnd, high_code, MOD_SHIFT, high_code)==FALSE)
				retVal = FALSE;
			if (RegisterHotKey(hwnd, low_code, NULL, high_code)==FALSE)
				retVal = FALSE;
		} else {
			//De-register this as an uppercase/lowercase letter
			if (UnregisterHotKey(hwnd, high_code)==FALSE)
				retVal = FALSE;
			if (UnregisterHotKey(hwnd, low_code)==FALSE)
				retVal = FALSE;
		}
	}

	//Switch to our target language.
	mmOn = on;

	//Change icon in the tray
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
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


BOOL turnOnControlkeys(HWND hwnd, BOOL on) 
{
	BOOL retVal = true;

	//Register control keys
	if (on==TRUE) {
		if (RegisterHotKey(hwnd, HOTKEY_SPACE, NULL, HOTKEY_SPACE)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_ENTER, NULL, VK_RETURN)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_LEFT, NULL, VK_LEFT)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_ESC, NULL, VK_ESCAPE)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_BACK, NULL, VK_BACK)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_RIGHT, NULL, VK_RIGHT)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_UP, NULL, VK_UP)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_DOWN, NULL, VK_DOWN)==FALSE)
			retVal = FALSE;

		//We count numbers as control keys too...
		if (RegisterHotKey(hwnd, HOTKEY_NUM0, NULL, VK_NUMPAD0)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM1, NULL, VK_NUMPAD1)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM2, NULL, VK_NUMPAD2)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM3, NULL, VK_NUMPAD3)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM4, NULL, VK_NUMPAD4)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM5, NULL, VK_NUMPAD5)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM6, NULL, VK_NUMPAD6)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM7, NULL, VK_NUMPAD7)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM8, NULL, VK_NUMPAD8)==FALSE)
			retVal = FALSE;
		if (RegisterHotKey(hwnd, HOTKEY_NUM9, NULL, VK_NUMPAD9)==FALSE)
			retVal = FALSE;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (RegisterHotKey(hwnd, i, NULL, i)==FALSE)
				retVal = FALSE;
		}
	} else {
		if (UnregisterHotKey(hwnd, HOTKEY_SPACE)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_ENTER)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_LEFT)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_ESC)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_BACK)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_RIGHT)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_UP)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_DOWN)==FALSE)
			retVal = FALSE;

		//Numbers
		if (UnregisterHotKey(hwnd, HOTKEY_NUM0)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM1)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM2)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM3)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM4)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM5)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM6)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM7)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM8)==FALSE)
			retVal = FALSE;
		if (UnregisterHotKey(hwnd, HOTKEY_NUM9)==FALSE)
			retVal = FALSE;
		for (int i=HOTKEY_0; i<=HOTKEY_9; i++) {
			if (UnregisterHotKey(hwnd, i)==FALSE)
				retVal = FALSE;
		}
	}

	controlKeysOn = on;
	return retVal;
}


/**
 * Main method for Windows applications
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Save for later; if we try retrieving it, we'll just get a bunch of conversion
	//  warnings. Plus, the hInstance should never change. 
	hInst = hInstance;

	//Create a window class
	LPCWSTR g_szClassName = _T("myWindowClass");

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

	//Set window class's parameters
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = g_DarkGrayBkgrd;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Create a handle to the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	hwnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		g_szClassName,
		_T("WaitZar"), 
		WS_POPUP, //No border or title bar
		100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hInstance, NULL
	);


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
	nid.hWnd = hwnd; //Cauess OUR window to receive notifications for this icon.
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

	//Add our icon to the tray
	Shell_NotifyIcon(NIM_ADD, &nid);

	//Set our hotkey
	if( registerInitialHotkey()==0 )
		MessageBox(NULL, _T("The main language shortcut could not be set up.\nWait Zar will not function properly."), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
	mmOn = FALSE;	

	//Initialize our romanisation string
	lstrcpy(currStr, _T(""));

	//Success?
	if(hwnd == NULL) {
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//If we got this far, let's try to load our file.
	if (loadModel() == FALSE) {
		DestroyWindow(hwnd);
	}

	//Also load user-specific words
	readUserWords();

	//Show it's ready by changing the shell icon
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
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
