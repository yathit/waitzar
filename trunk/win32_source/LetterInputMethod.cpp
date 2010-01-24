/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "LetterInputMethod.h"


void LetterInputMethod::handleEsc()
{
	//Need to do SOMETHING if we're not in help mode. 
	// (note: switching out from help mode is already handled)
}



void LetterInputMethod::handleBackspace()
{
	//Delete the letter in back of you (encoding-wise, not visibly)
	if (!currTypedStr.empty())
		currTypedStr.erase(currTypedStr.length()-1);
	recalculate();
}

void LetterInputMethod::handleDelete()
{
	//Delete the letter in front of you (encoding-wise, not visibly)
	//TODO: ADD LATER
}


void LetterInputMethod::handleRight()
{
	//Move the letter cursor one to the right
	//TODO: ADD LATER
}

void LetterInputMethod::handleLeft()
{
	//Move the letter cursor one to the left
	//TODO: ADD LATER
}


//Ignore numCode
void LetterInputMethod::handleNumber(int numCode, WPARAM wParam)
{
	this->handleKeyPress(wParam);
}

void LetterInputMethod::handleStop(bool isFull)
{
	unsigned short stopChar = model.getStopCharacter(isFull);
	//Possibly do nothing...
	//TODO: ADD LATER
}

void LetterInputMethod::handleEnter()
{
	//Select our word, add it to the dictionary temporarily.
	// Flag the new entry so it can be cleared later when the sentence is selected
	if (currStrZg.length()>0) {
		if (currStrDictID==-1) {
			wstring tempStr = waitzar::sortMyanmarString(currStr);
			userDefinedWords.push_back(tempStr);
			userDefinedWordsZg.push_back(currStrZg);
			currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());

			//Add it to the memory list
			helpKeyboard->addMemoryEntry(currStrZg.c_str(), "<no entry>");
		} else {
			//Add it to the memory list
			string revWord = model.reverseLookupWord(currStrDictID);
			helpKeyboard->addMemoryEntry(currStrZg.c_str(), revWord.c_str());
		}

		//Hide the help window
		turnOnHelpKeys(false);
		helpWindow->showWindow(false);
		memoryWindow->showWindow(false);
		//ShowHelpWindow(SW_HIDE);

		//Try to type this word
		BOOL typed = selectWord(currStrDictID, true);
		if (typed==TRUE && typePhrases) {
			mainWindow->showWindow(false);
			//ShowMainWindow(SW_HIDE);

			patSintIDModifier = 0;
			model.reset(false);
			currStr.clear();
			recalculate();
		}

		//We need to reset the trigrams here...
		sentence.updateTrigrams(model);
	}
}

void LetterInputMethod::handleSpace()
{
	//Select our word, add it to the dictionary temporarily.
	// Flag the new entry so it can be cleared later when the sentence is selected
	if (currStrZg.length()>0) {
		if (currStrDictID==-1) {
			wstring tempStr = waitzar::sortMyanmarString(currStr);
			userDefinedWords.push_back(tempStr);
			userDefinedWordsZg.push_back(currStrZg);
			currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());

			//Add it to the memory list
			helpKeyboard->addMemoryEntry(currStrZg.c_str(), "<no entry>");
		} else {
			//Add it to the memory list
			string revWord = model.reverseLookupWord(currStrDictID);
			helpKeyboard->addMemoryEntry(currStrZg.c_str(), revWord.c_str());
		}

		//Hide the help window
		turnOnHelpKeys(false);
		helpWindow->showWindow(false);
		memoryWindow->showWindow(false);
		//ShowHelpWindow(SW_HIDE);

		//Try to type this word
		BOOL typed = selectWord(currStrDictID, true);
		if (typed==TRUE && typePhrases) {
			mainWindow->showWindow(false);
			//ShowMainWindow(SW_HIDE);

			patSintIDModifier = 0;
			model.reset(false);
			currStr.clear();
			recalculate();
		}

		//We need to reset the trigrams here...
		sentence.updateTrigrams(model);

		keyWasUsed = true;
	}
}





void LetterInputMethod::handleKeyPress(WPARAM wParam)
{
	//Handle our help menu
	wstring nextBit = helpKeyboard->typeLetter(wParam);
	if (!nextBit.empty()) {
		//Valid letter
		currStr += nextBit;
		size_t len = currStr.length();

		//Special cases
		if (nextBit.length()==1 && nextBit[0]==L'\u1039') {
			//Combiner functions in reverse
			if (len>1 && canStack(currStr[len-2])) {
				currStr[len-1] = currStr[len-2];
				currStr[len-2] = nextBit[0];
			} else {
				currStr.erase(currStr.length()-1); //Not standard behavior, but let's avoid bad combinations.
			}
		} else if (nextBit == wstring(L"\u1004\u103A\u1039")) {
			//Kinzi can be typed after the consonant instead of before it.
			//For now, we only cover the general case of typing "kinzi" directly after a consonant
			if (len>3 && canStack(currStr[len-4])) {
				currStr[len-1] = currStr[len-4];
				currStr[len-4] = nextBit[0];
				currStr[len-3] = nextBit[1];
				currStr[len-2] = nextBit[2];
			}
		}


		//Pre-sort unicode strings (should be helpful)
		recalculate();

		//Is the main window visible?
		if (!mainWindow->isVisible()) {
			//Show it
			if (!typePhrases || !sentenceWindow->isVisible()) {
				ShowBothWindows(SW_SHOW);
			} else {
				mainWindow->showWindow(true);
				//ShowMainWindow(SW_SHOW);
			}
		}

		keyWasUsed = true;
	} else {
		//Check for system keys
		InputMethod::handleKeyPress(wParam);
	}
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
