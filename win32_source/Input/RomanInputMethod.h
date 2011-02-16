/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _ROMAN_INPUT_METHOD
#define _ROMAN_INPUT_METHOD

#include <stdexcept>

#include "MyWin32Window.h"
#include "Input/InputMethod.h"
#include "NGram/SentenceList.h"
#include "OnscreenKeyboard.h"


class RomanInputMethod : public InputMethod {

public:

	//Needed to add Roman-specific stuff
	virtual void init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords, LookupEngine* model, waitzar::SentenceList* sentence, const std::wstring& encoding, CONTROL_KEY_TYPE controlKeyStyle, bool typeBurmeseNumbers, bool typeNumeralConglomerates, bool suppressUppercase);

	//Override dispatch
	bool handleVKey(VirtKey& vkey);

	//Abstract implementation - keypresses
	void handleEsc();
	void handleBackspace(VirtKey& vkey);
	void handleDelete();
	void handleLeftRight(bool isRight, bool loopToZero);
	void handleUpDown(bool isDown);
	void handleCommit(bool strongCommit);
	void handleNumber(VirtKey& vkey);
	void handleStop(bool isFull, VirtKey& vkey);
	void handleKeyPress(VirtKey& vkey);
	void handleTab();


	//Abstract implementation - sentence and word
	std::vector< std::wstring > getTypedSentenceStrings();
	std::vector< std::pair<std::wstring, unsigned int> > getTypedCandidateStrings();
	void appendToSentence(wchar_t letter, int id);


	//Abstract implementation - simple
	bool isPlaceholder() { return false; }

	void reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset);

	//Override
	void treatAsHelpKeyboard(InputMethod* providingHelpFor, const Transformation* (*ConfigGetTransformation)(const std::wstring& fromEnc, const std::wstring& toEnc));

	void typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID);

	std::pair<int, std::string> lookupWord(std::wstring typedWord);

	//Real override
	std::wstring getTypedRomanString(bool asLowercase);

	std::pair<int, int> getPagingInfo() const;


private:
	//Romanization model
	//TODO: Chain constructors, set to null (C++ 0x)
	LookupEngine* model;
	waitzar::SentenceList* sentence;

	bool selectCurrWord();
	bool selectWord(int id);

	//Properties
	CONTROL_KEY_TYPE controlKeyStyle;
	bool typeBurmeseNumbers;
	bool typeNumeralConglomerates;
	bool suppressUppercase;

	//Saved
	bool typedStrContainsNoAlpha;
};






#endif //_ROMAN_INPUT_METHOD

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

