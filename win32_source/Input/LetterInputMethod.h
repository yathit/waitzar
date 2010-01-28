/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _LETTER_INPUT_METHOD
#define _LETTER_INPUT_METHOD

#include "MyWin32Window.h"
#include "Input/InputMethod.h"
#include "OnscreenKeyboard.h"

class LetterInputMethod : public InputMethod {

public:
	LetterInputMethod(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard);
	
	//Destructor
	~LetterInputMethod();

	//Abstract implementation - keypresses
	void handleEsc();
	void handleBackspace();
	void handleDelete();
	void handleLeftRight(bool isRight);
	void handleCommit(bool strongCommit);
	void handleNumber(int numCode, WPARAM wParam);
	void handleStop(bool isFull);
	void handleKeyPress(WPARAM wParam);

	//Abstract implementation - sentence and word
	std::wstring getTypedSentenceString();
	std::wstring getSentencePreCursorString();
	std::vector< std::pair<std::wstring, unsigned int> > getTypedCandidateStrings();
	void appendToSentence(wchar_t letter, int id);

	//Abstract implementation - simple
	bool isPlaceholder() { return false; }

	void reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset);

	void typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID);

	std::pair<int, std::string> lookupWord(std::wstring typedWord);

private:
	std::wstringstream typedSentenceStr;
	std::wstringstream typedCandidateStr;

	//myWin 2.1 rules for stacking
	bool canStack(wchar_t letter) { return (letter>=0x1000 && letter<=0x1003) || (letter>=0x1005 && letter<=0x1021); }

};


#endif //_LETTER_INPUT_METHOD

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

