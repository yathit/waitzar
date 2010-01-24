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

void LetterInputMethod::handleCommit(bool strongCommit)
{
	//Anything to commit?
	if (typedSentenceStr.str().empty()) 
		return;

	//If we are in help mode, add the word we chose to the dictionary, and flag 
	//   it so that it can be cleared later when the entire sentence is selected.
	//If in normal mode, commit the entire sentence. 
	if (!this->isHelpInput()) {
		//Just commit the current sentence.
		typeCurrentPhrase();
	} else {
		//Get its romanization, if it exists.
		string revWord = (currStrDictID!=-1) ? revWord = model.reverseLookupWord(currStrDictID) : "<no entry>";

		//Add it to the memory list
		helpKeyboard->addMemoryEntry(typedSentenceStr.str().c_str(), revWord.c_str());

		//Add it to the dictionary?
		if (currStrDictID==-1) {
			wstring tempStr = waitzar::sortMyanmarString(typedSentenceStr.str());
			userDefinedWords.push_back(tempStr);
			currStrDictID = -1*(systemDefinedWords.size()+userDefinedWords.size());
		}

		//Hide the help window
		turnOnHelpKeys(false);
		helpWindow->showWindow(false);
		memoryWindow->showWindow(false);

		//Try to type this word
		//TODO: Need to move this into the MainFile.cpp logic.
		//      We need to trigger switching back to the MAIN INPUT method, too.
		BOOL typed = selectWord(currStrDictID, true);
		if (typed==TRUE) {
			mainWindow->showWindow(false);

			patSintIDModifier = 0;
			model.reset(false);
			currStr.clear();
			recalculate();
		}
		//We need to reset the trigrams here...
		sentence.updateTrigrams(model);
		//END TODO

	}
}




void LetterInputMethod::handleKeyPress(WPARAM wParam)
{
	//Handle our help menu
	wstring nextBit = helpKeyboard->typeLetter(wParam);
	if (!nextBit.empty()) {
		//Valid letter
		typedRomanStr <<(char)wParam;
		typedSentenceStr <<nextBit

		//Save a temporary string to make calculations easier.
		wstring currStr = typedSentenceStr.str();
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

		//Save the results of our temporary calculations.
		typedSentenceStr.str(currStr);

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



std::wstring LetterInputMethod::getTypedSentenceString()
{
	return typedSentenceStr.str();
}


std::wstring LetterInputMethod::getTypedCandidateString()
{
	return typedCandidateStr.str();
}


void LetterInputMethod::appendToSentence(wchar_t letter, int id)
{
	typedSentenceStr <<letter;
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
