/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\onscreenkeyboard.h"


//Our main goal is to figure out the width/height
OnscreenKeyboard::OnscreenKeyboard(PulpCoreFont *titleFont, PulpCoreFont *keysFont, PulpCoreFont *foreFont, PulpCoreFont *shiftFont, PulpCoreImage *cornerImg)
{
	//Save for later
	this->titleFont = titleFont;
	this->keysFont = keysFont;
	this->keysFont->tintSelf(COLOR_LETTERS_LABEL);
	this->foreFont = foreFont;
	this->foreFont->tintSelf(COLOR_LETTERS_REGULAR);
	this->shiftFont = shiftFont;
	this->shiftFont->tintSelf(COLOR_LETTERS_REGULAR);
	this->cornerImg[0] = cornerImg;
	this->cornerSize = cornerImg->getWidth();
	for (int i=0; i<61; i++) 
		shiftedKeys[i] = false;

	this->setMode(MODE_HELP);

	//Determine the necessary size of our background image
	this->width = (BTN_WIDTHS[BUTTON_KEY]+h_gap)*13 + BTN_WIDTHS[BUTTON_BACKSPACE] + this->cornerSize*2;
	this->height = BTN_HEIGHT*5 + v_gap*4 + titleFont->getHeight()-2+2*this->cornerSize-2 + this->cornerSize;

	//Determine the necessary size of our memory image
	this->memWidth = this->foreFont->getStringWidth(L"\u1005\u1000\u1064\u102C\u1015\u1030") + this->keysFont->getStringWidth(L"singapore") + cornerImg->getWidth()*2 + 5;
	this->memHeight = this->height;

	//Init our keys
	int currY = 0;
	int currX = 0;
	int currRow = 0;
	int currRowID = 0;
	for (int i=0; i<keys_total; i++) {
		//Get properties
		keys[i].letterPalette = letter_types[i];

		//Lay it out
		keys[i].location.x = currX;
		keys[i].location.y = currY;

		//Set the next location
		currX += BTN_WIDTHS[keys[i].letterPalette] + h_gap;
		currRowID++;
		if (currRowID == keys_per_row[currRow]) {
			//Next row
			currX = 0;
			currY += BTN_HEIGHT + v_gap;
			currRow++;
			currRowID = 0;
		}
	}
}


void OnscreenKeyboard::setMode(int newMode) 
{
	this->mode = newMode;
}



//hdc must already be properly sized
//we'll init a pulp core image with the right size
void OnscreenKeyboard::init(HDC helpMainDC, HDC &helperBufferedDC, HBITMAP &helpBitmap)
{
	//Create a new device context
	bkgrdImg = new PulpCoreImage();
	bkgrdImg->init(this->width, this->height, 0x00000000, helpMainDC, helperBufferedDC, helpBitmap);

	//Save our device context
	this->underDC = helperBufferedDC;

	//Color some fonts
	this->foreFontBlue = new PulpCoreFont();
	this->foreFontBlue->init(foreFont, helpMainDC);
	this->foreFontBlue->tintSelf(COLOR_LETTERS_SHIFTED);
	this->shiftFontBlue = new PulpCoreFont();
	this->shiftFontBlue->init(shiftFont, helpMainDC);
	this->shiftFontBlue->tintSelf(COLOR_LETTERS_SHIFTED);

	//Create rotated copies of our one corner image
	for (int i=1; i<4; i++) {
		this->cornerImg[i] = new PulpCoreImage();
		this->cornerImg[i]->init(this->cornerImg[i-1], underDC);
		this->cornerImg[i]->rotateSelf90DegreesClockwise();
	}

	//Make our buttons, shifted and not
	for (int i=0; i<2; i++) {
		//Set our colors properly
		int borderColor = COLOR_KEY_BORDER_REG;
		int foreColor = COLOR_KEY_FOREGRD_REG;
		if (i==1) {
			borderColor = COLOR_KEY_BORDER_SHIFT;
			foreColor = COLOR_KEY_FOREGRD_SHIFT;
		}

		//Create the buttons
		for (int id=0; id<BUTTONS_IN_TOTAL; id++) {
			PulpCoreImage *btn = makeButton( BTN_WIDTHS[id], BTN_HEIGHT, COLOR_KEY_BKGRD, foreColor, borderColor);
			if (i==0)
				this->buttonsRegular[id] = btn;
			else
				this->buttonsShifted[id] = btn;
		}
	}

	//Make our header/body buttons, to be drawn once, and draw them
	PulpCoreImage *headerButton = makeButton(titleFont->getStringWidth(HELPWND_TITLE)+2*this->cornerSize, titleFont->getHeight()-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->width, this->height-headerButton->getHeight()+this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	headerButton->draw(underDC, 0, 0);
	bodyButton->draw(underDC, 0, headerButton->getHeight()-this->cornerSize);

	//Now we know where our keyboard begins
	keyboardOrigin.x = this->cornerSize;
	keyboardOrigin.y = headerButton->getHeight()-2;

	//Fix their messy intersection
	bkgrdImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	bkgrdImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	bkgrdImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);

	//Delete their un-necessary resources
	delete headerButton;
	delete bodyButton;

	//Now draw the string
	this->titleFont->tintSelf(0x000000);
	this->titleFont->drawString(underDC, HELPWND_TITLE, this->cornerSize, this->cornerSize);
	this->titleFont->tintSelf(0xFFFFFF);

	//Draw all our buttons (we'll just re-draw them when shifted, it saves space)
	for (int i=0; i<keys_total; i++) {
		drawKey(keys[i], i, false);
	}
}



