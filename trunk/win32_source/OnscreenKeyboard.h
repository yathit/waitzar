/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#define _UNICODE
#define UNICODE

//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
//#define _CRT_NON_CONFORMING_SWPRINTFS


#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <string>
#include <list>

#include "Pulp Core/PulpCoreImage.h"
#include "Pulp Core/PulpCoreFont.h"
#include "MyWin32Window.h"
#include "Hotkeys.h"
#include "Input/VirtKey.h"

//Useful constants
const int BUTTON_KEY       = 0; //33 x 43
const int BUTTON_BACKSPACE = 1; //67
const int BUTTON_UTILITY   = 2; //50, also backslash, tab
const int BUTTON_ENTER     = 3; //75
const int BUTTON_SHIFT_L   = 4; //76
const int BUTTON_SHIFT_R   = 5; //92
const int BUTTON_SPACEBAR  = 6; //152
const int BUTTON_CAPSLOCK  = 7; //59
const int BUTTONS_IN_TOTAL = 8;

const int BTN_HEIGHT = 43;
const int BTN_WIDTHS[] = {33, 67, 50, 75, 76, 92, 152, 59};
const int h_gap = 1;
const int v_gap = 2;

//Useful defines  --todo: move internal, make static.
const unsigned int COLOR_KEY_BKGRD =          0xFF9AA4E2;
const unsigned int COLOR_KEY_BORDER_REG =     0xFF606060;
const unsigned int COLOR_KEY_BORDER_SHIFT =   0xFF440066;
const unsigned int COLOR_KEY_FOREGRD_REG =    0xFFD3D3D3;
const unsigned int COLOR_KEY_FOREGRD_SHIFT =  0xFFFFAAAA;

const unsigned int COLOR_KEYBOARD_BKGRD =     0x00FFFFFF;
const unsigned int COLOR_KEYBOARD_FOREGRD =   0xFF9AA4E2;
const unsigned int COLOR_KEYBOARD_BORDER =    0xFF000000;

const unsigned int COLOR_LETTERS_LABEL =      0xFF606060;
const unsigned int COLOR_LETTERS_REGULAR =    0xFF000000;
const unsigned int COLOR_LETTERS_SHIFTED =    0xFF0019FF;

const unsigned int COLOR_CLOSEBTN_HIGHLIGHT =  0xFFDD0020;
const unsigned int COLOR_MINMAXBTN_HIGHLIGHT = 0xFF33FF77;

//Modes
enum {
	MODE_HELP = 0,
    MODE_INPUT
};


const std::wstring HELPWND_TITLE = L"WaitZar Word Finder";
const std::wstring MEMLIST_TITLE = L"Memory List";


//Useful class for our keys
class KbdKey {
public: 

	//Static
	POINT location;
	int letterPalette;

	//State
	bool mouseIsOver;
	bool kbdIsPressed;

	//Constructors
	KbdKey(const POINT& location, int letterPalette) : location(location), letterPalette(letterPalette), mouseIsOver(false), kbdIsPressed(false) {}

	//Helper
	bool isHighlighted() { return mouseIsOver || kbdIsPressed; }
};
const int keys_per_row[] = {14, 14, 13, 12, 8};
const int keys_total = 14 + 14 + 13 + 12 + 8;

//What type is each key on the keyboard?
const int letter_types[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_BACKSPACE, BUTTON_UTILITY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_UTILITY, BUTTON_CAPSLOCK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_ENTER, BUTTON_SHIFT_L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_SHIFT_R, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_SPACEBAR, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY};

