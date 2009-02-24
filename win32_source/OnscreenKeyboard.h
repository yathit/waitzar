/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#define _UNICODE
#define UNICODE

//Don't let Visual Studio warn us to use the _s functions
#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
#define _CRT_NON_CONFORMING_SWPRINTFS


#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "PulpCoreImage.h"
#include "PulpCoreFont.h"
#include "Hotkeys.h"

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

//Useful defines
#define COLOR_KEY_BKGRD         0xFF9AA4E2
#define COLOR_KEY_BORDER_REG    0xFF606060
#define COLOR_KEY_BORDER_SHIFT  0xFF440066
#define COLOR_KEY_FOREGRD_REG   0xFFD3D3D3
#define COLOR_KEY_FOREGRD_SHIFT 0xFFFFAAAA

#define COLOR_KEYBOARD_BKGRD    0x00FFFFFF
#define COLOR_KEYBOARD_FOREGRD  0xFF9AA4E2
#define COLOR_KEYBOARD_BORDER   0xFF000000

#define COLOR_LETTERS_LABEL     0xFF606060
#define COLOR_LETTERS_REGULAR   0xFF000000
#define COLOR_LETTERS_SHIFTED   0xFF0019FF

//Modes
#define MODE_HELP     0
#define MODE_INPUT    1


#define HELPWND_TITLE           _T("WaitZar Low-Level Input")


//Useful struct for our keys
struct key {
	POINT location;
	int letterPalette;
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


/**
 * This class exists to facilitate display of the on-screen keyboard, 
 * by managing all images and display surfaces. The goal is to minimize
 * memory and cycle wastage without going entirely overboard.
 */
class OnscreenKeyboard
{
public:
	OnscreenKeyboard(PulpCoreFont *titleFont, PulpCoreFont *keysFont, PulpCoreFont *foreFont, PulpCoreFont *shiftFont, PulpCoreImage *cornerImg);
	void init(HDC helpMainDC, HDC &helperBufferedDC, HBITMAP &helpBitmap);

	bool highlightKey(UINT hotkeyCode, bool highlightON);

	void setMode(int newMode);

	int getWidth();
	int getHeight();

private:
	//Help window context & image
	HDC underDC;
	PulpCoreImage *bkgrdImg;

	//Cached pics
	PulpCoreFont *titleFont;
	PulpCoreFont *keysFont;
	PulpCoreFont *foreFont;
	PulpCoreFont *shiftFont;
	PulpCoreFont *foreFontBlue;
	PulpCoreFont *shiftFontBlue;
	PulpCoreImage *cornerImg[4];

	//Buttons
	key keys[keys_total];

	//Run-time status of buttons
	bool shiftedKeys[61];

	//Useful helpers
	POINT keyboardOrigin;
	int cornerSize;
	int width;
	int height;
	int mode;

	//Are we in a shifted state?
	bool isShifted();

	//Button palettes
	PulpCoreImage *buttonsRegular[BUTTONS_IN_TOTAL];
	PulpCoreImage *buttonsShifted[BUTTONS_IN_TOTAL];

	//Make a button
	PulpCoreImage* makeButton(int width, int height, int bgARGB, int fgARGB, int borderARGB);
	void drawKey(key currKey, int keyID, bool isPressed);

	//Helper
	int getKeyPosition(UINT hkCode);


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

