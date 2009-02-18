/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\onscreenkeyboard.h"


//Our main goal is to figure out the width/height
OnscreenKeyboard::OnscreenKeyboard(PulpCoreFont *titleFont, PulpCoreImage *cornerImg)
{
	//Save for later
	this->titleFont = titleFont;
	this->cornerImg[0] = cornerImg;
	this->cornerSize = cornerImg->getWidth();

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
		keys[i].letter = letter[i];
		keys[i].lblRegular = mm_reg[i];
		keys[i].lblShifted = mm_shift[i];
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
		buttonsRegular[keys[i].letterPalette]->draw(underDC, keyboardOrigin.x+keys[i].location.x, keyboardOrigin.y+keys[i].location.y);
	}
}



/**
 * Returns true if this is an actual key-code that we track
 */
bool OnscreenKeyboard::highlightKey(WPARAM hotkeyCode, bool highlightON)
{
	//Get the key code
	int id = 0;
	switch (hotkeyCode)
	{
		case 't':
			id = 19;
			break;
		default:
			return false;
	}

	//Re-draw this key
	int pal = keys[id].letterPalette;
	PulpCoreImage *keyImg = buttonsRegular[pal];
	if (highlightON)
		keyImg = buttonsShifted[pal];
	keyImg->draw(underDC, keyboardOrigin.x+keys[id].location.x, keyboardOrigin.y+keys[id].location.y);

	//Succeeded
	return true;
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

