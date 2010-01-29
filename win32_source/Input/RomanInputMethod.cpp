/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "RomanInputMethod.h"


using namespace waitzar;
using std::vector;
using std::pair;
using std::string;
using std::wstring;


//WARNING: This is currently COPIED in RomanInputMethod.cpp
//TODO: C++ 0x, chaining constructors can eliminate this
RomanInputMethod::RomanInputMethod(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, const vector< pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, string systemDefinedWords)
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



//This takes responsibility for the model and sentence memory.
void RomanInputMethod::init(WordBuilder* model, SentenceList* sentence, bool typeBurmeseNumbers)
{
	this->model = model;
	this->sentence = sentence;
	this->typeBurmeseNumbers = typeBurmeseNumbers;
}


RomanInputMethod::~RomanInputMethod()
{
	if (model!=NULL) 
		delete model;
	if (sentence!=NULL) 
		delete sentence;
}


std::pair<int, std::string> RomanInputMethod::lookupWord(std::wstring typedWord)
{
	//Init
	std::pair<int, std::string> res;
	res.first = -1;

	//Get the ID
	for (unsigned int i=0; i<model->getTotalDefinedWords(); i++) {
		//Does the word match?
		wstring currWord = model->getWordString(i);
		if (currWord == typedWord) {
			res.first = i;
			break;
		}
	}

	//Get the romanization 
	res.second = model->reverseLookupWord(res.first);  //Will be empty if word id is -1

	return res;
}



void RomanInputMethod::handleEsc()
{
	//Escape out of the main window or the sentence, depending
	if (!mainWindow->isVisible()) {
		//Kill the entire sentence
		sentence->clear();
	} else {
		//Cancel the current word
		typedRomanStr.str(L"");
		viewChanged = true; //Shouldn't matter, but doesn't hurt.
	}
}



void RomanInputMethod::handleBackspace()
{
	if (!mainWindow->isVisible()) {
		//Delete the previous word in the sentence
		if (sentence->deletePrev(*model))
			viewChanged = true;
	} else {
		//Delete the previously-typed letter
		model->backspace();

		//Truncate...
		wstring newStr = !typedRomanStr.str().empty() ? typedRomanStr.str().substr(0, typedRomanStr.str().length()-1) : L"";
		typedRomanStr.str(newStr);
		viewChanged = true;
	}
}


void RomanInputMethod::handleDelete()
{
	if (!mainWindow->isVisible()) {
		//Delete the next word
		if (sentence->deleteNext())
			viewChanged = true;
	}
}


void RomanInputMethod::handleLeftRight(bool isRight)
{
	int amt = isRight ? 1 : -1;
	if (mainWindow->isVisible()) {
		//Move right/left within the current selection.
		if (model->moveRight(amt) == TRUE)
			viewChanged = true;
	} else {
		//Move right/left within the current phrase.
		if (sentence->moveCursorRight(amt, *model))
			viewChanged = true;
	}
}


void RomanInputMethod::handleNumber(int numCode, WPARAM wParam)
{
	if (mainWindow->isVisible()) {
		//Convert 1..0 to 0..9
		if (--numCode<0)
			numCode = 9;

		//Mangle as usual...
		if (wParam==HOTKEY_COMBINE || wParam==HOTKEY_SHIFT_COMBINE)
			numCode = -1;

		//Select this numbered word
		if (selectWord(numCode, true)) {
			typedRomanStr.str(L"");
			viewChanged = true;
		}
	} else if (typeBurmeseNumbers) {
		//Type this number
		sentence->insert(numCode);
		sentence->moveCursorRight(0, true, *model);

		viewChanged = true;
	}
}


void RomanInputMethod::handleStop(bool isFull)
{
	unsigned short stopChar = model->getStopCharacter(isFull);
	if (!mainWindow->isVisible()) {	
		//Otherwise, we perform the normal "enter" routine.
		typedStopChar = (wchar_t)stopChar;
		requestToTypeSentence = true;
	}
}



void RomanInputMethod::handleCommit(bool strongCommit)
{
	if (mainWindow->isVisible()) {
		//The model is visible: select that word
		if (selectWord(-1, false)) {
			//Reset, recalc
			typedRomanStr.str(L"");
			viewChanged = true;
		}
	} else {
		//A bit tricky here. If the cursor's at the end, we'll
		//  do HOTKEY_ENTER. But if not, we'll just advance the cursor.
		//Hopefully this won't confuse users so much.
		//Note: ENTER overrides this behavior.
		if (strongCommit) {
			//Type the entire sentence
			requestToTypeSentence = true;
		} else {
			if (sentence->getCursorIndex()==-1 || sentence->getCursorIndex()<((int)sentence->size()-1)) {
				sentence->moveCursorRight(1, *model);
				viewChanged = true;
			} else {
				//Type the entire sentence
				requestToTypeSentence = true;
			}
		}
	}
}