//List of offsets for each key, so that they line up perfectly
//  Second line is for shifted keys, if they exist
const int offsets_key[] = {0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 3, 1, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const int offset_fore[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, -2, 0, -3, 0, 0, 0, 3, 1, 1, 2, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, -1, 0, 0, -2, 0, 1, 1, 1, 0, 2, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const int offset_super[] = {
	-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 17, 0, 0, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 1, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//For the "help" function, we don't want users to type invalid words, so we eliminate the equals sign, etc.
const bool hide_for_help[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true, true, true, true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true, 0, true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true, true, 0, 0, 0, 0, 0, 0, 0, 0, true, true, true, true, true, true, true, true, true, true, true,
	true, 0, 0, 0, 0, true, true, 0, 0, true, true, true, true, true, true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true, 0, true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, true, true, 0, 0, 0, 0, 0, 0, 0, true, true, true, true,  true, true, true, true,  true, true, true, true
};

//The typed unicode letters returned by our keyboard
//  NOTE a valid wc-string; just an array of wchars, for once :)
const wchar_t typeableLetters[] = {
	0x1039, 0x1041, 0x1042, 0x1043, 0x1044, 0x1045, 0x1046, 0x1047, 0x1048, 0x1049, 0x1040, 0x0000, 0x0000, 0x0000, 0x0000, 0x1006, 0x1010, 0x1014, 0x1019, 0x1021, 0x1015, 0x1000, 0x1004, 0x101E, 0x1005, 0x101F, 0x2018, 0x104F, 0x0000, 0x1031, 0x103B, 0x102D, 0x103A, 0x102B, 0x1037, 0x103C, 0x102F, 0x1030, 0x1038, 0x1012, 0x0000, 0x0000, 0x1016, 0x1011, 0x1001, 0x101C, 0x1018, 0x100A, 0x102C, 0x101A, 0x002E, 0x104B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x100D, 0x100E, 0x100B, 0x0000, 0x0025, 0x002F, 0x101B, 0x1002, 0x0028, 0x0029, 0x0000, 0x0000, 0x0000, 0x0000, 0x1029, 0x101D, 0x103F, 0x1023, 0x1024, 0x104C, 0x1009, 0x104D, 0x1025, 0x100F, 0x1027, 0x2019, 0x0000, 0x0000, 0x1017, 0x103E, 0x102E, 0x0000, 0x103D, 0x1036, 0x1032, 0x102F, 0x1030, 0x0000, 0x1013, 0x0000, 0x0000, 0x1007, 0x100C, 0x1003, 0x1020, 0x1026, 0x1008, 0x102A, 0x002C, 0x104E, 0x104A, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

//Helper re-mapping. Unknown codes are -1 for now, please re-map later
const int keyboard_vk_codes[] = {
	VK_OEM_3, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK, VK_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', VK_OEM_4, VK_OEM_6, VK_OEM_5, -1, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_1, VK_OEM_7, VK_RETURN, VK_LSHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2, VK_RSHIFT, -1, -1, -1, VK_SPACE, -1, -1, -1, -1
};


/**
 * This class exists to facilitate display of the on-screen keyboard, 
 * by managing all images and display surfaces. The goal is to minimize
 * memory and cycle wastage without going entirely overboard.
 */
class OnscreenKeyboard
{
public:
	OnscreenKeyboard(DisplayMethod *titleFont, PulpCoreFont *keysFont, PulpCoreFont *foreFont, PulpCoreFont *shiftFont, PulpCoreFont *memoryFont, PulpCoreImage *cornerImg, PulpCoreImage *closeImg);
	void init(MyWin32Window *helpWindow, MyWin32Window *memoryWindow, void(*OnHelpTitleBtnClick)(unsigned int), void(*OnHelpTitleBtnOver)(unsigned int), void(*OnHelpTitleBtnOut)(unsigned int), void(*OnMemTitleBtnClick)(unsigned int), void(*OnMemTitleBtnOver)(unsigned int), void(*OnMemTitleBtnOut)(unsigned int));
	void initHelp(void(*OnTitleBtnClick)(unsigned int), void(*OnTitleBtnOver)(unsigned int), void(*OnTitleBtnOut)(unsigned int));
	void initHelpSmall();
	void initMemory(void(*OnTitleBtnClick)(unsigned int), void(*OnTitleBtnOver)(unsigned int), void(*OnTitleBtnOut)(unsigned int));
	void initMemorySmall();
	void initImagesEtc();

	bool highlightKey(unsigned int vkCode, char alphanum, bool modShift, bool highlightON);

	void setMode(int newMode);

	int getVirtualKeyID(unsigned int vkCode, char alphanum, bool modShift) const;

	//
	//NOTE: This will eventually become part of the myWin2.2 model; in fact, it might 
	//  be a good idea to start refactoring here.
	//
	std::wstring typeLetter(unsigned int vkCode, char alphanum, bool modShift);

	void addMemoryEntry(const std::wstring &my, const std::string &rom);
	size_t getMaxMemoryEntries() const;
	void refreshMemoryList();

	void highlightHelpTitleBtn(unsigned int btnID, bool isHighlighted);
	void highlightMemoryTitleBtn(unsigned int btnID, bool isHighlighted);
	bool closeHelpWindow(unsigned int btnID);
	void minmaxHelpWindow(unsigned int btnID);
	bool closeMemoryWindow(unsigned int btnID);
	void minmaxMemoryWindow(unsigned int btnID);

	void clickButton(unsigned int btnID);
	void highlightVirtKey(unsigned int btnID, bool isHighlighted);

	//NOTE: We need to call THIS instead of calling "helpWindow->show"
	void turnOnHelpMode(bool on, bool skipHelpWin, bool skipMemWin);
	bool isHelpEnabled();

	VirtKey getLastClickedVKey();

	int getWidth() const;
	int getHeight() const;
	int getMemoryWidth() const;
	int getMemoryHeight() const;

private:
	//Help window context & image
	//HDC underDC;
	MyWin32Window *helpWindow;
	PulpCoreImage *bkgrdImg;

	//Memory list context & image
	//HDC memoryDC;
	MyWin32Window *memoryWindow;
	PulpCoreImage *memoryImg;

	//The actual memory list
	std::list< std::pair<std::wstring, std::string> > memoryList;

	//Cached pics
	DisplayMethod *titleFont;
	PulpCoreFont *keysFont;
	PulpCoreFont *foreFont;
	PulpCoreFont *shiftFont;
	PulpCoreFont *memoryFont;
	PulpCoreImage *cornerImg[4];
	PulpCoreImage *closeImg;

	//Constants
	const static unsigned int title_btn_size = 18;
	const static unsigned int title_btn_margin = 2;

	//Saved button IDs, etc.
	RECT closeBtnRect;
	RECT minmaxBtnRect;
	unsigned int closeBtnID;
	unsigned int minmaxBtnID;
	bool closeBtnHighlight;
	bool minmaxBtnHighlight;

	//Copied a bit from the helpTitleBtns
	RECT closeMemBtnRect;
	RECT minmaxMemBtnRect;
	unsigned int closeMemBtnID;
	unsigned int minmaxMemBtnID;
	bool closeMemBtnHighlight;
	bool minmaxMemBtnHighlight;

	VirtKey lastClickedButton;

	//Buttons
	std::vector<KbdKey> keys;
	//KbdKey keys[keys_total];

	//Run-time status of buttons
	//bool shiftedKeys[61];

	//Useful helpers
	POINT keyboardOrigin;
	int cornerSize;
	int width;
	int height;
	int smallHeight;
	int memWidth;
	int memHeight;
	int smallMemHeight;
	int memEntriesStartY;
	int memEntriesYPlus;
	int memEntriesMax;
	int mode;

	bool helpIsOn;
	bool helpWinMinimized;
	bool memWinMinimized;

	//Are we in a shifted state?
	bool isShifted();

	//Is this the kind of key we can press manually?
	bool isPressableButton(size_t btnID);

	//Button palettes
	PulpCoreImage *buttonsRegular[BUTTONS_IN_TOTAL];
	PulpCoreImage *buttonsShifted[BUTTONS_IN_TOTAL];

	//Make a button
	PulpCoreImage* makeButton(int width, int height, int bgARGB, int fgARGB, int borderARGB);
	void drawKey(const KbdKey& currKey, int keyID, bool isPressed);

	//Helper
	int getKeyID(unsigned int vkCode, char alphanum, bool modShift) const;
	void drawTitleButtons();
	void drawMemTitleButtons();


};




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