void OnscreenKeyboard::initMemory(HDC memoryMainDC, HDC &memoryBuffDC, HBITMAP &memoryBitmap)
{
	//Create a new device context
	memoryImg = new PulpCoreImage();
	memoryImg->init(this->memWidth, this->memHeight, 0x00000000, memoryMainDC, memoryBuffDC, memoryBitmap);

	//Save our device context
	this->memoryDC = memoryBuffDC;

	//Make our header/body buttons, to be drawn once, and draw them
	PulpCoreImage *headerButton = makeButton(titleFont->getStringWidth(MEMLIST_TITLE)+2*this->cornerSize, titleFont->getHeight()-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->memWidth, this->memHeight-headerButton->getHeight()+this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	headerButton->draw(memoryDC, 0, 0);
	bodyButton->draw(memoryDC, 0, headerButton->getHeight()-this->cornerSize);

	//Fix their messy intersection
	memoryImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	memoryImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	memoryImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);

	//Delete their un-necessary resources
	delete headerButton;
	delete bodyButton;

	//Draw the title string
	this->titleFont->tintSelf(0x000000);
	this->titleFont->drawString(memoryDC, MEMLIST_TITLE, this->cornerSize, this->cornerSize);
	this->titleFont->tintSelf(0xFFFFFF);
}




/**
 * Is the keyboard in a shifted state? Affects what text is drawn.
 */
bool OnscreenKeyboard::isShifted()
{
	return shiftedKeys[getKeyID(HOTKEY_VIRT_LSHIFT)] || shiftedKeys[getKeyID(HOTKEY_VIRT_RSHIFT)];
}



//Helper function
void OnscreenKeyboard::drawKey(key currKey, int keyID, bool isPressed)
{
	//Draw the background
	int pal = currKey.letterPalette;
	PulpCoreImage *keyImg = buttonsRegular[pal];
	if (isPressed)
		keyImg = buttonsShifted[pal];
	keyImg->draw(underDC, keyboardOrigin.x+currKey.location.x, keyboardOrigin.y+currKey.location.y);

	//Draw the letter labels
	int xPos = keyboardOrigin.x+currKey.location.x+4;
	int yPos = keyboardOrigin.y+currKey.location.y+3;
	if (letter_types[keyID]==BUTTON_KEY) //Center it
		xPos += (5 - keysFont->getCharWidth(keyID)/2);
	keysFont->drawChar(underDC, keyID, xPos+offsets_key[keyID], yPos);

	//Prepare to draw keys
	int myKeyID = keyID;
	int myShiftKeyID = keyID + 61;
	PulpCoreFont *myFont = this->foreFont;
	PulpCoreFont *myShiftFont = this->shiftFontBlue;
	if (this->isShifted()) {
		myShiftKeyID = myKeyID;
		myKeyID += 61;
		myFont = this->foreFontBlue;
		myShiftFont = this->shiftFont;
	}

	//Draw the main label
	if (!hide_for_help[myKeyID]) {
		xPos = keyboardOrigin.x+currKey.location.x+keyImg->getWidth()/2-myFont->getCharWidth(myKeyID)/2;
		yPos = keyboardOrigin.y+currKey.location.y+19;
		myFont->drawChar(underDC, myKeyID, xPos+offset_fore[myKeyID], yPos);
	}

	//Draw the shifted label
	if (!hide_for_help[myShiftKeyID]) {
		xPos = keyboardOrigin.x+currKey.location.x+23-myShiftFont->getCharWidth(myShiftKeyID)/2;
		yPos = keyboardOrigin.y+currKey.location.y+3;
		myShiftFont->drawChar(underDC, myShiftKeyID, xPos+offset_super[myShiftKeyID], yPos);
	}
}



