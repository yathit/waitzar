/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "InputMethod.h"


void InputMethod::init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords, const std::wstring& encoding, CONTROL_KEY_TYPE controlKeyStyle, bool typeBurmeseNumbers, bool typeNumeralConglomerates, bool suppressUppercase)
{
	//Init
	providingHelpFor = NULL;
	viewChanged = false;
	requestToTypeSentence = false;
	//myenc2Uni = NULL;
	//uni2Romanenc = NULL;

	//Save
	this->mainWindow = mainWindow;
	this->sentenceWindow = sentenceWindow;
	this->helpWindow = helpWindow;
	this->memoryWindow = memoryWindow;
	this->systemWordLookup = systemWordLookup;
	this->systemDefinedWords = systemDefinedWords;
	this->helpKeyboard = helpKeyboard;
	this->encoding = encoding;
	this->suppressUppercase = suppressUppercase;

	//Index ZWS; -1 means "not found"
	this->zwsID = -1;
	this->zwsAlpha = L'\u200B';
	for (size_t i=0; i<systemWordLookup.size(); i++) {
		if (systemWordLookup[i].first==zwsAlpha) {
			//This represents a negative offset
			this->zwsID = -1-i;
			break;
		}
	}
}


void InputMethod::treatAsHelpKeyboard(InputMethod* providingHelpFor, std::function<void (const std::wstring& fromEnc, const std::wstring& toEnc, std::wstring& src)> ConfigGetAndTransformSrc)
{
	this->providingHelpFor = providingHelpFor;

	this->ConfigGetAndTransformText = ConfigGetAndTransformSrc;
	/*if (ConfigGetTransformation) {
		//Get a set of encoding transformations for the reverse lookup
		this->myenc2Uni = ConfigGetTransformation(this->encoding, L"unicode");
		this->uni2Romanenc = ConfigGetTransformation(L"unicode", providingHelpFor->encoding);

		//Get a set of encoding transformations for returning the Roman Input Method's encoding
		this->uni2Myenc = ConfigGetTransformation(L"unicode", this->encoding);
		this->romanenc2Uni = ConfigGetTransformation(providingHelpFor->encoding, L"unicode");
	}*/
}

bool InputMethod::isHelpInput()
{
	return this->providingHelpFor!=NULL;
}

void InputMethod::forceViewChanged()
{
	viewChanged = true;
}

bool InputMethod::getAndClearViewChanged()
{
	bool res = viewChanged;
	viewChanged = false;
	return res;
}

bool InputMethod::getAndClearRequestToTypeSentence()
{
	bool res = requestToTypeSentence;
	requestToTypeSentence = false;
	return res;
}

std::pair <std::string, std::wstring> InputMethod::getAndClearMostRecentRomanizationCheck()
{
	std::pair <std::string, std::wstring> res = mostRecentRomanizationCheck;
	mostRecentRomanizationCheck.first = "";
	mostRecentRomanizationCheck.second = L"";
	return res;
}



//Handle things our MainFile used to handle
//Return true if "processed"
bool InputMethod::handleVKey(VirtKey& vkey)
{
	//First, handle things that we know are locale-dependent.
	switch (vkey.vkCodeByLocale) {
		case VK_ESCAPE:
			//Close the window, exit help mode
			this->handleEsc();
			return true;

		case VK_BACK:
			//Back up
			this->handleBackspace(vkey);
			return true;

		case VK_DELETE:
			//Delete a phrase
			this->handleDelete();
			return true;

		case VK_RIGHT:
			//Advance the cursor, pick a word
			this->handleLeftRight(true, false);
			return true;

		case VK_LEFT:
			//Move the cursor back, pick a word
			this->handleLeftRight(false, false);
			return true;

		case VK_DOWN:
		case VK_NEXT: //Pagedown
			//Page
			this->handleUpDown(true);
			return true;

		case VK_UP:
		case VK_PRIOR: //Pageup
			//Page
			this->handleUpDown(false);
			return true;

		case VK_RETURN:// case HOTKEY_SHIFT_ENTER:
			//Handle word selection
			this->handleCommit(true);
			return true;

		case VK_TAB: //case HOTKEY_SHIFT_TAB:
			//Tab generally behaves as a right-arow.
			this->handleTab();
			return true;

		case VK_SPACE: //case HOTKEY_SHIFT_SPACE:
			//Handle word selection, cursor advancing
			if (vkey.modShift && !this->isHelpInput()) //ZWS override
				this->typeZWS();
			else
				this->handleCommit(false);
			return true;
		default:
			//Tricky here: we need to put the "system key" nonsense into the "handleKeyPress"  function
			// otherwise numbers won't work.
			this->handleKeyPress(vkey);
			return true;
	}
	return false;
}



//Handle system keys
void InputMethod::handleKeyPress(VirtKey& vkey)
{
	//Convert locale
	//TODO: Centralize this elsewhere
	vkey.considerByScancode();

	//Get an adjusted numcode.
	//int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9) ? HOTKEY_NUM0 : -1;
	//int numberValue = vkey.alphanum - '0'; //(base==-1) ? wParam : HOTKEY_0 + (int)wParam - base;

	//Check system keys, but ONLY if the sentence window is the only thing visible.
	if (!mainWindow->isVisible()) { //helpWindow implies mainWindow is visible.
		//wchar_t letter = '\0'; //Any defaults are fine.
		int numberValue = 0; //Any defaults are fine.
		for (size_t i=0; i<systemWordLookup.size(); i++) {
			if (systemWordLookup[i].first==vkey.alphanum()) {
				//This represents a negative offset
				numberValue = -1-i;
				//letter = systemWordLookup[i].second;
				break;
			}

			//Didn't find it?
			if (i==systemWordLookup.size()-1)
				return;
		}

		//Try to type this word; we now have its numCode and letter.
		this->appendToSentence(vkey.alphanum(), numberValue);
	}
}


void InputMethod::typeZWS()
{
	//Type ZWS, but ONLY if the sentence window is the only thing visible.
	if (!mainWindow->isVisible() && zwsID!=-1) {
		this->appendToSentence(zwsAlpha, zwsID);
		viewChanged = true;
	}
}


std::wstring InputMethod::getTypedRomanString(bool asLowercase)
{
	std::wstring ret = typedRomanStr.str();
	if (asLowercase || suppressUppercase)
		waitzar::loc_to_lower(ret);
	return ret;
}

std::pair<int, int> InputMethod::getPagingInfo() const
{
	return std::pair<int, int>(0, 0);
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
