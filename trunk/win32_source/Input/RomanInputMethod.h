/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _ROMAN_INPUT_METHOD
#define _ROMAN_INPUT_METHOD

#include "MyWin32Window.h"
#include "Input/InputMethod.h"
#include "NGram/SentenceList.h"
#include "OnscreenKeyboard.h"


template <class ModelType> //Either WordBuilder or BurglishBuilder, or write your own.
class RomanInputMethod : public InputMethod {

public:
	RomanInputMethod();
	virtual ~RomanInputMethod();

	//Needed to add Roman-specific stuff
	virtual void init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords, ModelType* model, waitzar::SentenceList<ModelType>* sentence);

	//Abstract implementation - keypresses
	void handleEsc();
	void handleBackspace(VirtKey& vkey);
	void handleDelete();
	void handleLeftRight(bool isRight, bool loopToZero);
	void handleUpDown(bool isDown);
	void handleCommit(bool strongCommit);
	void handleNumber(VirtKey& vkey, bool typeBurmeseNumbers);
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
	void treatAsHelpKeyboard(InputMethod* providingHelpFor);

	void typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID);

	std::pair<int, std::string> lookupWord(std::wstring typedWord);

	//Real override
	std::wstring getTypedRomanString(bool asLowercase);

	std::pair<int, int> getPagingInfo() const;


private:
	//Romanization model
	//TODO: Chain constructors, set to null (C++ 0x)
	ModelType* model;
	waitzar::SentenceList<ModelType>* sentence;

	bool selectCurrWord();
	bool selectWord(int id);

	bool typeBurmeseNumbers;

	bool typedStrContainsNoAlpha;
};



using namespace waitzar;
using std::vector;
using std::pair;
using std::string;
using std::wstring;


template <class ModelType>
RomanInputMethod<ModelType>::RomanInputMethod()
{
}



//This takes responsibility for the model and sentence memory.
template <class ModelType>
void RomanInputMethod<ModelType>::init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow,MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords, ModelType* model, waitzar::SentenceList<ModelType>* sentence)
{
	InputMethod::init(mainWindow, sentenceWindow, helpWindow, memoryWindow, systemWordLookup, helpKeyboard, systemDefinedWords);

	this->model = model;
	this->sentence = sentence;
}


template <class ModelType>
RomanInputMethod<ModelType>::~RomanInputMethod()
{
	if (model!=NULL) 
		delete model;
	if (sentence!=NULL) 
		delete sentence;
}


template <class ModelType>
std::pair<int, std::string> RomanInputMethod<ModelType>::lookupWord(std::wstring typedWord)
{
	return model->reverseLookupWord(typedWord);
}



template <class ModelType>
void RomanInputMethod<ModelType>::handleEsc()
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



