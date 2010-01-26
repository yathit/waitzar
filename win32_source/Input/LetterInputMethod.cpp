/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "LetterInputMethod.h"



//WARNING: This is currently COPIED in RomanInputMethod.cpp
//TODO: C++ 0x, chaining constructors can eliminate this
LetterInputMethod::LetterInputMethod(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, const vector< pair <int, unsigned short> > &systemWordLookup)
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
}



void LetterInputMethod::handleEsc()
{
	if (this->isHelpInput()) {
		//Flag for removal.
		this->providingHelpFor = NULL;
	} else {
		//Cancle the current sentence if not in help mode
		typedSentenceStr.str(L"");
	}
}



void LetterInputMethod::handleBackspace()
{
	//Delete a letter (performs differently in help mode)
	if (this->isHelpInput()) {
		//If help mode, delete a letter but don't hide the window
		wstring newStr = !typedCandidateStr.str().empty() ? typedCandidateStr.str().substr(0, typedCandidateStr.str().length()-1) : L"";
		typedCandidateStr.str(newStr);
		viewChanged = true;
	} else {
		// Otherwise, delete a letter from the sentece, and hide if nothing left
		wstring newStr = !typedSentenceStr.str().empty() ? typedSentenceStr.str().substr(0, typedSentenceStr.str().length()-1) : L"";
		typedSentenceStr.str(newStr);
		viewChanged = true;
	}
}

void LetterInputMethod::handleDelete()
{
	//Delete the letter in front of you (encoding-wise, not visibly)
	//TODO: ADD LATER
}


void LetterInputMethod::handleLeftRight(bool isRight)
{
	//Move the letter cursor one to the right/left
	//TODO: ADD LATER
}

//Ignore numCode
void LetterInputMethod::handleNumber(int numCode, WPARAM wParam)
{
	this->handleKeyPress(wParam);
}

void LetterInputMethod::handleStop(bool isFull)
{
	//Perform the normal "enter" routine.
	unsigned short stopChar = model.getStopCharacter(isFull);
	typedStopChar = (wchar_t)stopChar;
	requestToTypeSentence = true;
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
		requestToTypeSentence = true;
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

		//Type this word (should always succeed)
		providingHelpFor->appendToSentence('\0', currStrDictID);

		//Update trigrams
		sentence.updateTrigrams(model);

		//Flag for removal
		this->providingHelpFor = NULL;
	}
}




void LetterInputMethod::handleKeyPress(WPARAM wParam)
{
	//Handle our help menu
	wstring nextBit = helpKeyboard->typeLetter(wParam);
	if (!nextBit.empty()) {
		//Valid letter
		//typedRomanStr <<(char)wParam;
		if (isHelpInput())
			typedCandidateStr <<nextBit;
		else
			typedSentenceStr <<nextBit;

		//Save a temporary string to make calculations easier.
		wstring currStr = isHelpInput() ? typedCandidateStr.str() : typedSentenceStr.str();
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
		if (isHelpInput())
			typedCandidateStr.str(currStr);
		else
			typedSentenceStr.str(currStr);

		//Save a romanized string if in help mode
		if (this->isHelpInput()) {
			//Check each romanisation
			typedRomanStr.str(L"");
			for (unsigned int i=0; i<model.getTotalDefinedWords(); i++) {
				//Does this word match?
				wstring currWord = model.getWordString(i);
				if (currWord == extendedWordString) {
					//Build the roman word
					typedRomanStr <<L'(';
					typedRomanStr << model.reverseLookupWord(i);
					typedRomanStr <<L')';
					break;
				}
			}
		}

		//Trigger view change.
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

std::wstring LetterInputMethod::getSentencePreCursorString()
{
	//The cursor is always at the end
	//TODO: Can change this?
	return typedSentenceStr.str(); 
}


std::vector< std::pair<std::wstring, unsigned int> > LetterInputMethod::getTypedCandidateStrings()
{
	std::vector< std::pair<std::wstring, unsigned int> > res;
	res.push_back(std::pair<std::wstring, unsigned int>(typedCandidateStr.str(), 0));
	return res;
}


void LetterInputMethod::appendToSentence(wchar_t letter, int id)
{
	typedSentenceStr <<letter;
}


void LetterInputMethod::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
{
	//A "full" reset entails the others
	if (performFullReset)
		resetCandidates = resetRoman = resetSentence = true;

	if (resetCandidates)
		typedCandidateStr.str(L"");
	if (resetRoman)
		typedRomanStr.str(L"");
	if (resetSentence)
		typedSentenceStr.str(L"");
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