void RomanInputMethod::handleKeyPress(WPARAM wParam)
{
	//Handle regular letter-presses (as lowercase)
	//NOTE: ONLY handle letters
	int keyCode = (wParam >= HOTKEY_A && wParam <= HOTKEY_Z) ? (int)wParam+32 : (int)wParam;
	if (keyCode >= HOTKEY_A_LOW && keyCode <= HOTKEY_Z_LOW) {
		//Run this keypress into the model. Accomplish anything?
		if (!model->typeLetter(keyCode))
			return;

		//Update the romanized string, trigger repaint
		typedRomanStr <<(char)keyCode;
		viewChanged = true;
	} else {
		//Check for system keys
		InputMethod::handleKeyPress(wParam);
	}
}



bool RomanInputMethod::selectWord(int id, bool indexNegativeEntries)
{
	//Are there any words to use?
	int wordID = 0;
	if (!indexNegativeEntries) {
		//Ok, look it up in the model as usual
		std::pair<BOOL, UINT32> typedVal = model->typeSpace(id);
		if (typedVal.first == FALSE)
			return false;
		wordID = typedVal.second;
	}

	//Pat-sint clears the previous word
	if (id==-1)
		sentence->deletePrev(*model);

	//Insert into the current sentence, return
	sentence->insert(wordID);
	return true;
}


void RomanInputMethod::typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID)
{
	//Add it to the memory list
	mostRecentRomanizationCheck.first = roman;
	mostRecentRomanizationCheck.second = myanmar;

	//Add it to the dictionary?
	if (currStrDictID==-1) {
		wstring tempStr = waitzar::sortMyanmarString(myanmar);
		userDefinedWords.push_back(tempStr);
		currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());
	}

	//Type this word (should always succeed)
	this->appendToSentence('\0', currStrDictID);

	//Update trigrams
	sentence->updateTrigrams(*model);
}



std::wstring RomanInputMethod::buildSentenceStr(unsigned int stopAtID)
{
	std::wstringstream res;
	int currID = -1;
	for (std::list<int>::const_iterator it=sentence->begin(); (it!=sentence->end() && currID!=stopAtID); it++) {
		if (*it>0)
			res <<model->getWordString(*it);
		else {
			int id = -(*it)-1;
			if (id<(int)systemDefinedWords.size())
				res <<systemDefinedWords[id];
			else
				res <<userDefinedWords[id-systemDefinedWords.size()];
		}
		currID++;
	}

	return res.str();
}


std::wstring RomanInputMethod::getTypedSentenceString()
{
	//TODO: Cache the results (also add the typedStopChar elsewhere...)
	std::wstringstream res;
	res <<buildSentenceStr(sentence->size());
	if (typedStopChar!=L'\0')
		res <<typedStopChar;
	return res.str();
}

std::wstring RomanInputMethod::getSentencePreCursorString()
{
	//TODO: Cache the results
	return buildSentenceStr(sentence->getCursorIndex());
}


std::vector< std::pair<std::wstring, unsigned int> > RomanInputMethod::getTypedCandidateStrings()
{
	//TODO: cache the results
	std::vector< std::pair<std::wstring, unsigned int> > res;
	std::vector<UINT32> words = model->getPossibleWords();
	for (size_t i=0; i<words.size(); i++) {
		std::pair<std::wstring, unsigned int> item = std::pair<std::wstring, unsigned int>(model->getWordString(words[i]), 0);
		if (i<model->getFirstWordIndex())
			item.second = 1;
		if (model->getCurrSelectedID() == i-model->getFirstWordIndex())
			item.second = item.second==1 ? 3 : 2;
		res.push_back(item);
	}

	return res;
}


void RomanInputMethod::appendToSentence(wchar_t letter, int id)
{
	//Type it
	selectWord(id, true);

	//We need to reset the trigrams here...
	sentence->updateTrigrams(*model);

	//Repaint
	viewChanged = true;
}


void RomanInputMethod::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
{
	//A "full" reset entails the others
	if (performFullReset) {
		resetCandidates = resetRoman = resetSentence = true;
		userDefinedWords.clear();
	}

	//Equivalent to reset candidates or the roman string
	if (resetCandidates || resetRoman)  {
		typedRomanStr.str(L"");
		model->reset(performFullReset);
	}

	//Reset the sentence?
	if (resetSentence)
		sentence->clear();

	//Either way
	typedStopChar = L'\0';
}


void RomanInputMethod::treatAsHelpKeyboard(InputMethod* providingHelpFor)
{
	throw std::exception("Cannot use a Romanized keyboard for a help input method");
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
