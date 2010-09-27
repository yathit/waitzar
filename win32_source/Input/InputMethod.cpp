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
	myenc2Uni = NULL;
	uni2Romanenc = NULL;

	//Save
	this->mainWindow = mainWindow;
	this->sentenceWindow = sentenceWindow;
	this->helpWindow = helpWindow;
	this->memoryWindow = memoryWindow;
	this->systemWordLookup = systemWordLookup;
	this->systemDefinedWords = systemDefinedWords;
	this->helpKeyboard = helpKeyboard;

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


void InputMethod::treatAsHelpKeyboard(InputMethod* providingHelpFor, const Encoding& uniEnc, const Transformation* (*ConfigGetTransformation)(const Encoding& fromEnc, const Encoding& toEnc))
{
	this->providingHelpFor = providingHelpFor;

	if (ConfigGetTransformation!=NULL) {
		//Get a set of encoding transformations for the reverse lookup
		this->myenc2Uni = ConfigGetTransformation(this->encoding, uniEnc);
		this->uni2Romanenc = ConfigGetTransformation(uniEnc, providingHelpFor->encoding);

		//Get a set of encoding transformations for returning the Roman Input Method's encoding
		this->uni2Myenc = ConfigGetTransformation(uniEnc, this->encoding);
		this->romanenc2Uni = ConfigGetTransformation(providingHelpFor->encoding, uniEnc);
	}
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
void InputMethod::handleKeyPress(VirtKey& vkey)
{
	//Convert locale
	//TODO: Centralize this elsewhere
	vkey.stripLocale();

	//Get an adjusted numcode.
	//int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9) ? HOTKEY_NUM0 : -1;
	//int numberValue = vkey.alphanum - '0'; //(base==-1) ? wParam : HOTKEY_0 + (int)wParam - base;

	//Check system keys, but ONLY if the sentence window is the only thing visible.
	if (!mainWindow->isVisible()) { //helpWindow implies mainWindow is visible.
		//wchar_t letter = '\0'; //Any defaults are fine.
		int numberValue = 0; //Any defaults are fine.
		for (size_t i=0; i<systemWordLookup.size(); i++) {
			if (systemWordLookup[i].first==vkey.alphanum) {
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
		this->appendToSentence(vkey.alphanum, numberValue);
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
