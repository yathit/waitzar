/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "RomanInputMethod.h"


void RomanInputMethod::init(WordBuilder* model, SentenceList* sentence)
{
	this->model = model;
	this->sentence = sentence;
}



void RomanInputMethod::handleEsc()
{
	//Escape out of the main window or the sentence, depending
	if (!mainWindow->isVisible()) {
		//Kill the entire sentence
		sentence.clear();
	} else {
		//Cancel the current word
		//NOTE: if getTypedString() is empty, we can say that our candidate list has reset
		//      Let's try that for now...
		typedRomanStr.str(L"");
		viewChanged = true; //Shouldn't matter, but doesn't hurt.
	}
}



void RomanInputMethod::handleBackspace()
{
	if (!mainWindow->isVisible()) {
		//Delete the previous word in the sentence
		if (sentence.deletePrev(model))
			viewChanged = true;
	} else {
		model.backspace();

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
		if (sentence.deleteNext())
			viewChanged = true;
	}
}


void RomanInputMethod::handleRight()
{
	if (mainWindow->isVisible()) {
		//Move right/left within the current word.
		if (patSintIDModifier==-1) {
			patSintIDModifier = 0;
			viewChanged = true;
		} else if (model.moveRight(1) == TRUE)
			viewChanged = true;
	} else {
		//Move right/left within the current phrase.
		if (sentence.moveCursorRight(1, model))
			viewChanged = true;
	}
}


void RomanInputMethod::handleLeft()
{
	if (mainWindow->isVisible()) {
		if (model.moveRight(-1) == TRUE)
			viewChanged = true;
		else if (model.hasPostStr() && patSintIDModifier==0) {
			//Move left to our "pat-sint shortcut"
			patSintIDModifier = -1;
			viewChanged = true;
		}
	} else {
		//Move right/left within the current phrase.
		if (sentence.moveCursorRight(-1, model))
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
		if (wParam==HOTKEY_COMBINE || wParam==HOTKEY_SHIFT_COMBINE) {
			numCode = -1;
			patSintIDModifier = -1;
		} else
			patSintIDModifier = 0;

		//Select this numbered word
		if (selectWord(numCode, helpWindow->isVisible())==TRUE) {
			typedRomanStr.str(L"");
			viewChanged = true;
		}
	} else if (typeBurmeseNumbers) {
		//Type this number
		sentence.insert(numCode);
		sentence.moveCursorRight(0, true, model);

		viewChanged = true;
	}
}


void RomanInputMethod::handleStop(bool isFull)
{
	unsigned short stopChar = model.getStopCharacter(isFull);
	if (!mainWindow->isVisible()) {	
		//Otherwise, we perform the normal "enter" routine.
		typeCurrentPhrase(); //TODO: Append "stopChar"
	}
}



void RomanInputMethod::handleCommit(bool strongCommit)
{
	if (mainWindow->isVisible()) {
		//The model is visible: select that word
		if (selectWord(-1, helpWindow->isVisible())==TRUE) {
			//Hide the main window
			mainWindow->showWindow(false);

			//Reset
			patSintIDModifier = 0;
			model.reset(false);
			typedRomanStr.str(L"");

			//Recalc
			recalculate();
		}
	} else {
		//A bit tricky here. If the cursor's at the end, we'll
		//  do HOTKEY_ENTER. But if not, we'll just advance the cursor.
		//Hopefully this won't confuse users so much.
		//Note: ENTER overrides this behavior.
		if (strongCommit) {
			//Type the entire sentence
			typeCurrentPhrase();
		} else {
			if (sentence.getCursorIndex()==-1 || sentence.getCursorIndex()<((int)sentence.size()-1)) {
				sentence.moveCursorRight(1, model);
				recalculate();
			} else {
				//Type the entire sentence
				typeCurrentPhrase();
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
		if (!model.typeLetter(keyCode))
			return;

		//Update the romanized string
		typedRomanStr <<(char)keyCode;

		//Trigger 2 events
		viewChanged = true;
//		justTypedFirstLetter = true;
	} else {
		//Check for system keys
		InputMethod::handleKeyPress(wParam);
	}
}


std::wstring RomanInputMethod::buildSentenceStr(unsigned int stopAtID)
{
	std::wstringstream res;
	int currID = -1;
	for (std::list<int>::const_iterator it=sentence->begin(); (it!=sentence.end() && currID!=stopAtID); it++) {
		if (*it>0)
			res <<model->getWordString(*it);
		else {
			int id = -(*it)-1;
			if (id<systemDefinedWords.size())
				res <<systemDefinedWords[id]
			else
				res <<userDefinedWords[id-systemDefinedWords.size()];
		}
		currID++;
	}
	return res.str();
}


std::wstring RomanInputMethod::getTypedSentenceString()
{
	//TODO: Cache the results
	return buildSystemWordLookup(sentence->size());
}

std::wstring RomanInputMethod::getSentencePreCursorString()
{
	//TODO: Cache the results
	return buildSystemWordLookup(sentence->getCursorID());
}


std::vector< std::pair<std::wstring, unsigned int> > getTypedCandidateStrings()
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
	sentence.updateTrigrams(model);

	//Repaint
	viewChanged = true;
}


void RomanInputMethod::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
{
	//A "full" reset entails the others
	if (performFullReset)
		resetCandidates = resetRoman = resetSentence = true;

	//Equivalent to reset candidates or the roman string
	if (resetCandidates || resetRoman)  {
		typedRomanStr.str(L"");
		model.reset(performFullReset);
	}

	//Reset the sentence?
	if (resetSentence)
		sentence->clear();
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