/**
 * Returns the vkcode if this is an actual key-code that we track
 *  hotkeyCode must be either a letter, or a hotkey code (hotkeys.h)
 * Returns -1 if we don't track this
 */
bool OnscreenKeyboard::highlightKey(UINT hotkeyCode, bool highlightON)
{
	//Get the key code, unshifted
	int id = getKeyID(hotkeyCode);
	if (id>=61)
		id -= 61;
	if (id==-1)
		return false;
	
	//Mark as "highlighted"
	bool wasShifted = this->isShifted();
	shiftedKeys[id] = highlightON;
	if (wasShifted != this->isShifted()) {
		//We need to re-print the entire keyboard
		for (int i=0; i<61; i++) {
			drawKey(keys[i], i, shiftedKeys[i]);
		}
	} else {
		//Re-draw only this key
		drawKey(keys[id], id, highlightON);
	}

	//Succeeded
	return true;
}

int OnscreenKeyboard::getVirtualKeyID(UINT hotkeyCode)
{
	int id = getKeyID(hotkeyCode);
	if (id>=61)
		id -= 61;
	if (id==-1)
		return -1;

	return keyboard_vk_codes[id];
}


//Instead of returning a key position, returns an ID into a virtual array of such keys, 
//  with the supered keys from IDs 61 to 121.
//We could just use isShifted() for the super keys, but we don't want to rely on our thread.
//Also, some keys will never be used, like HOTKEY_SUPER_ENTER; we're just adding them for completeness.
int OnscreenKeyboard::getKeyID(UINT hkCode)
{
	switch (hkCode) 
	{
		case HOTKEY_COMBINE:
			return 0;
		case HOTKEY_1:
			return 1;
		case HOTKEY_2:
			return 2;
		case HOTKEY_3:
			return 3;
		case HOTKEY_4:
			return 4;
		case HOTKEY_5:
			return 5;
		case HOTKEY_6:
			return 6;
		case HOTKEY_7:
			return 7;
		case HOTKEY_8:
			return 8;
		case HOTKEY_9:
			return 9;
		case HOTKEY_0:
			return 10;
		case HOTKEY_MINUS:
			return 11;
		case HOTKEY_EQUALS:
			return 12;
		//case [HOTKEY_BACKSPACE]:
		//	return 13;
		//case [HOTKEY_TAB]:
		//	return 14;
		case 'q':
			return 15;
		case 'w':
			return 16;
		case 'e':
			return 17;
		case 'r':
			return 18;
		case 't':
			return 19;
		case 'y':
			return 20;
		case 'u':
			return 21;
		case 'i':
			return 22;
		case 'o':
			return 23;
		case 'p':
			return 24;
		case HOTKEY_LEFT_BRACKET:
			return 25;
		case HOTKEY_RIGHT_BRACKET:
			return 26;
		case HOTKEY_BACKSLASH:
			return 27;
		//case [HOTKEY_CAPSLOCK]:
		//	return 28;
		case 'a':
			return 29;
		case 's':
			return 30;
		case 'd':
			return 31;
		case 'f':
			return 32;
		case 'g':
			return 33;
		case 'h':
			return 34;
		case 'j':
			return 35;
		case 'k':
			return 36;
		case 'l':
			return 37;
		case HOTKEY_SEMICOLON:
			return 38;
		case HOTKEY_APOSTROPHE:
			return 39;
		case HOTKEY_ENTER:
			return 40;
		case HOTKEY_VIRT_LSHIFT:
			return 41;
		case 'z':
			return 42;
		case 'x':
			return 43;
		case 'c':
			return 44;
		case 'v':
			return 45;
		case 'b':
			return 46;
		case 'n':
			return 47;
		case 'm':
			return 48;
		case HOTKEY_COMMA:
			return 49;
		case HOTKEY_PERIOD:
			return 50;
		case HOTKEY_FORWARDSLASH:
			return 51;
		case HOTKEY_VIRT_RSHIFT:
			return 52;
		//case [HOTKEY_LEFT_CONTROL]:
		//	return 53;
		//case [HOTKEY_LEFT_WIN]:
		//	return 54;
		//case [HOTKEY_LEFT_ALT]:
		//	return 55;
		case HOTKEY_SPACE:
			return 56;
		//case [HOTKEY_RIGHT_ALT]:
		//	return 57;
		//case [HOTKEY_RIGHT_WIN]:
		//	return 58;
		//case [HOTKEY_CMENU]:
		//	return 59;
		//case [HOTKEY_RIGHT_CTRL]:
		//	return 60;
		case HOTKEY_SHIFT_COMBINE:
			return 61;
		case HOTKEY_SHIFT_1:
			return 62;
		case HOTKEY_SHIFT_2:
			return 63;
		case HOTKEY_SHIFT_3:
			return 64;
		case HOTKEY_SHIFT_4:
			return 65;
		case HOTKEY_SHIFT_5:
			return 66;
		case HOTKEY_SHIFT_6:
			return 67;
		case HOTKEY_SHIFT_7:
			return 68;
		case HOTKEY_SHIFT_8:
			return 69;
		case HOTKEY_SHIFT_9:
			return 70;
		case HOTKEY_SHIFT_0:
			return 71;
		case HOTKEY_SHIFT_MINUS:
			return 72;
		case HOTKEY_SHIFT_EQUALS:
			return 73;
		//case [HOTKEY_SUP_BACKSPACE]:
		//	return 74;
		//case [HOTKEY_SUP_TAB]:
		//	return 75;
		case 'Q':
			return 76;
		case 'W':
			return 77;
		case 'E':
			return 78;
		case 'R':
			return 79;
		case 'T':
			return 80;
		case 'Y':
			return 81;
		case 'U':
			return 82;
		case 'I':
			return 83;
		case 'O':
			return 84;
		case 'P':
			return 85;
		case HOTKEY_SHIFT_LEFT_BRACKET:
			return 86;
		case HOTKEY_SHIFT_RIGHT_BRACKET:
			return 87;
		case HOTKEY_SHIFT_BACKSLASH:
			return 88;
		//case [HOTKEY_SUP_CAPSLOCK]:
		//	return 89;
		case 'A':
			return 90;
		case 'S':
			return 91;
		case 'D':
			return 92;
		case 'F':
			return 93;
		case 'G':
			return 94;
		case 'H':
			return 95;
		case 'J':
			return 96;
		case 'K':
			return 97;
		case 'L':
			return 98;
		case HOTKEY_SHIFT_SEMICOLON:
			return 99;
		case HOTKEY_SHIFT_APOSTROPHE:
			return 100;
		case HOTKEY_SHIFT_ENTER:
			return 101;
		//case [HOTKEY_SUP_LSHIFT]:
		//	return 102;
		case 'Z':
			return 103;
		case 'X':
			return 104;
		case 'C':
			return 105;
		case 'V':
			return 106;
		case 'B':
			return 107;
		case 'N':
			return 108;
		case 'M':
			return 109;
		case HOTKEY_SHIFT_COMMA:
			return 110;
		case HOTKEY_SHIFT_PERIOD:
			return 111;
		case HOTKEY_SHIFT_FORWARDSLASH:
			return 112;
		//case [HOTKEY_SUP_RSHIFT]:
		//	return 113;
		//case [HOTKEY_SUP_LEFT_CONTROL]:
		//	return 114;
		//case [HOTKEY_SUP_LEFT_WIN]:
		//	return 115;
		//case [HOTKEY_SUP_LEFT_ALT]:
		//	return 116;
		case HOTKEY_SHIFT_SPACE:
			return 117;
		//case [HOTKEY_SUP_RIGHT_ALT]:
		//	return 118;
		//case [HOTKEY_SUP_RIGHT_WIN]:
		//	return 119;
		//case [HOTKEY_SUP_CMENU]:
		//	return 120;
		//case [HOTKEY_SUP_RIGHT_CTRL]:
		//	return 121;
		default:
			return -1;
	}
}




