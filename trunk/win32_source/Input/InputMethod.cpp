/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "InputMethod.h"

InputMethod::InputMethod()
{
}

InputMethod::~InputMethod()
{
}

void InputMethod::init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords)
{
	//Init
	providingHelpFor = NULL;
	viewChanged = false;
	requestToTypeSentence = false;

	//Save
	this->mainWindow = mainWindow;
	this->sentenceWindow = sentenceWindow;
	this->helpWindow = helpWindow;
	this->memoryWindow = memoryWindow;
	this->systemWordLookup = systemWordLookup;
	this->systemDefinedWords = systemDefinedWords;
	this->helpKeyboard = helpKeyboard;
}


void InputMethod::treatAsHelpKeyboard(InputMethod* providingHelpFor)
{
	this->providingHelpFor = providingHelpFor;
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



//Handle system keys
void InputMethod::handleKeyPress(WPARAM wParam, LPARAM lParam, bool isUpper)
{
	//Get an adjusted numcode.
	int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9) ? HOTKEY_NUM0 : -1;
	int numCode = (base==-1) ? wParam : HOTKEY_0 + (int)wParam - base;

	//Check system keys, but ONLY if the sentence window is the only thing visible.
	if (!mainWindow->isVisible()) { //helpWindow implies mainWindow is visible.
		wchar_t letter = '\0';
		for (size_t i=0; i<systemWordLookup.size(); i++) {
			if (systemWordLookup[i].first==numCode) {
				//This represents a negative offset
				numCode = -1-i;
				letter = systemWordLookup[i].second;
				break;
			}

			//Didn't find it?
			if (i==systemWordLookup.size()-1)
				return;
		}

		//Try to type this word; we now have its numCode and letter.
		this->appendToSentence(letter, numCode);
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