template <class ModelType>
void RomanInputMethod<ModelType>::handleBackspace(VirtKey& vkey)
{
	if (!mainWindow->isVisible()) {
		//Delete the previous word in the sentence
		if (sentence->deletePrev(*model))
			viewChanged = true;
	} else {
		//Delete the previously-typed letter
		model->backspace(sentence->getPrevTypedWord(*model));

		//Truncate...
		wstring newStr = !typedRomanStr.str().empty() ? typedRomanStr.str().substr(0, typedRomanStr.str().length()-1) : L"";
		typedRomanStr.str(L"");
		typedRomanStr <<newStr;
		viewChanged = true;
	}
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleDelete()
{
	if (!mainWindow->isVisible()) {
		//Delete the next word
		if (sentence->deleteNext())
			viewChanged = true;
	}
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleLeftRight(bool isRight, bool loopToZero)
{
	int amt = isRight ? 1 : -1;
	if (mainWindow->isVisible()) {
		//Move right/left within the current selection.
		if (model->moveRight(amt) == TRUE)
			viewChanged = true;
		else if (isRight && loopToZero) {
			//Force back to index zero
			model->moveRight(-1000); //Hacky, we should eventually set a better wraparound method.
			viewChanged = true;
		}
	} else {
		//Move right/left within the current phrase.
		if (sentence->moveCursorRight(amt, *model))
			viewChanged = true;
	}
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleTab()
{
	if (mainWindow->isVisible()) {
		//Change the selection, or make a selection (depending on the style)
		if (controlKeyStyle==CK_CHINESE) 
			handleLeftRight(true, true);
		else if (controlKeyStyle==CK_JAPANESE)
			handleCommit(true);
	} else {
		//Move the sentence window cursor
		handleLeftRight(true, false);
	}
	
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleUpDown(bool isDown)
{
	if (mainWindow->isVisible()) {
		if (model->pageUp(!isDown))
			viewChanged = true;
	}
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleNumber(VirtKey& vkey, bool typeBurmeseNumbers)
{
	//Special case: conglomerate numbers
	if (typeNumeralConglomerates && typeBurmeseNumbers && typedStrContainsNoAlpha) {
		if (model->typeLetter(vkey.alphanum, vkey.modShift, sentence->getPrevTypedWord(*model))) {
			typedRomanStr <<vkey.alphanum;
			viewChanged = true;
		}
	} else if (mainWindow->isVisible()) {
		//Convert 1..0 to 0..9
		int numMinOne = vkey.alphanum - '0' - 1;
		if (numMinOne<0)
			numMinOne = 9;

		//A bit of mangling for pages
		numMinOne += model->getCurrPage()*10;

		//Mangle a bit more as usual...
		//if (wParam==HOTKEY_COMBINE || wParam==HOTKEY_SHIFT_COMBINE)
		if (vkey.alphanum=='`' || vkey.alphanum=='~') //TODO: Find a better way to do this later. (Not crucial.)
			numMinOne = -1;

		//Select this numbered word
		if (selectWord(numMinOne)) {
			typedRomanStr.str(L"");
			viewChanged = true;
		}
	} else if (typeBurmeseNumbers) {
		//Type this number --ask the model for the number directly, to avoid crashing Burglish.
		sentence->insert(model->getSingleDigitID(vkey.alphanum - '0'));
		sentence->moveCursorRight(0, true, *model);

		viewChanged = true;
	}
}


template <class ModelType>
void RomanInputMethod<ModelType>::handleStop(bool isFull, VirtKey& vkey)
{
	unsigned short stopChar = model->getStopCharacter(isFull);
	if (!mainWindow->isVisible()) {	
		//Otherwise, we perform the normal "enter" routine.
		typedStopChar = (wchar_t)stopChar;
		requestToTypeSentence = true;
	}
}



template <class ModelType>
void RomanInputMethod<ModelType>::handleCommit(bool strongCommit)
{
	if (mainWindow->isVisible()) {
		//The model is visible; react to the control key style.
		if (!strongCommit && controlKeyStyle==CK_JAPANESE) {
			//Advance
			handleLeftRight(true, true);
		} else {
			//Select the current word
			if (selectCurrWord()) {
				//Reset, recalc
				typedRomanStr.str(L"");
				viewChanged = true;
			}
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




template <class ModelType>
void RomanInputMethod<ModelType>::handleKeyPress(VirtKey& vkey)
{
	//Handle regular letter-presses (as lowercase)
	//NOTE: ONLY handle letters
	if (vkey.alphanum>='a' && vkey.alphanum<='z') {
		//Run this keypress into the model. Accomplish anything?
		if (!model->typeLetter(vkey.alphanum, vkey.modShift, sentence->getPrevTypedWord(*model))) {
			//That's the end of the story if we're typing Chinese-style; or if there's no roman string.
			if (controlKeyStyle==CK_CHINESE || typedRomanStr.str().empty())
				return;

			//Otherwise, try typing the current string directly
			handleCommit(true);
			viewChanged = true;

			//Nothing left on the new string?
			if (!model->typeLetter(vkey.alphanum, vkey.modShift, sentence->getPrevTypedWord(*model)))
				return;
		}

		//Update the romanized string, trigger repaint
		typedStrContainsNoAlpha = false;
		typedRomanStr <<vkey.alphanum; //alphanum will always be something visible, or '\0' if nothing on the keyboard.
		viewChanged = true;
	} else {
		//Check for system keys
		InputMethod::handleKeyPress(vkey);
	}
}



template <class ModelType>
bool RomanInputMethod<ModelType>::selectCurrWord()
{
	return this->selectWord(model->getCurrSelectedID());
}




template <class ModelType>
bool RomanInputMethod<ModelType>::selectWord(int id)
{
	//Are there any words to use?
	std::pair<int, int> typedVal = model->typeSpace(id);
	if (typedVal.first<0)
		return false;
	int wordID = typedVal.first;

	//Pat-sint clears the previous word, changes what we're inserting
	if (typedVal.second>=0) {
		sentence->deletePrev(*model);
		wordID = typedVal.second;
	}

	//Insert into the current sentence, return
	sentence->insert(wordID);
	return true;
}


template <class ModelType>
void RomanInputMethod<ModelType>::typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID)
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



template <class ModelType>
vector<wstring> RomanInputMethod<ModelType>::getTypedSentenceStrings()
{
	//Results
	//TODO: Cache the results.
	vector<wstring> res;
	std::wstringstream line;
	std::wstringstream full;

	//Build
	int currID = -1;
	int actID = model->getCurrSelectedID()+model->getFirstWordIndex();
	int currReplacementID = (model->getPossibleWords().empty()) ? -1 : model->getWordCombinations()[actID];
	for (std::list<int>::const_iterator it=sentence->begin(); it!=sentence->end(); it++) {
		//Get the word
		int modID = -(*it)-1;
		wstring currWord = (*it>=0) ? model->getWordString(*it) : (modID<(int)systemDefinedWords.size()) ? wstring(1, systemDefinedWords[modID]) : userDefinedWords[modID-systemDefinedWords.size()];

		//Have we reached a transition?
		if (currID==sentence->getCursorIndex()-1 && currReplacementID!=-1) {
			//We're about to start the highlighted word.
			res.push_back(line.str());
			line.str(L"");

			//CHANGE the current word to the pat-sint replacement.
			currWord = model->getWordString(currReplacementID);
		} else if (currID==sentence->getCursorIndex()) {
			//We're at the cursor
			res.push_back(line.str());
			line.str(L"");
			if (res.size()==1)
				res.push_back(L"");
		}

		//Append the word
		line <<currWord;
		full <<currWord;

		//Increment
		currID++;
	}

	//Add the final entry, and the typedStopChar if necessary
	if (typedStopChar!=L'\0')
		line <<typedStopChar;
	res.push_back(line.str());

	//I think this might always be true...
	//TODO: Check the source and remove these tests.
	while (res.size()<3)
		res.push_back(L"");
	if (res.size()!=3)
		throw std::exception("Error! getTypedSentenceStr() must always return a vector of size 3+1");

	//Finally, add the full entry
	res.push_back(full.str());

	//Done
	return res;
}


//0 = reg. word
//1 = pat-sint
//2 = selected 
//4 = give it a "tilde" label
template <class ModelType>
vector< pair<wstring, unsigned int> > RomanInputMethod<ModelType>::getTypedCandidateStrings()
{
	//TODO: cache the results
	std::vector< std::pair<std::wstring, unsigned int> > res;
	std::vector<unsigned int> words = model->getPossibleWords();
	std::vector<int> combinations = model->getWordCombinations();
	for (size_t i=0; i<words.size(); i++) {
		std::pair<std::wstring, unsigned int> item = std::pair<std::wstring, unsigned int>(model->getWordString(words[i]), HF_NOTHING);
		
		//Get the previous word
		std::wstring prevWord = sentence->getPrevTypedWord(*model);

		//Color properly.
		if (combinations[i]!=-1) {
			item.second |= HF_PATSINT;
			if (model->canTypeShortcut())
				item.second |= HF_LABELTILDE;
		}
		if (model->getCurrSelectedID() == i-model->getFirstWordIndex())
			item.second |= HF_CURRSELECTION;
		res.push_back(item);
	}

	return res;
}


template <class ModelType>
void RomanInputMethod<ModelType>::appendToSentence(wchar_t letter, int id)
{
	//Type it
	selectWord(id);

	//We need to reset the trigrams here...
	sentence->updateTrigrams(*model);

	//Repaint
	viewChanged = true;
}


template <class ModelType>
void RomanInputMethod<ModelType>::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
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

	if (resetRoman)
		typedStrContainsNoAlpha = true;

	//Either way
	typedStopChar = L'\0';
}

//Add our paren string
template <class ModelType>
std::wstring RomanInputMethod<ModelType>::getTypedRomanString(bool asLowercase)
{
	//Initail string
	std::wstringstream res;
	res <<InputMethod::getTypedRomanString(asLowercase);

	//Add a paren string, if it exists
	if (!model->getParenString().empty())
		res <<L'(' <<model->getParenString() <<L')';

	//Done
	return res.str();
}

template <class ModelType>
std::pair<int, int> RomanInputMethod<ModelType>::getPagingInfo() const
{
	return std::pair<int, int>(model->getCurrPage(), model->getNumberOfPages());
}


template <class ModelType>
void RomanInputMethod<ModelType>::treatAsHelpKeyboard(InputMethod* providingHelpFor)
{
	throw std::exception("Cannot use a Romanized keyboard for a help input method");
}





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