wchar_t* OnscreenKeyboard::typeLetter(DWORD hotkeyCode)
{
	int id = getKeyID(hotkeyCode);
	if (id==-1 || hide_for_help[id])
		return NULL;

	//Special cases
	if (typeableLetters[id]==0x0000) {
		if (id==65)
			return L"\u1000\u103B\u1015\u103A";
		else if (id==88)
			return L"\u100B\u1039\u100C";
		else if (id==93)
			return L"\u1004\u103A\u1039";
		else if (id==99)
			return L"\u102B\u103A";
		return NULL;
	}

	lstrcpy(typedString, L"1");
	typedString[0] = typeableLetters[id];
	return typedString;
}




//Make a "button", with a given width and height (including border), background color (can be transparent) and
// foreground color. Border color is changed by calling "tint self" on cornerImg before this.
PulpCoreImage* OnscreenKeyboard::makeButton(int width, int height, int bgARGB, int fgARGB, int borderARGB)
{
	//Make an empty image
	PulpCoreImage *result = new PulpCoreImage();
	HDC resDC;
	HBITMAP resBMP;
	result->init(width, height, bgARGB, underDC, resDC, resBMP);

	//Draw the foreground before overlaying the corner images
	int buffer_offset = 3;
	result->fillRectangle(this->cornerSize-buffer_offset, 1, result->getWidth()-2*this->cornerSize+2*buffer_offset, this->cornerSize, fgARGB);
	result->fillRectangle(this->cornerSize-buffer_offset, result->getHeight()-this->cornerSize, result->getWidth()-2*this->cornerSize+2*buffer_offset, this->cornerSize-1, fgARGB);
	result->fillRectangle(1, this->cornerSize-buffer_offset, result->getWidth()-2, result->getHeight()-2*this->cornerSize+2*buffer_offset, fgARGB);

	//A few pixels require manual setting
	result->fillRectangle(3, 2, 1, 2, fgARGB);
	result->fillRectangle(2, 3, 1, 1, fgARGB);
	result->fillRectangle(result->getWidth()-1-3, 2, 1, 2, fgARGB);
	result->fillRectangle(result->getWidth()-1-2, 3, 1, 1, fgARGB);
	result->fillRectangle(result->getWidth()-1-3, result->getHeight()-4, 1, 2, fgARGB);
	result->fillRectangle(result->getWidth()-1-2, result->getHeight()-4, 1, 1, fgARGB);
	result->fillRectangle(3, result->getHeight()-4, 1, 2, fgARGB);
	result->fillRectangle(2, result->getHeight()-4, 1, 1, fgARGB);

	//Draw our corner images
	cornerImg[0]->tintSelf(borderARGB);
	cornerImg[0]->draw(resDC, 0, 0);
	cornerImg[1]->tintSelf(borderARGB);
	cornerImg[1]->draw(resDC, result->getWidth()-this->cornerSize, 0);
	cornerImg[2]->tintSelf(borderARGB);
	cornerImg[2]->draw(resDC, result->getWidth()-this->cornerSize, result->getHeight()-this->cornerSize);
	cornerImg[3]->tintSelf(borderARGB);
	cornerImg[3]->draw(resDC, 0, result->getHeight()-this->cornerSize);

	//Draw the remaining lines
	result->fillRectangle(this->cornerSize, 0, result->getWidth()-2*this->cornerSize, 2, borderARGB);
	result->fillRectangle(this->cornerSize, result->getHeight()-2, result->getWidth()-2*this->cornerSize, 2, borderARGB);
	result->fillRectangle(0, this->cornerSize, 2, result->getHeight()-2*this->cornerSize, borderARGB);
	result->fillRectangle(result->getWidth()-2, this->cornerSize, 2, result->getHeight()-2*this->cornerSize, borderARGB);

	//There, that was easy
	return result;
}

int OnscreenKeyboard::getWidth()
{
	return this->width;
}

int OnscreenKeyboard::getHeight()
{
	return this->height;
}

int OnscreenKeyboard::getMemoryWidth()
{
	return this->memWidth;
}

int OnscreenKeyboard::getMemoryHeight()
{
	return this->memHeight;
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

