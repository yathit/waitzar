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
#define COLOR_KEY_BORDER_SHIFT  0xFFFF0000
#define COLOR_KEY_FOREGRD_REG   0xFFD3D3D3
#define COLOR_KEY_FOREGRD_SHIFT 0xFF00FF00

#define COLOR_KEYBOARD_BKGRD    0x00FFFFFF
#define COLOR_KEYBOARD_FOREGRD  0xFF9AA4E2
#define COLOR_KEYBOARD_BORDER   0xFF000000

#define HELPWND_TITLE           _T("WaitZar Low-Level Input")


//Useful struct for our keys
struct key {
	POINT location;
	wchar_t letter;
	wchar_t lblRegular;
	wchar_t lblShifted;
	int letterPalette;
};
const int keys_per_row[] = {14, 14, 13, 12, 8};
const int keys_total = 14 + 14 + 13 + 12 + 8;

//For now, just have this here
const char letter[] = {'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'b', 't', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\', 'c', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 'e', 's', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 's', 'c', 'w', 'a', 's', 'a', 'w', 'm', 'c'};
const int letter_types[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_BACKSPACE, BUTTON_UTILITY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_UTILITY, BUTTON_CAPSLOCK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_ENTER, BUTTON_SHIFT_L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_SHIFT_R, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_SPACEBAR, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY, BUTTON_UTILITY};
const wchar_t mm_reg[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
const wchar_t mm_shift[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};



/**
 * This class exists to facilitate display of the on-screen keyboard, 
 * by managing all images and display surfaces. The goal is to minimize
 * memory and cycle wastage without going entirely overboard.
 */
class OnscreenKeyboard
{
public:
	OnscreenKeyboard(PulpCoreFont *titleFont, PulpCoreImage *cornerImg);
	void init(HDC helpMainDC, HDC &helperBufferedDC, HBITMAP &helpBitmap);

	void highlightKey(char keyCode, bool highlightON);

	int getWidth();
	int getHeight();

private:
	//Help window context & image
	HDC underDC;
	PulpCoreImage *bkgrdImg;

	//Cached pics
	PulpCoreFont *titleFont;
	PulpCoreImage *cornerImg[4];

	//Buttons
	key keys[keys_total];

	//Useful helpers
	POINT keyboardOrigin;
	int cornerSize;
	int width;
	int height;

	//Button palettes
	PulpCoreImage *buttonsRegular[BUTTONS_IN_TOTAL];
	PulpCoreImage *buttonsShifted[BUTTONS_IN_TOTAL];

	//Make a button
	PulpCoreImage* makeButton(int width, int height, int bgARGB, int fgARGB, int borderARGB);


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

