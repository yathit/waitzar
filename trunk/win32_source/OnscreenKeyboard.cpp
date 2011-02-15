/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\onscreenkeyboard.h"

using std::wstring;
using std::string;
using std::list;
using std::pair;


//Our main goal is to figure out the width/height
OnscreenKeyboard::OnscreenKeyboard(DisplayMethod *titleFont, PulpCoreFont *keysFont, PulpCoreFont *foreFont, PulpCoreFont *shiftFont, PulpCoreFont *memoryFont, PulpCoreImage *cornerImg, PulpCoreImage *closeImg) : lastClickedButton(0)
{
	//Save for later
	this->titleFont = titleFont;
	this->keysFont = keysFont;
	this->keysFont->setColor(((COLOR_LETTERS_LABEL&0xFF0000)>>16), ((COLOR_LETTERS_LABEL&0xFF00)>>8), (COLOR_LETTERS_LABEL&0xFF));
	this->foreFont = foreFont;
	this->foreFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));
	this->shiftFont = shiftFont;
	this->shiftFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));
	this->memoryFont = memoryFont;
	this->memoryFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));
	this->closeImg = closeImg;
	this->cornerImg[0] = cornerImg;
	this->cornerSize = cornerImg->getWidth();
	this->setMode(MODE_HELP);

	this->helpWinMinimized = false;
	this->memWinMinimized = false;
	this->closeBtnHighlight = false;
	this->closeMemBtnHighlight = false;
	this->minmaxBtnHighlight = false;
	this->minmaxMemBtnHighlight = false;
	this->helpIsOn = false;
	this->bkgrdImg = NULL;

	//Determine the necessary size of our background image
	//TODO: Again, fix the NULLs
	this->width = (BTN_WIDTHS[BUTTON_KEY]+h_gap)*13 + BTN_WIDTHS[BUTTON_BACKSPACE] + this->cornerSize*2;
	this->height = BTN_HEIGHT*5 + v_gap*4 + titleFont->getHeight(NULL)-2+2*this->cornerSize-2 + this->cornerSize;
	this->smallHeight = titleFont->getHeight(NULL) + 2*this->cornerSize-2; 

	//Determine the necessary size of our memory image
	this->memWidth = this->foreFont->getStringWidth(L"\u1005\u1000\u1064\u102C\u1015\u1030", NULL) + this->keysFont->getStringWidth(L"singapore", NULL) + cornerImg->getWidth()*2 + 5;
	this->memHeight = this->height;
	this->smallMemHeight = this->smallHeight;

	//Init our keys
	int currY = 0;
	int currX = 0;
	size_t currRow = 0;
	size_t currRowID = 0;
	for (size_t i=0; i<keys_total; i++) {
		//Get properties
		int pal = letter_types[i];

		//Lay it out
		POINT loc;
		loc.x = currX;
		loc.y = currY;

		//Save it
		keys.push_back(KbdKey(loc, pal));

		//Set the next location
		currX += BTN_WIDTHS[keys[keys.size()-1].letterPalette] + h_gap;
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
void OnscreenKeyboard::init(MyWin32Window *helpWindow, MyWin32Window *memoryWindow, void(*OnHelpTitleBtnClick)(unsigned int), void(*OnHelpTitleBtnOver)(unsigned int), void(*OnHelpTitleBtnOut)(unsigned int), void(*OnMemTitleBtnClick)(unsigned int), void(*OnMemTitleBtnOver)(unsigned int), void(*OnMemTitleBtnOut)(unsigned int))
{
	//Save our windows
	this->helpWindow = helpWindow;
	this->memoryWindow = memoryWindow;

	//Perform sub-init methods
	this->initImagesEtc();
	this->initHelp(OnHelpTitleBtnClick, OnHelpTitleBtnOver, OnHelpTitleBtnOut);
	this->initMemory(OnMemTitleBtnClick, OnMemTitleBtnOver, OnMemTitleBtnOut);
}


void OnscreenKeyboard::initImagesEtc()
{
	//Create a new device context
	bkgrdImg = new PulpCoreImage();
	helpWindow->initPulpCoreImage(bkgrdImg, this->width, this->height, 0x00000000);

	//Create rotated copies of our one corner image
	for (int i=1; i<4; i++) {
		this->cornerImg[i] = new PulpCoreImage();
		helpWindow->initPulpCoreImage(cornerImg[i], cornerImg[i-1]);
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
}



//NOTE: If we call this with "NULL" function pointers, it won't add their regions. This 
//      will allow us to call this function again after "minimizing" the window, so we 
//      can safely overwrite the window when minimized.
void OnscreenKeyboard::initHelp(void(*OnTitleBtnClick)(unsigned int), void(*OnTitleBtnOver)(unsigned int), void(*OnTitleBtnOut)(unsigned int))
{
	//Fill with "clear"
	bkgrdImg->fillRectangle(0, 0, this->width, this->height, 0x00000000);

	//Make our header/body buttons, to be drawn once, and draw them
	PulpCoreImage *headerButton = makeButton(title_btn_size*2 + title_btn_margin*3 + titleFont->getStringWidth(HELPWND_TITLE, NULL)+2*this->cornerSize, titleFont->getHeight(NULL)-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->width, this->height-headerButton->getHeight()+this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	helpWindow->drawImage(headerButton, 0, 0);

	//Add another header button for the encoding
	size_t boxMargin = 4;
	PulpCoreImage *headerButton2 = makeButton(boxMargin*2 + titleFont->getStringWidth(HELPWND_ENC_TITLE, NULL)+2*this->cornerSize, titleFont->getHeight(NULL)-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	size_t headerStart = helpWindow->getWidth()-headerButton2->getWidth();
	helpWindow->drawImage(headerButton2, headerStart, 0);

	//Draw the body
	helpWindow->drawImage(bodyButton, 0, headerButton->getHeight()-this->cornerSize);

	//Now we know where our keyboard begins
	keyboardOrigin.x = this->cornerSize;
	keyboardOrigin.y = headerButton->getHeight()-2;

	//Fix their messy intersection
	bkgrdImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	bkgrdImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	bkgrdImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);

	//Second messy intersection
	bkgrdImg->fillRectangle(headerStart+2, headerButton2->getHeight()-this->cornerSize, headerButton2->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	bkgrdImg->fillRectangle(headerStart+headerButton2->getWidth()-2, headerButton2->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	bkgrdImg->fillRectangle(headerStart, headerButton2->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);

	//Delete their un-necessary resources
	delete headerButton;
	delete headerButton2;
	delete bodyButton;

	//Draw a rectangle under the string
	bkgrdImg->fillRectangle(headerStart+this->cornerSize, this->cornerSize+3-boxMargin, titleFont->getStringWidth(HELPWND_ENC_TITLE, NULL)+2*boxMargin, titleFont->getHeight(NULL)-8+2*boxMargin, 0xFF777777);
	bkgrdImg->fillRectangle(1+headerStart+this->cornerSize, 1+this->cornerSize+3-boxMargin, titleFont->getStringWidth(HELPWND_ENC_TITLE, NULL)+2*boxMargin-2, titleFont->getHeight(NULL)-8+2*boxMargin-2, 0xFFA0A0A0);

	//Now draw the string
	this->titleFont->setColor(0x00, 0x00, 0x00);
	helpWindow->drawString(titleFont, HELPWND_TITLE, this->cornerSize + title_btn_size*2 + title_btn_margin*3, this->cornerSize);

	//Second string
	helpWindow->drawString(titleFont, HELPWND_ENC_TITLE, headerStart+this->cornerSize+boxMargin, this->cornerSize);
	this->titleFont->setColor(0xFF, 0xFF, 0xFF);

	//Layout and register our title bar buttons.
	closeBtnRect.left = closeBtnRect.top = this->cornerSize;
	closeBtnRect.right = closeBtnRect.bottom = closeBtnRect.left + title_btn_size;
	minmaxBtnRect.left = closeBtnRect.right + title_btn_margin;
	minmaxBtnRect.top = closeBtnRect.top;
	minmaxBtnRect.right = minmaxBtnRect.left + title_btn_size;
	minmaxBtnRect.bottom = minmaxBtnRect.top + title_btn_size;

	//Draw our title bar buttons
	drawTitleButtons();

	//Draw all our buttons (we'll just re-draw them when shifted, it saves space)
	for (size_t i=0; i<keys_total; i++) {
		drawKey(keys[i], i, false);
	}

	//Rebuild -- Don't auto-index.
	helpWindow->beginMassSubscription();

	//Subscribe a rectangle for all our keys
	if (OnTitleBtnClick!=NULL && OnTitleBtnOver!=NULL && OnTitleBtnOut!=NULL) {
		RECT r;
		for (size_t i=0; i<keys_total; i++) {
			//Scratch variables.
			POINT pos = keys[i].location;
			PulpCoreImage* img = buttonsRegular[keys[i].letterPalette];

			//Update our rectangle
			r.left = keyboardOrigin.x + pos.x;
			r.top = keyboardOrigin.y + pos.y;
			r.right = r.left + img->getWidth() - 1; 
			r.bottom = r.top + img->getHeight() - 1;

			//Subscribe
			helpWindow->subscribeRect(r, OnTitleBtnClick, OnTitleBtnOver, OnTitleBtnOut);
		}
	}

	//Register with callbacks for click/over/out
	if (OnTitleBtnClick!=NULL && OnTitleBtnOver!=NULL && OnTitleBtnOut!=NULL) {
		closeBtnID = helpWindow->subscribeRect(closeBtnRect, OnTitleBtnClick, OnTitleBtnOver, OnTitleBtnOut);
		minmaxBtnID = helpWindow->subscribeRect(minmaxBtnRect, OnTitleBtnClick, OnTitleBtnOver, OnTitleBtnOut);
	}

	//Rebuild the index
	helpWindow->endMassSubscription();
}


void OnscreenKeyboard::initHelpSmall()
{
	//First, clear it.
	bkgrdImg->fillRectangle(0, 0, this->width, this->smallHeight, 0x00000000);

	//Make our header/body buttons, to be drawn once, and draw them
	//NOTE: If we ever *do* decide to re-generate the DIB when resizing, we'll
	//      have to double-check the math here to make sure we don't overrun the array bounds somewhere.
	PulpCoreImage *headerButton = makeButton(title_btn_size*2 + title_btn_margin*3 + titleFont->getStringWidth(HELPWND_TITLE, NULL)+2*this->cornerSize, titleFont->getHeight(NULL)-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->width, 6, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	helpWindow->drawImage(headerButton, 0, 0);
	helpWindow->drawImage(bodyButton, 0, this->smallHeight-bodyButton->getHeight());

	//Fix their messy intersection; it's particularly bad this time.
	bkgrdImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	bkgrdImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	bkgrdImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);
	bkgrdImg->fillRectangle(this->cornerSize, this->smallHeight-2, this->width-2*this->cornerSize, 2, COLOR_KEYBOARD_BORDER);

	//Fix the lower-left corner
	int sY = this->smallHeight-this->cornerSize;
	bkgrdImg->fillRectangle(0, sY, this->cornerSize, this->cornerSize, 0x00000000);
	bkgrdImg->fillRectangle(1, sY, this->cornerSize-1, 3, COLOR_KEYBOARD_FOREGRD);
	bkgrdImg->fillRectangle(3, sY+3, this->cornerSize-3, 2, COLOR_KEYBOARD_FOREGRD);
	helpWindow->drawImage(cornerImg[3], 0, sY);

	//Fix the lower-right corner: stack cornerImg[1] on top of cornerImg[2]
	//We need to take 3 pixels off the top/bottom of the overlapping images. This will fix aliasing problems
	int sX = this->width-this->cornerSize-1;
	sY = this->smallHeight-bodyButton->getHeight();
	bkgrdImg->fillRectangle(sX, sY, this->cornerSize, bodyButton->getHeight(), 0x00000000);
	bkgrdImg->fillRectangle(sX, sY+2, 2, 2, COLOR_KEYBOARD_FOREGRD);
	helpWindow->drawImage(cornerImg[1], sX, sY, 0, 0, 0, 3);
	helpWindow->drawImage(cornerImg[2], sX, sY+this->cornerSize-3-2, 0, 0, 3, 0);

	//Delete their un-necessary resources
	delete headerButton;
	delete bodyButton;

	//Now draw the string
	this->titleFont->setColor(0x00, 0x00, 0x00);
	helpWindow->drawString(titleFont, HELPWND_TITLE, this->cornerSize + title_btn_size*2 + title_btn_margin*3, this->cornerSize);
	this->titleFont->setColor(0xFF, 0xFF, 0xFF);

	//Finally, re-draw the title buttons
	drawTitleButtons();
}



void OnscreenKeyboard::initMemorySmall()
{
	//First, clear it.
	memoryImg->fillRectangle(0, 0, this->memWidth, this->smallMemHeight, 0x00000000);

	//Make our header/body buttons, to be drawn once, and draw them
	//NOTE: If we ever *do* decide to re-generate the DIB when resizing, we'll
	//      have to double-check the math here to make sure we don't overrun the array bounds somewhere.
	PulpCoreImage *headerButton = makeButton(title_btn_size*2 + title_btn_margin*3 + titleFont->getStringWidth(MEMLIST_TITLE, NULL)+2*this->cornerSize, titleFont->getHeight(NULL)-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->memWidth, 6, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	memoryWindow->drawImage(headerButton, 0, 0);
	memoryWindow->drawImage(bodyButton, 0, this->smallMemHeight-bodyButton->getHeight());

	//Fix their messy intersection; it's particularly bad this time.
	memoryImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	memoryImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	memoryImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);
	memoryImg->fillRectangle(this->cornerSize, this->smallMemHeight-2, this->memWidth-2*this->cornerSize, 2, COLOR_KEYBOARD_BORDER);

	//Fix the lower-left corner
	int sY = this->smallMemHeight-this->cornerSize;
	memoryImg->fillRectangle(0, sY, this->cornerSize, this->cornerSize, 0x00000000);
	memoryImg->fillRectangle(1, sY, this->cornerSize-1, 3, COLOR_KEYBOARD_FOREGRD);
	memoryImg->fillRectangle(3, sY+3, this->cornerSize-3, 2, COLOR_KEYBOARD_FOREGRD);
	memoryWindow->drawImage(cornerImg[3], 0, sY);

	//Fix the lower-right corner: stack cornerImg[1] on top of cornerImg[2]
	//We need to take 3 pixels off the top/bottom of the overlapping images. This will fix aliasing problems
	int sX = this->memWidth-this->cornerSize-1;
	sY = this->smallMemHeight-bodyButton->getHeight();
	memoryImg->fillRectangle(sX, sY, this->cornerSize, bodyButton->getHeight(), 0x00000000);
	memoryImg->fillRectangle(sX, sY+2, 2, 2, COLOR_KEYBOARD_FOREGRD);
	memoryWindow->drawImage(cornerImg[1], sX, sY, 0, 0, 0, 3);
	memoryWindow->drawImage(cornerImg[2], sX, sY+this->cornerSize-3-2, 0, 0, 3, 0);

	//Delete their un-necessary resources
	delete headerButton;
	delete bodyButton;

	//Now draw the string
	this->titleFont->setColor(0x00, 0x00, 0x00);
	memoryWindow->drawString(titleFont, MEMLIST_TITLE, this->cornerSize + title_btn_size*2 + title_btn_margin*3, this->cornerSize);
	this->titleFont->setColor(0xFF, 0xFF, 0xFF);

	//Finally, re-draw the title buttons
	drawMemTitleButtons();
}



void OnscreenKeyboard::turnOnHelpMode(bool on, bool skipHelpWin, bool skipMemWin)
{
	//Avoid duplicate commands
	if (on == helpIsOn)
		return;

	//Set flag
	helpIsOn = on;

	//Un-shift lock?
	if (on)
		shiftLock = false;

	//Show the child windows
	if (!skipHelpWin) {
		//Re-paint all keys?
		if (on) {
			for (size_t i=0; i<keys_total; i++) {
				drawKey(keys[i], i, keys[i].isHighlighted());
			}
			helpWindow->repaintWindow();
		}
		helpWindow->showWindow(on);
	}
	if (!skipMemWin) {
		if (on) {
			refreshMemoryList();
			memoryWindow->repaintWindow();
		}

		memoryWindow->showWindow(on);
	}
}


bool OnscreenKeyboard::isHelpEnabled()
{
	return helpIsOn;
}



bool OnscreenKeyboard::isPressableButton(size_t btnID)
{
	unsigned int vkey = keyboard_vk_codes[btnID];
	return (vkey>='A'&&vkey<='Z')||(vkey>='0'&&vkey<='9')||(vkey==VK_OEM_4||vkey==VK_OEM_5||vkey==VK_OEM_1||vkey==VK_OEM_7||vkey==VK_OEM_COMMA||vkey==VK_OEM_3||vkey==VK_BACK||vkey==VK_LSHIFT||vkey==VK_RSHIFT);
}



bool OnscreenKeyboard::closeHelpWindow(unsigned int btnID)
{
	//Only respond to the correct button.
	if (btnID!=closeBtnID)
		return false;

	helpWindow->showWindow(false);
	return true;
}


void OnscreenKeyboard::minmaxHelpWindow(unsigned int btnID)
{
	//Only respond to the correct button
	if (btnID!=minmaxBtnID)
		return;

	//Reset button flags
	closeBtnHighlight = false;
	minmaxBtnHighlight = false;

	//Resize and re-draw the help window.
	helpWinMinimized = !helpWinMinimized;
	if (!helpWinMinimized) {
		helpWindow->resizeWindow(this->width, this->height, false);
		//helpWindow->createDoubleBufferedSurface();  //NOTE: We can keep the old surface; it's big enough.
		initHelp(NULL, NULL, NULL);
		helpWindow->repaintWindow();
		helpWindow->moveWindow(helpWindow->getXPos(), helpWindow->getYPos() - (this->height-this->smallHeight), true);
	} else {
		helpWindow->resizeWindow(this->width, this->smallHeight, false);
		//helpWindow->createDoubleBufferedSurface();  //NOTE: We can keep the old surface; it's big enough.
		initHelpSmall();
		helpWindow->repaintWindow();
		helpWindow->moveWindow(helpWindow->getXPos(), helpWindow->getYPos() + (this->height-this->smallHeight), true);
	}
}


bool OnscreenKeyboard::closeMemoryWindow(unsigned int btnID)
{
	//Only respond to the correct button.
	if (btnID!=closeMemBtnID)
		return false;

	memoryWindow->showWindow(false);
	return true;
}


void OnscreenKeyboard::minmaxMemoryWindow(unsigned int btnID)
{
	//Only respond to the correct button
	if (btnID!=minmaxMemBtnID)
		return;

	//Reset button flags
	closeMemBtnHighlight = false;
	minmaxMemBtnHighlight = false;

	//Resize and re-draw the help window.
	memWinMinimized = !memWinMinimized;
	if (!memWinMinimized) {
		memoryWindow->resizeWindow(this->memWidth, this->memHeight, false);
		//helpWindow->createDoubleBufferedSurface();  //NOTE: We can keep the old surface; it's big enough.
		initMemory(NULL, NULL, NULL);
		refreshMemoryList(); //Re-flash all memory entries
		memoryWindow->repaintWindow();
		memoryWindow->moveWindow(memoryWindow->getXPos(), memoryWindow->getYPos() - (this->memHeight-this->smallMemHeight), true);
	} else {
		memoryWindow->resizeWindow(this->memWidth, this->smallMemHeight, false);
		//helpWindow->createDoubleBufferedSurface();  //NOTE: We can keep the old surface; it's big enough.
		initMemorySmall();
		memoryWindow->repaintWindow();
		memoryWindow->moveWindow(memoryWindow->getXPos(), memoryWindow->getYPos() + (this->memHeight-this->smallMemHeight), true);
	}
}



void OnscreenKeyboard::highlightHelpTitleBtn(unsigned int btnID, bool isHighlighted)
{
	if (btnID == closeBtnID)
		closeBtnHighlight = isHighlighted;
	if (btnID == minmaxBtnID)
		minmaxBtnHighlight = isHighlighted;

	if ((btnID==closeBtnID || btnID==minmaxBtnID) && helpWindow->isVisible()) {
		drawTitleButtons();
		helpWindow->repaintWindow();
	}
}



VirtKey OnscreenKeyboard::getLastClickedVKey()
{
	return lastClickedButton;
}


void OnscreenKeyboard::clickButton(unsigned int btnID) 
{
	//Only respond to actual virtual keys
	if (btnID>=keys_total)
		return;

	//Set up
	lastClickedButton = VirtKey(keyboard_vk_codes[btnID], this->isShifted(), false, false);

	//Special case: handle shift (toggle)
	if (lastClickedButton.vkCode()==VK_LSHIFT || lastClickedButton.vkCode()==VK_RSHIFT) {
		shiftLock = !shiftLock;

		//Repaint
		for (size_t i=0; i<keys_total; i++) {
			drawKey(keys[i], i, keys[i].isHighlighted());
		}
		helpWindow->repaintWindow();
	} else if (shiftLock) {
		//Any clicked button will un-shift the keyboard
		shiftLock = false;

		//Repaint
		for (size_t i=0; i<keys_total; i++) {
			drawKey(keys[i], i, keys[i].isHighlighted());
		}
		helpWindow->repaintWindow();
	}

	//Set this button to "null" (roughly translated to "key '\0'") if we shouldn't type this key.
	//    It kind of breaks our encapsulation a bit, but this is how we do it.
	//       1) Store the result in a string in OnscreenKeyboard
	//       2) Have the main loop call "requestToType(helpKeyboard->getLastClickedVKey()" on the current input method
	//       3) Finally, handle all edge cases as they should be.
	if (!isPressableButton(btnID))
		lastClickedButton = VirtKey(0);
}


void OnscreenKeyboard::highlightVirtKey(unsigned int btnID, bool isHighlighted) 
{
	//Only respond to actual virtual keys
	if (btnID>=keys_total)
		return;

	//Don't highlight keys we can't press!
	//unsigned int vkey = keyboard_vk_codes[btnID];
	if (!isPressableButton(btnID))
		return;

	//Highlight/unhighlight
	bool wasHighlighted = keys[btnID].isHighlighted();
	keys[btnID].mouseIsOver = isHighlighted;
	if (wasHighlighted != keys[btnID].isHighlighted()) {
		drawKey(keys[btnID], btnID, keys[btnID].isHighlighted());
		helpWindow->repaintWindow();
	}
}


void OnscreenKeyboard::highlightMemoryTitleBtn(unsigned int btnID, bool isHighlighted)
{
	if (btnID == closeMemBtnID)
		closeMemBtnHighlight = isHighlighted;
	if (btnID == minmaxMemBtnID)
		minmaxMemBtnHighlight = isHighlighted;

	if ((btnID==closeMemBtnID || btnID==minmaxMemBtnID) && memoryWindow->isVisible()) {
		drawMemTitleButtons();
		memoryWindow->repaintWindow();
	}
}



void OnscreenKeyboard::drawTitleButtons()
{
	//We need to detect the state of each button; should it be highlighted, and are we minimized or maximized.

	//Fill the two buttons
	bkgrdImg->fillRectangle(closeBtnRect.left, closeBtnRect.top, closeBtnRect.right-closeBtnRect.left, closeBtnRect.bottom-closeBtnRect.top, 0xFF000000);
	bkgrdImg->fillRectangle(closeBtnRect.left+2, closeBtnRect.top+2, closeBtnRect.right-closeBtnRect.left-4, closeBtnRect.bottom-closeBtnRect.top-4, closeBtnHighlight?COLOR_CLOSEBTN_HIGHLIGHT:COLOR_KEY_BKGRD);
	bkgrdImg->fillRectangle(minmaxBtnRect.left, minmaxBtnRect.top, minmaxBtnRect.right-minmaxBtnRect.left, minmaxBtnRect.bottom-minmaxBtnRect.top, 0xFF000000);
	bkgrdImg->fillRectangle(minmaxBtnRect.left+2, minmaxBtnRect.top+2, minmaxBtnRect.right-minmaxBtnRect.left-4, minmaxBtnRect.bottom-minmaxBtnRect.top-4, minmaxBtnHighlight?COLOR_MINMAXBTN_HIGHLIGHT:COLOR_KEY_BKGRD);

	//Draw the close button
	helpWindow->drawImage(closeImg, closeBtnRect.left+2, closeBtnRect.top+2);

	//Draw the min/max button
	if (helpWinMinimized) {
		bkgrdImg->fillRectangle(minmaxBtnRect.left+4, minmaxBtnRect.top+5, closeBtnRect.right-closeBtnRect.left-8, closeBtnRect.bottom-closeBtnRect.top-10, 0xFF000000);
		bkgrdImg->fillRectangle(minmaxBtnRect.left+5, minmaxBtnRect.top+5+2, closeBtnRect.right-closeBtnRect.left-10, closeBtnRect.bottom-closeBtnRect.top-10-3, minmaxBtnHighlight?0xFFFFFFFF:COLOR_KEY_BKGRD);
	} else {
		bkgrdImg->fillRectangle(minmaxBtnRect.left+4, minmaxBtnRect.bottom-6, closeBtnRect.right-closeBtnRect.left-8, 2, 0xFF000000);
	}
}



//Copied from the "help" functions
void OnscreenKeyboard::drawMemTitleButtons()
{
	//Fill the two buttons
	memoryImg->fillRectangle(closeMemBtnRect.left, closeMemBtnRect.top, closeMemBtnRect.right-closeMemBtnRect.left, closeMemBtnRect.bottom-closeMemBtnRect.top, 0xFF000000);
	memoryImg->fillRectangle(closeMemBtnRect.left+2, closeMemBtnRect.top+2, closeMemBtnRect.right-closeMemBtnRect.left-4, closeMemBtnRect.bottom-closeMemBtnRect.top-4, closeMemBtnHighlight?COLOR_CLOSEBTN_HIGHLIGHT:COLOR_KEY_BKGRD);
	memoryImg->fillRectangle(minmaxMemBtnRect.left, minmaxMemBtnRect.top, minmaxMemBtnRect.right-minmaxMemBtnRect.left, minmaxMemBtnRect.bottom-minmaxMemBtnRect.top, 0xFF000000);
	memoryImg->fillRectangle(minmaxMemBtnRect.left+2, minmaxMemBtnRect.top+2, minmaxMemBtnRect.right-minmaxMemBtnRect.left-4, minmaxMemBtnRect.bottom-minmaxMemBtnRect.top-4, minmaxMemBtnHighlight?COLOR_MINMAXBTN_HIGHLIGHT:COLOR_KEY_BKGRD);

	//Draw the close button
	memoryWindow->drawImage(closeImg, closeMemBtnRect.left+2, closeMemBtnRect.top+2);

	//Draw the min/max button
	//NOTE: Shouldn't we be using "minmaxMemBtnRect" instead of "closeMemBtnRect" for width/height....
	//      Perhaps we'll refactor this into a "button" class anyway later
	if (memWinMinimized) {
		memoryImg->fillRectangle(minmaxMemBtnRect.left+4, minmaxMemBtnRect.top+5, closeMemBtnRect.right-closeMemBtnRect.left-8, closeMemBtnRect.bottom-closeMemBtnRect.top-10, 0xFF000000);
		memoryImg->fillRectangle(minmaxMemBtnRect.left+5, minmaxMemBtnRect.top+5+2, closeMemBtnRect.right-closeMemBtnRect.left-10, closeMemBtnRect.bottom-closeMemBtnRect.top-10-3, minmaxMemBtnHighlight?0xFFFFFFFF:COLOR_KEY_BKGRD);
	} else {
		memoryImg->fillRectangle(minmaxMemBtnRect.left+4, minmaxMemBtnRect.bottom-6, closeMemBtnRect.right-closeMemBtnRect.left-8, 2, 0xFF000000);
	}
}



void OnscreenKeyboard::initMemory(void(*OnTitleBtnClick)(unsigned int), void(*OnTitleBtnOver)(unsigned int), void(*OnTitleBtnOut)(unsigned int))
{
	//Create a new device context
	memoryImg = new PulpCoreImage();
	memoryWindow->initPulpCoreImage(memoryImg, this->memWidth, this->memHeight, 0x00000000);

	//Make our header/body buttons, to be drawn once, and draw them
	PulpCoreImage *headerButton = makeButton(title_btn_size*2 + title_btn_margin*3 + titleFont->getStringWidth(MEMLIST_TITLE, NULL)+2*this->cornerSize, titleFont->getHeight(NULL)-2+2*this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	PulpCoreImage *bodyButton = makeButton(this->memWidth, this->memHeight-headerButton->getHeight()+this->cornerSize, COLOR_KEYBOARD_BKGRD, COLOR_KEYBOARD_FOREGRD, COLOR_KEYBOARD_BORDER);
	memoryWindow->drawImage(headerButton, 0, 0);
	memoryWindow->drawImage(bodyButton, 0, headerButton->getHeight()-this->cornerSize);

	//Fix their messy intersection
	memoryImg->fillRectangle(2, headerButton->getHeight()-this->cornerSize, headerButton->getWidth()-4, this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	memoryImg->fillRectangle(0, headerButton->getHeight()-this->cornerSize, 2, this->cornerSize, COLOR_KEYBOARD_BORDER);
	memoryImg->fillRectangle(headerButton->getWidth()-2, headerButton->getHeight()-this->cornerSize, 2, 2, COLOR_KEYBOARD_BORDER);

	//Delete their un-necessary resources
	delete headerButton;
	delete bodyButton;

	//Now draw the string
	this->titleFont->setColor(0x00, 0x00, 0x00);
	memoryWindow->drawString(titleFont, MEMLIST_TITLE, this->cornerSize + title_btn_size*2 + title_btn_margin*3, this->cornerSize);
	this->titleFont->setColor(0xFF, 0xFF, 0xFF);

	//Layout and register our title bar buttons.
	closeMemBtnRect.left = closeMemBtnRect.top = this->cornerSize;
	closeMemBtnRect.right = closeMemBtnRect.bottom = closeMemBtnRect.left + title_btn_size;
	minmaxMemBtnRect.left = closeMemBtnRect.right + title_btn_margin;
	minmaxMemBtnRect.top = closeMemBtnRect.top;
	minmaxMemBtnRect.right = minmaxMemBtnRect.left + title_btn_size;
	minmaxMemBtnRect.bottom = minmaxMemBtnRect.top + title_btn_size;

	//Register with callbacks for click/over/out
	if (OnTitleBtnClick!=NULL && OnTitleBtnOver!=NULL && OnTitleBtnOut!=NULL) {
		closeMemBtnID = memoryWindow->subscribeRect(closeMemBtnRect, OnTitleBtnClick, OnTitleBtnOver, OnTitleBtnOut);
		minmaxMemBtnID = memoryWindow->subscribeRect(minmaxMemBtnRect, OnTitleBtnClick, OnTitleBtnOver, OnTitleBtnOut);
	}

	//Draw our title bar buttons
	drawMemTitleButtons();

	//Draw the title string
	//this->titleFont->setColor(0x00, 0x00, 0x00);
	//memoryWindow->drawString(titleFont, MEMLIST_TITLE, this->cornerSize, this->cornerSize);

	//Draw some heading strings
	this->titleFont->setColor(0x00, 0x00, 0x00);
	memoryWindow->drawString(titleFont, L"Myanmar", this->cornerSize, keyboardOrigin.y+1);
	memoryWindow->drawString(titleFont, L"Roman", this->getMemoryWidth()/2+5/2, keyboardOrigin.y+1);
	this->titleFont->setColor(0xFF, 0xFF, 0xFF);

	//Underline
	int yPos = keyboardOrigin.y+1+this->titleFont->getHeight(NULL) - 3;
	memoryImg->fillRectangle(this->cornerSize, yPos, this->titleFont->getStringWidth(L"Myanmar", NULL), 1, 0xFF333333);
	memoryImg->fillRectangle(this->cornerSize, 1+yPos, this->titleFont->getStringWidth(L"Myanmar", NULL), 1, 0xFF666666);
	memoryImg->fillRectangle(this->getMemoryWidth()/2+5/2, yPos, this->titleFont->getStringWidth(L"Roman", NULL), 1, 0xFF333333);
	memoryImg->fillRectangle(this->getMemoryWidth()/2+5/2, 1+yPos, this->titleFont->getStringWidth(L"Roman", NULL), 1, 0xFF666666);

	this->memEntriesYPlus = this->memoryFont->getHeight(NULL)-1;
	this->memEntriesStartY = yPos + memEntriesYPlus*4/9 + 2;	
	this->memEntriesMax = (this->getMemoryHeight()-this->cornerSize-memEntriesStartY)/memEntriesYPlus;	
}




/**
 * Is the keyboard in a shifted state? Affects what text is drawn.
 */
bool OnscreenKeyboard::isShifted()
{
	//NOTE: We can't consider isHighlighted(); we can ONLY count this as shifted if the key is PRESSED
	return shiftLock || keys[getKeyID(VK_LSHIFT, '\0', true)].kbdIsPressed || keys[getKeyID(VK_RSHIFT, '\0', true)].kbdIsPressed;
}



//Helper function
void OnscreenKeyboard::drawKey(const KbdKey& currKey, int keyID, bool isPressed)
{
	//Draw the background
	int pal = currKey.letterPalette;
	PulpCoreImage *keyImg = buttonsRegular[pal];
	if (isPressed || (shiftLock && (currKey.letterPalette==BUTTON_SHIFT_L || currKey.letterPalette==BUTTON_SHIFT_R)) )
		keyImg = buttonsShifted[pal];
	helpWindow->drawImage(keyImg, keyboardOrigin.x+currKey.location.x, keyboardOrigin.y+currKey.location.y);

	//Draw the letter labels
	int xPos = keyboardOrigin.x+currKey.location.x+4;
	int yPos = keyboardOrigin.y+currKey.location.y+3;
	if (letter_types[keyID]==BUTTON_KEY) //Center it
		xPos += (5 - keysFont->getCharWidth(keyID)/2);
	helpWindow->drawChar(keysFont, keyID, xPos+offsets_key[keyID], yPos);

	//Prepare to draw keys
	int myKeyID = keyID;
	int myShiftKeyID = keyID + 61;
	//PulpCoreFont *myFont = this->foreFont;
	//PulpCoreFont *myShiftFont = this->shiftFontBlue;
	if (!this->isShifted()) {
		//Regular colors
		this->foreFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));
		this->shiftFont->setColor(((COLOR_LETTERS_SHIFTED&0xFF0000)>>16), ((COLOR_LETTERS_SHIFTED&0xFF00)>>8), (COLOR_LETTERS_SHIFTED&0xFF));
	} else {
		//Inverted colors
		this->foreFont->setColor(((COLOR_LETTERS_SHIFTED&0xFF0000)>>16), ((COLOR_LETTERS_SHIFTED&0xFF00)>>8), (COLOR_LETTERS_SHIFTED&0xFF));
		this->shiftFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));

		//Also adjust the key ID
		myShiftKeyID = myKeyID;
		myKeyID += 61;
	}

	//Draw the main label
	if (!hide_for_help[myKeyID]) {
		xPos = keyboardOrigin.x+currKey.location.x+keyImg->getWidth()/2-foreFont->getCharWidth(myKeyID)/2;
		yPos = keyboardOrigin.y+currKey.location.y+19;
		helpWindow->drawChar(foreFont, myKeyID, xPos+offset_fore[myKeyID], yPos);
	}

	//Draw the shifted label
	if (!hide_for_help[myShiftKeyID]) {
		xPos = keyboardOrigin.x+currKey.location.x+23-shiftFont->getCharWidth(myShiftKeyID)/2;
		yPos = keyboardOrigin.y+currKey.location.y+3;
		helpWindow->drawChar(shiftFont, myShiftKeyID, xPos+offset_super[myShiftKeyID], yPos);
	}
}



bool OnscreenKeyboard::isShiftLocked()
{
	return shiftLock;
}



/**
 * Returns the vkcode if this is an actual key-code that we track
 *  hotkeyCode must be either a letter, or a hotkey code (hotkeys.h)
 * Returns -1 if we don't track this
 */
bool OnscreenKeyboard::highlightKey(unsigned int vkCode, char alphanum, bool modShift, bool highlightON)
{
	//Get the key code, unshifted
	int id = getKeyID(vkCode, alphanum, modShift);
	if (id>=61)
		id -= 61;
	if (id==-1)
		return false;
	
	//Mark as "highlighted"
	bool wasShifted = this->isShifted();
	bool wasHighlighted = keys[id].isHighlighted();
	keys[id].kbdIsPressed = highlightON;

	//If there's no change, don't bother redrawing.
	if (wasHighlighted == keys[id].isHighlighted() && !shiftLock && keys[id].letterPalette!=BUTTON_SHIFT_L && keys[id].letterPalette!=BUTTON_SHIFT_R)
		return true;

	//Update the status of our shift lock (all keys just turn it off)
	if (shiftLock)
		shiftLock = false;

	//Also don't draw if hidden or minimized; track anyway.
	if (!helpWindow->isVisible() || helpWinMinimized)
		return true;

	//Draw
	if (wasShifted != this->isShifted()) {
		//We need to re-print the entire keyboard
		for (int i=0; i<61; i++) {
			drawKey(keys[i], i, keys[i].isHighlighted());
		}
	} else {
		//Re-draw only this key
		drawKey(keys[id], id, keys[id].isHighlighted());
	}

	//Succeeded
	return true;
}

int OnscreenKeyboard::getVirtualKeyID(unsigned int vkCode, char alphanum, bool modShift) const
{
	int id = getKeyID(vkCode, alphanum, modShift);
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
int OnscreenKeyboard::getKeyID(unsigned int vkCode, char alphanum, bool modShift) const
{
	if (alphanum=='\0') {
		//Special keys
		switch (vkCode) 
		{
			case VK_RETURN:
				return modShift ? 101 : 40;
			case VK_LSHIFT:
				return 41;
			case VK_RSHIFT:
				return 52;
			case ' ': //Special case
				return modShift ? 117 : 56;
			default:
				return -1;
		}
	} else {
		switch (alphanum) 
		{
			case '`':
				return 0;
			case '1':
				return 1;
			case '2':
				return 2;
			case '3':
				return 3;
			case '4':
				return 4;
			case '5':
				return 5;
			case '6':
				return 6;
			case '7':
				return 7;
			case '8':
				return 8;
			case '9':
				return 9;
			case '0':
				return 10;
			case '-':
				return 11;
			case '=':
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
			case '[':
				return 25;
			case ']':
				return 26;
			case '\\':
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
			case ';':
				return 38;
			case '\'':
				return 39;
//			case HOTKEY_ENTER:
//				return 40;
//			case HOTKEY_VIRT_LSHIFT:
//				return 41;
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
			case ',':
				return 49;
			case '.':
				return 50;
			case '/':
				return 51;
//			case HOTKEY_VIRT_RSHIFT:
//				return 52;
			//case [HOTKEY_LEFT_CONTROL]:
			//	return 53;
			//case [HOTKEY_LEFT_WIN]:
			//	return 54;
			//case [HOTKEY_LEFT_ALT]:
			//	return 55;
//			case ' ':
//				return 56;
			//case [HOTKEY_RIGHT_ALT]:
			//	return 57;
			//case [HOTKEY_RIGHT_WIN]:
			//	return 58;
			//case [HOTKEY_CMENU]:
			//	return 59;
			//case [HOTKEY_RIGHT_CTRL]:
			//	return 60;
			case '~':
				return 61;
			case '!':
				return 62;
			case '@':
				return 63;
			case '#':
				return 64;
			case '$':
				return 65;
			case '%':
				return 66;
			case '^':
				return 67;
			case '&':
				return 68;
			case '*':
				return 69;
			case '(':
				return 70;
			case ')':
				return 71;
			case '_':
				return 72;
			case '+':
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
			case '{':
				return 86;
			case '}':
				return 87;
			case '|':
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
			case ':':
				return 99;
			case '"':
				return 100;
//			case HOTKEY_SHIFT_ENTER:
//				return 101;
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
			case '<':
				return 110;
			case '>':
				return 111;
			case '?':
				return 112;
			//case [HOTKEY_SUP_RSHIFT]:
			//	return 113;
			//case [HOTKEY_SUP_LEFT_CONTROL]:
			//	return 114;
			//case [HOTKEY_SUP_LEFT_WIN]:
			//	return 115;
			//case [HOTKEY_SUP_LEFT_ALT]:
			//	return 116;
//			case HOTKEY_SHIFT_SPACE:
//				return 117;
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
}




wstring OnscreenKeyboard::typeLetter(unsigned int vkCode, char alphanum, bool modShift)
{
	int id = getKeyID(vkCode, alphanum, modShift);
	if (id==-1 || hide_for_help[id])
		return L"";

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

	wstring ret;
	ret += typeableLetters[id];
	return ret;
}




//Make a "button", with a given width and height (including border), background color (can be transparent) and
// foreground color. Border color is changed by calling "tint self" on cornerImg before this.
PulpCoreImage* OnscreenKeyboard::makeButton(int width, int height, int bgARGB, int fgARGB, int borderARGB)
{
	//Make an empty image
	PulpCoreImage *result = new PulpCoreImage();
	//NOTE: Not sure if this will explode.
	HDC resDC;
	HBITMAP resBMP;
	result->init(width, height, bgARGB, helpWindow->WARNINGgetUnderDC(), resDC, resBMP);

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


void OnscreenKeyboard::addMemoryEntry(const std::wstring &my, const std::string &rom)
{
	//Does the list contain this word?
	for (list< pair<wstring,string> >::iterator keyItr = memoryList.begin(); keyItr != memoryList.end();keyItr++) {
		if (keyItr->first==my && keyItr->second==rom)
			return;
	}

	//Add the word to the end
	memoryList.push_back(pair<wstring,string>(my, rom));

	//Too many words?
	if (memoryList.size()>getMaxMemoryEntries())
		memoryList.pop_front();

	//Hidden/minimized?
	if (!memoryWindow->isVisible() || memWinMinimized)
		return;

	//Either way, we need to re-draw the word list
	refreshMemoryList();
}


void OnscreenKeyboard::clearAllMemoryEntries()
{
	if (memoryList.empty())
		return;

	memoryList.clear();

	//Hidden/minimized?
	if (!memoryWindow->isVisible() || memWinMinimized)
		return;

	refreshMemoryList();
}


void OnscreenKeyboard::refreshMemoryList()
{
	memoryImg->fillRectangle(this->cornerSize, memEntriesStartY, this->getMemoryWidth()-this->cornerSize*2, this->getMemoryHeight()-memEntriesStartY-this->cornerSize, COLOR_KEYBOARD_FOREGRD);
	int currY = memEntriesStartY;
	for (list< pair<wstring,string> >::iterator keyItr = memoryList.begin(); keyItr != memoryList.end();keyItr++) {
		this->memoryFont->setColor(((COLOR_LETTERS_REGULAR&0xFF0000)>>16), ((COLOR_LETTERS_REGULAR&0xFF00)>>8), (COLOR_LETTERS_REGULAR&0xFF));
		memoryWindow->drawString(memoryFont, keyItr->first, this->cornerSize, currY);
		memoryWindow->drawString(memoryFont, keyItr->second, this->getMemoryWidth()/2+5/2, currY);
		currY += memEntriesYPlus;
	}
}


size_t OnscreenKeyboard::getMaxMemoryEntries() const
{
	return this->memEntriesMax;
}

int OnscreenKeyboard::getWidth() const
{
	return this->width;
}

int OnscreenKeyboard::getHeight() const
{
	return this->height;
}

int OnscreenKeyboard::getMemoryWidth() const
{
	return this->memWidth;
}

int OnscreenKeyboard::getMemoryHeight() const
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

