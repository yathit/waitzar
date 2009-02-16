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
const int BUTTON_SHIFT     = 4; //92
const int BUTTON_SPACEBAR  = 5; //152
const int BUTTON_CAPSLOCK  = 6; //59
const int BUTTONS_IN_TOTAL = 7;

const int BTN_HEIGHT = 43;
const int BTN_WIDTHS[] = {33, 67, 50, 75, 92, 152, 59};
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

#define HELPWND_TITLE             _T("WaitZar Low-Level Input")



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

	int getWidth();
	int getHeight();

private:
	//Help window context & image
	HDC underDC;
	PulpCoreImage *bkgrdImg;

	//Cached pics
	PulpCoreFont *titleFont;
	PulpCoreImage *cornerImg[4];

	//Useful helpers
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

