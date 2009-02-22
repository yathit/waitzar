/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\onscreenkeyboard.h"


//Our main goal is to figure out the width/height
OnscreenKeyboard::OnscreenKeyboard(PulpCoreFont *titleFont, PulpCoreFont *keysFont, PulpCoreFont *foreFont, PulpCoreImage *cornerImg)
{
	//Save for later
	this->titleFont = titleFont;
	this->keysFont = keysFont;
	this->keysFont->tintSelf(COLOR_LETTERS_LABEL);
	this->foreFont = foreFont;
	this->foreFont->tintSelf(COLOR_LETTERS_REGULAR);
	this->cornerImg[0] = cornerImg;
	this->cornerSize = cornerImg->getWidth();
	for (int i=0; i<61; i++) 
		shiftedKeys[i] = false;

	//Determine the necessary size of our background image
	this->width = (BTN_WIDTHS[BUTTON_KEY]+h_gap)*13 + BTN_WIDTHS[BUTTON_BACKSPACE] + this->cornerSize*2;
	this->height = BTN_HEIGHT*5 + v_gap*4 + titleFont->getHeight()-2+2*this->cornerSize-2 + this->cornerSize;

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


/**
 * Is the keyboard in a shifted state? Affects what text is drawn.
 */
bool OnscreenKeyboard::isShifted()
{
	return shiftedKeys[getKeyPosition(HOTKEY_VIRT_LSHIFT)] || shiftedKeys[getKeyPosition(HOTKEY_VIRT_RSHIFT)];
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

	//Draw the key main label
	int shiftKeyID = keyID;
	PulpCoreFont *myFont = this->foreFont;
	if (this->isShifted()) {
		shiftKeyID += 61;
		myFont = this->foreFontBlue;
	}
	xPos = keyboardOrigin.x+currKey.location.x+keyImg->getWidth()/2-myFont->getCharWidth(shiftKeyID)/2;
	yPos = keyboardOrigin.y+currKey.location.y+19;
	myFont->drawChar(underDC, shiftKeyID, xPos+offset_fore[shiftKeyID], yPos);
}



/**
 * Returns true if this is an actual key-code that we track
 *  hotkeyCode must be either a captial letter, or a hotkey code (hotkeys.h)
 */
bool OnscreenKeyboard::highlightKey(UINT hotkeyCode, bool highlightON)
{
	//Get the key code
	int id = getKeyPosition(hotkeyCode);
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



int OnscreenKeyboard::getKeyPosition(UINT hkCode)
{
	//For now, we use a switch statement (it's fast, and simple)
	// Un-tested key codes are commented out for now
	switch (hkCode)
	{
		//case [HOTKEY_COMBINE]:
		//	return 0;
		//case '1':
		//	return 1;
		//case '2':
		//	return 2;
		//case '3':
		//	return 3
		//case '4':
		//	return 4
		//case '5':
		//	return 5;
		//case '6':
		// return 6;
		//case '7':
		//	return 7;
		//case '8':
		//	return 8;
		//case '9':
		//	return 9;
		//case '0':
		//	return 10;
		//case [HOTKEY_MINUS]:
		//	return 11;
		//case [HOTKEY_EQUALS]:
		//	return 12;
		//case HOTKEY_BACK:
		//	return 13;
		//case [HOTKEY_TAB]:
		//	return 14;
		case 'Q':
			return 15;
		case 'W':
			return 16;
		case 'E':
			return 17;
		case 'R':
			return 18;
		case 'T':
			return 19;
		case 'Y':
			return 20;
		case 'U':
			return 21;
		case 'I':
			return 22;
		case 'O':
			return 23;
		case 'P':
			return 24;
		//case [HOTKEY_BRACKET_L]:
		//	return 25;
		//case [HOTKEY_BRACKET_R]:
		//	return 26;
		//case [HOTKEY_BSLASH]:
		//	return 27;
		//case [HOTKEY_CAPS]:
		//	return 28;
		case 'A':
			return 29;
		case 'S':
			return 30;
		case 'D':
			return 31;
		case 'F':
			return 32;
		case 'G':
			return 33;
		case 'H':
			return 34;
		case 'J':
			return 35;
		case 'K':
			return 36;
		case 'L':
			return 37;
		//case [HOTKEY_SEMI]:
		//	return 38;
		//case [HOTKEY_QUOTE]:
		//	return 39;
		//case VK_RETURN:
		//	return 40;
		case HOTKEY_VIRT_LSHIFT:
			return 41;
		case 'Z':
			return 42;
		case 'X':
			return 43;
		case 'C':
			return 44;
		case 'V':
			return 45;
		case 'B':
			return 46;
		case 'N':
			return 47;
		case 'M':
			return 48;
		case HOTKEY_COMMA:
			return 49;
		case HOTKEY_PERIOD:
			return 50;
		//case [HOTKEY_FSLASH]:
		//	return 51;
		case HOTKEY_VIRT_RSHIFT:
		  return 52;
		//case [HOTKEY_CONTROL_L]:
		//	return 53;
		//case [HOTKEY_WIN_L]:
		//	return 54;
		//case [HOTKEY_ALT_L]:
		//	return 55;
		//case HOTKEY_SPACE:
		//	return 56;
		//case [HOTKEY_ALT_R]:
		//	return 57;
		//case [HOTKEY_WIN_R]:
		//	return 58;
		//case [HOTKEY_CONTEXT_MENU]:
		//	return 59;
		//case [HOTKEY_CTRL_R]:
		//	return 60;
		default:
			return -1;
	}
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

